// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// All rights reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include <thread>
#include <atomic>
#include <sys/time.h>
#include <vidc/media/msm_media_info.h>
#include <inttypes.h>
#include <getopt.h>
#include <BufferAllocator/BufferAllocatorWrapper.h>

#include "vpp.h"

using namespace std::chrono_literals;
#define UNUSED(x) (void)(x)
#define AIS_MIN_INPUT_WIDTH 96
#define AIS_MIN_INPUT_HEIGHT 96
#define AIS_3X_MAX_INPUT_WIDTH 720
#define AIS_3X_MAX_INPUT_HEIGHT 480
#define AIS_2X_MAX_INPUT_WIDTH 1280
#define AIS_2X_MAX_INPUT_HEIGHT 720
#define MAX_FILE_NAME 64

enum {
    PRIO_ERROR = 0x1,
    PRIO_INFO = 0x2,
    PRIO_HIGH = 0x4,
    PRIO_LOW = 0x8
};

int ais_test_debug_level = PRIO_ERROR;

#define WAIT_BUF_TIME 5s

#define DEBUG_LOG(level, fmt, args...)    \
    do {                                  \
        if (level & ais_test_debug_level) \
            printf(fmt "\n", ##args);     \
    } while (0)

#define DEBUG_ERROR(fmt, args...) DEBUG_LOG(PRIO_ERROR, fmt, ##args)
#define DEBUG_INFO(fmt, args...) DEBUG_LOG(PRIO_INFO, fmt, ##args)
#define DEBUG_HIGH(fmt, args...) DEBUG_LOG(PRIO_HIGH, fmt, ##args)
#define DEBUG_LOW(fmt, args...) DEBUG_LOG(PRIO_LOW, fmt, ##args)

enum {
    AIS_TEST_INPUT = 0,
    AIS_TEST_OUTPUT = 1,
};

static vpp_color_format in_format = VPP_COLOR_FORMAT_NV12_VENUS;
static vpp_color_format out_format = VPP_COLOR_FORMAT_NV12_VENUS;
static int input_width = 0, output_width = 0, input_height = 0, output_height = 0;
static int output_stride = 0, output_scanline = 0;
static int scale_ratio_setting = 0;
static const char* infile = NULL;
static const char* outfile = NULL;
static bool enable_dump = true;

class file_op;

class main_loop
{
public:
    bool start();
    bool stop();

private:
    std::atomic_bool is_running;
    std::mutex mutex;
    std::condition_variable cond_var;
};

bool main_loop::start()
{
    DEBUG_INFO("main loop is running");

    is_running.store(true);
    while (is_running.load()) {
        std::unique_lock<std::mutex> lock(mutex);
        cond_var.wait(lock);
    }
    DEBUG_INFO("exit from main loop");

    return true;
}

bool main_loop::stop()
{
    is_running.store(false);
    cond_var.notify_one();

    return true;
}

typedef struct vpp_client_ctx
{
    void* vpp_ctx;

    std::queue<vpp_buffer> in_queue;
    std::mutex in_queue_mutex;
    std::condition_variable in_buf_done;

    std::queue<vpp_buffer> out_queue;
    std::mutex out_queue_mutex;
    std::condition_variable out_buf_done;

    std::mutex flush_mutex;
    std::condition_variable flush_cond;

    file_op* in_file;
    file_op* out_file;

    main_loop* loop;
    uint32_t in_valid_size;
    uint32_t out_valid_size;

    bool first_time;
    uint32_t num_input_queue;
    uint32_t num_output_done;
    std::mutex monitor_mutex;
    std::condition_variable monitor_cond;
} vpp_client_ctx;

class file_op {
public:
    file_op(const std::string& fileName, const std::string& mode);
    ~file_op();
    int read_data(void* ptr, int size);
    int write_data(void* ptr, int size);

private:
    FILE* mFp;
};

file_op::file_op(const std::string& fileName, const std::string& mode)
{
    mFp = fopen(fileName.c_str(), mode.c_str());
    if (!mFp)
        perror("Failed ");
}

int file_op::read_data(void* ptr, int size)
{
    int read_len = 0;
    if (mFp) {
        read_len = fread(ptr, 1, size, mFp);
    }

    return read_len;
}

int file_op::write_data(void* ptr, int size)
{
    int write_len = 0;

    if (mFp) {
        write_len = fwrite(ptr, 1, size, mFp);
        if (write_len != size) {
            perror("Failed to read: ");
        } else {
            fflush(mFp);
        }
    }

    return write_len;
}

file_op::~file_op()
{
    if (mFp) {
        fclose(mFp);
    }
}

void print_vpp_buffer(const char* tag, struct vpp_buffer* buf)
{
    if (buf) {
        vpp_mem_buffer* mem_buf = &buf->pixel;
        DEBUG_LOW("%s pts:%" PRIu64 " flags:%u fd:%d offset:%d alloc_len:%u"
            " filled_len:%u valid_data_len:%u mapped:%p",
            tag, buf->timestamp, buf->flags,
            mem_buf->fd, mem_buf->offset, mem_buf->alloc_len,
            mem_buf->filled_len, mem_buf->valid_data_len, mem_buf->pvMapped);
    }
}

// if reach eof, return true
bool copy_from_file(file_op* file, struct vpp_buffer* buf, uint32_t valid_size)
{
    bool is_eof = false;
    int fd = -1;
    int buf_size = valid_size;
    char* bufaddr = NULL;
    char* src_buf = NULL;

    if (NULL == file || NULL == buf)
    {
        DEBUG_ERROR("invalid file:%p buf:%p", file, buf);
        goto exit;
    }

    fd = buf->pixel.fd;
    src_buf = (char*)calloc(1, buf_size);

    if (NULL == src_buf)
    {
        goto exit;
    }

    if (!file->read_data(src_buf, buf_size)) {
        is_eof = true;
        goto done;
    }

    bufaddr = (char*)mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (bufaddr) {
        memcpy(bufaddr, src_buf, buf_size);
        munmap(bufaddr, buf_size);
    }

done:
    free(src_buf);
exit:

    return is_eof;
}

void store_to_file(file_op* file, struct vpp_buffer* buf, uint32_t valid_size)
{
    int fd = buf->pixel.fd;
    int buf_size = valid_size;
    char* dest_buf = (char*)malloc(buf_size);
    int write_len = 0;

    if (dest_buf) {
        memset(dest_buf, 0, buf_size);

        char* bufaddr = (char*)mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (bufaddr) {
            memcpy(dest_buf, bufaddr, buf_size);
            munmap(bufaddr, buf_size);
        }

        write_len = file->write_data(dest_buf, buf_size);
        if (write_len != buf_size) {
            DEBUG_ERROR("write file error");
        }

        free(dest_buf);
    }
}

void print_vpp_requirements(struct vpp_requirements* req)
{
    if (req) {
        DEBUG_INFO("get buf req in:SD:%u HD:%u FHD:%u UHD:%u"
            " out:SD:%u HD:%u FHD:%u UHD:%u",
            req->buf_req[0].in_req, req->buf_req[1].in_req,
            req->buf_req[2].in_req, req->buf_req[3].in_req,
            req->buf_req[0].out_req, req->buf_req[1].out_req,
            req->buf_req[2].out_req, req->buf_req[3].out_req);
    }
}

void input_buffer_done(void* pv, struct vpp_buffer* buf)
{
    vpp_client_ctx* ctx = (vpp_client_ctx*)pv;
    print_vpp_buffer("ibd", buf);
    {
        std::lock_guard<std::mutex> lock(ctx->in_queue_mutex);
        ctx->in_queue.push(*buf);
    }

    ctx->in_buf_done.notify_one();
}

void output_buffer_done(void* pv, struct vpp_buffer* buf)
{
    vpp_client_ctx* ctx = (vpp_client_ctx*)pv;
    print_vpp_buffer("obd", buf);
    if (buf->pixel.filled_len) {
        {
            std::unique_lock<std::mutex> lock(ctx->monitor_mutex);
            ctx->num_output_done++;
        }
        if (enable_dump) {
            store_to_file(ctx->out_file, buf, ctx->out_valid_size);
        }
    }

    {
        std::lock_guard<std::mutex> lock(ctx->out_queue_mutex);
        ctx->out_queue.push(*buf);
    }

    ctx->out_buf_done.notify_one();
}

void vpp_event(void* pv, struct vpp_event e)
{
    vpp_client_ctx* client_ctx = (vpp_client_ctx*)pv;
    switch (e.type) {
    case VPP_EVENT_FLUSH_DONE:
        DEBUG_INFO("flush done port:%d", e.flush_done.port);
        if (VPP_PORT_INPUT == e.flush_done.port) {
            client_ctx->flush_cond.notify_one();
            DEBUG_INFO("input port flush done");
        } else if (VPP_PORT_OUTPUT == e.flush_done.port) {
            client_ctx->flush_cond.notify_one();
            DEBUG_INFO("output port flush done");
        }
        break;
    case VPP_EVENT_RECONFIG_DONE:
        DEBUG_INFO("reconfig done status:%u", e.port_reconfig_done.reconfig_status);
        print_vpp_requirements(&e.port_reconfig_done.req);
        break;
    case VPP_EVENT_ERROR:
        DEBUG_ERROR("receive an error");
        break;
    case VPP_EVENT_DRAIN_DONE:
        DEBUG_INFO("drain done");
        if (client_ctx->loop) {
            client_ctx->loop->stop();
        }
        break;
    default:
        break;
    }
}

class Thread {
public:
    explicit Thread(const std::string& name)
        : mThreadObj(nullptr)
        , mName(name)
    {
        mThreadRunning.store(false);
    }

    virtual ~Thread();

    bool startThread();

    inline bool isThreadRunning()
    {
        std::lock_guard<std::mutex> lock(mThreadLock);
        return mThreadRunning == true;
    }

    inline void requestStopThread()
    {
        std::lock_guard<std::mutex> lock(mThreadLock);
        mThreadStopRequested.store(true);
    }

    bool joinThread();

protected:
    virtual void threadLoop() = 0;

    inline bool shouldExitThread()
    {
        std::lock_guard<std::mutex> lock(mThreadLock);
        return mThreadStopRequested;
    }

private:
    std::mutex mThreadLock;
    std::condition_variable mThreadStartEvent;
    std::shared_ptr<std::thread> mThreadObj;
    std::atomic_bool mThreadRunning;
    std::atomic_bool mThreadStopRequested;
    std::string mName;
};

Thread::~Thread()
{
    if (mThreadRunning) {
        // joinThread();
        DEBUG_ERROR("FATAL: (%s) is still running for the dying object !!", mName.c_str());
    }
}

bool Thread::startThread()
{
    auto disptacher = [&](Thread& r) {
        DEBUG_INFO("thread %s is now running..", mName.c_str());
        {
            std::unique_lock<std::mutex> lock(mThreadLock);
            mThreadRunning.store(true);
        }
        mThreadStartEvent.notify_one();
        r.threadLoop();
        DEBUG_INFO("thread %s stopped..", mName.c_str());
    };
    if (mThreadRunning) {
        return false;
    }

    std::unique_lock<std::mutex> lock(mThreadLock);
    mThreadStopRequested.store(false);
    mThreadObj = std::make_shared<std::thread>(disptacher, std::ref(*this));
    if (mThreadObj) {
        mThreadStartEvent.wait_for(lock, 10s);
        if (!mThreadRunning) {
            DEBUG_ERROR("Timed-out waiting for thread (%s) to start!", mName.c_str());
            mThreadObj->detach();
            mThreadObj = nullptr;
            return false;
        }
        DEBUG_INFO("Started thread (%s)", mName.c_str());
    } else {
        DEBUG_ERROR("Failed to start thread (%s)!", mName.c_str());
        return false;
    }
    return true;
}

bool Thread::joinThread()
{
    {
        std::lock_guard<std::mutex> lock(mThreadLock);
        if (!mThreadRunning) {
            return false;
        }
    }

    if (nullptr != mThreadObj) {
        if (mThreadObj->joinable()) {
            mThreadObj->join();
        }
    }

    {
        std::lock_guard<std::mutex> lock(mThreadLock);
        mThreadObj = nullptr;
        mThreadRunning.store(false);
        mThreadStopRequested.store(false);
    }
    return true;
}

class input_thread : public Thread {
public:
    input_thread(const std::string& name, vpp_client_ctx* ctx);
    virtual ~input_thread() = default;

private:
    vpp_client_ctx* ctx;
    void threadLoop();
};

input_thread::input_thread(const std::string& name, vpp_client_ctx* ctx)
    : Thread::Thread(name)
    , ctx(ctx)
{
}

void input_thread::threadLoop()
{
    DEBUG_INFO("starting input thread loop");
    vpp_buffer buf;
    uint64_t buf_idx = 0;
    uint64_t duration = 33 * 1000;
    while (!shouldExitThread()) {
        std::unique_lock<std::mutex> listLock(ctx->in_queue_mutex);
        if (ctx->in_queue.empty()) {
            DEBUG_LOW("waiting for input buffer");
            if (std::cv_status::timeout == ctx->in_buf_done.wait_for(listLock, WAIT_BUF_TIME)) {
                DEBUG_ERROR("waiting for input buffer time out");
                break;
            }
        } else {
            buf = ctx->in_queue.front();
            if (!copy_from_file(ctx->in_file, &buf, ctx->in_valid_size)) {
                ctx->in_queue.pop();
                buf.timestamp = buf_idx * duration;
                buf.cookie_in_to_out = (void*)buf_idx;
                print_vpp_buffer("ibq", &buf);
                vpp_queue_buf(ctx->vpp_ctx, VPP_PORT_INPUT, &buf);
                {
                    std::unique_lock<std::mutex> lock(ctx->monitor_mutex);
                    if (!ctx->first_time) {
                        ctx->monitor_cond.notify_one();
                        ctx->first_time = true;
                    }
                    ctx->num_input_queue++;
                }
                buf_idx++;
            } else {
                vpp_drain(ctx->vpp_ctx);
                DEBUG_INFO("stop input thread");
                requestStopThread();
            }
        }
    }

    DEBUG_INFO("exited input thread loop");
}

class output_thread : public Thread {
public:
    output_thread(const std::string& name, vpp_client_ctx* ctx);
    virtual ~output_thread() = default;

private:
    vpp_client_ctx* ctx;
    void threadLoop();
};

output_thread::output_thread(const std::string& name, vpp_client_ctx* ctx)
    : Thread::Thread(name)
    , ctx(ctx)
{
}

void output_thread::threadLoop()
{
    DEBUG_INFO("starting output thread loop");
    vpp_buffer buf;
    while (!shouldExitThread()) {
        std::unique_lock<std::mutex> listLock(ctx->out_queue_mutex);
        if (ctx->out_queue.empty()) {
            DEBUG_LOW("waiting for output buffer");
            if (std::cv_status::timeout == ctx->out_buf_done.wait_for(listLock, WAIT_BUF_TIME)) {
                DEBUG_ERROR("waiting for output buffer time out");
                break;
            }
        } else {
            buf = ctx->out_queue.front();
            ctx->out_queue.pop();
            print_vpp_buffer("obq", &buf);
            vpp_queue_buf(ctx->vpp_ctx, VPP_PORT_OUTPUT, &buf);
        }
    }

    DEBUG_INFO("exited output thread loop");
}

class monitor_thread : public Thread {
public:
    monitor_thread(const std::string& name, vpp_client_ctx* ctx);
    void stop();
    virtual ~monitor_thread() = default;

private:
    uint64_t current_time_ms()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec * 1e6 + tv.tv_usec) / 1000;
    }

    vpp_client_ctx* ctx;
    void threadLoop();
    uint32_t prev_num_input_queue;
    uint32_t prev_num_output_done;
    uint64_t last_sys_time_ms;
};

monitor_thread::monitor_thread(const std::string& name, vpp_client_ctx* ctx)
    : Thread::Thread(name)
    , ctx(ctx)
    , prev_num_input_queue(0)
    , prev_num_output_done(0)
    , last_sys_time_ms(0)
{
}

void monitor_thread::stop()
{
    std::unique_lock<std::mutex> lock(ctx->monitor_mutex);
    ctx->monitor_cond.notify_one();
}

void monitor_thread::threadLoop()
{
    DEBUG_INFO("starting monitor thread loop");
    while (!shouldExitThread()) {
        std::unique_lock<std::mutex> lock(ctx->monitor_mutex);
        ctx->monitor_cond.wait_for(lock, 1s);

        uint64_t now = current_time_ms();

        if (last_sys_time_ms) {
            uint64_t time_diff_ms = now - last_sys_time_ms;

            double avg_in_fps = (ctx->num_input_queue - prev_num_input_queue) * 1000.0 / time_diff_ms;
            double avg_out_fps = (ctx->num_output_done - prev_num_output_done) * 1000.0 / time_diff_ms;

            DEBUG_INFO("input queue:%u->%u output done:%u->%u time diff:%" PRIu64 "ms",
                prev_num_input_queue, ctx->num_input_queue,
                prev_num_output_done, ctx->num_output_done, time_diff_ms);
            DEBUG_INFO("avg in fps:%.1f out fps:%.1f", avg_in_fps, avg_out_fps);

            prev_num_input_queue = ctx->num_input_queue;
            prev_num_output_done = ctx->num_output_done;
        }
        last_sys_time_ms = now;
    }

    DEBUG_INFO("exited monitor thread loop");
}

typedef struct DmaBuffer {
    int fd;
    int handle;
    int size;
    int format;
    int width;
    int height;
    bool ubwc_flags;
    void* priv;
} DmaBuffer;

class Allocator {
public:
    Allocator();
    virtual ~Allocator();
    virtual bool allocateBuffer(struct DmaBuffer* buffer);
    virtual void freeBuffer(struct DmaBuffer* buffer);
};

Allocator::Allocator()
{
}

Allocator::~Allocator()
{
}

bool Allocator::allocateBuffer(struct DmaBuffer* buffer)
{
    UNUSED(buffer);

    return true;
}

void Allocator::freeBuffer(struct DmaBuffer* buffer)
{
    UNUSED(buffer);
}

class DmaAllocator : public Allocator {
public:
    DmaAllocator();
    ~DmaAllocator();
    bool allocateBuffer(struct DmaBuffer* buffer) override;
    void freeBuffer(struct DmaBuffer* buffer) override;

private:
    BufferAllocator* mAlloc;
};

DmaAllocator::DmaAllocator()
{
    mAlloc = CreateDmabufHeapBufferAllocator();
    if (!mAlloc) {
        DEBUG_ERROR("Failed to create dma allocator");
    }
}

DmaAllocator::~DmaAllocator()
{
    if (mAlloc) {
        FreeDmabufHeapBufferAllocator(mAlloc);
    }
}

// For NV12 only
uint32_t calculate_valid_size(uint32_t format, uint32_t width, uint32_t height)
{
    uint32_t size = 0;
    uint32_t y_stride = 0;
    uint32_t uv_stride = 0;
    uint32_t y_sclines = 0;
    uint32_t uv_sclines = 0;

    y_stride = VENUS_Y_STRIDE(format, width);
    y_sclines = VENUS_Y_SCANLINES(format, height);

    uv_stride = VENUS_UV_STRIDE(format, width);
    uv_sclines = VENUS_UV_SCANLINES(format, height);

    size = y_stride * y_sclines + uv_stride * uv_sclines;

    return size;
}

bool DmaAllocator::allocateBuffer(struct DmaBuffer* buffer)
{
    bool ret = true;

    if (mAlloc) {
        if (buffer->format == COLOR_FMT_NV12) {
            buffer->size = VENUS_BUFFER_SIZE(buffer->format, buffer->width, buffer->height);
        } else if (buffer->format == COLOR_FMT_NV12_UBWC) {
            buffer->size = VENUS_BUFFER_SIZE_USED(buffer->format, buffer->width, buffer->height, 0);
        } else {
            ret = false;
            DEBUG_ERROR("not suported format %d", buffer->format);
        }

        if (ret) {
            buffer->fd = DmabufHeapAlloc(mAlloc, "qcom,system-uncached", buffer->size, 0, 0);

            DEBUG_HIGH("created dma buf size %d, wxh %dx%d, fd %d", buffer->size,
                buffer->width, buffer->height, buffer->fd);
        }
    }

    return ret;
}

void DmaAllocator::freeBuffer(struct DmaBuffer* buffer)
{
    if (!buffer) {
        DEBUG_ERROR("Buffer param pointer is NULL");
        return;
    }

    DEBUG_HIGH("free dma buf size %d, fd %d",
        buffer->size, buffer->fd);

    if (buffer->fd) {
        close(buffer->fd);
        buffer->fd = -1;
    }
}

bool fill_vppbuf_with_dmabuf(vpp_buffer* vpp_buf, DmaBuffer* buf)
{
    bool ret = false;

    if (buf && vpp_buf) {
        vpp_buf->pixel.fd = buf->fd;
        vpp_buf->pixel.alloc_len = buf->size;
        vpp_buf->pixel.filled_len = buf->size;
        vpp_buf->pixel.valid_data_len = buf->size;
        vpp_buf->pvGralloc = buf->priv;
        ret = true;
    }

    return ret;
}

bool fill_dmabuf_with_vppbuf(DmaBuffer* buf, vpp_buffer* vpp_buf)
{
    bool ret = false;

    if (buf && vpp_buf) {
        buf->fd = vpp_buf->pixel.fd;
        buf->size = (int)vpp_buf->pixel.alloc_len;
        buf->priv = vpp_buf->pvGralloc;
        ret = true;
    }

    return ret;
}

static int parse_resolution(const char* args, int direction)
{
    int ret;
    int width, height;

    ret = sscanf(args, "%dx%d", &width, &height);
    if (ret != 2) {
        DEBUG_ERROR("Invalid args: %s", args);
        return -1;
    }

    if (width <= 0 || width > AIS_2X_MAX_INPUT_WIDTH
        || height <= 0 || height > AIS_2X_MAX_INPUT_HEIGHT) {
        DEBUG_ERROR("Not supported width:height: %dx%d", width, height);
        return -1;
    }

    switch (direction) {
    case AIS_TEST_INPUT:
        input_width = width;
        input_height = height;
        break;
    case AIS_TEST_OUTPUT:
        output_width = width;
        output_height = height;
        break;
    default:
        break;
    }

    return 0;
}

static int parse_format(const char* arg, int direction)
{
    vpp_color_format format;

    DEBUG_INFO("%s format: %s", (AIS_TEST_INPUT == direction) ? "input" : "output", arg);

    if (!strncmp(arg, "NV12_UBWC", 9))
        format = VPP_COLOR_FORMAT_UBWC_NV12;
    else if (!strncmp(arg, "NV12", 4))
        format = VPP_COLOR_FORMAT_NV12_VENUS;
    else
        return -1;

    switch (direction) {
    case AIS_TEST_INPUT:
        in_format = format;
        break;
    case AIS_TEST_OUTPUT:
        out_format = format;
        break;
    default:
        break;
    }

    return 0;
}

static struct option longopts[] = {
    { "input-resolution", required_argument, NULL, 'i' },
    { "input-file", required_argument, NULL, 'I' },
    { "output-file", required_argument, NULL, 'O' },
    { "input-format", required_argument, NULL, 'f' },
    { "scale-ratio", required_argument, NULL, 'r' },
    { "enable-dump", required_argument, NULL, 'D' },
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 }
};

static void help()
{
    int idx = 0;

    printf("=============================\n");
    printf("vpp_ais_test args... \n");
    printf("=============================\n\n");
    printf("  -i --%s=WIDTHxHEIGHT\n", longopts[idx++].name);
    printf("  -I --%s=FILE\n", longopts[idx++].name);
    printf("  -O --%s=FILE\n", longopts[idx++].name);
    printf("  -f --%s=FORMAT as NV12,NV12_UBWC\n", longopts[idx++].name);
    printf("  -r --%s=SCALE RATIO 2X or 3X \n", longopts[idx++].name);
    printf("  -D --%s=ENABLE/DISABLE DUMP, enabled by default \n", longopts[idx++].name);
    printf("  -h --help\n");
    printf("Example:\n  ");
    printf("ais-test -i 512x512 -o 1024x1024 -I 512p.nv12 -O 1024p.nv12 -f NV12 \n");
    printf("=============================\n");
    exit(0);
}

static int parse_args(int argc, char** argv)
{
    int command;

    while ((command = getopt_long(argc, argv, "i:I:O:f:r:D:h", longopts, NULL)) != -1) {
        switch (command) {
        case 'i':
            if (parse_resolution(optarg, AIS_TEST_INPUT) != 0) {
                DEBUG_ERROR("error parsing input resolution, width: %d, height: %d",
                    input_width, input_height);
                return -1;
            }
            break;
        case 'I':
            infile = optarg;
            if (!infile)
                return -1;
            break;
        case 'O':
            outfile = optarg;
            if (!outfile)
                return -1;
            break;
        case 'f':
            if (parse_format(optarg, AIS_TEST_INPUT) != 0) {
                DEBUG_ERROR("error parsing input format, format: %d", in_format);
                return -1;
            }
            break;
        case 'r':
            scale_ratio_setting = atoi(optarg);
            if (scale_ratio_setting <= 0 || scale_ratio_setting > 3) {
                DEBUG_ERROR("Invalid scale ratio %d", scale_ratio_setting);
                return -1;
            }
            break;
        case 'D':
            enable_dump = atoi(optarg);
            DEBUG_INFO("output dump is %s", enable_dump ? "enabled" : "disabled");
            break;
        case 'h':
            help();
            break;
        default:
            DEBUG_ERROR("invaild argument");
            help();
            break;
        }
    }

    if (infile == NULL || input_width <= 0 || input_height <= 0 || output_width < 0 || output_height < 0)
        help();

    return 0;
}

enum color_fmts convert_vpp_color_fmt(vpp_color_format vpp_format)
{
    color_fmts ret = COLOR_FMT_NV12;

    switch (vpp_format) {
    case VPP_COLOR_FORMAT_NV12_VENUS:
        break;
    case VPP_COLOR_FORMAT_UBWC_NV12:
        ret = COLOR_FMT_NV12_UBWC;
        break;
    default:
        DEBUG_ERROR("not supported vpp format %d", vpp_format);
        break;
    }

    return ret;
}

static void init_debug_level()
{
    char* env_ptr = getenv("AIS_UNIT_DEBUG");
    ais_test_debug_level = env_ptr ? atoi(env_ptr) : PRIO_ERROR;
    printf("init debug level %d\n", ais_test_debug_level);
}

static uint32_t max_support_ratio(uint32_t width, uint32_t height)
{
    uint32_t ratio = 1;

    if (width < AIS_MIN_INPUT_WIDTH
        || height < AIS_MIN_INPUT_HEIGHT
        || width > AIS_2X_MAX_INPUT_WIDTH
        || height > AIS_2X_MAX_INPUT_HEIGHT) {
        ratio = 1;
    } else if (width <= AIS_3X_MAX_INPUT_WIDTH
        && height <= AIS_3X_MAX_INPUT_HEIGHT) {
        //  3X
        ratio = 3;
    } else {
        //  2X
        ratio = 2;
    }

    return ratio;
}

int main(int argc, char** argv)
{
    struct vpp_callbacks cb;
    struct vpp_requirements req;
    struct hqv_control ctrl;
    vpp_error ret;
    void* ctx;
    enum vpp_port in_port = VPP_PORT_INPUT;
    enum vpp_port out_port = VPP_PORT_OUTPUT;
    struct vpp_port_param in_param;
    struct vpp_port_param out_param;
    enum color_fmts in_fmt;
    enum color_fmts out_fmt;
    main_loop loop;

    init_debug_level();

    if (parse_args(argc, argv) != 0) {
        DEBUG_ERROR("Invalid arguments");
        help();
        return -1;
    }

    int max_ratio = max_support_ratio(input_width, input_height);

    DEBUG_INFO("set scale ratio:%d", scale_ratio_setting);
    if (scale_ratio_setting) {
        scale_ratio_setting = std::min(scale_ratio_setting, max_ratio);
    } else {
        scale_ratio_setting = max_ratio;
    }

    DEBUG_INFO("actual scale ratio:%d", scale_ratio_setting);

    output_width = input_width * scale_ratio_setting;
    output_height = input_height * scale_ratio_setting;

    // For AIS, input format is equal to output format.
    out_format = in_format;
    in_fmt = convert_vpp_color_fmt(in_format);
    out_fmt = convert_vpp_color_fmt(out_format);

    std::shared_ptr<Allocator> allocator = std::make_shared<DmaAllocator>();

    vpp_client_ctx client_ctx;
    client_ctx.first_time = false;
    client_ctx.num_input_queue = 0;
    client_ctx.num_output_done = 0;

    file_op in_file(infile, "r");

    char out_file_name[MAX_FILE_NAME] = { 0 };

    output_stride = VENUS_Y_STRIDE(out_fmt, output_width);
    output_scanline = VENUS_Y_SCANLINES(out_fmt, output_height);

    if (out_format == VPP_COLOR_FORMAT_NV12_VENUS) {
        snprintf(out_file_name, sizeof(out_file_name), "%dx%d.%s", output_stride, output_scanline, "nv12");
        client_ctx.in_valid_size = calculate_valid_size(in_fmt, input_width, input_height);
        client_ctx.out_valid_size = calculate_valid_size(out_fmt, output_width, output_height);
    } else if (out_format == VPP_COLOR_FORMAT_UBWC_NV12) {
        snprintf(out_file_name, sizeof(out_file_name), "%dx%d.%s", output_stride, output_scanline, "ubwc");
        client_ctx.in_valid_size = VENUS_BUFFER_SIZE_USED(in_fmt, input_width, input_height, 0);
        client_ctx.out_valid_size = VENUS_BUFFER_SIZE_USED(out_fmt, output_width, output_height, 0);
    }

    DEBUG_INFO("output file name:%s", out_file_name);

    outfile = out_file_name;

    client_ctx.in_file = &in_file;
    file_op out_file(outfile, "w+");
    client_ctx.out_file = &out_file;
    client_ctx.loop = &loop;

    DEBUG_INFO("in valid size %u", client_ctx.in_valid_size);
    DEBUG_INFO("out valid size %u", client_ctx.out_valid_size);

    cb.pv = &client_ctx;
    cb.input_buffer_done = input_buffer_done;
    cb.output_buffer_done = output_buffer_done;
    cb.vpp_event = vpp_event;

    ctx = vpp_init(0, cb);

    client_ctx.vpp_ctx = ctx;

    std::shared_ptr<input_thread> in_thread = std::make_shared<input_thread>("input_buffer_thread", &client_ctx);
    std::shared_ptr<output_thread> out_thread = std::make_shared<output_thread>("output_buffer_thread", &client_ctx);
    std::shared_ptr<monitor_thread> monit_thread = std::make_shared<monitor_thread>("monitor_thread", &client_ctx);

    memset(&ctrl, 0, sizeof(struct hqv_control));
    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_AIS;
    ctrl.ais.mode = HQV_MODE_AUTO;
    ctrl.ais.roi.enable = 0;
    ctrl.ais.classification = 10;

    vpp_set_ctrl(ctx, ctrl);

    in_param.width = input_width;
    in_param.height = input_height;
    in_param.stride = VENUS_Y_STRIDE(in_fmt, in_param.width);
    in_param.scanlines = VENUS_Y_SCANLINES(in_fmt, in_param.height);
    in_param.fmt = in_format;

    DEBUG_INFO("in w:h:%ux%u stride:scan:%ux%u", in_param.width, in_param.height,
        in_param.stride, in_param.scanlines);

    out_param.width = output_width;
    out_param.height = output_height;
    out_param.stride = output_stride;
    out_param.scanlines = output_scanline;
    out_param.fmt = out_format;

    DEBUG_INFO("out w:h:%ux%u stride:scan:%ux%u", out_param.width, out_param.height,
        out_param.stride, out_param.scanlines);

    vpp_set_parameter(ctx, in_port, in_param);
    vpp_set_parameter(ctx, out_port, out_param);

    DmaBuffer in_buf = {
        .fd = 0,
        .handle = 0,
        .size = 0,
        .format = in_fmt,
        .width = input_width,
        .height = input_height,
        .ubwc_flags = false,
        .priv = NULL
    };
    DmaBuffer out_buf = {
        .fd = 0,
        .handle = 0,
        .size = 0,
        .format = out_fmt,
        .width = output_width,
        .height = output_height,
        .ubwc_flags = false,
        .priv = NULL
    };
    vpp_buffer vpp_in_buf;
    vpp_buffer vpp_out_buf;

    vpp_get_buf_requirements(ctx, &req);
    print_vpp_requirements(&req);

    ret = (vpp_error)vpp_open(ctx);

    int in_req_cnt = req.buf_req[VPP_RESOLUTION_HD].in_req;
    int out_req_cnt = req.buf_req[VPP_RESOLUTION_HD].out_req;
    DEBUG_LOW("buf req: in:%d out:%d", in_req_cnt, out_req_cnt);

    for (int i = 0; i < in_req_cnt; i++) {
        memset(&vpp_in_buf, 0, sizeof(vpp_buffer));
        allocator->allocateBuffer(&in_buf);
        fill_vppbuf_with_dmabuf(&vpp_in_buf, &in_buf);
        {
            std::unique_lock<std::mutex> listLock(client_ctx.in_queue_mutex);
            client_ctx.in_queue.push(vpp_in_buf);
            DEBUG_LOW("add input buffer to queue");
        }
    }

    for (int i = 0; i < out_req_cnt; i++) {
        memset(&vpp_out_buf, 0, sizeof(vpp_buffer));
        allocator->allocateBuffer(&out_buf);
        fill_vppbuf_with_dmabuf(&vpp_out_buf, &out_buf);
        {
            std::unique_lock<std::mutex> listLock(client_ctx.out_queue_mutex);
            print_vpp_buffer("obq", &vpp_out_buf);
            vpp_queue_buf(ctx, VPP_PORT_OUTPUT, &vpp_out_buf);
        }
    }

    in_thread->startThread();
    out_thread->startThread();
    monit_thread->startThread();

    loop.start();

    // retrieve all buffers
    in_thread->requestStopThread();
    vpp_flush(ctx, VPP_PORT_INPUT);
    {
        std::unique_lock<std::mutex> flush_lock(client_ctx.flush_mutex);
        client_ctx.flush_cond.wait_for(flush_lock, 1s);
    }

    out_thread->requestStopThread();
    vpp_flush(ctx, VPP_PORT_OUTPUT);
    {
        std::unique_lock<std::mutex> flush_lock(client_ctx.flush_mutex);
        client_ctx.flush_cond.wait_for(flush_lock, 1s);
    }

    monit_thread->requestStopThread();
    monit_thread->stop();

    while (!client_ctx.in_queue.empty()) {
        vpp_buffer buf;
        DmaBuffer dma_buf;
        memset(&buf, 0, sizeof(vpp_buffer));
        memset(&dma_buf, 0, sizeof(DmaBuffer));

        buf = client_ctx.in_queue.front();
        fill_dmabuf_with_vppbuf(&dma_buf, &buf);
        client_ctx.in_queue.pop();
        allocator->freeBuffer(&dma_buf);
        DEBUG_LOW("free input buffer");
    }

    while (!client_ctx.out_queue.empty()) {
        vpp_buffer buf;
        DmaBuffer dma_buf;
        memset(&buf, 0, sizeof(vpp_buffer));
        memset(&dma_buf, 0, sizeof(DmaBuffer));

        buf = client_ctx.out_queue.front();
        fill_dmabuf_with_vppbuf(&dma_buf, &buf);
        client_ctx.out_queue.pop();
        allocator->freeBuffer(&dma_buf);
        DEBUG_LOW("free output buffer");
    }

    ret = (vpp_error)vpp_close(ctx);

    vpp_term(ctx);

    in_thread->joinThread();
    out_thread->joinThread();
    monit_thread->joinThread();

    return 0;
}

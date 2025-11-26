/*
 **************************************************************************************************
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

#include "vpp.h"
#include <gtest/gtest.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <map>
#include <utility>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <thread>
#include <atomic>
#include <list>
#include <vidc/media/msm_media_info.h>
#include <inttypes.h>
#ifdef _LINUX_
#include <gbm.h>
#include <gbm_priv.h>
#else
#include <BufferAllocator/BufferAllocatorWrapper.h>
#endif

using namespace std::chrono_literals;
#define UNUSED(x) (void)(x)

enum {
  PRIO_ERROR=0x1,
  PRIO_INFO=0x2,
  PRIO_HIGH=0x4,
  PRIO_LOW=0x8
};

int vpp_test_debug_level = PRIO_ERROR;

#define WAIT_BUF_TIME 5s

#define DEBUG_LOG(level, fmt, args...) \
    do { \
        if (level & vpp_test_debug_level) \
            printf(fmt "\n", ##args); \
    } while (0)

#define DEBUG_ERROR(fmt, args...) DEBUG_LOG(PRIO_ERROR, fmt, ##args)
#define DEBUG_INFO(fmt, args...)  DEBUG_LOG(PRIO_INFO, fmt, ##args)
#define DEBUG_HIGH(fmt, args...)  DEBUG_LOG(PRIO_HIGH, fmt, ##args)
#define DEBUG_LOW(fmt, args...)   DEBUG_LOG(PRIO_LOW, fmt, ##args)

class FileOp;

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
    FileOp *in_file;
    FileOp *out_file;
} vpp_client_ctx;

/*
 * Component Test class
 */

class HypVPPTest
{
    public:
        bool vpp;
        void* vpp_ctx;

};

class FileOp
{
    public:
        FileOp(const std::string& fileName, const std::string& mode);
        ~FileOp();
        int read_data(void *ptr, int size);
        int write_data(void *ptr, int size);
    private:
        FILE *mFp;

};

FileOp::FileOp(const std::string& fileName, const std::string& mode)
{
    mFp = fopen(fileName.c_str(), mode.c_str());
    if (!mFp)
        perror("Failed ");
}

int FileOp::read_data(void *ptr, int size)
{
    int read_len = 0;
    if (mFp)
    {
        read_len = fread(ptr, 1, size, mFp);
    }

    return read_len;
}

int FileOp::write_data(void *ptr, int size)
{
    int write_len = 0;

    if (mFp)
    {
        write_len = fwrite(ptr, 1, size, mFp);
        if (write_len != size)
        {
            perror("Failed to read: ");
        }
        else
        {
            fflush(mFp);
        }
    }

    return write_len;
}

FileOp::~FileOp()
{
    if (mFp)
    {
        fclose(mFp);
    }
}

#ifdef _LINUX_
int convert_to_gbm_fmt (int color_format)
{
    int gbm_fmt = 0;

    switch (color_format) {
        case COLOR_FMT_NV12:
        case COLOR_FMT_NV12_UBWC:
            gbm_fmt = GBM_FORMAT_NV12;
            break;
        default:
            DEBUG_ERROR("format 0x%x is not supported", (unsigned int)color_format);
            break;
    }

    return gbm_fmt;
}
#endif

void print_vpp_mem_buffer (struct vpp_mem_buffer *mem_buf)
{
    if (mem_buf)
    {
        DEBUG_INFO("fd:%d offset:%d alloc_len:%u filled_len:%u valid_data_len:%u",
                mem_buf->fd, mem_buf->offset, mem_buf->alloc_len,
                mem_buf->filled_len, mem_buf->valid_data_len);
    }
}

void print_vpp_buffer (struct vpp_buffer *buf)
{
    if (buf)
    {
        print_vpp_mem_buffer (&buf->pixel);
        DEBUG_INFO("flags:%u timestamp:%" PRIu64, buf->flags, buf->timestamp);
    }
}

// if reach eof, return true
bool copy_from_file(FileOp *file, struct vpp_buffer *buf)
{
    // TODO return if file is null
    int fd = buf->pixel.fd;
    int buf_size = VENUS_BUFFER_SIZE_USED(COLOR_FMT_NV12_UBWC, 1280, 720, 0);
    char *src_buf = (char*)malloc(buf_size);
    char *bufaddr = nullptr;
    bool is_eof = false;

    if (NULL == src_buf)
      return is_eof;

    memset(src_buf, 0, buf_size);

    if (!file->read_data(src_buf, buf_size))
    {
        is_eof = true;
        goto done;
    }

    bufaddr = (char*)mmap(NULL, buf_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (bufaddr)
    {
        memcpy(bufaddr, src_buf, buf_size);
    }
    munmap(bufaddr, buf_size);

done:
    free (src_buf);

    return is_eof;
}

void store_to_file(FileOp *file, struct vpp_buffer *buf)
{
    int fd = buf->pixel.fd;
    int buf_size = VENUS_BUFFER_SIZE_USED(COLOR_FMT_NV12_UBWC, 1280, 720, 0);
    char *dest_buf = (char*)malloc(buf_size);
    int write_len = 0;

    if (dest_buf) {
        memset(dest_buf, 0, buf_size);

        char *bufaddr = (char*)mmap(NULL, buf_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if (bufaddr)
        {
            memcpy(dest_buf, bufaddr, buf_size);
        }

        write_len = file->write_data(dest_buf, buf_size);
        if (write_len != buf_size)
        {
            DEBUG_ERROR("write file error");
        }

        munmap(bufaddr, buf_size);
        free (dest_buf);
    }
}

void print_vpp_requirements(struct vpp_requirements *req)
{
    if (req)
    {
        DEBUG_INFO("get buffer requirement in_req_cnt:SD:%u HD:%u FHD:%u UHD:%u"
                   " out_req_cnt:SD:%u HD:%u FHD:%u UHD:%u",
                   req->buf_req[0].in_req, req->buf_req[1].in_req,
                   req->buf_req[2].in_req, req->buf_req[3].in_req,
                   req->buf_req[0].out_req, req->buf_req[1].out_req,
                   req->buf_req[2].out_req, req->buf_req[3].out_req);
    }
}

void input_buffer_done(void *pv, struct vpp_buffer *buf)
{
    vpp_client_ctx *ctx = (vpp_client_ctx *)pv;
    DEBUG_LOW("input buffer done");
    print_vpp_buffer(buf);
    {
        std::lock_guard<std::mutex> lock(ctx->in_queue_mutex);
        ctx->in_queue.push(*buf);
    }

    DEBUG_LOW("notify input buffer");
    ctx->in_buf_done.notify_one();
}

void output_buffer_done(void *pv, struct vpp_buffer *buf)
{
    vpp_client_ctx *ctx = (vpp_client_ctx *)pv;
    DEBUG_LOW("output buffer done");
    print_vpp_buffer(buf);
#ifdef DUMP
    store_to_file(ctx->out_file, buf);
#endif
    {
        std::lock_guard<std::mutex> lock(ctx->out_queue_mutex);
        ctx->out_queue.push(*buf);
    }

    ctx->out_buf_done.notify_one();
}

void vpp_event(void *pv, struct vpp_event e)
{
    vpp_client_ctx *client_ctx = (vpp_client_ctx *)pv;
    switch (e.type) {
        case VPP_EVENT_FLUSH_DONE:
            DEBUG_INFO("flush done port:%d", e.flush_done.port);
            if (VPP_PORT_INPUT == e.flush_done.port)
            {
                client_ctx->flush_cond.notify_one();
                DEBUG_INFO("input port flush done");
            }
            else if (VPP_PORT_OUTPUT == e.flush_done.port)
            {
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
        default:
            break;
    }
}

class Thread {
 public:
    explicit Thread(const std::string& name)
        : mThreadObj(nullptr),
          mName(name) {
        mThreadRunning.store(false);
    }

    virtual ~Thread();

    bool startThread();

    inline bool isThreadRunning() {
        std::lock_guard<std::mutex> lock(mThreadLock);
        return mThreadRunning == true;
    }

    inline void requestStopThread() {
        std::lock_guard<std::mutex> lock(mThreadLock);
        mThreadStopRequested.store(true);
    }

    bool joinThread();

 protected:
    virtual void threadLoop() = 0;

    inline bool shouldExitThread() {
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

Thread::~Thread() {
    if (mThreadRunning) {
        // joinThread();
        DEBUG_ERROR("FATAL: (%s) is still running for the dying object !!", mName.c_str());
    }
}

bool Thread::startThread() {
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

bool Thread::joinThread() {
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

class input_thread : public Thread
{
    public:
        input_thread(const std::string& name, vpp_client_ctx* ctx);
        virtual ~input_thread() = default;
    private:
        vpp_client_ctx* ctx;
        void threadLoop();
};

input_thread::input_thread(const std::string& name, vpp_client_ctx* ctx):
    Thread::Thread(name), ctx(ctx)
{
}

void input_thread::threadLoop()
{
    DEBUG_INFO("starting input thread loop");
    vpp_buffer buf;
    uint64_t buf_idx = 0;
    uint64_t duration = 33 * 1000;
    while (!shouldExitThread())
    {
        std::unique_lock<std::mutex> listLock(ctx->in_queue_mutex);
        if(ctx->in_queue.empty())
        {
            DEBUG_LOW("waiting for input buffer");
            if (std::cv_status::timeout == ctx->in_buf_done.wait_for(listLock, WAIT_BUF_TIME))
            {
                DEBUG_ERROR("waiting for input buffer time out");
                break;
            }
        }
        else
        {
            buf = ctx->in_queue.front();
            ctx->in_queue.pop();
            if (!copy_from_file(ctx->in_file, &buf))
            {
                buf.timestamp = buf_idx * duration;
                print_vpp_buffer(&buf);
                vpp_queue_buf(ctx->vpp_ctx, VPP_PORT_INPUT, &buf);
                DEBUG_LOW("queue input buffer");
                buf_idx++;
            }
            else
            {
                buf.pixel.filled_len = 0;
                buf.pixel.valid_data_len = 0;
                buf.flags |= VPP_BUFFER_FLAG_EOS;
                DEBUG_LOW("queue input buffer with EOS");
                vpp_queue_buf(ctx->vpp_ctx, VPP_PORT_INPUT, &buf);
                DEBUG_INFO("stop input thread");
                requestStopThread();
            }
        }
    }

    DEBUG_INFO("exited input thread loop");
}

class output_thread : public Thread
{
    public:
        output_thread(const std::string& name, vpp_client_ctx* ctx);
        virtual ~output_thread() = default;
    private:
        vpp_client_ctx* ctx;
        void threadLoop();
};

output_thread::output_thread(const std::string& name, vpp_client_ctx* ctx):
    Thread::Thread(name), ctx(ctx)
{
}

void output_thread::threadLoop()
{
    DEBUG_INFO("starting output thread loop");
    vpp_buffer buf;
    while (!shouldExitThread())
    {
        std::unique_lock<std::mutex> listLock(ctx->out_queue_mutex);
        if(ctx->out_queue.empty())
        {
            DEBUG_LOW("waiting for output buffer");
            if (std::cv_status::timeout == ctx->out_buf_done.wait_for(listLock, WAIT_BUF_TIME))
            {
                DEBUG_ERROR("waiting for output buffer time out");
                break;
            }
        }
        else
        {
            buf = ctx->out_queue.front();
            if (buf.flags & VPP_BUFFER_FLAG_EOS)
            {
                // keep the output buffer with EOS in queue,
                // so that it can be freed in the end
                DEBUG_INFO("receive EOS, stop output thread");
                requestStopThread();
            }
            else
            {
                ctx->out_queue.pop();
                store_to_file(ctx->out_file, &buf);
                DEBUG_LOW("queue output buffer");
                vpp_queue_buf(ctx->vpp_ctx, VPP_PORT_OUTPUT, &buf);
            }
        }
    }

    DEBUG_INFO("exited output thread loop");
}

typedef struct DmaBuffer {
    int fd;
    int handle;
    void *ptr;
    int size;
    int format;
    int width;
    int height;
    bool ubwc_flags;
    void *priv;
} DmaBuffer;

class Allocator
{
public:
    Allocator();
    virtual ~Allocator();
    virtual bool allocateBuffer(struct DmaBuffer *buffer);
    virtual void freeBuffer(struct DmaBuffer *buffer);
};

Allocator::Allocator()
{
}

Allocator::~Allocator()
{
}

bool Allocator::allocateBuffer(struct DmaBuffer *buffer)
{
    UNUSED(buffer);

    return true;
}

void Allocator::freeBuffer(struct DmaBuffer *buffer)
{
    UNUSED(buffer);
}

#ifdef _LINUX_
class GbmAllocator: public Allocator
{
public:
    GbmAllocator();
    ~GbmAllocator();
    bool allocateBuffer(struct DmaBuffer *buffer) override;
    void freeBuffer(struct DmaBuffer *buffer) override;

private:
    int mGbmDevFd;
    struct gbm_device *mGbmDevice;
};

GbmAllocator::GbmAllocator()
{
#define GBMDEV_DEVICE_NODE "/dev/dri/renderD128"
    mGbmDevFd = open (GBMDEV_DEVICE_NODE, O_RDWR | O_CLOEXEC);
    if (mGbmDevFd < 0) {
        DEBUG_ERROR ("failed to open gbm device %s", GBMDEV_DEVICE_NODE);
        return;
    }

    mGbmDevice = gbm_create_device (mGbmDevFd);
    if (NULL == mGbmDevice) {
        DEBUG_ERROR ("failed to create gbm_device");
        close (mGbmDevFd);
        return;
    }
}

GbmAllocator::~GbmAllocator()
{
    if (mGbmDevice) {
        gbm_device_destroy(mGbmDevice);
        close (mGbmDevFd);
        mGbmDevice = NULL;
        mGbmDevFd = -1;
    }
}

bool GbmAllocator::allocateBuffer(struct DmaBuffer *buffer)
{
    uint32_t flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
    void *va = NULL;
    struct gbm_bo *bo = NULL;
    int gbm_fmt = 0;

    if (!buffer) {
        DEBUG_ERROR("Buffer param pointer is NULL");
        return false;
    }

    if (!buffer->width || !buffer->height) {
        DEBUG_ERROR("Buffer param width/height is NULL");
        return false;
    }
    if (!buffer->format) {
        DEBUG_ERROR("Buffer format is NULL");
        return false;
    }

    gbm_fmt = convert_to_gbm_fmt(buffer->format);
    if (!gbm_fmt) {
        DEBUG_ERROR("Buffer format is not supported");
        return false;
    }

    DEBUG_INFO("create a gbm_bo(width:%d, heigh:%d) for format %d",
               buffer->width, buffer->height, buffer->format);

    if (true == buffer->ubwc_flags)
        flags |= GBM_BO_USAGE_UBWC_ALIGNED_QTI;

    bo = gbm_bo_create(mGbmDevice, buffer->width,
                     buffer->height, gbm_fmt, flags);
    if (NULL == bo) {
        DEBUG_ERROR ("failed to create a bo");
        return false;
    }

    buffer->priv = (void*)bo;
    buffer->fd = bo->ion_fd;

    if (buffer->fd < 0) {
        DEBUG_ERROR ("bo_fd:%d are invalid", buffer->fd);
        goto fail;
    }

    if ((int)bo->size < (int)buffer->size)
        DEBUG_HIGH ("gbm buffer size should be more than the size of input buffer");

    buffer->size = bo->size;
    va = mmap(NULL, buffer->size, PROT_READ|PROT_WRITE, MAP_SHARED, buffer->fd, 0);
    if (MAP_FAILED == va) {
        DEBUG_ERROR ("failed to map buffer of size = %d, fd = %d", buffer->size, buffer->fd);
        goto fail;
    }

    DEBUG_INFO("created gbm_bo %p(%u x %u, fmt %u, size %u, stride %u), exported fd %d, va:%p",
               bo, bo->width, bo->height, bo->format,
               bo->size, bo->stride, buffer->fd, va);

    buffer->ptr = va;

    return true;

fail:
    if (bo)
        gbm_bo_destroy (bo);
    buffer->priv = NULL;

    return false;
}

void GbmAllocator::freeBuffer(struct DmaBuffer *buffer)
{
    struct gbm_bo *bo = NULL;

    if (!buffer) {
        DEBUG_ERROR ("Buffer param pointer is NULL");
        return;
    }

    if (buffer->ptr) {
        DEBUG_LOW("unmap va:%p", buffer->ptr);
        munmap (buffer->ptr, buffer->size);
        buffer->ptr = NULL;
    }

    if (buffer->priv) {
        bo = (struct gbm_bo*)buffer->priv;
        gbm_bo_destroy (bo);
        buffer->priv = NULL;
        buffer->fd = -1;
    }
}

#else
class DmaAllocator: public Allocator
{
public:
    DmaAllocator();
    ~DmaAllocator();
    bool allocateBuffer(struct DmaBuffer *buffer) override;
    void freeBuffer(struct DmaBuffer *buffer) override;

private:
    BufferAllocator *mAlloc;
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

bool DmaAllocator::allocateBuffer(struct DmaBuffer *buffer)
{
    void* va = NULL;

    if (mAlloc) {
       buffer->size = VENUS_BUFFER_SIZE(buffer->format, buffer->width, buffer->height);
       buffer->fd = DmabufHeapAlloc(mAlloc, "qcom,system-uncached", buffer->size, 0, 0);
       va = mmap(NULL, buffer->size, PROT_READ|PROT_WRITE, MAP_SHARED, buffer->fd, 0);
       if (MAP_FAILED == va) {
           DEBUG_ERROR ("failed to map buffer of size = %u, fd = 0x%x", buffer->size, buffer->fd);
           goto fail;
       }

       buffer->ptr = va;

       DEBUG_INFO("created dma buf size %u, exported fd %d, va:%p",
               buffer->size, buffer->fd, va);
    }

    return true;

fail:
    return false;
}

void DmaAllocator::freeBuffer(struct DmaBuffer *buffer)
{
    if (!buffer) {
        DEBUG_ERROR ("Buffer param pointer is NULL");
        return;
    }

    if (buffer->ptr) {
        DEBUG_LOW("unmap va:%p", buffer->ptr);
        munmap(buffer->ptr, buffer->size);
        buffer->ptr = NULL;
    }

    if (buffer->fd) {
        close(buffer->fd);
        buffer->fd = -1;
    }
}
#endif

bool fill_vppbuf_with_dmabuf (vpp_buffer* vpp_buf, DmaBuffer* buf)
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

bool fill_dmabuf_with_vppbuf (DmaBuffer* buf, vpp_buffer* vpp_buf)
{
    bool ret = false;

    if (buf && vpp_buf) {
        buf->fd = vpp_buf->pixel.fd;
        buf->size = (int)vpp_buf->pixel.alloc_len;
        buf->priv = (struct gbm_bo*)vpp_buf->pvGralloc;
        ret = true;
    }

    return ret;
}


class main_loop {
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
        if (std::cv_status::timeout == cond_var.wait_for(lock, WAIT_BUF_TIME)) {
            DEBUG_HIGH("time out, exit from main loop");
            break;
        }
    }
    DEBUG_INFO("exit from main loop");

    return true;
}

bool main_loop::stop()
{
    is_running.store(false);
    return true;
}


TEST(HypVPPTest, InitTest)
{
    std::shared_ptr<HypVPPTest> hyp_vpp_test = std::make_shared<HypVPPTest>();
    struct vpp_callbacks cb;

    cb.input_buffer_done = input_buffer_done;
    cb.output_buffer_done = output_buffer_done;
    cb.vpp_event = vpp_event;

    if (hyp_vpp_test) {
        hyp_vpp_test->vpp_ctx = vpp_init(0, cb);
        ASSERT_NE(hyp_vpp_test->vpp_ctx, nullptr);

        vpp_term(hyp_vpp_test->vpp_ctx);
    }
}

TEST(HypVPPTest, SetCtrlTest)
{
    std::shared_ptr<HypVPPTest> hyp_vpp_test = std::make_shared<HypVPPTest>();
    struct vpp_callbacks cb;
    vpp_client_ctx *pv = (vpp_client_ctx *)malloc(sizeof(vpp_client_ctx));
    void* ctx = NULL;
    vpp_error ret = VPP_ERR;
    struct hqv_control ctrl;
    enum vpp_port in_port = VPP_PORT_INPUT;
    enum vpp_port out_port = VPP_PORT_OUTPUT;
    struct vpp_port_param in_param;
    struct vpp_port_param out_param;
    bool is_ok = false;

    cb.pv = pv;
    cb.input_buffer_done = input_buffer_done;
    cb.output_buffer_done = output_buffer_done;
    cb.vpp_event = vpp_event;

    ctx = vpp_init(0, cb);
    if (hyp_vpp_test) {
        hyp_vpp_test->vpp_ctx = ctx;
        ASSERT_NE(hyp_vpp_test->vpp_ctx, nullptr);
    }

    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_FRC;
    ctrl.frc.num_segments = 1;
    ctrl.frc.segments = (struct vpp_ctrl_frc_segment *)
                        calloc(ctrl.frc.num_segments, sizeof(struct vpp_ctrl_frc_segment));

    if (ctrl.frc.segments) {
        ctrl.frc.segments[0].mode = HQV_FRC_MODE_SMOOTH_MOTION;
        ctrl.frc.segments[0].level = HQV_FRC_LEVEL_MED;
        ctrl.frc.segments[0].interp = HQV_FRC_INTERP_2X;
        ctrl.frc.segments[0].ts_start = 0;
        ctrl.frc.segments->frame_copy_on_fallback = 0;
        ctrl.frc.segments->frame_copy_input = 0;
        ctrl.frc.segments->smart_fallback = 0;

        vpp_set_ctrl(ctx, ctrl);

        free(ctrl.frc.segments);
    }

    in_param.width = 1280;
    in_param.height = 720;
    in_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, in_param.width);
    in_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, in_param.height);
    in_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    out_param.width = 1280;
    out_param.height = 720;
    out_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, out_param.width);
    out_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, out_param.height);
    out_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    vpp_set_parameter(ctx, in_port, in_param);
    vpp_set_parameter(ctx, out_port, out_param);

    ret = (vpp_error)vpp_open(ctx);
    is_ok = (ret == VPP_OK);
    ASSERT_NE (is_ok, false);

    ret = (vpp_error)vpp_close(ctx);
    is_ok = (ret == VPP_OK);
    ASSERT_NE (is_ok, false);

    vpp_term(ctx);

    free(pv);
}

TEST(HypVPPTest, GetBufReqTest)
{
    std::shared_ptr<HypVPPTest> hyp_vpp_test = std::make_shared<HypVPPTest>();
    struct vpp_callbacks cb;
    vpp_client_ctx *pv = (vpp_client_ctx *)malloc(sizeof(vpp_client_ctx));
    void* ctx = NULL;
    vpp_error ret = VPP_ERR;
    struct hqv_control ctrl;
    enum vpp_port in_port = VPP_PORT_INPUT;
    enum vpp_port out_port = VPP_PORT_OUTPUT;
    struct vpp_port_param in_param;
    struct vpp_port_param out_param;
    struct vpp_requirements req;

    cb.pv = pv;
    cb.input_buffer_done = input_buffer_done;
    cb.output_buffer_done = output_buffer_done;
    cb.vpp_event = vpp_event;

    ctx = vpp_init(0, cb);
    if (hyp_vpp_test) {
        hyp_vpp_test->vpp_ctx = ctx;
        ASSERT_NE(hyp_vpp_test->vpp_ctx, nullptr);
    }

    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_FRC;
    ctrl.frc.num_segments = 1;
    ctrl.frc.segments = (struct vpp_ctrl_frc_segment *)
                        calloc(ctrl.frc.num_segments, sizeof(struct vpp_ctrl_frc_segment));

    if (ctrl.frc.segments) {
        ctrl.frc.segments[0].mode = HQV_FRC_MODE_SMOOTH_MOTION;
        ctrl.frc.segments[0].level = HQV_FRC_LEVEL_MED;
        ctrl.frc.segments[0].interp = HQV_FRC_INTERP_2X;
        ctrl.frc.segments[0].ts_start = 0;
        ctrl.frc.segments->frame_copy_on_fallback = 0;
        ctrl.frc.segments->frame_copy_input = 1;
        ctrl.frc.segments->smart_fallback = 0;

        vpp_set_ctrl(ctx, ctrl);

        free(ctrl.frc.segments);
    }

    in_param.width = 1280;
    in_param.height = 720;
    in_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, in_param.width);
    in_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, in_param.height);
    in_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    out_param.width = 1280;
    out_param.height = 720;
    out_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, out_param.width);
    out_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, out_param.height);
    out_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    vpp_set_parameter(ctx, in_port, in_param);
    vpp_set_parameter(ctx, out_port, out_param);

    ret = (vpp_error)vpp_get_buf_requirements(ctx, &req);
    bool is_ok = (ret == VPP_OK);
    ASSERT_NE (is_ok, false);
    print_vpp_requirements(&req);

    int in_req_cnt = req.buf_req[VPP_RESOLUTION_HD].in_req;
    int out_req_cnt = req.buf_req[VPP_RESOLUTION_HD].out_req;

    DEBUG_INFO("get buf req: in:%d out:%d", in_req_cnt, out_req_cnt);

    vpp_term(ctx);

    free(pv);
}

TEST(HypVPPTest, FRCTest)
{
    struct vpp_callbacks cb;
    struct vpp_requirements req;
    struct hqv_control ctrl;
    vpp_error ret = VPP_ERR;
    void* ctx = NULL;
    enum vpp_port in_port = VPP_PORT_INPUT;
    enum vpp_port out_port = VPP_PORT_OUTPUT;
    struct vpp_port_param in_param;
    struct vpp_port_param out_param;
    main_loop loop;
    FileOp in_file("720p.ubwc", "r");
    FileOp out_file("720p-frc.ubwc", "w+");
    bool is_ok = false;

#ifdef _LINUX_
    std::shared_ptr<Allocator> allocator = std::make_shared<GbmAllocator>();
#else
    std::shared_ptr<Allocator> allocator = std::make_shared<DmaAllocator>();
#endif
    vpp_client_ctx client_ctx;
    client_ctx.in_file = &in_file;
    client_ctx.out_file = &out_file;

    cb.pv = &client_ctx;
    cb.input_buffer_done = input_buffer_done;
    cb.output_buffer_done = output_buffer_done;
    cb.vpp_event = vpp_event;

    ctx = vpp_init(0, cb);
    ASSERT_NE(ctx, nullptr);

    client_ctx.vpp_ctx = ctx;

    std::shared_ptr<input_thread> in_thread =
        std::make_shared<input_thread>("input_buffer_thread", &client_ctx);
    std::shared_ptr<output_thread> out_thread =
        std::make_shared<output_thread>("output_buffer_thread", &client_ctx);

    ctrl.mode = HQV_MODE_MANUAL;
    ctrl.ctrl_type = HQV_CONTROL_FRC;
    ctrl.frc.num_segments = 1;
    ctrl.frc.segments = (struct vpp_ctrl_frc_segment *)
                        calloc(ctrl.frc.num_segments, sizeof(struct vpp_ctrl_frc_segment));

    if (ctrl.frc.segments) {
        ctrl.frc.segments[0].mode = HQV_FRC_MODE_SMOOTH_MOTION;
        ctrl.frc.segments[0].level = HQV_FRC_LEVEL_MED;
        ctrl.frc.segments[0].interp = HQV_FRC_INTERP_2X;
        ctrl.frc.segments[0].ts_start = 0;
        ctrl.frc.segments->frame_copy_on_fallback = 0;
        ctrl.frc.segments->frame_copy_input = 1;
        ctrl.frc.segments->smart_fallback = 0;

        vpp_set_ctrl(ctx, ctrl);

        free(ctrl.frc.segments);
    }

    in_param.width = 1280;
    in_param.height = 720;
    in_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, in_param.width);
    in_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, in_param.height);
    in_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    out_param.width = 1280;
    out_param.height = 720;
    out_param.stride = VENUS_Y_STRIDE(COLOR_FMT_NV12_UBWC, out_param.width);
    out_param.scanlines = VENUS_Y_SCANLINES(COLOR_FMT_NV12_UBWC, out_param.height);
    out_param.fmt = VPP_COLOR_FORMAT_UBWC_NV12;

    vpp_set_parameter(ctx, in_port, in_param);
    vpp_set_parameter(ctx, out_port, out_param);

    DmaBuffer in_buf = {.fd = 0,
                        .handle = 0,
                        .ptr = NULL,
                        .size = 0,
                        .format = COLOR_FMT_NV12_UBWC,
                        .width = 1280,
                        .height = 720,
                        .ubwc_flags = true,
                        .priv = NULL};
    DmaBuffer out_buf = {.fd = 0,
                         .handle = 0,
                         .ptr = NULL,
                         .size = 0,
                         .format = COLOR_FMT_NV12_UBWC,
                         .width = 1280,
                         .height = 720,
                         .ubwc_flags = true,
                         .priv = NULL};
    vpp_buffer vpp_in_buf;
    vpp_buffer vpp_out_buf;

    vpp_get_buf_requirements(ctx, &req);
    print_vpp_requirements(&req);

    ret = (vpp_error)vpp_open(ctx);
    is_ok = (ret == VPP_OK);
    ASSERT_NE (is_ok, false);

    int in_req_cnt = req.buf_req[VPP_RESOLUTION_HD].in_req;
    int out_req_cnt = req.buf_req[VPP_RESOLUTION_HD].out_req;
    DEBUG_LOW("buf req: in:%d out:%d", in_req_cnt, out_req_cnt);

    for (int i=0; i<in_req_cnt; i++) {
        memset(&vpp_in_buf, 0, sizeof(vpp_buffer));
        if (allocator) {
            allocator->allocateBuffer(&in_buf);
            fill_vppbuf_with_dmabuf(&vpp_in_buf, &in_buf);
            {
                std::unique_lock<std::mutex> listLock(client_ctx.in_queue_mutex);
                client_ctx.in_queue.push(vpp_in_buf);
                DEBUG_LOW("add input buffer to queue");
                print_vpp_buffer(&vpp_in_buf);
            }
        }
    }

    for (int i=0; i<out_req_cnt; i++) {
        memset(&vpp_out_buf, 0, sizeof(vpp_buffer));
        if (allocator) {
            allocator->allocateBuffer(&out_buf);
            fill_vppbuf_with_dmabuf(&vpp_out_buf, &out_buf);
            {
                std::unique_lock<std::mutex> listLock(client_ctx.out_queue_mutex);
                vpp_queue_buf(ctx, VPP_PORT_OUTPUT, &vpp_out_buf);
                DEBUG_LOW("queue output buffer");
            }
        }
    }

    in_thread->startThread();
    out_thread->startThread();

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

    while (!client_ctx.in_queue.empty()) {
        vpp_buffer buf;
        DmaBuffer dma_buf;
        memset(&buf, 0, sizeof(vpp_buffer));
        memset(&dma_buf, 0, sizeof(DmaBuffer));

        buf = client_ctx.in_queue.front();
        fill_dmabuf_with_vppbuf(&dma_buf, &buf);
        client_ctx.in_queue.pop();
        if (allocator) {
            allocator->freeBuffer(&dma_buf);
            DEBUG_LOW("free input buffer");
        }
    }

    while (!client_ctx.out_queue.empty()) {
        vpp_buffer buf;
        DmaBuffer dma_buf;
        memset(&buf, 0, sizeof(vpp_buffer));
        memset(&dma_buf, 0, sizeof(DmaBuffer));

        buf = client_ctx.out_queue.front();
        fill_dmabuf_with_vppbuf(&dma_buf, &buf);
        client_ctx.out_queue.pop();
        if (allocator) {
            allocator->freeBuffer(&dma_buf);
            DEBUG_LOW("free output buffer");
        }
    }

    ret = (vpp_error)vpp_close(ctx);
    is_ok = (ret == VPP_OK);
    ASSERT_NE (is_ok, false);

    vpp_term(ctx);

    in_thread->joinThread();
    out_thread->joinThread();
}

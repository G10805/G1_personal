/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#include "V4L2Wrapper.h"

#include <fcntl.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <error.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "ais_log.h"
#include "qcarcam_types.h"

namespace ais_v4l2_test
{
static uint32 toV4L2Format(uint32 fmt)
{
    uint32 v4l2_fmt = 0;
    switch(fmt)
    {
    case QCARCAM_YUV_UYVY:
        v4l2_fmt = V4L2_PIX_FMT_UYVY ;
        CAM_MSG(HIGH,"fmt %u %u\n", V4L2_PIX_FMT_BGR32, V4L2_PIX_FMT_UYVY);
        break;
    default:
        CAM_MSG(ERROR,"can't support this format %u", fmt);
        break;
    }

    return v4l2_fmt;
}

V4L2Wrapper::V4L2Wrapper(const string& devpath, IO_MODE io_mode)
    : m_strDevPath(devpath)
    , m_fd(-1)
    , m_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    , m_io_mode(io_mode)
{
    switch(io_mode)
    {
    case IO_MODE_MMAP:
        m_mem_type = V4L2_MEMORY_MMAP;
        break;
    case IO_MODE_DMABUF:
        m_mem_type = V4L2_MEMORY_MMAP;
        break;
    default:
        CAM_MSG(ERROR,"Unsupported io_mode");
        // this value is meaningless, just in case m_mem_type is not initialized.
        m_mem_type = V4L2_MEMORY_MMAP;
        break;

    }
}

V4L2Wrapper::~V4L2Wrapper()
{
}

int V4L2Wrapper::connect(bool block)
{
    if (m_fd < 0)
    {
        if (block)
            m_fd = open(m_strDevPath.c_str(), O_RDWR);
        else
            m_fd = open(m_strDevPath.c_str(), O_RDWR | O_NONBLOCK );

        if (m_fd < 0)
        {
            CAM_MSG(ERROR, "Failed to open device %s  err= %s", m_strDevPath.c_str(), strerror(-errno));
            return 1;
        }
    }

    struct v4l2_capability cap;
    int rc = io_ctrl(VIDIOC_QUERYCAP, &cap);
    if (rc)
    {
        CAM_MSG(ERROR, "fail to querycap");
    }
    else
    {
        CAM_MSG(HIGH, "succeed to querycap");
    }

    CAM_MSG(HIGH, "succeed to connect %s fd %d", m_strDevPath.c_str(), m_fd);
    return 0;
}

void V4L2Wrapper::disconnect()
{
    uint32 num = m_bufferVec.size();
    for (uint32 idx =0; idx < num; ++idx)
    {
        if (m_io_mode == IO_MODE_MMAP)
        {
            munmap(m_bufferVec[idx].planes[0].p_buf, m_bufferVec[idx].planes[0].size);
            close(m_bufferVec[idx].planes[0].fd);
        }
        else if (m_io_mode == IO_MODE_MMAP)
        {
            //TODO, check whether need to munmap
        }
    }
    m_bufferVec.clear();

    close(m_fd);
    m_fd = -1;
}

int V4L2Wrapper::streamon()
{
    //Stream ON
    if (io_ctrl (VIDIOC_STREAMON, &m_type) < 0)
    {
        CAM_MSG(ERROR, "VIDIOC_STREAMON returned Error err= %s\n", strerror(-errno));
        return 1;
    }
    else
    {
        CAM_MSG(ERROR, "VIDIOC_STREAMON success\n");
    }
    return 0;
}

int V4L2Wrapper::streamoff()
{

    if (io_ctrl (VIDIOC_STREAMOFF, &m_type) < 0)
    {
        CAM_MSG(ERROR, "VIDIOC_STREAMOFF returned Error \n");
        return 1;
    }
    else
    {
        CAM_MSG(ERROR, "VIDIOC_STREAMOFF success\n");
    }

    return 0;
}

int V4L2Wrapper::request_mmapbuf_bottomhalf(uint32 num)
{
    v4l2_buffer device_buffer;
    int rc = 0;

    memset(&device_buffer, 0, sizeof(device_buffer));

    for (uint32 idx =0;idx < num; ++idx)
    {
        device_buffer.type = m_type;
        device_buffer.memory = m_mem_type;
        device_buffer.index = idx;

        // Use QUERYBUF to ensure our buffer/device is in good shape,
        // and fill out remaining fields.
        rc = io_ctrl(VIDIOC_QUERYBUF, &device_buffer);
        if (rc)
        {
            CAM_MSG(ERROR, "VIDIOC_QUERYBUF returned Error err= %s", strerror(-errno));
            return 1;
        }

        Buffer_t& buf = m_bufferVec[idx];
        memset(&buf, 0, sizeof(buf));
        buf.n_planes = 1;
        buf.planes[0].size = device_buffer.length;

        buf.planes[0].p_buf = mmap(NULL, device_buffer.length,
                                  PROT_READ | PROT_WRITE,
                                  MAP_SHARED,
                                  m_fd, device_buffer.m.offset);
        CAM_MSG(HIGH, "buf info size %u", buf.planes[0].size);
        if(buf.planes[0].p_buf == NULL)
        {
            CAM_MSG(ERROR, "mmap failed err= %s offset %x", strerror(-errno), device_buffer.m.offset);
            return 1;
        }
    }

    return 0;
}

int V4L2Wrapper::request_dmabuf_bottomhalf(uint32 num, qcarcam_buffers_t *buffers)
{
    v4l2_buffer device_buffer;
    int rc = 0;
    struct v4l2_exportbuffer expbuf = { 0 };

    memset(&device_buffer, 0, sizeof(device_buffer));

    for (uint32 idx =0;idx < num; ++idx)
    {
        device_buffer.type = m_type;
        device_buffer.memory = m_mem_type;
        device_buffer.index = idx;

        // Use QUERYBUF to ensure our buffer/device is in good shape,
        // and fill out remaining fields.
        rc = io_ctrl(VIDIOC_QUERYBUF, &device_buffer);
        if (rc)
        {
            CAM_MSG(ERROR, "VIDIOC_QUERYBUF returned Error err= %s", strerror(-errno));
            return 1;
        }

        expbuf.type = m_type;
        expbuf.index = idx;
        expbuf.plane = 1;
        expbuf.flags = O_CLOEXEC | O_RDWR;

        if (io_ctrl(VIDIOC_EXPBUF, &expbuf) < 0)
        {
            CAM_MSG(ERROR, "EXPBUF failed err= %s index %x", strerror(-errno), idx);
            return 1;
        }

        Buffer_t& buf = m_bufferVec[idx];
        memset(&buf, 0, sizeof(buf));
        if (buffers != NULL)
        {
            buffers->buffers[idx].planes[0].p_buf = (void *)(uintptr_t)expbuf.fd;
        }
        else
        {
            CAM_MSG(ERROR, "invalid buffer address");
            return 1;
        }

        buf.planes[0].fd = expbuf.fd;
        enqbuf(NULL, idx);
        CAM_MSG(HIGH, "[device %s] qbuf %d fd %d", m_strDevPath.c_str(), idx,
            expbuf.fd);
    }

    return 0;
}

int V4L2Wrapper::requestbuf(uint32 num, qcarcam_buffers_t *buffers, bool use_buf_width)
{
    v4l2_requestbuffers req_buffers;
    memset(&req_buffers, 0, sizeof(req_buffers));
    req_buffers.type = m_type;
    req_buffers.memory = m_mem_type;
    req_buffers.count = num;
    req_buffers.reserved[0] = use_buf_width;

    int rc = io_ctrl(VIDIOC_REQBUFS, &req_buffers);
    if (rc)
    {
        CAM_MSG(ERROR, "VIDIOC_REQBUFS returned Error err= %s", strerror(-errno));
        return 1;
    }
    else
    {
        CAM_MSG(HIGH, "VIDIOC_REQBUFS succeed");
    }

    if (num > 0)
    {
        m_bufferVec.resize(num);

        switch(m_io_mode)
        {
        case IO_MODE_MMAP:
            rc = request_mmapbuf_bottomhalf(num);
            break;
        case IO_MODE_DMABUF:
            rc = request_dmabuf_bottomhalf(num, buffers);
            break;
        default:
            CAM_MSG(ERROR, "Unsupported m_io_mode");
            break;
        }
    }

    return rc;
}

int V4L2Wrapper::releasebufs()
{
    requestbuf(0, NULL);
    for (uint32 i = 0; i < m_bufferVec.size(); ++i)
    {
        close(m_bufferVec[i].planes[0].fd);
    }

    m_bufferVec.clear();

    return 0;
}

int V4L2Wrapper::deqbuf(uint32 &idx, BufferDesc* desc, struct ais_v4l2_buffer_ext_t* ext)
{
    struct v4l2_buffer capture_buf;
    int rc = 0;

    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type = m_type;
    capture_buf.memory = m_mem_type;

    if (ext)
    {
        uint64* p_reserved = (uint64*)&capture_buf.reserved2;
        *p_reserved = (uint64)ext;
    }

    rc =  io_ctrl(VIDIOC_DQBUF, &capture_buf);
    if (-EAGAIN == rc)
    {
        CAM_MSG(ERROR, "VIDIOC_DQBUF returned Error err= %s", strerror(-errno));
        return -1;
    }
    else if (rc < 0)
    {
        CAM_MSG(HIGH, "VIDIOC_DQBUF again");
        return -1;
    }

    idx = capture_buf.index;
    desc->_ptr = m_bufferVec[idx].planes[0].p_buf;
    // The v4l2 only use 1 plane, even for the NV12 format
    desc->_len = m_bufferVec[idx].planes[0].size;
    desc->_seq = capture_buf.sequence;
    desc->_timestamp = capture_buf.timestamp;
    return 0;
}

int V4L2Wrapper::enqbuf(const BufferDesc* desc, uint32 idx)
{
    v4l2_buffer device_buffer;
    memset(&device_buffer, 0, sizeof(device_buffer));
    device_buffer.type = m_type;
    device_buffer.memory = m_mem_type;
    device_buffer.index = idx;

    if(io_ctrl(VIDIOC_QBUF, &device_buffer) < 0)
    {
        CAM_MSG(ERROR, "VIDIOC_QBUF returned Error err= %s\n", strerror(-errno));
        return 1;
    }

    return 0;
}

int V4L2Wrapper::setformat(const StreamFormat& format)
{
    int rc = 0;
    struct v4l2_format v;
    v.fmt.pix.width = format._uiw;
    v.fmt.pix.height = format._uih;
    v.fmt.pix.pixelformat = toV4L2Format(format._format);
    v.fmt.pix.sizeimage = format._size; //Set size that we get from the camera init
    rc = io_ctrl(VIDIOC_S_FMT, &v);
    if (rc < 0)
    {
        CAM_MSG(ERROR, "Failed to send IOCTL open VIDIOC_S_FMT  err= %s",strerror(-errno));
        return 1;
    }
    return 0;
}

int V4L2Wrapper::getformats()
{
    struct v4l2_fmtdesc v;

    memset(&v, 0, sizeof(v));
    v.type = m_type;
    v.index = 0;

    CAM_MSG(HIGH, "get formats" );

    while(io_ctrl(VIDIOC_ENUM_FMT, &v)>=0)
    {
        v.index++;
        m_supportedformats.push_back(v.pixelformat);
        CAM_MSG(HIGH, "format %u", v.pixelformat);
    }

    if (errno != EINVAL)
    {
        CAM_MSG(ERROR, "ENUM_FMT fails at index %d: %s", v.index, strerror(errno));
        return 1;
    }
    return 0;
}

int V4L2Wrapper::getformat(StreamFormat& fmt)
{
    int rc = 0;
    struct v4l2_format v;
    memset(&v, 0, sizeof(v));
    v.type = m_type;

    rc = io_ctrl(VIDIOC_G_FMT, &v);
    if (rc < 0)
    {
        CAM_MSG(ERROR, "Failed to send IOCTL open VIDIOC_G_FMT  err= %s", strerror(-errno));
        return 1;
    }

    fmt._format = v.fmt.pix.pixelformat;
    fmt._uiw = v.fmt.pix.width;
    fmt._uih = v.fmt.pix.height;
    fmt._bytesperline = v.fmt.pix.bytesperline;

    CAM_MSG(HIGH, "w %u h %u bpl %u format 0x%x", fmt._uiw, fmt._uih, fmt._bytesperline, fmt._format);

    return 0;
}

qcarcam_color_fmt_t V4L2Wrapper::toQcarcamFormat(unsigned int fmt)
{
    qcarcam_color_fmt_t q_fmt = QCARCAM_FMT_UYVY_8;
    switch(fmt)
    {
        case V4L2_PIX_FMT_UYVY:
            q_fmt = QCARCAM_FMT_UYVY_8;
            break;
        case V4L2_PIX_FMT_NV12:
            q_fmt = QCARCAM_FMT_NV12;
            break;
        case V4L2_PIX_FMT_YUV420:
            q_fmt = QCARCAM_FMT_UYVY_12;
            break;
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_BGR24:
            q_fmt = QCARCAM_FMT_RGB_888;
            break;
        default:
            break;
    }

    return q_fmt;
}

int V4L2Wrapper::dump(uint32 buf_idx, const string& file)
{
    FILE *fp;
    size_t numBytesWritten = 0;
    size_t numByteToWrite = 0;
    unsigned char *buf_ptr = NULL;

    numByteToWrite = m_bufferVec[buf_idx].planes[0].size;

    fp = fopen(file.c_str(), "w+b");
    CAM_MSG(ERROR, "dumping qcarcam frame %s numByteToWrite : %u ", file.c_str(), numByteToWrite);

    if (0 != fp)
    {
        buf_ptr = (unsigned char*)m_bufferVec[buf_idx].planes[0].p_buf;
       // dcache_flush_dma(buf_ptr, numByteToWrite);
        numBytesWritten = fwrite((void *)buf_ptr, sizeof(unsigned char), numByteToWrite, fp);
        fclose(fp);

        if (numBytesWritten != numByteToWrite )
        {
             CAM_MSG(ERROR, "numByteToWrite %lu, numBytesWritten %lu, failed to write file", numByteToWrite, numBytesWritten);
             return 1;
        }
    }
    else
    {
        CAM_MSG(ERROR, "failed to open file");
        return 1;
    }

    return 0;
}

int V4L2Wrapper::io_ctrl(uint32 type, void* pData)
{
    return ioctl(m_fd, type, pData);
}

}

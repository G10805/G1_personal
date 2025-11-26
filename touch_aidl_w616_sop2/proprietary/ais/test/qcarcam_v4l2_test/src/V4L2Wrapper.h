/* ===========================================================================
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 =========================================================================== */

#ifndef _AIS_V4L2_WRAPPER_H
#define _AIS_V4L2_WRAPPER_H

#include <string>
#include <vector>
#include <string.h>
#include "qcarcam_types.h"
#include "AEEStdDef.h"
#include <linux/videodev2.h>
#include <media/ais_v4l2loopback.h>

using std::string;
using std::vector;

namespace ais_v4l2_test
{
class V4L2Wrapper
{
public:
    enum IO_MODE
    {
        IO_MODE_MMAP = 0,
        IO_MODE_USERPTR,
        IO_MODE_DMABUF,
    };

    struct StreamFormat
    {
        uint32 _format;
        uint32 _uiw;
        uint32 _uih;
        uint32 _size;
        uint32 _bytesperline;
    };

    struct BufferDesc
    {
        int _fd;
        void* _ptr;
        uint32 _len;
        uint32 _seq;
        struct timeval _timestamp;
    };

    enum MEM_TYPE
    {
        MEM_MMAP,
        MEM_USERPTR,
        MEM_DMABUF,
    };

    /// @brief Buffer plane definition
    struct Plane_t
    {
        unsigned int width;  ///< width in pixels
        unsigned int height; ///< height in pixels
        unsigned int stride; ///< stride in bytes
        unsigned int size;   ///< size in bytes
        void*        p_buf;
        int             fd;
    };

    /// @brief Buffer definition
    struct Buffer_t
    {
        Plane_t planes[QCARCAM_MAX_NUM_PLANES];
        unsigned int n_planes;
    } ;

public:
    V4L2Wrapper(const string& devpath, IO_MODE io_mode);
    ~V4L2Wrapper();

    int connect(bool block);
    void disconnect();
    int streamon();
    int streamoff();
    int requestbuf(uint32 num, qcarcam_buffers_t *buffers, bool use_buf_width=false);
    int releasebufs();
    int deqbuf(uint32 &idx, BufferDesc* desc, struct ais_v4l2_buffer_ext_t* ext=NULL);
    int enqbuf(const BufferDesc* desc, uint32 idx);
    int setformat(const StreamFormat&);
    int getformats();
    int getformat(StreamFormat& fmt);
    int dump(uint32 buf_idx, const string& file);
    int getfd() { return m_fd; }
    static qcarcam_color_fmt_t toQcarcamFormat(unsigned int fmt);

private:
    int io_ctrl(uint32 type, void* pData);
    int    request_mmapbuf_bottomhalf(uint32 num);
    int    request_dmabuf_bottomhalf(uint32 num, qcarcam_buffers_t *buffers);

    // The camera device path. For example, /dev/video0.
    const string m_strDevPath;
    // The opened device fd.
    int m_fd;

    vector<Buffer_t>    m_bufferVec;
    vector<uint32>    m_supportedformats;
    uint32 m_type;
    IO_MODE m_io_mode;
    enum v4l2_memory m_mem_type;
};

}

#endif
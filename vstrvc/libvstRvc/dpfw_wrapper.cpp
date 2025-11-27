#include<dpfw_wrapper.h>
#include<videodev2.h>
#include <DpAsyncBlitStream.h>

static DpAsyncBlitStream *m_mdp_stream;

int dpfw_mdp_create(void)
{
    if (!m_mdp_stream) {
        m_mdp_stream = new DpAsyncBlitStream();
        if (m_mdp_stream == NULL) {
            printf("create MDP stream fail");
            return -1;
        }
    }
    return 0;
}


int dpfw_mdp_delete(void)
{
    if (!m_mdp_stream) {
        delete m_mdp_stream;
        m_mdp_stream = NULL;
    }
    return 0;
}

DpColorFormat v4l2ToDpformat(unsigned int format)
{
    switch (format) {
    case V4L2_PIX_FMT_YUYV:
        return DP_COLOR_YUYV;
    case V4L2_PIX_FMT_BGR24:
        return DP_COLOR_BGR888;
    case V4L2_PIX_FMT_RGB24:
        return DP_COLOR_RGB888;
    case V4L2_PIX_FMT_ARGB32:
        return DP_COLOR_ARGB8888;
    case V4L2_PIX_FMT_ABGR32:
        return DP_COLOR_ABGR8888;
    case V4L2_PIX_FMT_NV12_HYFBC:
        return DP_COLOR_NV12_HYFBC;
    default:
        printf("v4l2ToDpformat, unknown format = %u", format);
        return DP_COLOR_UNKNOWN;
    }
}

int getBpp(DpColorFormat format)
{
    switch (format) {
    case DP_COLOR_YUYV:
        return 2;
    case DP_COLOR_BGR888:
    case DP_COLOR_RGB888:
        return 3;
    case DP_COLOR_ARGB8888:
    case DP_COLOR_ABGR8888:
        return 4;
    case DP_COLOR_NV12_HYFBC:
        return 3; // 1.5 need to fix this
    default:
        printf("v4l2ToDpformat, unknown format = %u", format);
        return DP_COLOR_UNKNOWN;
    }
}

int dpfw_mdp_make_frame(void* __attribute__((unused)) in_va, void* __attribute__((unused)) out_va, struct dpfw_mdp_device* mdp_dev)
{
    int waitFence = 0;
    int timeout = 1000;
    int status = 0;
    unsigned int job = 0;
    //void* pVABaseList[3] = {in_va,0,0};
    //void* pMVABaseList[3] = {0};
    //void* pDstVABaseList[3] = {out_va,0,0};
    //void* pDstMVABaseList[3] = {0};
    unsigned int src_width = mdp_dev->src_w;
    unsigned int src_height = mdp_dev->src_h;
    unsigned int dst_width = mdp_dev->dst_w;
    unsigned int dst_height = mdp_dev->dst_h;
    DpColorFormat src_fmt = v4l2ToDpformat(mdp_dev->src_fmt);
    DpColorFormat dst_fmt = v4l2ToDpformat(mdp_dev->dst_fmt);
    unsigned int srcSizeList[3] = {0};
    unsigned int dstSizeList[3] = {0};
    int in_planeNumber = DP_COLOR_GET_PLANE_COUNT(src_fmt);
    int out_planeNumber = DP_COLOR_GET_PLANE_COUNT(dst_fmt);

    if (src_fmt == DP_COLOR_NV12_HYFBC)
        srcSizeList[0] = src_width * src_height * getBpp(src_fmt) / 2;
    else
        srcSizeList[0] = src_width * src_height * getBpp(src_fmt);

    if (dst_fmt == DP_COLOR_NV12_HYFBC)
        dstSizeList[0] = dst_width * dst_height * getBpp(dst_fmt) / 2;
    else
        dstSizeList[0] = dst_width * dst_height * getBpp(dst_fmt);




    // buf add convert va to pa
    //if (in_va != NULL)
        //pMVABaseList[0] = m_mdp_stream->VaToPa(in_va);
    //else
        //printf("dpfw_mdp_make_frame: in_va = NULL\n");

    //if (out_va != NULL)
        //pDstMVABaseList[0] = m_mdp_stream->VaToPa(out_va);
    //else
        //printf("dpfw_mdp_make_frame: out_va = NULL\n");

    //printf("dpfw_mdp_make_frame:  in pa=0x%x out pa=0x%x\n", pMVABaseList[0], pDstMVABaseList[0]);

    // trigger mdp
    m_mdp_stream->createJob(job, waitFence);
    m_mdp_stream->setConfigBegin(job);

    // source buffer config
    // for NV12_Hyfbc, 1 plane
    //pMVABaseList[0] = mdp_in_addr.mva;
    //pVABaseList[0] = in_va;
    //m_mdp_stream->setSrcBuffer(pVABaseList, pMVABaseList, srcSizeList, 1, -1);
    m_mdp_stream->setSrcBuffer(mdp_dev->src_fd, srcSizeList, in_planeNumber);

    //m_mdp_stream->setSrcConfig(src_width, src_height, DP_COLOR_NV12_HYFBC, eInterlace_None);
    m_mdp_stream->setSrcConfig(src_width, src_height, src_fmt, eInterlace_None);

    // dst buffer config
    // for YUYV, only 1 plane
    //pDstMVABaseList[0] = mdp_out_addr.mva;
    //pDstVABaseList[0] = out_va;
    //m_mdp_stream->setDstBuffer(0, pDstVABaseList, pDstMVABaseList, dstSizeList, 1, -1);
    m_mdp_stream->setDstBuffer(0, mdp_dev->dst_fd, dstSizeList, out_planeNumber);

    m_mdp_stream->setDstConfig(0, dst_width, dst_height, dst_fmt);
    m_mdp_stream->setConfigEnd();
    status = m_mdp_stream->invalidate();
    if (status != 0) {
        printf("invalid even async fail:%d\n", status);
        return status;
    }
    // wait for fence release;
    status = sync_wait(waitFence, timeout);
    if (status < 0) {
        printf("wait even fence timeout:%d\n", status);
        return status;
    }
    close(waitFence);
    waitFence = 0;

    return status;
}
/*!
 * @file vpp_fe.cpp
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

 * @services    Implements the VPP library interface
 */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include <stdint.h>
#include "vpp.h"
#include "hyp_vpp.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

int vpp_debug_level = HYP_PRIO_ERROR;

uint32_t vpp_boot()
{
    return HYPVPP_STATUS_SUCCESS;
}

uint32_t vpp_shutdown()
{
    return HYPVPP_STATUS_SUCCESS;
}

void *vpp_init(uint32_t flags, struct vpp_callbacks cb)
{
    size_t pos = 0;
#ifdef _LINUX_
    char *env_ptr = getenv("HYPVPP_DEBUG_LEVEL");
    if (env_ptr) {
        vpp_debug_level = std::stoul(env_ptr, &pos, 16);
    }
#elif defined(_ANDROID_)
    char property_value[PROPERTY_VALUE_MAX] = {0};

    property_get("vendor.hypvpp.debug.level", property_value, "1");
    vpp_debug_level = std::stoul(property_value, &pos, 16);
#endif
    HYP_VPP_MSG_INFO("VPP init flags 0x%x debug level %d", flags, vpp_debug_level);

    return hyp_vpp_init(flags, cb);
}

void vpp_term(void *ctx)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;

    if (!ctx)
    {
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
        status = HYPVPP_STATUS_INVALID_CONFIG;
    }

    if (HYPVPP_STATUS_SUCCESS == status)
    {
        status = hyp_vpp_term(ctx);
        if (HYPVPP_STATUS_SUCCESS != status)
        {
            HYP_VPP_MSG_ERROR("error result:%d", status);
        }
    }
}

uint32_t vpp_open(void *ctx)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx)
    {
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
        ret = VPP_ERR_PARAM;
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_open(ctx);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_close(void *ctx)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx)
    {
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
        ret = VPP_ERR_PARAM;
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_close(ctx);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

static void vpp_dump_control(struct hqv_control ctrl)
{
    HYP_VPP_MSG_INFO("mode=%d, ctrl_type=%d", ctrl.mode, ctrl.ctrl_type);

    switch (ctrl.ctrl_type) {
        case HQV_CONTROL_CADE:
            LOG_CADE(&ctrl.cade);
            break;
        case HQV_CONTROL_DI:
            LOG_DI(&ctrl.di);
            break;
        case HQV_CONTROL_TNR:
            LOG_TNR(&ctrl.tnr);
            break;
        case HQV_CONTROL_CNR:
            LOG_CNR(&ctrl.cnr);
            break;
        case HQV_CONTROL_AIE:
            LOG_AIE(&ctrl.aie);
            break;
        case HQV_CONTROL_FRC:
            LOG_FRC(&ctrl.frc);
            break;
        case HQV_CONTROL_EAR:
            LOG_EAR(&ctrl.ear);
            break;
        case HQV_CONTROL_QBR:
            LOG_QBR(&ctrl.qbr);
            break;
        case HQV_CONTROL_AIS:
            LOG_AIS(&ctrl.ais);
            break;
        default:
            break;
    }
}

uint32_t vpp_set_ctrl(void *ctx, struct hqv_control ctrl)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    vpp_dump_control(ctrl);

    if (!ctx || (HQV_MODE_MAX <= ctrl.mode || HQV_MODE_OFF > ctrl.mode))
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p mode:%d", ctx, ctrl.mode);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_set_ctrl(ctx, ctrl);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_set_parameter(void *ctx, enum vpp_port port, struct vpp_port_param param)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx || (VPP_PORT_INPUT != port && VPP_PORT_OUTPUT != port))
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p port:%d", ctx, port);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_set_param(ctx, port, param);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_get_buf_requirements(void *ctx, struct vpp_requirements *req)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx || !req)
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p req:%p", ctx, req);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_get_buf_requirements(ctx, req);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_queue_buf(void *ctx, enum vpp_port port, struct vpp_buffer *buf)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    HYP_VPP_MSG_INFO("port=%d, buf=%p", port, buf);

    if (!ctx || !buf || (VPP_PORT_INPUT != port && VPP_PORT_OUTPUT != port))
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter cxt:%p port:%d buf=%p", ctx, port, buf);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_queue_buf(ctx, port, buf);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_reconfigure(void *ctx,
                         struct vpp_port_param input_param,
                         struct vpp_port_param output_param)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx)
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_reconfigure(ctx, input_param, output_param);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_flush(void *ctx, enum vpp_port port)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx || (VPP_PORT_INPUT != port && VPP_PORT_OUTPUT != port))
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p port:%d", ctx, port);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_flush(ctx, port);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_drain(void *ctx)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx)
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_drain(ctx);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

uint32_t vpp_set_vid_prop(void *ctx, struct video_property prop)
{
    hypvpp_status_type status = HYPVPP_STATUS_SUCCESS;
    vpp_error ret = VPP_OK;

    if (!ctx)
    {
        ret = VPP_ERR_PARAM;
        HYP_VPP_MSG_ERROR("error input parameter ctx:%p", ctx);
    }

    if (ret == VPP_OK)
    {
        status = hyp_vpp_set_vid_prop(ctx, prop);
        ret = hypvpp_status_to_vpp_error(status);
    }

    return ret;
}

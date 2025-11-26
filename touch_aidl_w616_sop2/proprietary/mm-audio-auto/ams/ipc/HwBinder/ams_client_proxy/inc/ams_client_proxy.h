/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
    int32_t ams_core_init(void);
    uint32_t ams_core_open(void);
    int32_t ams_core_ioctl(uint32_t handle, uint32_t cmd, void *params, uint32_t size);
    int32_t ams_core_close(uint32_t hndl);
#ifdef __cplusplus
}
#endif
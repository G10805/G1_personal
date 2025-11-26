/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef LOC_DIAG_IFACE_API_H
#define LOC_DIAG_IFACE_API_H

#include <stdint.h>
#include <loc_cfg.h>
#include <loc_misc_utils.h>

/**This is to indicate if the buffer is allocated from Diag or malloc.
* This is needed for cases where Diag service is not up
*/
typedef enum {
   BUFFER_INVALID = 0,
   BUFFER_FROM_MALLOC, /**The diag buffer is allocated from heap using malloc.
                        * Used for diag messages that are generated prio to DIag
                        * susbsystem being initialised*/
   BUFFER_FROM_DIAG    /**The diag buffer is allocated from diag heap using log_alloc.*/
} diagBuffSrc;

struct LocDiagInterface {
    size_t size;
    void* (*logAlloc)(uint32_t diagId, size_t size, diagBuffSrc *bufferSrc);
    void (*logCommit)(void *pData, diagBuffSrc bufferSrc, uint32_t diagId, size_t size);
};

// Entry point to the library
typedef const LocDiagInterface* (getLocDiagIfaceFunc)();

static inline LocDiagInterface* loadLocDiagIfaceInterface() {
    static LocDiagInterface *diagIface = nullptr;

    if (nullptr == diagIface) {
        int loadDiagIfaceLib = 1;
        const loc_param_s_type gps_conf_params[] = {
            {"LOC_DIAGIFACE_ENABLED", &loadDiagIfaceLib, nullptr, 'n'}
        };
        UTIL_READ_CONF(LOC_PATH_GPS_CONF, gps_conf_params);
        if (0 != loadDiagIfaceLib) {
            void* libHandle = nullptr;
            getLocDiagIfaceFunc* getter = (getLocDiagIfaceFunc*)dlGetSymFromLib(
                    libHandle, "liblocdiagiface.so", "getLocDiagIface");
            if (nullptr != getter) {
                diagIface = (LocDiagInterface*)(*getter)();
            }
        }
    }
    return diagIface;
}

#endif /**LOC_DIAG_IFACE_API_H*/

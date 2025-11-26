/* ===========================================================================
 * Copyright (c) 2019-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 * =========================================================================== */

#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <hidl/HidlSupport.h>
#include "ais_hidl_camera.h"
#include <cutils/properties.h>
#include "qcarcam.h"

using vendor::qti::automotive::qcarcam::V1_1::IQcarCamera;
using namespace vendor::qti::automotive::qcarcam::V1_0::implementation;
using namespace android;

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

static pthread_mutex_t mutex_abort = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_abort = PTHREAD_COND_INITIALIZER;

#ifdef QCC_SOC_OVERRIDE

static bool isMakena()
{
    int     socFd = 0;
    char    buf[32]  = { 0 };
    int     chipsetVersion = -1;
    char    fileName[] = "/sys/devices/soc0/soc_id";

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        int ret = read(socFd, buf, sizeof(buf) - 1);

        if (-1 == ret)
            return false;
        else
            chipsetVersion = atoi(buf);

        close(socFd);

        if (chipsetVersion == 460) //CHIP_ID_SA8295P = 460
            return true;
    }

    return false;
}

#endif

int main() {
    ALOGE("AIS HIDL service is starting");

    char value[92];
    int QCC_FLAG_ENABLED = 0;
    property_get("ro.boot.camera.qcc.version", value, "");
    ALOGE("value of property for qcc: %s", value);
    QCC_FLAG_ENABLED = (strlen(value) > 0) ? 1:0;

    if (QCC_FLAG_ENABLED) {
        if ((value[0] - '0') != QCARCAM_VERSION_MAJOR) {
            ALOGE("Blocking qcarcam service for QCC.4.0");
            pthread_cond_wait(&cond_abort, &mutex_abort);
            return 1;
        }
    }
    else {
#ifdef QCC_SOC_OVERRIDE
    if (isMakena()) {
        ALOGE("Blocking qcarcam service for QCC.4.0");
        pthread_cond_wait(&cond_abort, &mutex_abort);
        return 1;
    }
#endif
    }
    ALOGE("AIS HIDL service (QCC4.0) is started...");

    android::sp<IQcarCamera> service = new ais_hidl_camera();

    /*Once ais_server shuts down socket with parameter SHUT_RDWR, if hidl server sends any msg to it,
    * an EPIPE error is returned and a SIGPIPE signal is triggered causing process to exit. So, setting SIGPIPE signal
    * to be ignored */
    signal(SIGPIPE, SIG_IGN);

    configureRpcThreadpool(1, true /* callerWillJoin */);

    const status_t status = service->registerAsService();
    LOG_ALWAYS_FATAL_IF(status != OK, "registerAsService() error: status=%d", status);
    if (status == OK)
    {
        ALOGD("AIS HIDL service service is ready.");
        joinRpcThreadpool();
    } else {
        ALOGE("Could not register AIS HIDL service (%d).", status);
    }

    // In normal operation, we don't expect the thread pool to exit
    ALOGE("AIS HIDL service is shutting down");
    return -1;
}

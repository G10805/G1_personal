/*
 * Copyright (c) 2020-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
 *
 * Copyright (C) 2016-2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <atomic>

#include <hidl/HidlTransportSupport.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>
#include <utils/Log.h>
#include <cutils/properties.h>

#include "ServiceNames.h"
#include "EvsEnumerator.h"
#include "EvsGlDisplay.h"
#include "qcarcam.h"

// libhidl:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

// Generated HIDL files
using android::hardware::automotive::evs::V1_1::IEvsEnumerator;
using android::hardware::automotive::evs::V1_1::IEvsDisplay;
using android::frameworks::automotive::display::V1_0::IAutomotiveDisplayProxyService;

// The namespace in which all our implementation code lives
using namespace android::hardware::automotive::evs::V1_1::implementation;
using namespace android;

/*abort condition*/
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
    LOG(INFO) << "EVS Hardware Enumerator service is starting";

    char value[92];
    int QCC_FLAG_ENABLED = 0;
    property_get("ro.boot.camera.qcc.version", value, "");
    LOG(ERROR) << "value of property for qcc:" << value;
    QCC_FLAG_ENABLED = (strlen(value) > 0) ? 1:0;

    if (QCC_FLAG_ENABLED) {
        if ((value[0] - '0') != QCARCAM_VERSION_MAJOR) {
            LOG(ERROR) << "Blocking EVS service for QCC.6.0";
            pthread_cond_wait(&cond_abort, &mutex_abort);
            return 1;
        }
    }
    else {
#ifdef QCC_SOC_OVERRIDE
    if (!isMakena()) {
        LOG(ERROR) << "Blocking EVS service for QCC.6.0";
        pthread_cond_wait(&cond_abort, &mutex_abort);
        return 1;
    }
#endif
    }
    LOG(ERROR) << "EVS service (QCC.6.0) started...";

    android::sp<IAutomotiveDisplayProxyService> carWindowService =
        IAutomotiveDisplayProxyService::getService("default");
    if (carWindowService == nullptr) {
        LOG(ERROR) << "Cannot use AutomotiveDisplayProxyService.  Exiting.";
        return 1;
    }

#ifdef EVS_DEBUG
    SetMinimumLogSeverity(android::base::DEBUG);
#endif


    android::sp<IEvsEnumerator> service = new EvsEnumerator(carWindowService);

    // Once socket is shutdown with parameter SHUT_RDWR, if evs sends any msg to it,
    // an EPIPE error is returned and a SIGPIPE signal is triggered causing process to exit. So, setting SIGPIPE signal
    // to be ignored
    signal(SIGPIPE, SIG_IGN);

    configureRpcThreadpool(1, true /* callerWillJoin */);

    // Register our service -- if somebody is already registered by our name,
    // they will be killed (their thread pool will throw an exception).
    status_t status = service->registerAsService(kEnumeratorServiceName);
    if (status == OK) {
        LOG(DEBUG) << kEnumeratorServiceName << " is ready.";
        joinRpcThreadpool();
    } else {
        LOG(ERROR) << "Could not register service " << kEnumeratorServiceName
                   << " (" << status << ").";
    }

    // In normal operation, we don't expect the thread pool to exit
    LOG(ERROR) << "EVS Hardware Enumerator is shutting down";
    return 1;
}

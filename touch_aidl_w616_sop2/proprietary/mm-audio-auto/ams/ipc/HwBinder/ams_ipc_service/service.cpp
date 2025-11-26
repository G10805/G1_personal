/**
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define LOG_TAG "vendor.qti.hardware.AMSIPC@1.0-service"
#include <log/log.h>
#include <hidl/LegacySupport.h>
#include "AMS.h"

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::defaultPassthroughServiceImplementation;
using android::hardware::joinRpcThreadpool;
using vendor::qti::hardware::AMSIPC::implementation::AMS;
using vendor::qti::hardware::AMSIPC::V1_0::IAMS;

#ifdef PLATFORM_MSMNILE_AU
static void place_marker(char const *name)
{
   int fd=open("/sys/kernel/boot_kpi/kpi_values", O_WRONLY);
   if (fd > 0)
   {
       /* Only allow marker text shorter than MARKER_STRING_WIDTH */
       char earlyapp[100] = {0};
       strlcpy(earlyapp, name, sizeof(earlyapp));
       write(fd, earlyapp, strlen(earlyapp));
       close(fd);
   }
}
#endif

int main()
{
#ifdef PLATFORM_MSMNILE_AU
    place_marker("M - AMS Service Starting...");
#endif
    sp<IAMS> service = new AMS();
    AMS *temp = static_cast<AMS *>(service.get());
    // no need to check for temp->is_ams_initialized(),
    // since ams is uninitilized after service startup.
    // audiod will initialize ams by calling ams_core_init.
    if (temp)
    {
        // configureRpcThreadpool(16, true /*callerWillJoin*/);
        configureRpcThreadpool(1, true /*callerWillJoin*/); // 1 thread
        if (android::OK != service->registerAsService())
        {
            ALOGE("%s:AMS service cannot be registered!", __func__);
            return 1;
        }
#ifdef PLATFORM_MSMNILE_AU
        place_marker("M - AMS Service Created...");
#endif
        ALOGD("%s:AMS service is registered!", __func__);
        joinRpcThreadpool();
    }
    return 1;
};

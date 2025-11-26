/*===========================================================================

*//** @file hyp_dvr_display_be.h

Copyright (c) 2019-2020,2022 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/

/*===========================================================================
                             Edit History

$Header:  $

when(mm/dd/yy)     who         what, where, why
-------------      --------    -------------------------------------------------------
05/27/22           nd          Fix KW issues
01/15/20           sh          Bringup DVR on LA GVM on Hana
01/22/19           sh          Impement HIDL interface for DVR
============================================================================*/
#ifndef VENDOR_QTI_DVR_V1_0_DVR_H
#define VENDOR_QTI_DVR_V1_0_DVR_H

#include "hyp_debug.h"
#include "hyp_dvr.h"
#include "hyp_dvrpriv.h"
#include "MMSignal.h"
#include "hyp_buffer_manager.h"
#include <vendor/qti/dvr/1.0/IDvrDisplay.h>
#include <hardware/hardware.h>
#include <hidl/Status.h>
#include "MMThread.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "vendor.qti.dvr@1.0-service"

using namespace android;

namespace vendor {
namespace qti {
namespace dvr {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_handle;
using ::vendor::qti::dvr::V1_0::IDvrDisplay;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct DvrDisplay : public IDvrDisplay {
public:
    DvrDisplay();
    ~DvrDisplay();
    Return<void> getDisplayInfo(getDisplayInfo_cb _hidl_cb) override;
    Return<int32_t> registerCallback(const sp<IDvrDisplayStreamCB>& streamObj) override;
    Return<int32_t> setDisplayBuffers(const hidl_handle& hidl) override;

private:
    void start_preview_thread();
    void stop_preview_thread();
    static int thread_func(void* handle);
    hdvr_status_type hdvr_version_check(hdvr_session_t* hdvr_session, hypdvr_msg_type* msg);
    void hdvr_be_ioctl(hdvr_session_t* hdvr_session, hypdvr_msg_type* msg);
    void hdvr_be_open(hdvr_session_t* hdvr_session, hypdvr_msg_type* msg);
    void hdvr_be_close(hdvr_session_t* hdvr_session, hypdvr_msg_type* msg);
    hdvr_status_type hdvr_handle_request(hdvr_session_t* hdvr_session);
    hdvr_status_type is_display_ready();
    hdvr_status_type display_init();
    void display_buffer(char* va, uint32 size);
    void display_deinit(uint32 size);

    DvrDisplayInfo mDisplayInfo;
    MM_HANDLE   mThread;
    boolean mDisplayReady;
    sp<IDvrDisplayStreamCB> mDisplayStreamObj;
};

extern "C" IDvrDisplay* HIDL_FETCH_IDvrDisplay(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace dvr
}  // namespace qti
}  // namespace vendor

#endif  // VENDOR_QTI_DVR_V1_0_DVR_H

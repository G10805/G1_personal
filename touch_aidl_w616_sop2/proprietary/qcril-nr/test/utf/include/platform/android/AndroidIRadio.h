/*===========================================================================
 *
 *    Copyright (c) 2020 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#include "ril_service.h"
#include "ril_utf_hidl_services.h"
#include "ril_utf_ril_api.h"
#include "platform/common/CommonPlatform.h"
#include "platform/android/IRadioResponseClientImpl.h"
#include "platform/android/IRadioIndicationClientImpl.h"

#include <android/hardware/radio/1.6/IRadio.h>

#ifdef OFF
#undef OFF
#endif

class AndroidIRadio : public CommonPlatform {
public:
    AndroidIRadio() {
        utfIRadio = nullptr;
        bFirstCall = true;
        dataOnRequest = [](::android::sp<::android::hardware::radio::V1_6::IRadio>, int, void *, size_t, RIL_Token) {
            return 1;
        };
    }
    void Register(RIL_RadioFunctions *callbacks) override;
    int OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) override;
    void setDataAPIs(::android::sp<IRadioResponseClientImpl>, ::android::sp<IRadioIndicationClientImpl>,
        std::function<int(::android::sp<::android::hardware::radio::V1_6::IRadio> utfIRadioData, int, void *, size_t, RIL_Token)> onRequest);
private:
    ::android::sp<::android::hardware::radio::V1_6::IRadio> utfIRadio;
    ::android::sp<IRadioResponseClientImpl> dataRespClient;
    ::android::sp<IRadioIndicationClientImpl> dataIndClient;
    std::function<int(::android::sp<::android::hardware::radio::V1_6::IRadio> utfIRadioData, int, void *, size_t, RIL_Token)> dataOnRequest;
    bool bFirstCall;
};

AndroidIRadio& getAndroidIRadio();

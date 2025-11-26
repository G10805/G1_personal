/*==============================================================================
  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
#pragma once

#include <vendor/qti/hardware/data/iwlan/1.1/IIWlan.h>
#include <vendor/qti/hardware/data/iwlan/1.1/IIWlanIndication.h>
#include <vendor/qti/hardware/data/iwlan/1.1/IIWlanResponse.h>

#include "IWlanImpl_1_0.h"
#include "DataConfig.h"

using namespace android::hardware;
using namespace rildata;
using namespace vendor::qti::hardware::data;

namespace vendor::qti::hardware::data::iwlan::V1_1::impl {

vector<radio::V1_5::LinkAddress> convertLinkAddressToHidlVector(vector<rildata::LinkAddress_t> addresses);
void convertTrafficDescriptor(radio::V1_6::TrafficDescriptor& trafficDescriptor,
                              const rildata::TrafficDescriptor_t& result);
void convertDCResultToHAL(DataCallResult_t result, radio::V1_6::SetupDataCallResult& halResult);
rildata::DataProfileInfo_t convertHidlDataProfileInfoToRil(const radio::V1_5::DataProfileInfo& profile);

template <class T>
class IWlanImpl : public iwlan::V1_0::impl::IWlanImpl<T> {
public:
    IWlanImpl() = default;
    virtual ~IWlanImpl() = default;
    Return<void> setResponseFunctions(
        const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
        const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) override;
    virtual Return<void> setResponseFunctions_nolock(
        const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
        const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication);
    static const DataHalServiceImplVersion &getVersion();
    virtual void clearResponseFunctions();
    virtual void clearResponseFunctions_nolock();
    Return<void> getDataCallList_1_1(int32_t serial);
    Return<void> setupDataCall_1_1(int32_t serial, const radio::V1_5::DataProfileInfo& dataProfile,
        bool allowRoaming, radio::V1_2::DataRequestReason reason,
        const hidl_vec<radio::V1_5::LinkAddress>& addresses, const hidl_vec<hidl_string>& dnses,
        const radio::V1_6::OptionalTrafficDescriptor& trafficDescriptor, bool matchAllRuleAllowed);
    virtual void onDataCallListChange(std::vector<DataCallResult_t> dcList);
    virtual void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t response, int32_t serial, Message::Callback::Status status);
    virtual void onUnthrottleApn(std::string apn);

private:
    sp<iwlan::V1_1::IIWlanResponse> mIWlanResponse;
    sp<iwlan::V1_1::IIWlanIndication> mIWlanIndication;

    sp<iwlan::V1_1::IIWlanResponse> getIWlanResponse();
    sp<iwlan::V1_1::IIWlanIndication> getIWlanIndication();
};

template<>
const DataHalServiceImplVersion &IWlanImpl<iwlan::V1_0::IIWlan>::getVersion();

template<class T>
Return<void> IWlanImpl<T>::setResponseFunctions(
    const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
    const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) {

    QCRIL_LOG_INFO("IWlanImpl::setResponseFunctions");
    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(V1_0::impl::IWlanImpl<T>::mCallbackLock);
        V1_1::impl::IWlanImpl<T>::setResponseFunctions_nolock(iwlanResponse, iwlanIndication);
    }
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::setResponseFunctions_nolock(
    const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
    const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) {
    QCRIL_LOG_DEBUG("IWlanImpl::setResponseFunctions_nolock. iwlanResponseParam: %s. iwlanIndicationParam: %s",
            iwlanResponse ? "<not null>" : "<null>",
            iwlanIndication ? "<not null>" : "<null>");

    V1_1::impl::IWlanImpl<T>::mIWlanResponse =
        iwlan::V1_1::IIWlanResponse::castFrom(iwlanResponse).withDefault(nullptr);
    V1_1::impl::IWlanImpl<T>::mIWlanIndication =
        iwlan::V1_1::IIWlanIndication::castFrom(iwlanIndication).withDefault(nullptr);
    V1_0::impl::IWlanImpl<T>::setResponseFunctions_nolock(iwlanResponse, iwlanIndication);
    return Void();
}

template<class T>
void IWlanImpl<T>::clearResponseFunctions() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(V1_0::impl::IWlanImpl<T>::mCallbackLock);
    clearResponseFunctions_nolock();
}

template<class T>
void IWlanImpl<T>::clearResponseFunctions_nolock() {
    mIWlanIndication = NULL;
    IWlanImpl<T>::resetWakeLock();
    V1_0::impl::IWlanImpl<T>::clearResponseFunctions_nolock();
}

template<class T>
sp<iwlan::V1_1::IIWlanResponse> IWlanImpl<T>::getIWlanResponse() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(V1_0::impl::IWlanImpl<T>::mCallbackLock);
    return mIWlanResponse;
}

template<class T>
sp<iwlan::V1_1::IIWlanIndication> IWlanImpl<T>::getIWlanIndication() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(V1_0::impl::IWlanImpl<T>::mCallbackLock);
    return mIWlanIndication;
}

template<class T>
Return<void> IWlanImpl<T>::getDataCallList_1_1(int32_t serial) {

    auto msg = std::make_shared<GetIWlanDataCallListRequestMessage>();

    GenericCallback<DataCallListResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<DataCallListResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                radio::V1_0::RadioError e = V1_0::impl::convertMsgToRadioError(status, responseDataPtr->respErr);
                V1_0::IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getIWlanDataCallList cb invoked status %d respErr %d", status, responseDataPtr->respErr);
                QCRIL_LOG_DEBUG("Call list size = %d", responseDataPtr->call.size());
                for(size_t i=0;i<responseDataPtr->call.size();i++) {
                    DataCallResult_t *callinst = &(responseDataPtr->call[i]);
                    QCRIL_LOG_DEBUG("cid %d", callinst->cid);
                    QCRIL_LOG_DEBUG("cause %d", callinst->cause);
                    QCRIL_LOG_DEBUG("suggestedRetryTime %ld", callinst->suggestedRetryTime);
                    QCRIL_LOG_DEBUG("type %s", callinst->type.c_str());
                    QCRIL_LOG_DEBUG("ifname %s", callinst->ifname.c_str());
                    QCRIL_LOG_DEBUG("addresses %s", callinst->addresses.c_str());
                    QCRIL_LOG_DEBUG("dnses %s", callinst->dnses.c_str());
                    QCRIL_LOG_DEBUG("gateways %s", callinst->gateways.c_str());
                    QCRIL_LOG_DEBUG("pcscf %s", callinst->pcscf.c_str());
                    QCRIL_LOG_DEBUG("mtuV4 %d", callinst->mtuV4);
                    QCRIL_LOG_DEBUG("mtuV6 %d", callinst->mtuV6);
                    QCRIL_LOG_DEBUG("tdSize %d", callinst->trafficDescriptors.size());
                }

                hidl_vec<radio::V1_6::SetupDataCallResult> dcResultList;
                dcResultList.resize(responseDataPtr->call.size());
                int i=0;
                for (DataCallResult_t entry: responseDataPtr->call) {
                    convertDCResultToHAL(entry, dcResultList[i++]);
                }

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getDataCallListResponse_1_1(responseInfo, dcResultList);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send getDataCallList response. Exception : %s", retStatus.description().c_str());
                    }
                } else {
                    QCRIL_LOG_ERROR("%s: IWlanService resp cb is NULL", __FUNCTION__);
                }

            }
        }));
    if (msg) {
        msg->setCallback(&cb);
        msg->dispatch();
    }
    else {
        QCRIL_LOG_ERROR("%s: IWlanService msg is NULL", __FUNCTION__);
    }
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::setupDataCall_1_1(int32_t serial, const radio::V1_5::DataProfileInfo& dataProfile,
    bool allowRoaming, radio::V1_2::DataRequestReason reason,
    const hidl_vec<radio::V1_5::LinkAddress>& addresses, const hidl_vec<hidl_string>& dnses,
    const radio::V1_6::OptionalTrafficDescriptor& trafficDescriptor, bool matchAllRuleAllowed) {
    QCRIL_LOG_DEBUG("IWLAN setupDataCall_1_1");
    std::vector<std::string> iwlanAddresses;
    std::vector<std::string> iwlanDnses;

    for (size_t i = 0 ; i < addresses.size(); i++ ) {
        iwlanAddresses.push_back(addresses[i].address);
    }
    for (size_t i = 0 ; i < dnses.size(); i++ ) {
        iwlanDnses.push_back(dnses[i]);
    }

    auto msg = std::make_shared<SetupDataCallRequestMessage_1_6>(
        serial,
        RequestSource_t::IWLAN,
        AccessNetwork_t::IWLAN,
        convertHidlDataProfileInfoToRil(dataProfile),
        allowRoaming,
        (DataRequestReason_t)reason,
        iwlanAddresses,
        iwlanDnses,
        matchAllRuleAllowed,
        std::make_shared<std::function<void(int32_t)>>(std::bind(&IWlanImpl<T>::acknowledgeRequest, this, std::placeholders::_1)));

    if (msg) {
      if(trafficDescriptor.getDiscriminator()
         == radio::V1_6::OptionalTrafficDescriptor::hidl_discriminator::value) {
        TrafficDescriptor_t td;
        if(trafficDescriptor.value().dnn.getDiscriminator()
           == radio::V1_6::OptionalDnn::hidl_discriminator::value) {
          td.dnn = trafficDescriptor.value().dnn.value();
        }
        if(trafficDescriptor.value().osAppId.getDiscriminator()
           == radio::V1_6::OptionalOsAppId::hidl_discriminator::value) {
          std::vector<uint8_t> osAppId (trafficDescriptor.value().osAppId.value().osAppId.begin(),
                                        trafficDescriptor.value().osAppId.value().osAppId.end());
          td.osAppId = osAppId;
        }
        msg->setOptionalTrafficDescriptor(td);
      }

      GenericCallback<SetupDataCallResponse_t> cb([serial](std::shared_ptr<Message>,
                                                          Message::Callback::Status status,
                                                          std::shared_ptr<SetupDataCallResponse_t> rsp) -> void {
          if (rsp != nullptr) {
              auto indMsg = std::make_shared<SetupDataCallIWlanResponseIndMessage>(*rsp, serial, status);
              if (indMsg != nullptr) {
                  indMsg->broadcast();
              } else {
                  QCRIL_LOG_DEBUG("setup data call cb failed to allocate message status %d respErr %d", status, rsp->respErr);
              }
          } else {
              QCRIL_LOG_ERROR("setupDataCall_1_1 resp is nullptr");
          }
      });
      msg->setCallback(&cb);
      msg->dispatch();
    }
    return Void();
}

template<class T>
void IWlanImpl<T>::onDataCallListChange(std::vector<DataCallResult_t> dcList) {
#ifdef QMI_RIL_UTF
    string rilVersion = qcril_get_property_value("vendor.radio.utf.version", "1.4");
    QCRIL_LOG_DEBUG("RadioImpl read property version %s", rilVersion.c_str());
    if (rilVersion != "1.6") {
        return V1_0::impl::IWlanImpl<T>::onDataCallListChange(dcList);
    }
#endif
    QCRIL_LOG_DEBUG("IWLAN 1.1 Data Call List Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        V1_0::impl::IWlanImpl<T>::acquireWakeLock();
        hidl_vec<radio::V1_6::SetupDataCallResult> dcResultList;
        dcResultList.resize(dcList.size());
        int i=0;
        for (DataCallResult_t entry: dcList) {
            convertDCResultToHAL(entry, dcResultList[i++]);
        }
        Return<void> retStatus =
             indicationCb->dataCallListChangeIndication_1_1(dcResultList);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send dataCallListChangeIndication_1_1. Exception : %s", retStatus.description().c_str());
        }
    }else {
        return V1_0::impl::IWlanImpl<T>::onDataCallListChange(dcList);
    }
}

template<class T>
void IWlanImpl<T>::onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t rsp, int32_t serial, Message::Callback::Status status) {
#ifdef QMI_RIL_UTF
    string rilVersion = qcril_get_property_value("vendor.radio.utf.version", "1.4");
    QCRIL_LOG_DEBUG("RadioImpl read property version %s", rilVersion.c_str());
    if (rilVersion != "1.6") {
        return V1_0::impl::IWlanImpl<T>::onSetupDataCallIWlanResponseIndMessage(rsp, serial, status);
    }
#endif
    QCRIL_LOG_DEBUG("IWLAN 1.1 SetupDataCall response indication");
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        V1_0::impl::IWlanImpl<T>::acquireWakeLock();
        radio::V1_6::SetupDataCallResult dcResult = {};
        V1_0::IWlanResponseInfo responseInfo {.serial = serial, .error = radio::V1_0::RadioError::NO_MEMORY};
        radio::V1_0::RadioError e = V1_0::impl::convertMsgToRadioError(status, rsp.respErr);
        QCRIL_LOG_DEBUG("status %d respErr %d", status, rsp.respErr);
        QCRIL_LOG_DEBUG("cause = %d", rsp.call.cause);
        QCRIL_LOG_DEBUG("suggestedRetryTime = %d", rsp.call.suggestedRetryTime);
        QCRIL_LOG_DEBUG("cid = %d", rsp.call.cid);
        QCRIL_LOG_DEBUG("active = %d", rsp.call.active);
        QCRIL_LOG_DEBUG("type = %s", rsp.call.type.c_str());
        QCRIL_LOG_DEBUG("ifname = %s", rsp.call.ifname.c_str());
        QCRIL_LOG_DEBUG("addresses = %s", rsp.call.addresses.c_str());
        QCRIL_LOG_DEBUG("dnses = %s", rsp.call.dnses.c_str());
        QCRIL_LOG_DEBUG("gateways = %s", rsp.call.gateways.c_str());
        QCRIL_LOG_DEBUG("pcscf = %s", rsp.call.pcscf.c_str());
        QCRIL_LOG_DEBUG("mtu = %d", rsp.call.mtu);
        convertDCResultToHAL(rsp.call, dcResult);
        responseInfo = {.serial = serial, .error = e};
        QCRIL_LOG_DEBUG("setup data call 1.1 cb invoked serial %d error %d", responseInfo.serial, responseInfo.error);
        Return<void> retStatus =
                responseCb->setupDataCallResponse_1_1(responseInfo, dcResult);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send setupDataCall response. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService response cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::onUnthrottleApn(std::string apn) {
    QCRIL_LOG_DEBUG("IWLAN unthrottleApn indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        V1_0::impl::IWlanImpl<T>::acquireWakeLock();
        Return<void> retStatus = indicationCb->unthrottleApn(apn);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send unthrottleApn ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

}

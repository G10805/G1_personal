/*==============================================================================
  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
#pragma once

#include <mutex>
#include <time.h>
#include <signal.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlan.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlanIndication.h>
#include <vendor/qti/hardware/data/iwlan/1.0/IIWlanResponse.h>

#include "DataHalServiceImplFactory.h"
#include "IWlanServiceBase.h"
#include "MessageCommon.h"
#include "QtiMutex.h"
#include "DataModule.h"

#include "GetAllQualifiedNetworkRequestMessage.h"
#include "GetIWlanDataRegistrationStateRequestMessage.h"
#include "GetIWlanDataCallListRequestMessage.h"
#include "DeactivateDataCallRequestMessage.h"
#include "IWLANCapabilityHandshake.h"

using android::sp;
using android::wp;
using android::hardware::Void;
using android::hardware::Return;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;
using android::hardware::hidl_handle;
using vendor::qti::hardware::data::iwlan::IWlanServiceBase;
using namespace android::hardware;
using namespace rildata;
using namespace vendor::qti::hardware::data;

namespace vendor::qti::hardware::data::iwlan::V1_0::impl {

radio::V1_0::RadioError convertMsgToRadioError(Message::Callback::Status status, ResponseError_t respErr);
void convertDCResultToHAL(DataCallResult_t result, radio::V1_4::SetupDataCallResult& halResult);
void convertQualifiedNetworksToHAL(std::vector<QualifiedNetwork_t> qnList,
        hidl_vec<iwlan::V1_0::QualifiedNetworks>& qNetworks);
std::vector<hidl_string> convertAddrStringToHidlStringVector(const std::string &addr);
radio::V1_4::PdpProtocolType convertStringToPdpProtocolType(const std::string &type);
std::string convertPdpProtocolTypeToString(const radio::V1_4::PdpProtocolType protocol);
rildata::DataProfileInfo_t convertHidlDataProfileInfoToRil(const radio::V1_4::DataProfileInfo& profile);

template <class T>
class IWlanImpl : public T, public IWlanServiceBase {
public:
    IWlanImpl() = default;
    virtual ~IWlanImpl() = default;
    bool registerService(int instanceId) override;
    Return<void> setResponseFunctions(
        const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
        const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) override;
    virtual Return<void> setResponseFunctions_nolock(
        const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
        const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication);
    Return<void> setupDataCall(int32_t serial, const radio::V1_4::DataProfileInfo& dataProfile, bool allowRoaming,
                               radio::V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
                               const hidl_vec<hidl_string>& dnses) override;
    Return<void> deactivateDataCall(int32_t serial, int32_t cid, radio::V1_2::DataRequestReason reason) override;
    Return<void> getDataCallList(int32_t serial) override;
    Return<void> getDataRegistrationState(int32_t serial) override;
    Return<void> getAllQualifiedNetworks(int32_t serial) override;
    Return<void> responseAcknowledgement() override;
    Return<void> iwlanDisabled() override;
    Return<void> debug(const hidl_handle& handle, const hidl_vec<hidl_string>&) override;

    virtual void onDataRegistrationStateChange();
    virtual void onDataCallListChange(std::vector<DataCallResult_t> dcList);
    virtual void onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList);
    virtual void onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t response, int32_t serial, Message::Callback::Status status);
    virtual void onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t response, int32_t serial, Message::Callback::Status status);
    virtual void clearResponseFunctions();
    virtual void clearResponseFunctions_nolock();
    virtual void acknowledgeRequest(int32_t serial);
    static const DataHalServiceImplVersion &getVersion();

private:
    sp<iwlan::V1_0::IIWlanResponse> mIWlanResponse;
    sp<iwlan::V1_0::IIWlanIndication> mIWlanIndication;

    sp<iwlan::V1_0::IIWlanResponse> getIWlanResponse();
    sp<iwlan::V1_0::IIWlanIndication> getIWlanIndication();
};

template<>
const DataHalServiceImplVersion &IWlanImpl<iwlan::V1_0::IIWlan>::getVersion();

template<class T>
bool IWlanImpl<T>::registerService(int instanceId) {
    std::string serviceName = "slot" + std::to_string(instanceId + 1);
    auto ret = T::registerAsService(serviceName);
    QCRIL_LOG_INFO("[IWlanImpl]: %s %s", T::descriptor, (ret == android::OK ? " registered" : " failed to register"));
    iwlanSvcWakelock = "iwlan_svc_wl_" + std::to_string(instanceId);
    return ret == android::OK;
}

template<class T>
Return<void> IWlanImpl<T>::setResponseFunctions(
    const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
    const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) {

    QCRIL_LOG_INFO("IWlanImpl::setResponseFunctions");
    {
        std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
        IWlanImpl<T>::setResponseFunctions_nolock(iwlanResponse, iwlanIndication);
    }
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::setResponseFunctions_nolock(
    const sp<iwlan::V1_0::IIWlanResponse>& iwlanResponse,
    const sp<iwlan::V1_0::IIWlanIndication>& iwlanIndication) {

    if (mIWlanResponse != nullptr) {
        mIWlanResponse->unlinkToDeath(this);
    }

    QCRIL_LOG_DEBUG("IWlanImpl::setResponseFunctions_nolock. iwlanResponseParam: %s. iwlanIndicationParam: %s",
            iwlanResponse ? "<not null>" : "<null>",
            iwlanIndication ? "<not null>" : "<null>");
    mIWlanResponse = iwlanResponse;
    mIWlanIndication = iwlanIndication;
    iwlanResponse->linkToDeath(this, 0);

    //Since at least one of the services has been created, which means QTI
    //IWLAN is being used and we need to tell modem to turn on AP assist mode
    //This is for initial handshake

    auto msg =
        std::make_shared<IWLANCapabilityHandshake>(true);

    if (msg) {
          GenericCallback<ModemIWLANCapability_t> cb([this](std::shared_ptr<Message> msg,
                                                       Message::Callback::Status status,
                                                       std::shared_ptr<ModemIWLANCapability_t> rsp) -> void {
              if (msg && rsp) {

                  QCRIL_LOG_DEBUG("IWLANCapabilityHandshake cb invoked status %d ", status);

                  auto indicationCb = getIWlanIndication();
                  if (indicationCb != NULL) {

                      if ((status != Message::Callback::Status::SUCCESS) ||
                          (*rsp == ModemIWLANCapability_t::not_present)) {
                          Return<void> retStatus =
                          indicationCb->modemSupportNotPresent();
                          if (!retStatus.isOk()) {
                              QCRIL_LOG_ERROR("Unable to send support notification ind. Exception : %s",
                                              retStatus.description().c_str());
                          }
                      }


                  } else {
                      QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
                  }
              }

        });
        msg->setCallback(&cb);
        msg->dispatch();
    }
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::setupDataCall(int32_t serial, const radio::V1_4::DataProfileInfo& dataProfile, bool allowRoaming,
                            radio::V1_2::DataRequestReason reason, const hidl_vec<hidl_string>& addresses,
                            const hidl_vec<hidl_string>& dnses) {

    std::vector<std::string> iwlanAddresses;
    std::vector<std::string> iwlanDnses;

    for (size_t i = 0 ; i < addresses.size(); i++ ) {
        iwlanAddresses.push_back(addresses[i]);
    }
    for (size_t i = 0 ; i < dnses.size(); i++ ) {
        iwlanDnses.push_back(dnses[i]);
    }

    DataProfileInfo_t rilDataProfile = convertHidlDataProfileInfoToRil(dataProfile);
    auto msg =
      std::make_shared<SetupDataCallRequestMessage>(
        serial,
        RequestSource_t::IWLAN,
        AccessNetwork_t::IWLAN,
        rilDataProfile,
        allowRoaming,
        (DataRequestReason_t)reason,
        iwlanAddresses,
        iwlanDnses,
        std::make_shared<std::function<void(int32_t)>>(std::bind(&IWlanImpl<T>::acknowledgeRequest, this, std::placeholders::_1)));

        if (msg) {
            GenericCallback<SetupDataCallResponse_t> cb([serial](std::shared_ptr<Message>,
                                                                 Message::Callback::Status status,
                                                                 std::shared_ptr<SetupDataCallResponse_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<SetupDataCallIWlanResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("IWLAN setup data call cb failed to allocate message status %d respErr %d", status, rsp->respErr);
                }
            } else {
                QCRIL_LOG_ERROR("IWLAN setupDataCall resp is nullptr");
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }

    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::deactivateDataCall(int32_t serial, int32_t cid, radio::V1_2::DataRequestReason reason) {

   auto msg =
        std::make_shared<DeactivateDataCallRequestMessage>(
            serial,
            cid,
            (DataRequestReason_t)reason,
            std::make_shared<std::function<void(int32_t)>>(std::bind(&IWlanImpl::acknowledgeRequest, this, std::placeholders::_1)));
    if (msg) {
        GenericCallback<ResponseError_t> cb([serial](std::shared_ptr<Message>,
                                                     Message::Callback::Status status,
                                                     std::shared_ptr<ResponseError_t> rsp) -> void {
            if (rsp != nullptr) {
                auto indMsg = std::make_shared<DeactivateDataCallIWlanResponseIndMessage>(*rsp, serial, status);
                if (indMsg != nullptr) {
                    indMsg->broadcast();
                } else {
                    QCRIL_LOG_DEBUG("IWLAN deactivate data call cb failed to allocate message status %d respErr %d", status, *rsp);
                }
            } else {
                QCRIL_LOG_ERROR("IWLAN deactivateDataCall resp is nullptr");
            }
        });
        msg->setCallback(&cb);
        msg->dispatch();
    }

    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::getDataCallList(int32_t serial) {

    auto msg = std::make_shared<GetIWlanDataCallListRequestMessage>();

    GenericCallback<DataCallListResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<DataCallListResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                radio::V1_0::RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
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
                    QCRIL_LOG_DEBUG("mtu %d", callinst->mtu);
                }

                hidl_vec<radio::V1_4::SetupDataCallResult> dcResultList;
                dcResultList.resize(responseDataPtr->call.size());
                int i=0;
                for (DataCallResult_t entry: responseDataPtr->call) {
                    convertDCResultToHAL(entry, dcResultList[i++]);
                }

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getDataCallListResponse(responseInfo, dcResultList);
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
Return<void> IWlanImpl<T>::getDataRegistrationState(int32_t serial) {

    auto msg = std::make_shared<GetIWlanDataRegistrationStateRequestMessage>();

    GenericCallback<IWlanDataRegistrationStateResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<IWlanDataRegistrationStateResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                radio::V1_0::RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getDataRegistrationState cb invoked status %d respErr %d", status, responseDataPtr->respErr);

                IWlanDataRegStateResult regResult = {
                    .regState = (radio::V1_0::RegState)responseDataPtr->regState,
                    .reasonForDenial = responseDataPtr->reasonForDenial};

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getDataRegistrationStateResponse(responseInfo, regResult);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send data reg state response. Exception : %s", retStatus.description().c_str());
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
Return<void> IWlanImpl<T>::getAllQualifiedNetworks(int32_t serial) {

    auto msg = std::make_shared<GetAllQualifiedNetworkRequestMessage>();

    GenericCallback<QualifiedNetworkResult_t> cb(
        ([this, serial](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<QualifiedNetworkResult_t> responseDataPtr) -> void {
            if (solicitedMsg && responseDataPtr) {
                radio::V1_0::RadioError e = convertMsgToRadioError(status, responseDataPtr->respErr);
                IWlanResponseInfo responseInfo = {
                    .serial = serial, .error = e};
                QCRIL_LOG_DEBUG("getAllQualifiedNetworks cb invoked status %d respErr %d", status, responseDataPtr->respErr);

                hidl_vec<iwlan::V1_0::QualifiedNetworks> qNetworks;
                convertQualifiedNetworksToHAL(responseDataPtr->qualifiedNetwork, qNetworks);

                auto responseCb = getIWlanResponse();
                if (responseCb != NULL) {
                    Return<void> retStatus =
                        responseCb->getAllQualifiedNetworksResponse(responseInfo, qNetworks);
                    if (!retStatus.isOk()) {
                        QCRIL_LOG_ERROR("Unable to send qualifiedNetworks response. Exception : %s", retStatus.description().c_str());
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
Return<void> IWlanImpl<T>::responseAcknowledgement() {
    releaseWakeLock();
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::iwlanDisabled() {
    auto msg =
        std::make_shared<IWLANCapabilityHandshake>(false);

    if (msg) {
        msg->dispatch();
    }
    return Void();
}

template<class T>
Return<void> IWlanImpl<T>::debug(const hidl_handle& handle, const hidl_vec<hidl_string>&) {
    if (handle != nullptr && handle->numFds >= 1) {
        int fd = handle->data[0];
        getDataModule().dump(fd);
    }
    return Void();
}

template<class T>
void IWlanImpl<T>::onDataRegistrationStateChange() {
    QCRIL_LOG_DEBUG("IWLAN Data Registration State Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        Return<void> retStatus =
             indicationCb->dataRegistrationStateChangeIndication();
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send dataRegistrationStateChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::onDataCallListChange(std::vector<DataCallResult_t> dcList) {
    QCRIL_LOG_DEBUG("IWLAN Data Call List Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        hidl_vec<radio::V1_4::SetupDataCallResult> dcResultList;
        dcResultList.resize(dcList.size());
        int i=0;
        for (DataCallResult_t entry: dcList) {
            convertDCResultToHAL(entry, dcResultList[i++]);
        }
        Return<void> retStatus =
             indicationCb->dataCallListChangeIndication(dcResultList);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send dataCallListChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::onQualifiedNetworksChange(std::vector<QualifiedNetwork_t> qnList) {
    QCRIL_LOG_DEBUG("IWLAN Qualified Networks Change indication");
    auto indicationCb = getIWlanIndication();
    if (indicationCb != NULL) {
        acquireWakeLock();
        hidl_vec<V1_0::QualifiedNetworks> qNetworks;
        convertQualifiedNetworksToHAL(qnList, qNetworks);
        Return<void> retStatus =
             indicationCb->qualifiedNetworksChangeIndication(qNetworks);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send QualifiedNetworksChange ind. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService ind cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::onSetupDataCallIWlanResponseIndMessage(SetupDataCallResponse_t rsp, int32_t serial, Message::Callback::Status status) {
    QCRIL_LOG_DEBUG("IWLAN SetupDataCall response indication");
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        acquireWakeLock();
        radio::V1_4::SetupDataCallResult dcResult = {};
        IWlanResponseInfo responseInfo {.serial = serial, .error = radio::V1_0::RadioError::NO_MEMORY};
        radio::V1_0::RadioError e = convertMsgToRadioError(status, rsp.respErr);
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
        QCRIL_LOG_DEBUG("setup data call cb invoked serial %d error %d", responseInfo.serial, responseInfo.error);
        Return<void> retStatus =
                responseCb->setupDataCallResponse(responseInfo, dcResult);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send setupDataCall response. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService response cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::onDeactivateDataCallIWlanResponseIndMessage(ResponseError_t rsp, int32_t serial, Message::Callback::Status status) {
    QCRIL_LOG_DEBUG("IWLAN DeactivateDataCall response indication");
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        acquireWakeLock();
        IWlanResponseInfo responseInfo = {.serial = serial, .error = radio::V1_0::RadioError::NO_MEMORY};
        radio::V1_0::RadioError e = convertMsgToRadioError(status, rsp);
        responseInfo = {.serial = serial, .error = e};
        QCRIL_LOG_DEBUG("IWLAN deactivate data call cb invoked serial %d error %d", responseInfo.serial, responseInfo.error);
        Return<void> retStatus =
            responseCb->deactivateDataCallResponse(responseInfo);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send deactivateDataCall response. Exception : %s", retStatus.description().c_str());
        }
    }else {
        QCRIL_LOG_ERROR("%s: IWlanService response cb is NULL", __FUNCTION__);
    }
}

template<class T>
void IWlanImpl<T>::clearResponseFunctions() {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    clearResponseFunctions_nolock();
}

template<class T>
void IWlanImpl<T>::clearResponseFunctions_nolock() {
    mIWlanResponse = NULL;
    mIWlanIndication = NULL;
    IWlanImpl<T>::resetWakeLock();
}

template<class T>
void IWlanImpl<T>::acknowledgeRequest(int32_t serial) {
    QCRIL_LOG_ERROR("acknowledgeRequest %d", serial);
    auto responseCb = getIWlanResponse();
    if (responseCb != NULL) {
        Return<void> retStatus = responseCb->acknowledgeRequest(serial);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send acknowledgeRequest. Exception : %s", retStatus.description().c_str());
        }
    }
    else {
        QCRIL_LOG_ERROR("Response cb ptr is NULL");
    }
}

template<class T>
sp<iwlan::V1_0::IIWlanResponse> IWlanImpl<T>::getIWlanResponse() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIWlanResponse;
}

template<class T>
sp<iwlan::V1_0::IIWlanIndication> IWlanImpl<T>::getIWlanIndication() {
    std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    return mIWlanIndication;
}

}

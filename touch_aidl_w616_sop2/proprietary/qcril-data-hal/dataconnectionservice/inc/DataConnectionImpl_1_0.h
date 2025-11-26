/*
 * Copyright (c) 2018-2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <vendor/qti/hardware/data/connection/1.0/IDataConnection.h>
#include "framework/TimeKeeper.h"
#include "DataConnectionServiceBase.h"
#include "DataHalServiceImplFactory.h"
#include "MessageCommon.h"
#include "GetBearerAllocationRequestMessage.h"
#include "GetAllBearerAllocationsRequestMessage.h"
#include "RegisterBearerAllocationUpdateRequestMessage.h"

using android::hardware::Return;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_vec;
using android::hardware::hidl_bitfield;
using ::android::hardware::hidl_string;
using ::android::hardware::Void;
using android::sp;
using android::wp;
using rildata::RatType_t;
using rildata::AllocatedBearer_t;
using rildata::AllocatedBearerResult_t;
using rildata::ResponseError_t;

using vendor::qti::hardware::data::connection::V1_0::StatusCode;
using vendor::qti::hardware::data::connection::V1_0::ErrorReason;
using vendor::qti::hardware::data::connection::V1_0::RatType;
using vendor::qti::hardware::data::connection::V1_0::AllocatedBearers;
using vendor::qti::hardware::data::connection::V1_0::BearerInfo;
using namespace vendor::qti::hardware::data;

#ifndef PROPERTY_VALUE_MAX
  #define PROPERTY_VALUE_MAX 256
#endif

namespace vendor::qti::hardware::data::connection::V1_0::impl {

template <class T>
class DataConnectionImpl : public T, public DataConnectionServiceBase {
public:
    DataConnectionImpl();
    ~DataConnectionImpl();
    Return<StatusCode> getBearerAllocation(int32_t cid, const sp<connection::V1_0::IDataConnectionResponse>& cb) override;
    Return<StatusCode> getAllBearerAllocations(const sp<connection::V1_0::IDataConnectionResponse>& cb) override;
    Return<StatusCode> registerForAllBearerAllocationUpdates(const sp<connection::V1_0::IDataConnectionIndication>& cb) override;
    virtual void notifyBearerAllocationUpdate(std::shared_ptr<AllocatedBearerResult_t> bearers);
    static const DataHalServiceImplVersion &getVersion();
    bool registerService(int instanceId);

private:
    static void transpose(const AllocatedBearer_t& srcType, AllocatedBearers& dstType);
    static void transpose(const ResponseError_t& srcType, ErrorReason& dstType);
    static void transpose(const RatType_t& srcType, hidl_bitfield<RatType>& dstType);
    uint64_t generateCookie();
    uint64_t registerResponseCb(const sp<connection::V1_0::IDataConnectionResponse>& cb);
    uint64_t registerIndicationCb(const sp<connection::V1_0::IDataConnectionIndication>& cb);
    virtual void clearResponseFunctions(uint64_t cookie);
    virtual void sendBearerAllocationResult(uint64_t cookie,
                                    std::shared_ptr<AllocatedBearerResult_t> result);
    virtual void sendAllBearerAllocationsResult(uint64_t cookie,
                                        std::shared_ptr<AllocatedBearerResult_t> result);

    std::mutex cookieMutex;
    std::mutex registeredResponsesMutex;
    std::mutex registeredIndicationsMutex;

    uint64_t nextCookie;
    std::map<uint64_t, const sp<connection::V1_0::IDataConnectionResponse>> registeredResponses;
    std::map<uint64_t, const sp<connection::V1_0::IDataConnectionIndication>> registeredIndications;
};

template<>
DataConnectionImpl<V1_0::IDataConnection>::DataConnectionImpl();

template<>
const DataHalServiceImplVersion &DataConnectionImpl<V1_0::IDataConnection>::getVersion();

/**
 * DataConnectionImpl::DataConnectionImpl()
 *
 * @brief
 * Constructor for IDataConnection HAL implementation. Registers the HAL service per
 * slot id
 */
template <class T>
DataConnectionImpl<T>::DataConnectionImpl(): nextCookie(0), registeredResponses { },
        registeredIndications { } {
    Log::getInstance().d("DataConnectionImpl()");
}

/**
 * DataConnectionImpl::~DataConnectionImpl()
 *
 * @brief
 * Destructor for IDataConnection HAL implementation. Release shared pointer references
 */
template <class T>
DataConnectionImpl<T>::~DataConnectionImpl() {
}

/**
 * DataConnectionImpl::generateCookie()
 *
 * @brief
 * Generates a unique cookie to used to track each client Response/Indication callback
 *
 * @return uint64_t Unique cookie
 */
template <class T>
bool DataConnectionImpl<T>::registerService(int instanceId) {
    std::string serviceName = "slot";
    android::status_t status = -1;
    status = T::registerAsService(serviceName + std::to_string(instanceId + 1));
    if(status != android::OK) {
        Log::getInstance().d("DataConnection HAL registration failed.");
        return false;
    }
    return true;
}

/**
 * DataConnectionImpl::generateCookie()
 *
 * @brief
 * Generates a unique cookie to used to track each client Response/Indication callback
 *
 * @return uint64_t Unique cookie
 */
template <class T>
uint64_t DataConnectionImpl<T>::generateCookie() {
    uint64_t newCookie;
    cookieMutex.lock();
    newCookie = nextCookie++;
    cookieMutex.unlock();
    return newCookie;
}

/**
 * DataConnectionImpl::registerResponseCb()
 *
 * @brief
 * Stores the client's response callback to the map to be invoked once DataModule completes the
 * request
 *
 * @return uint64_t The cookie used to identify the callback in the Response map
 */
template <class T>
uint64_t DataConnectionImpl<T>::registerResponseCb(const sp<connection::V1_0::IDataConnectionResponse>& cb) {
    uint64_t cookie = generateCookie();
    if (cb == nullptr) {
        Log::getInstance().d("registerResponseCb cb is null");
        return cookie;
    }
    registeredResponsesMutex.lock();
    Log::getInstance().d("+cookie "+ std::to_string(cookie));
    registeredResponses.insert({cookie, cb});
    registeredResponsesMutex.unlock();
    cb->linkToDeath(this, cookie);
    return cookie;
}

/**
 * DataConnectionImpl::registerIndicationCb()
 *
 * @brief
 * Stores the client's indication callback to the map to be invoked every time a QMI indication
 * is received for DataConnection HAL
 *
 * @return uint64_t The cookie used to identify the callback in the Indications map
 */
template <class T>
uint64_t DataConnectionImpl<T>::registerIndicationCb(const sp<connection::V1_0::IDataConnectionIndication>& cb) {
    uint64_t cookie = generateCookie();
    if (cb == nullptr) {
        Log::getInstance().d("registerIndicationCb cb is null");
        return cookie;
    }
    registeredIndicationsMutex.lock();
    if (registeredIndications.empty()) {
        auto requestMsg = std::make_shared<RegisterBearerAllocationUpdateRequestMessage>(true);
        requestMsg->dispatch();
    }
    Log::getInstance().d("+cookie "+ std::to_string(cookie));
    registeredIndications.insert({cookie, cb});
    registeredIndicationsMutex.unlock();
    cb->linkToDeath(this, cookie);
    return cookie;
}

/**
 * DataConnectionImpl::notifyBearerAllocationUpdate()
 *
 * @brief
 * Notifies all registered clients of a bearer allocation update and sends the
 * updated bearer allocation list.
 */
template <class T>
void DataConnectionImpl<T>::notifyBearerAllocationUpdate(std::shared_ptr<AllocatedBearerResult_t> result) {
    Log::getInstance().d("DataConnectionImpl::notifyBearerAllocationUpdate");
    if (result == nullptr) {
        Log::getInstance().d("bearer allocation update with null");
        return;
    }
    std::vector<AllocatedBearers> bearersList;
    for (auto connection : result->connections) {
        AllocatedBearers bearers;
        transpose(connection, bearers);
        bearersList.push_back(bearers);
    }

    // invoke indication callback on all registered clients
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    std::lock_guard<std::mutex> lock(registeredIndicationsMutex);
    for (auto it = registeredIndications.begin(); it != registeredIndications.end(); ++it) {
        const sp<connection::V1_0::IDataConnectionIndication> clientCb = it->second;
        if (clientCb != nullptr) {
            clientCb->onBearerAllocationUpdate(bearersList);
        }
    }
}

/**
 * DataConnectionImpl::getBearerAllocation()
 *
 * @brief
 * Implementation for IDataConnection's getBearerAllocation API. This is a
 * public api that can be used to query bearer allocation info for a
 * specified call id.
 */
template <class T>
Return<StatusCode> DataConnectionImpl<T>::getBearerAllocation(
    int32_t cid,
    const sp<connection::V1_0::IDataConnectionResponse>& cb
) {
    if (cb == nullptr) {
        Log::getInstance().d("getBearerAllocation cb is null");
        return StatusCode::OK;
    }
    uint64_t cookie = registerResponseCb(cb);
    Log::getInstance().d("DataConnectionImpl::getBearerAllocation()" + std::to_string(cookie));
    auto requestMsg = std::make_shared<GetBearerAllocationRequestMessage>(cid);
    auto responseFn = std::bind(&DataConnectionImpl::sendBearerAllocationResult, this, cookie, std::placeholders::_3);
    GenericCallback<AllocatedBearerResult_t> requestCb(responseFn);
    requestMsg->setCallback(&requestCb);
    requestMsg->dispatch();
    return StatusCode::OK;
}

/**
 * DataConnectionImpl::getAllBearerAllocations()
 *
 * @brief
 * Implementation for IDataConnection's getAllBearerAllocations API. This is a
 * public api that can be used to query bearer allocation info for all
 * calls.
 */
template <class T>
Return<StatusCode> DataConnectionImpl<T>::getAllBearerAllocations(
    const sp<connection::V1_0::IDataConnectionResponse>& cb
) {
    uint64_t cookie = registerResponseCb(cb);
    Log::getInstance().d("DataConnectionImpl::getAllBearerAllocations()" + std::to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("getAllBearerAllocations cb is null");
        return StatusCode::OK;
    }
    auto requestMsg = std::make_shared<GetAllBearerAllocationsRequestMessage>();
    auto responseFn = std::bind(&DataConnectionImpl::sendAllBearerAllocationsResult, this, cookie, std::placeholders::_3);
    GenericCallback<AllocatedBearerResult_t> requestCb(responseFn);
    requestMsg->setCallback(&requestCb);
    requestMsg->dispatch();
    return StatusCode::OK;
}

/**
 * DataConnectionImpl::sendBearerAllocationResult()
 *
 * @brief
 * This invokes the client's Response callback when querying for bearer
 * allocation. This is passed as a GenericCallback inside the
 * GetBearerAllocationRequestMessage to DataModule. DataModule
 * shall call this once bearer allocation information is retrieved
 */
template <class T>
void DataConnectionImpl<T>::sendBearerAllocationResult(
    uint64_t cookie,
    std::shared_ptr<AllocatedBearerResult_t> result
) {
    registeredResponsesMutex.lock();
    const sp<connection::V1_0::IDataConnectionResponse> cb = registeredResponses[cookie];
    registeredResponsesMutex.unlock();
    Log::getInstance().d("DataConnectionImpl::sendBearerAllocationResult()" + std::to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("sendBearerAllocationResult() client callback is null");
        return;
    }
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    AllocatedBearers bearers = {};
    ErrorReason error = ErrorReason::HARDWARE_ERROR;
    if (result == nullptr) {
        Log::getInstance().d("bearer list is null");
        cb->onBearerAllocationResponse(error, bearers);
        return;
    }
    transpose(result->error, error);
    if (result->error != ResponseError_t::NO_ERROR ||
        result->connections.empty()) {
        Log::getInstance().d("getBearerAllocation returned error");
        cb->onBearerAllocationResponse(error, bearers);
        return;
    }

    AllocatedBearer_t bearerResult = result->connections.front();
    transpose(bearerResult, bearers);
    cb->onBearerAllocationResponse(error, bearers);
}

/**
 * DataConnectionImpl::sendAllBearerAllocationsResult()
 *
 * @brief
 * This invokes the client's Response callback when querying for bearer
 * allocation. This is passed as a GenericCallback inside the
 * GetAllBearerAllocationsRequestMessage to DataModule. DataModule
 * shall call this once bearer allocation information is retrieved
 */
template <class T>
void DataConnectionImpl<T>::sendAllBearerAllocationsResult(
    uint64_t cookie,
    std::shared_ptr<AllocatedBearerResult_t> result
) {
    registeredResponsesMutex.lock();
    const sp<connection::V1_0::IDataConnectionResponse> cb = registeredResponses[cookie];
    registeredResponsesMutex.unlock();
    Log::getInstance().d("DataConnectionImpl::sendBearerAllocationResult()" + std::to_string(cookie));
    if (cb == nullptr) {
        Log::getInstance().d("sendAllBearerAllocationsResult() client callback is null");
        return;
    }
    acquireWakelock(BEARER_ALLOCATION_TIMEOUT);
    std::vector<AllocatedBearers> bearersList;
    ErrorReason error = ErrorReason::HARDWARE_ERROR;
    if (result == nullptr) {
        Log::getInstance().d("bearer list is null");
        cb->onAllBearerAllocationsResponse(error, bearersList);
        return;
    }
    transpose(result->error, error);
    if (result->error != ResponseError_t::NO_ERROR) {
        Log::getInstance().d("getAllBearerAllocations returned error");
        cb->onAllBearerAllocationsResponse(error, bearersList);
        return;
    }
    for (auto connection : result->connections) {
        AllocatedBearers bearers = {};
        transpose(connection, bearers);
        bearersList.push_back(bearers);
    }
    cb->onAllBearerAllocationsResponse(error, bearersList);
}

/**
 * DataConnectionImpl::registerForAllBearerAllocationUpdates()
 *
 * @brief
 * Implementation for IDataConnection's registerForAllBearerAllocationUpdates
 * API. This is a public api that can be used to register for bearer allocation
 * updates for all calls.
 */
template <class T>
Return<StatusCode> DataConnectionImpl<T>::registerForAllBearerAllocationUpdates(
    const sp<connection::V1_0::IDataConnectionIndication>& cb
)
{
    Log::getInstance().d("DataConnectionImpl::registerForBearerAllocationUpdates");
    registerIndicationCb(cb);
    return StatusCode::OK;
}

template <class T>
void DataConnectionImpl<T>::clearResponseFunctions(uint64_t cookie) {
    registeredResponsesMutex.lock();
    registeredResponses.erase(cookie);
    registeredResponsesMutex.unlock();
    registeredIndicationsMutex.lock();
    registeredIndications.erase(cookie);
    if (registeredIndications.empty()) {
        auto requestMsg = std::make_shared<RegisterBearerAllocationUpdateRequestMessage>(false);
        requestMsg->dispatch();
    }
    registeredIndicationsMutex.unlock();
}

template <class T>
void DataConnectionImpl<T>::transpose(
    const AllocatedBearer_t& connection,
    AllocatedBearers& bearers
) {
    bearers.cid = connection.cid;
    bearers.apn = connection.apn;
    std::vector<BearerInfo> dataBearers;
    for (auto connectionBearers : connection.bearers) {
        BearerInfo bearerInfo;
        bearerInfo.bearerId = connectionBearers.bearerId;
        transpose(connectionBearers.uplink, bearerInfo.uplink);
        transpose(connectionBearers.downlink, bearerInfo.downlink);
        dataBearers.push_back(bearerInfo);
    }
    bearers.bearers = dataBearers;
}

template <class T>
void DataConnectionImpl<T>::transpose(
    const RatType_t& commonType,
    hidl_bitfield<RatType>& ratType
) {
    switch (commonType) {
        case RatType_t::RAT_4G:
            ratType = static_cast<uint32_t>(RatType::RAT_4G);
            break;
        case RatType_t::RAT_5G:
            ratType = static_cast<uint32_t>(RatType::RAT_5G);
            break;
        case RatType_t::RAT_SPLITED:
            ratType = static_cast<uint32_t>(RatType::RAT_4G | RatType::RAT_5G);
            break;
        case RatType_t::RAT_UNSPECIFIED:
        default:
            ratType = static_cast<uint32_t>(RatType::UNSPECIFIED);
            break;
    }
}

template <class T>
void DataConnectionImpl<T>::transpose(
    const ResponseError_t& commonError,
    ErrorReason& halError
) {
    switch (commonError) {
        case ResponseError_t::NO_ERROR:
            halError = ErrorReason::NO_ERROR;
            break;
        case ResponseError_t::CALL_NOT_AVAILABLE:
            halError = ErrorReason::CALL_NOT_AVAILABLE;
            break;
        default:
            halError = ErrorReason::HARDWARE_ERROR;
            break;
    }
}

}

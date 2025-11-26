/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include <cutils/properties.h>
#include <vendor/qti/hardware/data/connection/1.1/IDataConnection.h>
#include "DataConnectionImpl_1_0.h"

namespace vendor::qti::hardware::data::connection::V1_1::impl {

template <class T>
class DataConnectionImpl : public connection::V1_0::impl::DataConnectionImpl<T> {
public:
    DataConnectionImpl() {}
    virtual ~DataConnectionImpl() {}
    Return<void> getConfig(const hidl_string& key,const hidl_string& defaultValue,
        connection::V1_1::IDataConnection::getConfig_cb _hidl_cb) override;
    static const DataHalServiceImplVersion &getVersion();
};

template<>
DataConnectionImpl<connection::V1_1::IDataConnection>::DataConnectionImpl();

template<>
const DataHalServiceImplVersion &DataConnectionImpl<connection::V1_1::IDataConnection>::getVersion();

/**
 * DataConnectionImpl::getConfig()
 *
 * @brief
 * This API queries the vendor property value
 * and returns it to the client
 */
template <class T>
Return<void> DataConnectionImpl<T>::getConfig(const hidl_string& key, const hidl_string& defaultValue,
    connection::V1_1::IDataConnection::getConfig_cb _hidl_cb)
{
    Log::getInstance().d("DataConnectionImpl::getConfig");
    char prop[PROPERTY_VALUE_MAX] = {'\0'};
    property_get(key.c_str(), prop, defaultValue.c_str());
    std::string configValue(prop);
    _hidl_cb(configValue);
    return Void();
}

}

/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "DataConnectionImpl_1_1.h"

namespace vendor::qti::hardware::data::connection::V1_1::impl {

template <>
DataConnectionImpl<connection::V1_1::IDataConnection>::DataConnectionImpl():
    connection::V1_0::impl::DataConnectionImpl<connection::V1_1::IDataConnection>() {}

template <>
const DataHalServiceImplVersion& DataConnectionImpl<connection::V1_1::IDataConnection>::getVersion() {
  static DataHalServiceImplVersion version(1, 1);
  return version;
}

static void __attribute__((constructor)) registerDataConnectionImpl();
void registerDataConnectionImpl() {
  QCRIL_LOG_INFO("Calling registerDataConnectionImpl");
  getHalServiceImplFactory<DataConnectionServiceBase>().registerImpl<DataConnectionImpl<connection::V1_1::IDataConnection>>();
}

}
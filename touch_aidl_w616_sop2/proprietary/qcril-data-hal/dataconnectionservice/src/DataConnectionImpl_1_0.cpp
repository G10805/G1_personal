/*
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "DataConnectionImpl_1_0.h"

namespace vendor::qti::hardware::data::connection::V1_0::impl {

template <>
DataConnectionImpl<connection::V1_0::IDataConnection>::DataConnectionImpl():
    DataConnectionServiceBase() {}

template <>
const DataHalServiceImplVersion& DataConnectionImpl<connection::V1_0::IDataConnection>::getVersion() {
  static DataHalServiceImplVersion version(1, 0);
  return version;
}

static void __attribute__((constructor)) registerDataConnectionImpl_1_0();
void registerDataConnectionImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerDataConnectionImpl_1_0");
  getHalServiceImplFactory<DataConnectionServiceBase>().registerImpl<DataConnectionImpl<connection::V1_0::IDataConnection>>();
}

}
/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "interfaces/common.h"
#include <string>
#include <vector>

namespace qcril {
namespace interfaces {

struct RilGetModemActivityResult_t: public qcril::interfaces::BasePayload {
  RIL_ActivityStatsInfo respData;
};

struct RilGetBaseBandResult_t: public qcril::interfaces::BasePayload {
  std::string version;
  template<typename T>
        inline RilGetBaseBandResult_t(T&& _version):
                    version(std::forward<T>(_version)) {}
};

struct RilDeviceIdentityResult_t: public qcril::interfaces::BasePayload {
  std::string imei;
  std::string imeisv;
  std::string esn;
  std::string meid;
  template<typename T1, typename T2, typename T3, typename T4>
  inline RilDeviceIdentityResult_t(T1&& _imei, T2&& _imeisv, T3&& _esn, T4&& _meid):
        imei(std::forward<T1>(_imei)), imeisv(std::forward<T2>(_imeisv)),
        esn(std::forward<T3>(_esn)), meid(std::forward<T4>(_meid)) {}
};

enum class ImeiType {
  IMEI_TYPE_INVALID,
  IMEI_TYPE_PRIMARY,
  IMEI_TYPE_SECONDARY,
};

class RilGetImeiInfoResult_t: public qcril::interfaces::BasePayload {
private:
  ImeiType mImeiType;
  std::string mImei;

public:
  RilGetImeiInfoResult_t() {
    mImeiType = qcril::interfaces::ImeiType::IMEI_TYPE_INVALID;
    mImei = "";
  }

  void setImei(std::string imei) { mImei = imei; }
  void setImeiType(ImeiType imeiType) { mImeiType = imeiType; }

  ImeiType getImeiType() { return mImeiType; }
  std::string getImei() { return mImei; }
};
}  // namespace interfaces
}  // namespace qcril

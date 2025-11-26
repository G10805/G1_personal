/*==============================================================================
              Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
              All Rights Reserved.
              Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#define TAG "IWLAN"
#include "IWlanImpl_1_0.h"
#include <framework/Log.h>

#include "DataModule.h"

#define SPACE_DELIMITER 0x20

namespace vendor::qti::hardware::data::iwlan::V1_0::impl {

template <>
const DataHalServiceImplVersion& IWlanImpl<iwlan::V1_0::IIWlan>::getVersion() {
  static DataHalServiceImplVersion version(1, 0);
  return version;
}

static void __attribute__((constructor)) registerIWlanImpl_1_0();
void registerIWlanImpl_1_0() {
  QCRIL_LOG_INFO("Calling registerIWlanImpl_1_0");
  getHalServiceImplFactory<IWlanServiceBase>().registerImpl<IWlanImpl<iwlan::V1_0::IIWlan>>();
}

std::vector<hidl_string> convertAddrStringToHidlStringVector(const std::string &addr) {
    string tmpString;
    char delimiter = SPACE_DELIMITER;
    vector<hidl_string> hidlAddressesVector;
    stringstream ssAddresses(addr);
    while(getline(ssAddresses, tmpString, delimiter)) {
        hidlAddressesVector.push_back(tmpString);
    }
    return hidlAddressesVector;
}

radio::V1_4::PdpProtocolType convertStringToPdpProtocolType(const std::string &type) {
    radio::V1_4::PdpProtocolType protocolType = radio::V1_4::PdpProtocolType::UNKNOWN;

    if (type.compare("IP") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::IP;
    }
    else if (type.compare("IPV6") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::IPV6;
    }
    else if (type.compare("IPV4V6") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::IPV4V6;
    }
    else if (type.compare("PPP") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::PPP;
    }
    else if (type.compare("NON_IP") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::NON_IP;
    }
    else if (type.compare("UNSTRUCTURED") == 0) {
        protocolType = radio::V1_4::PdpProtocolType::UNSTRUCTURED;
    }
    else {
        protocolType = radio::V1_4::PdpProtocolType::UNKNOWN;
    }

    return protocolType;
}

std::string convertPdpProtocolTypeToString(const radio::V1_4::PdpProtocolType protocol) {
    string protocolType;
    switch(protocol) {
        case radio::V1_4::PdpProtocolType::IP:
        protocolType = string("IP");
        break;

        case radio::V1_4::PdpProtocolType::IPV6:
        protocolType = string("IPV6");
        break;

        case radio::V1_4::PdpProtocolType::IPV4V6:
        protocolType = string("IPV4V6");
        break;

        case radio::V1_4::PdpProtocolType::PPP:
        protocolType = string("PPP");
        break;

        case radio::V1_4::PdpProtocolType::UNKNOWN:
        case radio::V1_4::PdpProtocolType::NON_IP:
        case radio::V1_4::PdpProtocolType::UNSTRUCTURED:
        default:
        protocolType = string("UNKNOWN");
        break;
    }
    return protocolType;
}

rildata::DataProfileInfo_t convertHidlDataProfileInfoToRil(const radio::V1_4::DataProfileInfo& profile) {
    rildata::DataProfileInfo_t rilProfile = {};
    rilProfile.profileId = (rildata::DataProfileId_t)profile.profileId;
    rilProfile.apn = profile.apn;
    rilProfile.protocol = convertPdpProtocolTypeToString(profile.protocol);
    rilProfile.roamingProtocol = convertPdpProtocolTypeToString(profile.roamingProtocol);
    rilProfile.authType = (rildata::ApnAuthType_t)profile.authType;
    rilProfile.username = profile.user;
    rilProfile.password = profile.password;
    rilProfile.dataProfileInfoType = (rildata::DataProfileInfoType_t)profile.type;
    rilProfile.maxConnsTime = profile.maxConnsTime;
    rilProfile.maxConns = profile.maxConns;
    rilProfile.waitTime = profile.waitTime;
    rilProfile.enableProfile = profile.enabled;
    rilProfile.supportedApnTypesBitmap = (rildata::ApnTypes_t)profile.supportedApnTypesBitmap;
    rilProfile.bearerBitmap = (rildata::RadioAccessFamily_t)profile.bearerBitmap;
    rilProfile.mtu = profile.mtu;
    rilProfile.preferred = profile.preferred;
    rilProfile.persistent = profile.persistent;
    return rilProfile;
}

radio::V1_0::RadioError convertMsgToRadioError(Message::Callback::Status status, ResponseError_t respErr) {
    radio::V1_0::RadioError ret = radio::V1_0::RadioError::INTERNAL_ERR;
    if (status == Message::Callback::Status::SUCCESS) {
      switch (respErr) {
        case ResponseError_t::NO_ERROR: ret = radio::V1_0::RadioError::NONE; break;
        case ResponseError_t::NOT_SUPPORTED: ret = radio::V1_0::RadioError::REQUEST_NOT_SUPPORTED; break;
        case ResponseError_t::INVALID_ARGUMENT: ret = radio::V1_0::RadioError::INVALID_ARGUMENTS; break;
        default: break;
      }
    }
    return ret;
}

void convertDCResultToHAL(DataCallResult_t result,  radio::V1_4::SetupDataCallResult& halResult) {
    halResult.cause = (::android::hardware::radio::V1_4::DataCallFailCause) result.cause;
    halResult.suggestedRetryTime = result.suggestedRetryTime;
    halResult.cid = result.cid;
    halResult.active = (::android::hardware::radio::V1_4::DataConnActiveStatus) result.active;
    halResult.type = convertStringToPdpProtocolType(result.type);
    halResult.ifname = result.ifname;
    halResult.addresses = convertAddrStringToHidlStringVector(result.addresses);
    halResult.dnses = convertAddrStringToHidlStringVector(result.dnses);
    halResult.gateways = convertAddrStringToHidlStringVector(result.gateways);
    halResult.pcscf = convertAddrStringToHidlStringVector(result.pcscf);
    halResult.mtu = result.mtu;
}

void convertQualifiedNetworksToHAL(std::vector<QualifiedNetwork_t> qnList,
           ::android::hardware::hidl_vec<::vendor::qti::hardware::data::iwlan::V1_0::QualifiedNetworks>& qNetworks) {
    qNetworks.resize(qnList.size());
    int i=0;
    for (QualifiedNetwork_t entry: qnList) {
        qNetworks[i].apnType = (::android::hardware::radio::V1_0::ApnTypes)entry.apnType;
        qNetworks[i].networks = entry.network;
        i++;
    }
}

}

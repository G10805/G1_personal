/*==============================================================================
              Copyright (c) 2021 Qualcomm Technologies, Inc.
              All Rights Reserved.
              Confidential and Proprietary - Qualcomm Technologies, Inc.
==============================================================================*/
/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#define TAG "IWLAN"
#include "IWlanImpl_1_1.h"
#include <framework/Log.h>

namespace vendor::qti::hardware::data::iwlan::V1_1::impl {

vector<radio::V1_5::LinkAddress> convertLinkAddressToHidlVector(vector<rildata::LinkAddress_t> addresses) {
    vector<radio::V1_5::LinkAddress> hidlAddressesVector;
    for (rildata::LinkAddress_t addr : addresses) {
        radio::V1_5::LinkAddress hidlAddr = {
            .address = addr.address,
            .properties = (int)addr.properties,
            .deprecationTime = addr.deprecationTime,
            .expirationTime = addr.expirationTime,
        };
        hidlAddressesVector.push_back(hidlAddr);
    }
    return hidlAddressesVector;
}

void convertTrafficDescriptor(radio::V1_6::TrafficDescriptor& trafficDescriptor,
                              const rildata::TrafficDescriptor_t& result)
{
  if (result.dnn.has_value()) {
    trafficDescriptor.dnn.value(result.dnn.value());
  }
  if (result.osAppId.has_value()) {
    radio::V1_6::OsAppId id;
    id.osAppId = result.osAppId.value();
    trafficDescriptor.osAppId.value(id);
  }
}

void convertDCResultToHAL(DataCallResult_t result,  radio::V1_6::SetupDataCallResult& halResult) {
    halResult.cause = (::android::hardware::radio::V1_6::DataCallFailCause) result.cause;
    halResult.suggestedRetryTime = result.suggestedRetryTime;
    halResult.cid = result.cid;
    halResult.active = (::android::hardware::radio::V1_4::DataConnActiveStatus) result.active;
    halResult.type = V1_0::impl::convertStringToPdpProtocolType(result.type);
    halResult.ifname = result.ifname;
    halResult.addresses = convertLinkAddressToHidlVector(result.linkAddresses);
    halResult.dnses = V1_0::impl::convertAddrStringToHidlStringVector(result.dnses);
    halResult.gateways = V1_0::impl::convertAddrStringToHidlStringVector(result.gateways);
    halResult.pcscf = V1_0::impl::convertAddrStringToHidlStringVector(result.pcscf);
    halResult.mtuV4 = result.mtuV4;
    halResult.mtuV6 = result.mtuV6;
    halResult.handoverFailureMode = (::android::hardware::radio::V1_6::HandoverFailureMode) result.handoverFailureMode;
    unsigned int tdSize = result.trafficDescriptors.size();
    if(tdSize > 0) {
      halResult.trafficDescriptors.resize(tdSize);
      for(int i = 0; i< tdSize; i++) {
        convertTrafficDescriptor(halResult.trafficDescriptors[i], result.trafficDescriptors[i]);
      }
    }
}

rildata::DataProfileInfo_t convertHidlDataProfileInfoToRil(const radio::V1_5::DataProfileInfo& profile) {
    rildata::DataProfileInfo_t rilProfile = {};
    rilProfile.profileId = (rildata::DataProfileId_t)profile.profileId;
    rilProfile.apn = profile.apn;
    rilProfile.protocol = V1_0::impl::convertPdpProtocolTypeToString(profile.protocol);
    rilProfile.roamingProtocol = V1_0::impl::convertPdpProtocolTypeToString(profile.roamingProtocol);
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
    rilProfile.mtu = profile.mtuV4;
    rilProfile.preferred = profile.preferred;
    rilProfile.persistent = profile.persistent;
    return rilProfile;
}

template <>
const DataHalServiceImplVersion& IWlanImpl<iwlan::V1_1::IIWlan>::getVersion() {
  static DataHalServiceImplVersion version(1, 1);
  return version;
}

static void __attribute__((constructor)) registerIWlanImpl_1_1();
void registerIWlanImpl_1_1() {
  QCRIL_LOG_INFO("Calling registerIWlanImpl_1_1");
  getHalServiceImplFactory<IWlanServiceBase>().registerImpl<IWlanImpl<iwlan::V1_1::IIWlan>>();
}

}

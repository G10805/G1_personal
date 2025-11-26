/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "supplicant_sta_network_msg.h"

#include "AuthAlgMask.pb.h"
#include "DppConnectionKeys.pb.h"
#include "EapMethod.pb.h"
#include "EapPhase2Method.pb.h"
#include "GroupCipherMask.pb.h"
#include "GroupMgmtCipherMask.pb.h"
#include "GsmRand.pb.h"
#include "HalStatus.pb.h"
#include "ISupplicantStaNetwork.pb.h"
#include "ISupplicantStaNetworkCallback.pb.h"
#include "IfaceType.pb.h"
#include "KeyMgmtMask.pb.h"
#include "NetworkRequestEapSimGsmAuthParams.pb.h"
#include "NetworkRequestEapSimUmtsAuthParams.pb.h"
#include "NetworkResponseEapSimGsmAuthParams.pb.h"
#include "NetworkResponseEapSimUmtsAuthParams.pb.h"
#include "OcspType.pb.h"
#include "PairwiseCipherMask.pb.h"
#include "ProtoMask.pb.h"
#include "SaeH2eMode.pb.h"
#include "TlsVersion.pb.h"
#include "TransitionDisableIndication.pb.h"

using EnableReq = ::wifi::supplicant::ISupplicantStaNetwork_EnableReq;
using EnableSaePkOnlyModeReq = ::wifi::supplicant::ISupplicantStaNetwork_EnableSaePkOnlyModeReq;
using EnableTlsSuiteBEapPhase1ParamReq = ::wifi::supplicant::ISupplicantStaNetwork_EnableTlsSuiteBEapPhase1ParamReq;
using GetAuthAlgCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetAuthAlgCfm;
using GetBssidCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetBssidCfm;
using GetEapAltSubjectMatchCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapAltSubjectMatchCfm;
using GetEapAnonymousIdentityCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapAnonymousIdentityCfm;
using GetEapCACertCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapCACertCfm;
using GetEapCAPathCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapCAPathCfm;
using GetEapClientCertCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapClientCertCfm;
using GetEapDomainSuffixMatchCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapDomainSuffixMatchCfm;
using GetEapEngineCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapEngineCfm;
using GetEapEngineIdCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapEngineIdCfm;
using GetEapIdentityCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapIdentityCfm;
using GetEapMethodCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapMethodCfm;
using GetEapPasswordCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapPasswordCfm;
using GetEapPhase2MethodCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapPhase2MethodCfm;
using GetEapPrivateKeyIdCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapPrivateKeyIdCfm;
using GetEapSubjectMatchCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEapSubjectMatchCfm;
using GetEdmgCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetEdmgCfm;
using GetGroupCipherCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetGroupCipherCfm;
using GetGroupMgmtCipherCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetGroupMgmtCipherCfm;
using GetIdCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetIdCfm;
using GetIdStrCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetIdStrCfm;
using GetInterfaceNameCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetInterfaceNameCfm;
using GetKeyMgmtCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetKeyMgmtCfm;
using GetOcspCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetOcspCfm;
using GetPairwiseCipherCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetPairwiseCipherCfm;
using GetProtoCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetProtoCfm;
using GetPskCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetPskCfm;
using GetPskPassphraseCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetPskPassphraseCfm;
using GetRequirePmfCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetRequirePmfCfm;
using GetSaePasswordCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetSaePasswordCfm;
using GetSaePasswordIdCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetSaePasswordIdCfm;
using GetScanSsidCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetScanSsidCfm;
using GetSsidCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetSsidCfm;
using GetTypeCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetTypeCfm;
using GetWapiCertSuiteCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetWapiCertSuiteCfm;
using GetWepKeyCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetWepKeyCfm;
using GetWepKeyReq = ::wifi::supplicant::ISupplicantStaNetwork_GetWepKeyReq;
using GetWepTxKeyIdxCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetWepTxKeyIdxCfm;
using GetWpsNfcConfigurationTokenCfm = ::wifi::supplicant::ISupplicantStaNetwork_GetWpsNfcConfigurationTokenCfm;
using OnNetworkEapSimGsmAuthRequestInd = ::wifi::supplicant::ISupplicantStaNetworkCallback_OnNetworkEapSimGsmAuthRequestInd;
using OnNetworkEapSimUmtsAuthRequestInd = ::wifi::supplicant::ISupplicantStaNetworkCallback_OnNetworkEapSimUmtsAuthRequestInd;
using OnServerCertificateAvailableInd = ::wifi::supplicant::ISupplicantStaNetworkCallback_OnServerCertificateAvailableInd;
using OnTransitionDisableInd = ::wifi::supplicant::ISupplicantStaNetworkCallback_OnTransitionDisableInd;
using SendNetworkEapIdentityResponseReq = ::wifi::supplicant::ISupplicantStaNetwork_SendNetworkEapIdentityResponseReq;
using SendNetworkEapSimGsmAuthResponseReq = ::wifi::supplicant::ISupplicantStaNetwork_SendNetworkEapSimGsmAuthResponseReq;
using SendNetworkEapSimUmtsAuthResponseReq = ::wifi::supplicant::ISupplicantStaNetwork_SendNetworkEapSimUmtsAuthResponseReq;
using SendNetworkEapSimUmtsAutsResponseReq = ::wifi::supplicant::ISupplicantStaNetwork_SendNetworkEapSimUmtsAutsResponseReq;
using SetAuthAlgReq = ::wifi::supplicant::ISupplicantStaNetwork_SetAuthAlgReq;
using SetBssidReq = ::wifi::supplicant::ISupplicantStaNetwork_SetBssidReq;
using SetDppKeysReq = ::wifi::supplicant::ISupplicantStaNetwork_SetDppKeysReq;
using SetEapAltSubjectMatchReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapAltSubjectMatchReq;
using SetEapAnonymousIdentityReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapAnonymousIdentityReq;
using SetEapCACertReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapCACertReq;
using SetEapCAPathReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapCAPathReq;
using SetEapClientCertReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapClientCertReq;
using SetEapDomainSuffixMatchReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapDomainSuffixMatchReq;
using SetEapEncryptedImsiIdentityReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapEncryptedImsiIdentityReq;
using SetEapEngineIDReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapEngineIDReq;
using SetEapEngineReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapEngineReq;
using SetEapErpReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapErpReq;
using SetEapIdentityReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapIdentityReq;
using SetEapMethodReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapMethodReq;
using SetEapPasswordReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapPasswordReq;
using SetEapPhase2MethodReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapPhase2MethodReq;
using SetEapPrivateKeyIdReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapPrivateKeyIdReq;
using SetEapSubjectMatchReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEapSubjectMatchReq;
using SetEdmgReq = ::wifi::supplicant::ISupplicantStaNetwork_SetEdmgReq;
using SetGroupCipherReq = ::wifi::supplicant::ISupplicantStaNetwork_SetGroupCipherReq;
using SetGroupMgmtCipherReq = ::wifi::supplicant::ISupplicantStaNetwork_SetGroupMgmtCipherReq;
using SetIdStrReq = ::wifi::supplicant::ISupplicantStaNetwork_SetIdStrReq;
using SetKeyMgmtReq = ::wifi::supplicant::ISupplicantStaNetwork_SetKeyMgmtReq;
using SetMinimumTlsVersionEapPhase1ParamReq = ::wifi::supplicant::ISupplicantStaNetwork_SetMinimumTlsVersionEapPhase1ParamReq;
using SetOcspReq = ::wifi::supplicant::ISupplicantStaNetwork_SetOcspReq;
using SetPairwiseCipherReq = ::wifi::supplicant::ISupplicantStaNetwork_SetPairwiseCipherReq;
using SetPmkCacheReq = ::wifi::supplicant::ISupplicantStaNetwork_SetPmkCacheReq;
using SetProactiveKeyCachingReq = ::wifi::supplicant::ISupplicantStaNetwork_SetProactiveKeyCachingReq;
using SetProtoReq = ::wifi::supplicant::ISupplicantStaNetwork_SetProtoReq;
using SetPskPassphraseReq = ::wifi::supplicant::ISupplicantStaNetwork_SetPskPassphraseReq;
using SetPskReq = ::wifi::supplicant::ISupplicantStaNetwork_SetPskReq;
using SetRequirePmfReq = ::wifi::supplicant::ISupplicantStaNetwork_SetRequirePmfReq;
using SetRoamingConsortiumSelectionReq = ::wifi::supplicant::ISupplicantStaNetwork_SetRoamingConsortiumSelectionReq;
using SetSaeH2eModeReq = ::wifi::supplicant::ISupplicantStaNetwork_SetSaeH2eModeReq;
using SetSaePasswordIdReq = ::wifi::supplicant::ISupplicantStaNetwork_SetSaePasswordIdReq;
using SetSaePasswordReq = ::wifi::supplicant::ISupplicantStaNetwork_SetSaePasswordReq;
using SetScanSsidReq = ::wifi::supplicant::ISupplicantStaNetwork_SetScanSsidReq;
using SetSsidReq = ::wifi::supplicant::ISupplicantStaNetwork_SetSsidReq;
using SetStrictConservativePeerModeReq = ::wifi::supplicant::ISupplicantStaNetwork_SetStrictConservativePeerModeReq;
using SetUpdateIdentifierReq = ::wifi::supplicant::ISupplicantStaNetwork_SetUpdateIdentifierReq;
using SetWapiCertSuiteReq = ::wifi::supplicant::ISupplicantStaNetwork_SetWapiCertSuiteReq;
using SetWepKeyReq = ::wifi::supplicant::ISupplicantStaNetwork_SetWepKeyReq;
using SetWepTxKeyIdxReq = ::wifi::supplicant::ISupplicantStaNetwork_SetWepTxKeyIdxReq;

using AuthAlgMask_P = ::wifi::supplicant::aam::AuthAlgMask;
using DppConnectionKeys_P = ::wifi::supplicant::DppConnectionKeys;
using EapMethod_P = ::wifi::supplicant::em::EapMethod;
using EapPhase2Method_P = ::wifi::supplicant::epm::EapPhase2Method;
using GroupCipherMask_P = ::wifi::supplicant::gcm::GroupCipherMask;
using GroupMgmtCipherMask_P = ::wifi::supplicant::GroupMgmtCipherMask;
using GsmRand_P = ::wifi::supplicant::GsmRand;
using HalStatus_P = ::wifi::supplicant::HalStatus;
using IfaceType_P = ::wifi::supplicant::it::IfaceType;
using KeyMgmtMask_P = ::wifi::supplicant::kmm::KeyMgmtMask;
using NetworkRequestEapSimGsmAuthParams_P = ::wifi::supplicant::NetworkRequestEapSimGsmAuthParams;
using NetworkRequestEapSimUmtsAuthParams_P = ::wifi::supplicant::NetworkRequestEapSimUmtsAuthParams;
using NetworkResponseEapSimGsmAuthParams_P = ::wifi::supplicant::NetworkResponseEapSimGsmAuthParams;
using NetworkResponseEapSimUmtsAuthParams_P = ::wifi::supplicant::NetworkResponseEapSimUmtsAuthParams;
using OcspType_P = ::wifi::supplicant::ot::OcspType;
using PairwiseCipherMask_P = ::wifi::supplicant::pcm::PairwiseCipherMask;
using ProtoMask_P = ::wifi::supplicant::pm::ProtoMask;
using SaeH2eMode_P = ::wifi::supplicant::shm::SaeH2eMode;
using TlsVersion_P = ::wifi::supplicant::tv::TlsVersion;
using TransitionDisableIndication_P = ::wifi::supplicant::TransitionDisableIndication;


bool SupplicantStaNetworkSerializeDisableReq(vector<uint8_t>& payload)
{
    return true;
}

static void HalStatus2Message(const HalStatusParam& status, HalStatus_P& msgSupplicantStaNetwork)
{
    msgSupplicantStaNetwork.set_status(status.status);
    msgSupplicantStaNetwork.set_info(status.info);
}

bool SupplicantStaNetworkSerializeDisableCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeEnableReq(bool noConnect, vector<uint8_t>& payload)
{
    EnableReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_noconnect(noConnect);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeEnableCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeEnableSaePkOnlyModeReq(bool enable, vector<uint8_t>& payload)
{
    EnableSaePkOnlyModeReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeEnableSaePkOnlyModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamReq(bool enable, vector<uint8_t>& payload)
{
    EnableTlsSuiteBEapPhase1ParamReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetAuthAlgReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetAuthAlgCfm(const HalStatusParam& status, AuthAlgMask result, vector<uint8_t>& payload)
{
    GetAuthAlgCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((AuthAlgMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetBssidReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetBssidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetBssidCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapAltSubjectMatchReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapAltSubjectMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapAltSubjectMatchCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapAnonymousIdentityReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapAnonymousIdentityCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetEapAnonymousIdentityCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapCACertReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapCACertCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapCACertCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapCAPathReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapCAPathCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapCAPathCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapClientCertReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapClientCertCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapClientCertCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapDomainSuffixMatchReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapDomainSuffixMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapDomainSuffixMatchCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapEngineReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapEngineCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    GetEapEngineCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapEngineIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapEngineIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapEngineIdCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapIdentityReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapIdentityCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetEapIdentityCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapMethodReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapMethodCfm(const HalStatusParam& status, EapMethod result, vector<uint8_t>& payload)
{
    GetEapMethodCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((EapMethod_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapPasswordReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapPasswordCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetEapPasswordCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapPhase2MethodReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapPhase2MethodCfm(const HalStatusParam& status, EapPhase2Method result, vector<uint8_t>& payload)
{
    GetEapPhase2MethodCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((EapPhase2Method_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapPrivateKeyIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapPrivateKeyIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapPrivateKeyIdCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEapSubjectMatchReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEapSubjectMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetEapSubjectMatchCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetEdmgReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetEdmgCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    GetEdmgCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetGroupCipherReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetGroupCipherCfm(const HalStatusParam& status, GroupCipherMask result, vector<uint8_t>& payload)
{
    GetGroupCipherCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((GroupCipherMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetGroupMgmtCipherReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetGroupMgmtCipherCfm(const HalStatusParam& status, GroupMgmtCipherMask result, vector<uint8_t>& payload)
{
    GetGroupMgmtCipherCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((GroupMgmtCipherMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetIdCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetIdStrReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetIdStrCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetIdStrCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetInterfaceNameReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetInterfaceNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetInterfaceNameCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetKeyMgmtReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetKeyMgmtCfm(const HalStatusParam& status, KeyMgmtMask result, vector<uint8_t>& payload)
{
    GetKeyMgmtCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((KeyMgmtMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetOcspReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetOcspCfm(const HalStatusParam& status, OcspType result, vector<uint8_t>& payload)
{
    GetOcspCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((OcspType_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetPairwiseCipherReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetPairwiseCipherCfm(const HalStatusParam& status, PairwiseCipherMask result, vector<uint8_t>& payload)
{
    GetPairwiseCipherCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((PairwiseCipherMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetProtoReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetProtoCfm(const HalStatusParam& status, ProtoMask result, vector<uint8_t>& payload)
{
    GetProtoCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((ProtoMask_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetPskReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetPskCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetPskCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetPskPassphraseReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetPskPassphraseCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetPskPassphraseCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetRequirePmfReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetRequirePmfCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    GetRequirePmfCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetSaePasswordReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetSaePasswordCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetSaePasswordCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetSaePasswordIdReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetSaePasswordIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetSaePasswordIdCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetScanSsidReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetScanSsidCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload)
{
    GetScanSsidCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetSsidReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetSsidCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetTypeReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload)
{
    GetTypeCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result((IfaceType_P) result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetWapiCertSuiteReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetWapiCertSuiteCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload)
{
    GetWapiCertSuiteCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetWepKeyReq(int32_t keyIdx, vector<uint8_t>& payload)
{
    GetWepKeyReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_keyidx(keyIdx);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetWepKeyCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetWepKeyCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetWepTxKeyIdxReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetWepTxKeyIdxCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload)
{
    GetWepTxKeyIdxCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    msgSupplicantStaNetwork.set_result(result);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetWpsNfcConfigurationTokenCfm msgSupplicantStaNetwork;
    HalStatus_P* status_p = msgSupplicantStaNetwork.mutable_status();
    HalStatus2Message(status, *status_p);
    string resultStr;
    Vector2String(result, resultStr);
    msgSupplicantStaNetwork.set_result(resultStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeRegisterCallbackReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeRegisterCallbackCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSelectReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSelectCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseReq(const vector<uint8_t>& identity, const vector<uint8_t>& encryptedIdentity, vector<uint8_t>& payload)
{
    SendNetworkEapIdentityResponseReq msgSupplicantStaNetwork;
    string identityStr;
    Vector2String(identity, identityStr);
    msgSupplicantStaNetwork.set_identity(identityStr);
    string encryptedIdentityStr;
    Vector2String(encryptedIdentity, encryptedIdentityStr);
    msgSupplicantStaNetwork.set_encryptedidentity(encryptedIdentityStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NetworkResponseEapSimGsmAuthParams2Message(const NetworkResponseEapSimGsmAuthParams& params, NetworkResponseEapSimGsmAuthParams_P& msgSupplicantStaNetwork)
{
    string kc;
    Vector2String(params.kc, kc);
    msgSupplicantStaNetwork.set_kc(kc);
    string sres;
    Vector2String(params.sres, sres);
    msgSupplicantStaNetwork.set_sres(sres);
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseReq(const vector<NetworkResponseEapSimGsmAuthParams>& params, vector<uint8_t>& payload)
{
    SendNetworkEapSimGsmAuthResponseReq msgSupplicantStaNetwork;
    int size_params = params.size();
    if (0 < size_params)
    {
        for (int index = 0; index < size_params; index++)
        {
            NetworkResponseEapSimGsmAuthParams_P *params_p = msgSupplicantStaNetwork.add_params();
            NetworkResponseEapSimGsmAuthParams2Message(params[index], *params_p);
        }
    }
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureReq(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureCfm(vector<uint8_t>& payload)
{
    return true;
}

static void NetworkResponseEapSimUmtsAuthParams2Message(const NetworkResponseEapSimUmtsAuthParams& params, NetworkResponseEapSimUmtsAuthParams_P& msgSupplicantStaNetwork)
{
    string res;
    Vector2String(params.res, res);
    msgSupplicantStaNetwork.set_res(res);
    string ik;
    Vector2String(params.ik, ik);
    msgSupplicantStaNetwork.set_ik(ik);
    string ck;
    Vector2String(params.ck, ck);
    msgSupplicantStaNetwork.set_ck(ck);
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseReq(const NetworkResponseEapSimUmtsAuthParams& params, vector<uint8_t>& payload)
{
    SendNetworkEapSimUmtsAuthResponseReq msgSupplicantStaNetwork;
    NetworkResponseEapSimUmtsAuthParams_P* params_p = msgSupplicantStaNetwork.mutable_params();
    NetworkResponseEapSimUmtsAuthParams2Message(params, *params_p);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseReq(const vector<uint8_t>& auts, vector<uint8_t>& payload)
{
    SendNetworkEapSimUmtsAutsResponseReq msgSupplicantStaNetwork;
    string autsStr;
    Vector2String(auts, autsStr);
    msgSupplicantStaNetwork.set_auts(autsStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetAuthAlgReq(AuthAlgMask authAlgMask, vector<uint8_t>& payload)
{
    SetAuthAlgReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_authalgmask((AuthAlgMask_P) authAlgMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetAuthAlgCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetBssidReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload)
{
    SetBssidReq msgSupplicantStaNetwork;
    string bssidStr;
    Vector2String(bssid, bssidStr);
    msgSupplicantStaNetwork.set_bssid(bssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetBssidCfm(vector<uint8_t>& payload)
{
    return true;
}

static void DppConnectionKeys2Message(const DppConnectionKeys& keys, DppConnectionKeys_P& msgSupplicantStaNetwork)
{
    string connector;
    Vector2String(keys.connector, connector);
    msgSupplicantStaNetwork.set_connector(connector);
    string cSign;
    Vector2String(keys.cSign, cSign);
    msgSupplicantStaNetwork.set_csign(cSign);
    string netAccessKey;
    Vector2String(keys.netAccessKey, netAccessKey);
    msgSupplicantStaNetwork.set_netaccesskey(netAccessKey);
}

bool SupplicantStaNetworkSerializeSetDppKeysReq(const DppConnectionKeys& keys, vector<uint8_t>& payload)
{
    SetDppKeysReq msgSupplicantStaNetwork;
    DppConnectionKeys_P* keys_p = msgSupplicantStaNetwork.mutable_keys();
    DppConnectionKeys2Message(keys, *keys_p);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetDppKeysCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapAltSubjectMatchReq(const string& match, vector<uint8_t>& payload)
{
    SetEapAltSubjectMatchReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_match(match);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapAltSubjectMatchCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapAnonymousIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload)
{
    SetEapAnonymousIdentityReq msgSupplicantStaNetwork;
    string identityStr;
    Vector2String(identity, identityStr);
    msgSupplicantStaNetwork.set_identity(identityStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapAnonymousIdentityCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapCACertReq(const string& path, vector<uint8_t>& payload)
{
    SetEapCACertReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_path(path);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapCACertCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapCAPathReq(const string& path, vector<uint8_t>& payload)
{
    SetEapCAPathReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_path(path);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapCAPathCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapClientCertReq(const string& path, vector<uint8_t>& payload)
{
    SetEapClientCertReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_path(path);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapClientCertCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapDomainSuffixMatchReq(const string& match, vector<uint8_t>& payload)
{
    SetEapDomainSuffixMatchReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_match(match);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapDomainSuffixMatchCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload)
{
    SetEapEncryptedImsiIdentityReq msgSupplicantStaNetwork;
    string identityStr;
    Vector2String(identity, identityStr);
    msgSupplicantStaNetwork.set_identity(identityStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapEngineReq(bool enable, vector<uint8_t>& payload)
{
    SetEapEngineReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapEngineCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapEngineIDReq(const string& id, vector<uint8_t>& payload)
{
    SetEapEngineIDReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapEngineIDCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapErpReq(bool enable, vector<uint8_t>& payload)
{
    SetEapErpReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapErpCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload)
{
    SetEapIdentityReq msgSupplicantStaNetwork;
    string identityStr;
    Vector2String(identity, identityStr);
    msgSupplicantStaNetwork.set_identity(identityStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapIdentityCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapMethodReq(EapMethod method, vector<uint8_t>& payload)
{
    SetEapMethodReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_method((EapMethod_P) method);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapMethodCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapPasswordReq(const vector<uint8_t>& password, vector<uint8_t>& payload)
{
    SetEapPasswordReq msgSupplicantStaNetwork;
    string passwordStr;
    Vector2String(password, passwordStr);
    msgSupplicantStaNetwork.set_password(passwordStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapPasswordCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapPhase2MethodReq(EapPhase2Method method, vector<uint8_t>& payload)
{
    SetEapPhase2MethodReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_method((EapPhase2Method_P) method);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapPhase2MethodCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapPrivateKeyIdReq(const string& id, vector<uint8_t>& payload)
{
    SetEapPrivateKeyIdReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapPrivateKeyIdCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEapSubjectMatchReq(const string& match, vector<uint8_t>& payload)
{
    SetEapSubjectMatchReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_match(match);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEapSubjectMatchCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetEdmgReq(bool enable, vector<uint8_t>& payload)
{
    SetEdmgReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetEdmgCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetGroupCipherReq(GroupCipherMask groupCipherMask, vector<uint8_t>& payload)
{
    SetGroupCipherReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_groupciphermask((GroupCipherMask_P) groupCipherMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetGroupCipherCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetGroupMgmtCipherReq(GroupMgmtCipherMask groupMgmtCipherMask, vector<uint8_t>& payload)
{
    SetGroupMgmtCipherReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_groupmgmtciphermask((GroupMgmtCipherMask_P) groupMgmtCipherMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetGroupMgmtCipherCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetIdStrReq(const string& idStr, vector<uint8_t>& payload)
{
    SetIdStrReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_idstr(idStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetIdStrCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetKeyMgmtReq(KeyMgmtMask keyMgmtMask, vector<uint8_t>& payload)
{
    SetKeyMgmtReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_keymgmtmask((KeyMgmtMask_P) keyMgmtMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetKeyMgmtCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetOcspReq(OcspType ocspType, vector<uint8_t>& payload)
{
    SetOcspReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_ocsptype((OcspType_P) ocspType);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetOcspCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetPairwiseCipherReq(PairwiseCipherMask pairwiseCipherMask, vector<uint8_t>& payload)
{
    SetPairwiseCipherReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_pairwiseciphermask((PairwiseCipherMask_P) pairwiseCipherMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetPairwiseCipherCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetPmkCacheReq(const vector<uint8_t>& serializedEntry, vector<uint8_t>& payload)
{
    SetPmkCacheReq msgSupplicantStaNetwork;
    string serializedEntryStr;
    Vector2String(serializedEntry, serializedEntryStr);
    msgSupplicantStaNetwork.set_serializedentry(serializedEntryStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetPmkCacheCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetProactiveKeyCachingReq(bool enable, vector<uint8_t>& payload)
{
    SetProactiveKeyCachingReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetProactiveKeyCachingCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetProtoReq(ProtoMask protoMask, vector<uint8_t>& payload)
{
    SetProtoReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_protomask((ProtoMask_P) protoMask);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetProtoCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetPskReq(const vector<uint8_t>& psk, vector<uint8_t>& payload)
{
    SetPskReq msgSupplicantStaNetwork;
    string pskStr;
    Vector2String(psk, pskStr);
    msgSupplicantStaNetwork.set_psk(pskStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetPskCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetPskPassphraseReq(const string& psk, vector<uint8_t>& payload)
{
    SetPskPassphraseReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_psk(psk);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetPskPassphraseCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetRequirePmfReq(bool enable, vector<uint8_t>& payload)
{
    SetRequirePmfReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetRequirePmfCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetSaeH2eModeReq(SaeH2eMode mode, vector<uint8_t>& payload)
{
    SetSaeH2eModeReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_mode((SaeH2eMode_P) mode);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetSaeH2eModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetSaePasswordReq(const string& saePassword, vector<uint8_t>& payload)
{
    SetSaePasswordReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_saepassword(saePassword);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetSaePasswordCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetSaePasswordIdReq(const string& saePasswordId, vector<uint8_t>& payload)
{
    SetSaePasswordIdReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_saepasswordid(saePasswordId);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetSaePasswordIdCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetScanSsidReq(bool enable, vector<uint8_t>& payload)
{
    SetScanSsidReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetScanSsidCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetSsidReq(const vector<uint8_t>& ssid, vector<uint8_t>& payload)
{
    SetSsidReq msgSupplicantStaNetwork;
    string ssidStr;
    Vector2String(ssid, ssidStr);
    msgSupplicantStaNetwork.set_ssid(ssidStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetSsidCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetUpdateIdentifierReq(int32_t id, vector<uint8_t>& payload)
{
    SetUpdateIdentifierReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_id(id);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetUpdateIdentifierCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetWapiCertSuiteReq(const string& suite, vector<uint8_t>& payload)
{
    SetWapiCertSuiteReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_suite(suite);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetWapiCertSuiteCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetWepKeyReq(int32_t keyIdx, const vector<uint8_t>& wepKey, vector<uint8_t>& payload)
{
    SetWepKeyReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_keyidx(keyIdx);
    string wepKeyStr;
    Vector2String(wepKey, wepKeyStr);
    msgSupplicantStaNetwork.set_wepkey(wepKeyStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetWepKeyCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetWepTxKeyIdxReq(int32_t keyIdx, vector<uint8_t>& payload)
{
    SetWepTxKeyIdxReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_keyidx(keyIdx);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetWepTxKeyIdxCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionReq(const vector<uint8_t>& selectedRcoi, vector<uint8_t>& payload)
{
    SetRoamingConsortiumSelectionReq msgSupplicantStaNetwork;
    string selectedRcoiStr;
    Vector2String(selectedRcoi, selectedRcoiStr);
    msgSupplicantStaNetwork.set_selectedrcoi(selectedRcoiStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamReq(TlsVersion tlsVersion, vector<uint8_t>& payload)
{
    SetMinimumTlsVersionEapPhase1ParamReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_tlsversion((TlsVersion_P) tlsVersion);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeSetStrictConservativePeerModeReq(bool enable, vector<uint8_t>& payload)
{
    SetStrictConservativePeerModeReq msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_enable(enable);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeSetStrictConservativePeerModeCfm(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkSerializeOnNetworkEapIdentityRequestInd(vector<uint8_t>& payload)
{
    return true;
}

static void GsmRand2Message(const GsmRand& rands, GsmRand_P& msgSupplicantStaNetwork)
{
    string data;
    Vector2String(rands.data, data);
    msgSupplicantStaNetwork.set_data(data);
}

static void NetworkRequestEapSimGsmAuthParams2Message(const NetworkRequestEapSimGsmAuthParams& params, NetworkRequestEapSimGsmAuthParams_P& msgSupplicantStaNetwork)
{
    int size_rands = params.rands.size();
    if (0 < size_rands)
    {
        for (int index = 0; index < size_rands; index++)
        {
            GsmRand_P *rands_p = msgSupplicantStaNetwork.add_rands();
            GsmRand2Message(params.rands[index], *rands_p);
        }
    }
}

bool SupplicantStaNetworkSerializeOnNetworkEapSimGsmAuthRequestInd(const NetworkRequestEapSimGsmAuthParams& params, vector<uint8_t>& payload)
{
    OnNetworkEapSimGsmAuthRequestInd msgSupplicantStaNetwork;
    NetworkRequestEapSimGsmAuthParams_P* params_p = msgSupplicantStaNetwork.mutable_params();
    NetworkRequestEapSimGsmAuthParams2Message(params, *params_p);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

static void NetworkRequestEapSimUmtsAuthParams2Message(const NetworkRequestEapSimUmtsAuthParams& params, NetworkRequestEapSimUmtsAuthParams_P& msgSupplicantStaNetwork)
{
    string rand;
    Vector2String(params.rand, rand);
    msgSupplicantStaNetwork.set_rand(rand);
    string autn;
    Vector2String(params.autn, autn);
    msgSupplicantStaNetwork.set_autn(autn);
}

bool SupplicantStaNetworkSerializeOnNetworkEapSimUmtsAuthRequestInd(const NetworkRequestEapSimUmtsAuthParams& params, vector<uint8_t>& payload)
{
    OnNetworkEapSimUmtsAuthRequestInd msgSupplicantStaNetwork;
    NetworkRequestEapSimUmtsAuthParams_P* params_p = msgSupplicantStaNetwork.mutable_params();
    NetworkRequestEapSimUmtsAuthParams2Message(params, *params_p);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeOnTransitionDisableInd(TransitionDisableIndication ind, vector<uint8_t>& payload)
{
    OnTransitionDisableInd msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_ind((TransitionDisableIndication_P) ind);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeOnServerCertificateAvailableInd(int32_t depth, const vector<uint8_t>& subject, const vector<uint8_t>& certHash, const vector<uint8_t>& certBlob, vector<uint8_t>& payload)
{
    OnServerCertificateAvailableInd msgSupplicantStaNetwork;
    msgSupplicantStaNetwork.set_depth(depth);
    string subjectStr;
    Vector2String(subject, subjectStr);
    msgSupplicantStaNetwork.set_subject(subjectStr);
    string certHashStr;
    Vector2String(certHash, certHashStr);
    msgSupplicantStaNetwork.set_certhash(certHashStr);
    string certBlobStr;
    Vector2String(certBlob, certBlobStr);
    msgSupplicantStaNetwork.set_certblob(certBlobStr);
    return ProtoMessageSerialize(&msgSupplicantStaNetwork, payload);
}

bool SupplicantStaNetworkSerializeOnPermanentIdReqDeniedInd(vector<uint8_t>& payload)
{
    return true;
}

bool SupplicantStaNetworkParseDisableReq(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2HalStatus(const HalStatus_P& msgSupplicantStaNetwork, HalStatusParam& status)
{
    status.status = msgSupplicantStaNetwork.status();
    status.info = msgSupplicantStaNetwork.info();
}

bool SupplicantStaNetworkParseDisableCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseEnableReq(const uint8_t* data, size_t length, bool& param)
{
    EnableReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.noconnect();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseEnableCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseEnableSaePkOnlyModeReq(const uint8_t* data, size_t length, bool& param)
{
    EnableSaePkOnlyModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseEnableSaePkOnlyModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamReq(const uint8_t* data, size_t length, bool& param)
{
    EnableTlsSuiteBEapPhase1ParamReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetAuthAlgReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetAuthAlgCfm(const uint8_t* data, size_t length, GetAuthAlgCfmStaNetworkParam& param)
{
    GetAuthAlgCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (AuthAlgMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetBssidReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetBssidCfm(const uint8_t* data, size_t length, GetBssidCfmStaNetworkParam& param)
{
    GetBssidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapAltSubjectMatchReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapAltSubjectMatchCfm(const uint8_t* data, size_t length, GetEapAltSubjectMatchCfmStaNetworkParam& param)
{
    GetEapAltSubjectMatchCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapAnonymousIdentityReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapAnonymousIdentityCfm(const uint8_t* data, size_t length, GetEapAnonymousIdentityCfmStaNetworkParam& param)
{
    GetEapAnonymousIdentityCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapCACertReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapCACertCfm(const uint8_t* data, size_t length, GetEapCACertCfmStaNetworkParam& param)
{
    GetEapCACertCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapCAPathReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapCAPathCfm(const uint8_t* data, size_t length, GetEapCAPathCfmStaNetworkParam& param)
{
    GetEapCAPathCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapClientCertReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapClientCertCfm(const uint8_t* data, size_t length, GetEapClientCertCfmStaNetworkParam& param)
{
    GetEapClientCertCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapDomainSuffixMatchReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapDomainSuffixMatchCfm(const uint8_t* data, size_t length, GetEapDomainSuffixMatchCfmStaNetworkParam& param)
{
    GetEapDomainSuffixMatchCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapEngineReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapEngineCfm(const uint8_t* data, size_t length, GetEapEngineCfmStaNetworkParam& param)
{
    GetEapEngineCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapEngineIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapEngineIdCfm(const uint8_t* data, size_t length, GetEapEngineIdCfmStaNetworkParam& param)
{
    GetEapEngineIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapIdentityReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapIdentityCfm(const uint8_t* data, size_t length, GetEapIdentityCfmStaNetworkParam& param)
{
    GetEapIdentityCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapMethodReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapMethodCfm(const uint8_t* data, size_t length, GetEapMethodCfmStaNetworkParam& param)
{
    GetEapMethodCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (EapMethod) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapPasswordReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapPasswordCfm(const uint8_t* data, size_t length, GetEapPasswordCfmStaNetworkParam& param)
{
    GetEapPasswordCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapPhase2MethodReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapPhase2MethodCfm(const uint8_t* data, size_t length, GetEapPhase2MethodCfmStaNetworkParam& param)
{
    GetEapPhase2MethodCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (EapPhase2Method) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapPrivateKeyIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapPrivateKeyIdCfm(const uint8_t* data, size_t length, GetEapPrivateKeyIdCfmStaNetworkParam& param)
{
    GetEapPrivateKeyIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEapSubjectMatchReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEapSubjectMatchCfm(const uint8_t* data, size_t length, GetEapSubjectMatchCfmStaNetworkParam& param)
{
    GetEapSubjectMatchCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetEdmgReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetEdmgCfm(const uint8_t* data, size_t length, GetEdmgCfmStaNetworkParam& param)
{
    GetEdmgCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetGroupCipherReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetGroupCipherCfm(const uint8_t* data, size_t length, GetGroupCipherCfmStaNetworkParam& param)
{
    GetGroupCipherCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (GroupCipherMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetGroupMgmtCipherReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetGroupMgmtCipherCfm(const uint8_t* data, size_t length, GetGroupMgmtCipherCfmStaNetworkParam& param)
{
    GetGroupMgmtCipherCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (GroupMgmtCipherMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmStaNetworkParam& param)
{
    GetIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetIdStrReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetIdStrCfm(const uint8_t* data, size_t length, GetIdStrCfmStaNetworkParam& param)
{
    GetIdStrCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetInterfaceNameReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetInterfaceNameCfm(const uint8_t* data, size_t length, GetInterfaceNameCfmStaNetworkParam& param)
{
    GetInterfaceNameCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetKeyMgmtReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetKeyMgmtCfm(const uint8_t* data, size_t length, GetKeyMgmtCfmStaNetworkParam& param)
{
    GetKeyMgmtCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (KeyMgmtMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetOcspReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetOcspCfm(const uint8_t* data, size_t length, GetOcspCfmStaNetworkParam& param)
{
    GetOcspCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (OcspType) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetPairwiseCipherReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetPairwiseCipherCfm(const uint8_t* data, size_t length, GetPairwiseCipherCfmStaNetworkParam& param)
{
    GetPairwiseCipherCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (PairwiseCipherMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetProtoReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetProtoCfm(const uint8_t* data, size_t length, GetProtoCfmStaNetworkParam& param)
{
    GetProtoCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (ProtoMask) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetPskReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetPskCfm(const uint8_t* data, size_t length, GetPskCfmStaNetworkParam& param)
{
    GetPskCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetPskPassphraseReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetPskPassphraseCfm(const uint8_t* data, size_t length, GetPskPassphraseCfmStaNetworkParam& param)
{
    GetPskPassphraseCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetRequirePmfReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetRequirePmfCfm(const uint8_t* data, size_t length, GetRequirePmfCfmStaNetworkParam& param)
{
    GetRequirePmfCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetSaePasswordReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetSaePasswordCfm(const uint8_t* data, size_t length, GetSaePasswordCfmStaNetworkParam& param)
{
    GetSaePasswordCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetSaePasswordIdReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetSaePasswordIdCfm(const uint8_t* data, size_t length, GetSaePasswordIdCfmStaNetworkParam& param)
{
    GetSaePasswordIdCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetScanSsidReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetScanSsidCfm(const uint8_t* data, size_t length, GetScanSsidCfmStaNetworkParam& param)
{
    GetScanSsidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetSsidReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmStaNetworkParam& param)
{
    GetSsidCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetTypeReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmStaNetworkParam& param)
{
    GetTypeCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = (IfaceType) msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetWapiCertSuiteReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetWapiCertSuiteCfm(const uint8_t* data, size_t length, GetWapiCertSuiteCfmStaNetworkParam& param)
{
    GetWapiCertSuiteCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetWepKeyReq(const uint8_t* data, size_t length, int32_t& param)
{
    GetWepKeyReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.keyidx();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetWepKeyCfm(const uint8_t* data, size_t length, GetWepKeyCfmStaNetworkParam& param)
{
    GetWepKeyCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetWepTxKeyIdxReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetWepTxKeyIdxCfm(const uint8_t* data, size_t length, GetWepTxKeyIdxCfmStaNetworkParam& param)
{
    GetWepTxKeyIdxCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        param.result = msg.result();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseGetWpsNfcConfigurationTokenReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseGetWpsNfcConfigurationTokenCfm(const uint8_t* data, size_t length, GetWpsNfcConfigurationTokenCfmStaNetworkParam& param)
{
    GetWpsNfcConfigurationTokenCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2HalStatus(msg.status(), param.status);
        String2Vector(msg.result(), param.result);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseRegisterCallbackReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseRegisterCallbackCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSelectReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSelectCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapIdentityResponseReq(const uint8_t* data, size_t length, SendNetworkEapIdentityResponseReqStaNetworkParam& param)
{
    SendNetworkEapIdentityResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.identity(), param.identity);
        String2Vector(msg.encryptedidentity(), param.encryptedIdentity);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSendNetworkEapIdentityResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NetworkResponseEapSimGsmAuthParams(const NetworkResponseEapSimGsmAuthParams_P& msgSupplicantStaNetwork, NetworkResponseEapSimGsmAuthParams& params)
{
    String2Vector(msgSupplicantStaNetwork.kc(), params.kc);
    String2Vector(msgSupplicantStaNetwork.sres(), params.sres);
}

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseReq(const uint8_t* data, size_t length, vector<NetworkResponseEapSimGsmAuthParams>& param)
{
    SendNetworkEapSimGsmAuthResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_params = msg.params_size();
        if (0 < size_params)
        {
            param.resize(size_params);
            for (int index = 0; index < size_params; index++)
            {
                Message2NetworkResponseEapSimGsmAuthParams(msg.params(index), param[index]);
            }
        }
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureReq(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2NetworkResponseEapSimUmtsAuthParams(const NetworkResponseEapSimUmtsAuthParams_P& msgSupplicantStaNetwork, NetworkResponseEapSimUmtsAuthParams& params)
{
    String2Vector(msgSupplicantStaNetwork.res(), params.res);
    String2Vector(msgSupplicantStaNetwork.ik(), params.ik);
    String2Vector(msgSupplicantStaNetwork.ck(), params.ck);
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseReq(const uint8_t* data, size_t length, NetworkResponseEapSimUmtsAuthParams& param)
{
    SendNetworkEapSimUmtsAuthResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NetworkResponseEapSimUmtsAuthParams(msg.params(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SendNetworkEapSimUmtsAutsResponseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.auts(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetAuthAlgReq(const uint8_t* data, size_t length, AuthAlgMask& param)
{
    SetAuthAlgReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (AuthAlgMask) msg.authalgmask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetAuthAlgCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetBssidReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetBssidReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.bssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetBssidCfm(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2DppConnectionKeys(const DppConnectionKeys_P& msgSupplicantStaNetwork, DppConnectionKeys& keys)
{
    String2Vector(msgSupplicantStaNetwork.connector(), keys.connector);
    String2Vector(msgSupplicantStaNetwork.csign(), keys.cSign);
    String2Vector(msgSupplicantStaNetwork.netaccesskey(), keys.netAccessKey);
}

bool SupplicantStaNetworkParseSetDppKeysReq(const uint8_t* data, size_t length, DppConnectionKeys& param)
{
    SetDppKeysReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2DppConnectionKeys(msg.keys(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetDppKeysCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapAltSubjectMatchReq(const uint8_t* data, size_t length, string& param)
{
    SetEapAltSubjectMatchReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.match();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapAltSubjectMatchCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapAnonymousIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetEapAnonymousIdentityReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.identity(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapAnonymousIdentityCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapCACertReq(const uint8_t* data, size_t length, string& param)
{
    SetEapCACertReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.path();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapCACertCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapCAPathReq(const uint8_t* data, size_t length, string& param)
{
    SetEapCAPathReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.path();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapCAPathCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapClientCertReq(const uint8_t* data, size_t length, string& param)
{
    SetEapClientCertReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.path();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapClientCertCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapDomainSuffixMatchReq(const uint8_t* data, size_t length, string& param)
{
    SetEapDomainSuffixMatchReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.match();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapDomainSuffixMatchCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapEncryptedImsiIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetEapEncryptedImsiIdentityReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.identity(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapEncryptedImsiIdentityCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapEngineReq(const uint8_t* data, size_t length, bool& param)
{
    SetEapEngineReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapEngineCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapEngineIDReq(const uint8_t* data, size_t length, string& param)
{
    SetEapEngineIDReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapEngineIDCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapErpReq(const uint8_t* data, size_t length, bool& param)
{
    SetEapErpReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapErpCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetEapIdentityReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.identity(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapIdentityCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapMethodReq(const uint8_t* data, size_t length, EapMethod& param)
{
    SetEapMethodReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (EapMethod) msg.method();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapMethodCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapPasswordReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetEapPasswordReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.password(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapPasswordCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapPhase2MethodReq(const uint8_t* data, size_t length, EapPhase2Method& param)
{
    SetEapPhase2MethodReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (EapPhase2Method) msg.method();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapPhase2MethodCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapPrivateKeyIdReq(const uint8_t* data, size_t length, string& param)
{
    SetEapPrivateKeyIdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapPrivateKeyIdCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEapSubjectMatchReq(const uint8_t* data, size_t length, string& param)
{
    SetEapSubjectMatchReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.match();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEapSubjectMatchCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetEdmgReq(const uint8_t* data, size_t length, bool& param)
{
    SetEdmgReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetEdmgCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetGroupCipherReq(const uint8_t* data, size_t length, GroupCipherMask& param)
{
    SetGroupCipherReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (GroupCipherMask) msg.groupciphermask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetGroupCipherCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetGroupMgmtCipherReq(const uint8_t* data, size_t length, GroupMgmtCipherMask& param)
{
    SetGroupMgmtCipherReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (GroupMgmtCipherMask) msg.groupmgmtciphermask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetGroupMgmtCipherCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetIdStrReq(const uint8_t* data, size_t length, string& param)
{
    SetIdStrReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.idstr();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetIdStrCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetKeyMgmtReq(const uint8_t* data, size_t length, KeyMgmtMask& param)
{
    SetKeyMgmtReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (KeyMgmtMask) msg.keymgmtmask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetKeyMgmtCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetOcspReq(const uint8_t* data, size_t length, OcspType& param)
{
    SetOcspReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (OcspType) msg.ocsptype();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetOcspCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetPairwiseCipherReq(const uint8_t* data, size_t length, PairwiseCipherMask& param)
{
    SetPairwiseCipherReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (PairwiseCipherMask) msg.pairwiseciphermask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetPairwiseCipherCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetPmkCacheReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetPmkCacheReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.serializedentry(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetPmkCacheCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetProactiveKeyCachingReq(const uint8_t* data, size_t length, bool& param)
{
    SetProactiveKeyCachingReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetProactiveKeyCachingCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetProtoReq(const uint8_t* data, size_t length, ProtoMask& param)
{
    SetProtoReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (ProtoMask) msg.protomask();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetProtoCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetPskReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetPskReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.psk(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetPskCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetPskPassphraseReq(const uint8_t* data, size_t length, string& param)
{
    SetPskPassphraseReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.psk();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetPskPassphraseCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetRequirePmfReq(const uint8_t* data, size_t length, bool& param)
{
    SetRequirePmfReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetRequirePmfCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetSaeH2eModeReq(const uint8_t* data, size_t length, SaeH2eMode& param)
{
    SetSaeH2eModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (SaeH2eMode) msg.mode();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetSaeH2eModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetSaePasswordReq(const uint8_t* data, size_t length, string& param)
{
    SetSaePasswordReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.saepassword();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetSaePasswordCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetSaePasswordIdReq(const uint8_t* data, size_t length, string& param)
{
    SetSaePasswordIdReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.saepasswordid();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetSaePasswordIdCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetScanSsidReq(const uint8_t* data, size_t length, bool& param)
{
    SetScanSsidReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetScanSsidCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetSsidReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetSsidReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.ssid(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetSsidCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetUpdateIdentifierReq(const uint8_t* data, size_t length, int32_t& param)
{
    SetUpdateIdentifierReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.id();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetUpdateIdentifierCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetWapiCertSuiteReq(const uint8_t* data, size_t length, string& param)
{
    SetWapiCertSuiteReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.suite();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetWapiCertSuiteCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetWepKeyReq(const uint8_t* data, size_t length, SetWepKeyReqStaNetworkParam& param)
{
    SetWepKeyReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.keyIdx = msg.keyidx();
        String2Vector(msg.wepkey(), param.wepKey);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetWepKeyCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetWepTxKeyIdxReq(const uint8_t* data, size_t length, int32_t& param)
{
    SetWepTxKeyIdxReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.keyidx();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetWepTxKeyIdxCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetRoamingConsortiumSelectionReq(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    SetRoamingConsortiumSelectionReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.selectedrcoi(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetRoamingConsortiumSelectionCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamReq(const uint8_t* data, size_t length, TlsVersion& param)
{
    SetMinimumTlsVersionEapPhase1ParamReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (TlsVersion) msg.tlsversion();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseSetStrictConservativePeerModeReq(const uint8_t* data, size_t length, bool& param)
{
    SetStrictConservativePeerModeReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.enable();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseSetStrictConservativePeerModeCfm(const uint8_t* data, size_t length)
{
    return true;
}

bool SupplicantStaNetworkParseOnNetworkEapIdentityRequestInd(const uint8_t* data, size_t length)
{
    return true;
}

static void Message2GsmRand(const GsmRand_P& msgSupplicantStaNetwork, GsmRand& rands)
{
    String2Vector(msgSupplicantStaNetwork.data(), rands.data);
}

static void Message2NetworkRequestEapSimGsmAuthParams(const NetworkRequestEapSimGsmAuthParams_P& msgSupplicantStaNetwork, NetworkRequestEapSimGsmAuthParams& params)
{
    int size_rands = msgSupplicantStaNetwork.rands_size();
    if (0 < size_rands)
    {
        params.rands.resize(size_rands);
        for (int index = 0; index < size_rands; index++)
        {
            Message2GsmRand(msgSupplicantStaNetwork.rands(index), params.rands[index]);
        }
    }
}

bool SupplicantStaNetworkParseOnNetworkEapSimGsmAuthRequestInd(const uint8_t* data, size_t length, NetworkRequestEapSimGsmAuthParams& param)
{
    OnNetworkEapSimGsmAuthRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NetworkRequestEapSimGsmAuthParams(msg.params(), param);
        return true;
    }
    return false;
}

static void Message2NetworkRequestEapSimUmtsAuthParams(const NetworkRequestEapSimUmtsAuthParams_P& msgSupplicantStaNetwork, NetworkRequestEapSimUmtsAuthParams& params)
{
    String2Vector(msgSupplicantStaNetwork.rand(), params.rand);
    String2Vector(msgSupplicantStaNetwork.autn(), params.autn);
}

bool SupplicantStaNetworkParseOnNetworkEapSimUmtsAuthRequestInd(const uint8_t* data, size_t length, NetworkRequestEapSimUmtsAuthParams& param)
{
    OnNetworkEapSimUmtsAuthRequestInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        Message2NetworkRequestEapSimUmtsAuthParams(msg.params(), param);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseOnTransitionDisableInd(const uint8_t* data, size_t length, TransitionDisableIndication& param)
{
    OnTransitionDisableInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = (TransitionDisableIndication) msg.ind();
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseOnServerCertificateAvailableInd(const uint8_t* data, size_t length, OnServerCertificateAvailableIndStaNetworkParam& param)
{
    OnServerCertificateAvailableInd msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param.depth = msg.depth();
        String2Vector(msg.subject(), param.subject);
        String2Vector(msg.certhash(), param.certHash);
        String2Vector(msg.certblob(), param.certBlob);
        return true;
    }
    return false;
}

bool SupplicantStaNetworkParseOnPermanentIdReqDeniedInd(const uint8_t* data, size_t length)
{
    return true;
}
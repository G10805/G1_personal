/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <aidl/android/hardware/wifi/supplicant/ISupplicantStaNetwork.h>
#include <aidl/android/hardware/wifi/supplicant/ISupplicantStaNetworkCallback.h>

#include "supplicant_msg_common.h"

using std::array;
using std::string;
using std::vector;

using namespace aidl::android::hardware::wifi::supplicant;


typedef struct
{
    HalStatusParam status;
    AuthAlgMask result;
} GetAuthAlgCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetBssidCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapAltSubjectMatchCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetEapAnonymousIdentityCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapCACertCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapCAPathCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapClientCertCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapDomainSuffixMatchCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} GetEapEngineCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapEngineIdCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetEapIdentityCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    EapMethod result;
} GetEapMethodCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetEapPasswordCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    EapPhase2Method result;
} GetEapPhase2MethodCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapPrivateKeyIdCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetEapSubjectMatchCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} GetEdmgCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    GroupCipherMask result;
} GetGroupCipherCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    GroupMgmtCipherMask result;
} GetGroupMgmtCipherCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetIdCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetIdStrCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetInterfaceNameCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    KeyMgmtMask result;
} GetKeyMgmtCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    OcspType result;
} GetOcspCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    PairwiseCipherMask result;
} GetPairwiseCipherCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    ProtoMask result;
} GetProtoCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetPskCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetPskPassphraseCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} GetRequirePmfCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetSaePasswordCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetSaePasswordIdCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    bool result;
} GetScanSsidCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetSsidCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    IfaceType result;
} GetTypeCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    string result;
} GetWapiCertSuiteCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetWepKeyCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    int32_t result;
} GetWepTxKeyIdxCfmStaNetworkParam;

typedef struct
{
    HalStatusParam status;
    vector<uint8_t> result;
} GetWpsNfcConfigurationTokenCfmStaNetworkParam;

typedef struct
{
    vector<uint8_t> identity;
    vector<uint8_t> encryptedIdentity;
} SendNetworkEapIdentityResponseReqStaNetworkParam;

typedef struct
{
    int32_t keyIdx;
    vector<uint8_t> wepKey;
} SetWepKeyReqStaNetworkParam;

typedef struct
{
    int32_t depth;
    vector<uint8_t> subject;
    vector<uint8_t> certHash;
    vector<uint8_t> certBlob;
} OnServerCertificateAvailableIndStaNetworkParam;

bool SupplicantStaNetworkSerializeDisableReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeDisableCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableReq(bool noConnect, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableSaePkOnlyModeReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableSaePkOnlyModeCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableSuiteBEapOpenSslCiphersCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeEnableTlsSuiteBEapPhase1ParamCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetAuthAlgReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetAuthAlgCfm(const HalStatusParam& status, AuthAlgMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetBssidReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetBssidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapAltSubjectMatchReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapAltSubjectMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapAnonymousIdentityReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapAnonymousIdentityCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapCACertReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapCACertCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapCAPathReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapCAPathCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapClientCertReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapClientCertCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapDomainSuffixMatchReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapDomainSuffixMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapEngineReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapEngineCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapEngineIdReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapEngineIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapIdentityReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapIdentityCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapMethodReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapMethodCfm(const HalStatusParam& status, EapMethod result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPasswordReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPasswordCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPhase2MethodReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPhase2MethodCfm(const HalStatusParam& status, EapPhase2Method result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPrivateKeyIdReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapPrivateKeyIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapSubjectMatchReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEapSubjectMatchCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEdmgReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetEdmgCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetGroupCipherReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetGroupCipherCfm(const HalStatusParam& status, GroupCipherMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetGroupMgmtCipherReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetGroupMgmtCipherCfm(const HalStatusParam& status, GroupMgmtCipherMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetIdReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetIdCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetIdStrReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetIdStrCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetInterfaceNameReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetInterfaceNameCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetKeyMgmtReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetKeyMgmtCfm(const HalStatusParam& status, KeyMgmtMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetOcspReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetOcspCfm(const HalStatusParam& status, OcspType result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPairwiseCipherReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPairwiseCipherCfm(const HalStatusParam& status, PairwiseCipherMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetProtoReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetProtoCfm(const HalStatusParam& status, ProtoMask result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPskReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPskCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPskPassphraseReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetPskPassphraseCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetRequirePmfReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetRequirePmfCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSaePasswordReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSaePasswordCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSaePasswordIdReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSaePasswordIdCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetScanSsidReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetScanSsidCfm(const HalStatusParam& status, bool result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSsidReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetSsidCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetTypeReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetTypeCfm(const HalStatusParam& status, IfaceType result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWapiCertSuiteReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWapiCertSuiteCfm(const HalStatusParam& status, const string& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWepKeyReq(int32_t keyIdx, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWepKeyCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWepTxKeyIdxReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWepTxKeyIdxCfm(const HalStatusParam& status, int32_t result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeGetWpsNfcConfigurationTokenCfm(const HalStatusParam& status, const vector<uint8_t>& result, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeRegisterCallbackReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeRegisterCallbackCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSelectReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSelectCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseReq(const vector<uint8_t>& identity, const vector<uint8_t>& encryptedIdentity, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapIdentityResponseCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthFailureCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseReq(const vector<NetworkResponseEapSimGsmAuthParams>& params, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimGsmAuthResponseCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureReq(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthFailureCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseReq(const NetworkResponseEapSimUmtsAuthParams& params, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAuthResponseCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseReq(const vector<uint8_t>& auts, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSendNetworkEapSimUmtsAutsResponseCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetAuthAlgReq(AuthAlgMask authAlgMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetAuthAlgCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetBssidReq(const vector<uint8_t>& bssid, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetBssidCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetDppKeysReq(const DppConnectionKeys& keys, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetDppKeysCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapAltSubjectMatchReq(const string& match, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapAltSubjectMatchCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapAnonymousIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapAnonymousIdentityCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapCACertReq(const string& path, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapCACertCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapCAPathReq(const string& path, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapCAPathCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapClientCertReq(const string& path, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapClientCertCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapDomainSuffixMatchReq(const string& match, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapDomainSuffixMatchCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEncryptedImsiIdentityCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEngineReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEngineCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEngineIDReq(const string& id, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapEngineIDCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapErpReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapErpCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapIdentityReq(const vector<uint8_t>& identity, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapIdentityCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapMethodReq(EapMethod method, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapMethodCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPasswordReq(const vector<uint8_t>& password, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPasswordCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPhase2MethodReq(EapPhase2Method method, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPhase2MethodCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPrivateKeyIdReq(const string& id, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapPrivateKeyIdCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapSubjectMatchReq(const string& match, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEapSubjectMatchCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEdmgReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetEdmgCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetGroupCipherReq(GroupCipherMask groupCipherMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetGroupCipherCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetGroupMgmtCipherReq(GroupMgmtCipherMask groupMgmtCipherMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetGroupMgmtCipherCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetIdStrReq(const string& idStr, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetIdStrCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetKeyMgmtReq(KeyMgmtMask keyMgmtMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetKeyMgmtCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetOcspReq(OcspType ocspType, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetOcspCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPairwiseCipherReq(PairwiseCipherMask pairwiseCipherMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPairwiseCipherCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPmkCacheReq(const vector<uint8_t>& serializedEntry, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPmkCacheCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetProactiveKeyCachingReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetProactiveKeyCachingCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetProtoReq(ProtoMask protoMask, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetProtoCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPskReq(const vector<uint8_t>& psk, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPskCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPskPassphraseReq(const string& psk, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetPskPassphraseCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetRequirePmfReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetRequirePmfCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaeH2eModeReq(SaeH2eMode mode, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaeH2eModeCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaePasswordReq(const string& saePassword, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaePasswordCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaePasswordIdReq(const string& saePasswordId, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSaePasswordIdCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetScanSsidReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetScanSsidCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSsidReq(const vector<uint8_t>& ssid, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetSsidCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetUpdateIdentifierReq(int32_t id, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetUpdateIdentifierCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWapiCertSuiteReq(const string& suite, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWapiCertSuiteCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWepKeyReq(int32_t keyIdx, const vector<uint8_t>& wepKey, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWepKeyCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWepTxKeyIdxReq(int32_t keyIdx, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetWepTxKeyIdxCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionReq(const vector<uint8_t>& selectedRcoi, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetRoamingConsortiumSelectionCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamReq(TlsVersion tlsVersion, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetMinimumTlsVersionEapPhase1ParamCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetStrictConservativePeerModeReq(bool enable, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeSetStrictConservativePeerModeCfm(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnNetworkEapIdentityRequestInd(vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnNetworkEapSimGsmAuthRequestInd(const NetworkRequestEapSimGsmAuthParams& params, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnNetworkEapSimUmtsAuthRequestInd(const NetworkRequestEapSimUmtsAuthParams& params, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnTransitionDisableInd(TransitionDisableIndication ind, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnServerCertificateAvailableInd(int32_t depth, const vector<uint8_t>& subject, const vector<uint8_t>& certHash, const vector<uint8_t>& certBlob, vector<uint8_t>& payload);

bool SupplicantStaNetworkSerializeOnPermanentIdReqDeniedInd(vector<uint8_t>& payload);

bool SupplicantStaNetworkParseDisableReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseDisableCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseEnableReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseEnableCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseEnableSaePkOnlyModeReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseEnableSaePkOnlyModeCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseEnableSuiteBEapOpenSslCiphersCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseEnableTlsSuiteBEapPhase1ParamCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetAuthAlgReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetAuthAlgCfm(const uint8_t* data, size_t length, GetAuthAlgCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetBssidReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetBssidCfm(const uint8_t* data, size_t length, GetBssidCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapAltSubjectMatchReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapAltSubjectMatchCfm(const uint8_t* data, size_t length, GetEapAltSubjectMatchCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapAnonymousIdentityReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapAnonymousIdentityCfm(const uint8_t* data, size_t length, GetEapAnonymousIdentityCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapCACertReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapCACertCfm(const uint8_t* data, size_t length, GetEapCACertCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapCAPathReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapCAPathCfm(const uint8_t* data, size_t length, GetEapCAPathCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapClientCertReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapClientCertCfm(const uint8_t* data, size_t length, GetEapClientCertCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapDomainSuffixMatchReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapDomainSuffixMatchCfm(const uint8_t* data, size_t length, GetEapDomainSuffixMatchCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapEngineReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapEngineCfm(const uint8_t* data, size_t length, GetEapEngineCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapEngineIdReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapEngineIdCfm(const uint8_t* data, size_t length, GetEapEngineIdCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapIdentityReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapIdentityCfm(const uint8_t* data, size_t length, GetEapIdentityCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapMethodReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapMethodCfm(const uint8_t* data, size_t length, GetEapMethodCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapPasswordReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapPasswordCfm(const uint8_t* data, size_t length, GetEapPasswordCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapPhase2MethodReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapPhase2MethodCfm(const uint8_t* data, size_t length, GetEapPhase2MethodCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapPrivateKeyIdReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapPrivateKeyIdCfm(const uint8_t* data, size_t length, GetEapPrivateKeyIdCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEapSubjectMatchReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEapSubjectMatchCfm(const uint8_t* data, size_t length, GetEapSubjectMatchCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetEdmgReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetEdmgCfm(const uint8_t* data, size_t length, GetEdmgCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetGroupCipherReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetGroupCipherCfm(const uint8_t* data, size_t length, GetGroupCipherCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetGroupMgmtCipherReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetGroupMgmtCipherCfm(const uint8_t* data, size_t length, GetGroupMgmtCipherCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetIdReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetIdCfm(const uint8_t* data, size_t length, GetIdCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetIdStrReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetIdStrCfm(const uint8_t* data, size_t length, GetIdStrCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetInterfaceNameReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetInterfaceNameCfm(const uint8_t* data, size_t length, GetInterfaceNameCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetKeyMgmtReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetKeyMgmtCfm(const uint8_t* data, size_t length, GetKeyMgmtCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetOcspReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetOcspCfm(const uint8_t* data, size_t length, GetOcspCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetPairwiseCipherReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetPairwiseCipherCfm(const uint8_t* data, size_t length, GetPairwiseCipherCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetProtoReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetProtoCfm(const uint8_t* data, size_t length, GetProtoCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetPskReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetPskCfm(const uint8_t* data, size_t length, GetPskCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetPskPassphraseReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetPskPassphraseCfm(const uint8_t* data, size_t length, GetPskPassphraseCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetRequirePmfReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetRequirePmfCfm(const uint8_t* data, size_t length, GetRequirePmfCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetSaePasswordReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetSaePasswordCfm(const uint8_t* data, size_t length, GetSaePasswordCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetSaePasswordIdReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetSaePasswordIdCfm(const uint8_t* data, size_t length, GetSaePasswordIdCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetScanSsidReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetScanSsidCfm(const uint8_t* data, size_t length, GetScanSsidCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetSsidReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetSsidCfm(const uint8_t* data, size_t length, GetSsidCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetTypeReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetTypeCfm(const uint8_t* data, size_t length, GetTypeCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetWapiCertSuiteReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetWapiCertSuiteCfm(const uint8_t* data, size_t length, GetWapiCertSuiteCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetWepKeyReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaNetworkParseGetWepKeyCfm(const uint8_t* data, size_t length, GetWepKeyCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetWepTxKeyIdxReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetWepTxKeyIdxCfm(const uint8_t* data, size_t length, GetWepTxKeyIdxCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseGetWpsNfcConfigurationTokenReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseGetWpsNfcConfigurationTokenCfm(const uint8_t* data, size_t length, GetWpsNfcConfigurationTokenCfmStaNetworkParam& param);

bool SupplicantStaNetworkParseRegisterCallbackReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseRegisterCallbackCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSelectReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSelectCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapIdentityResponseReq(const uint8_t* data, size_t length, SendNetworkEapIdentityResponseReqStaNetworkParam& param);

bool SupplicantStaNetworkParseSendNetworkEapIdentityResponseCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthFailureCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseReq(const uint8_t* data, size_t length, vector<NetworkResponseEapSimGsmAuthParams>& param);

bool SupplicantStaNetworkParseSendNetworkEapSimGsmAuthResponseCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureReq(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthFailureCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseReq(const uint8_t* data, size_t length, NetworkResponseEapSimUmtsAuthParams& param);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAuthResponseCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSendNetworkEapSimUmtsAutsResponseCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetAuthAlgReq(const uint8_t* data, size_t length, AuthAlgMask& param);

bool SupplicantStaNetworkParseSetAuthAlgCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetBssidReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetBssidCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetDppKeysReq(const uint8_t* data, size_t length, DppConnectionKeys& param);

bool SupplicantStaNetworkParseSetDppKeysCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapAltSubjectMatchReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapAltSubjectMatchCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapAnonymousIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetEapAnonymousIdentityCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapCACertReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapCACertCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapCAPathReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapCAPathCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapClientCertReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapClientCertCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapDomainSuffixMatchReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapDomainSuffixMatchCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapEncryptedImsiIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetEapEncryptedImsiIdentityCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapEngineReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetEapEngineCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapEngineIDReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapEngineIDCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapErpReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetEapErpCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapIdentityReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetEapIdentityCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapMethodReq(const uint8_t* data, size_t length, EapMethod& param);

bool SupplicantStaNetworkParseSetEapMethodCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapPasswordReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetEapPasswordCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapPhase2MethodReq(const uint8_t* data, size_t length, EapPhase2Method& param);

bool SupplicantStaNetworkParseSetEapPhase2MethodCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapPrivateKeyIdReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapPrivateKeyIdCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEapSubjectMatchReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetEapSubjectMatchCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetEdmgReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetEdmgCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetGroupCipherReq(const uint8_t* data, size_t length, GroupCipherMask& param);

bool SupplicantStaNetworkParseSetGroupCipherCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetGroupMgmtCipherReq(const uint8_t* data, size_t length, GroupMgmtCipherMask& param);

bool SupplicantStaNetworkParseSetGroupMgmtCipherCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetIdStrReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetIdStrCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetKeyMgmtReq(const uint8_t* data, size_t length, KeyMgmtMask& param);

bool SupplicantStaNetworkParseSetKeyMgmtCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetOcspReq(const uint8_t* data, size_t length, OcspType& param);

bool SupplicantStaNetworkParseSetOcspCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetPairwiseCipherReq(const uint8_t* data, size_t length, PairwiseCipherMask& param);

bool SupplicantStaNetworkParseSetPairwiseCipherCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetPmkCacheReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetPmkCacheCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetProactiveKeyCachingReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetProactiveKeyCachingCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetProtoReq(const uint8_t* data, size_t length, ProtoMask& param);

bool SupplicantStaNetworkParseSetProtoCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetPskReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetPskCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetPskPassphraseReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetPskPassphraseCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetRequirePmfReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetRequirePmfCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetSaeH2eModeReq(const uint8_t* data, size_t length, SaeH2eMode& param);

bool SupplicantStaNetworkParseSetSaeH2eModeCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetSaePasswordReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetSaePasswordCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetSaePasswordIdReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetSaePasswordIdCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetScanSsidReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetScanSsidCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetSsidReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetSsidCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetUpdateIdentifierReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaNetworkParseSetUpdateIdentifierCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetWapiCertSuiteReq(const uint8_t* data, size_t length, string& param);

bool SupplicantStaNetworkParseSetWapiCertSuiteCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetWepKeyReq(const uint8_t* data, size_t length, SetWepKeyReqStaNetworkParam& param);

bool SupplicantStaNetworkParseSetWepKeyCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetWepTxKeyIdxReq(const uint8_t* data, size_t length, int32_t& param);

bool SupplicantStaNetworkParseSetWepTxKeyIdxCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetRoamingConsortiumSelectionReq(const uint8_t* data, size_t length, vector<uint8_t>& param);

bool SupplicantStaNetworkParseSetRoamingConsortiumSelectionCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamReq(const uint8_t* data, size_t length, TlsVersion& param);

bool SupplicantStaNetworkParseSetMinimumTlsVersionEapPhase1ParamCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseSetStrictConservativePeerModeReq(const uint8_t* data, size_t length, bool& param);

bool SupplicantStaNetworkParseSetStrictConservativePeerModeCfm(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseOnNetworkEapIdentityRequestInd(const uint8_t* data, size_t length);

bool SupplicantStaNetworkParseOnNetworkEapSimGsmAuthRequestInd(const uint8_t* data, size_t length, NetworkRequestEapSimGsmAuthParams& param);

bool SupplicantStaNetworkParseOnNetworkEapSimUmtsAuthRequestInd(const uint8_t* data, size_t length, NetworkRequestEapSimUmtsAuthParams& param);

bool SupplicantStaNetworkParseOnTransitionDisableInd(const uint8_t* data, size_t length, TransitionDisableIndication& param);

bool SupplicantStaNetworkParseOnServerCertificateAvailableInd(const uint8_t* data, size_t length, OnServerCertificateAvailableIndStaNetworkParam& param);

bool SupplicantStaNetworkParseOnPermanentIdReqDeniedInd(const uint8_t* data, size_t length);
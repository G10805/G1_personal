/* This is an auto-generated source file. */

#include "common_util.h"
#include "proto_common.h"
#include "non_standard_cert_msg.h"

#include "INonStandardCertCallback.pb.h"

using GetBlobCfm = ::wifi::supplicant::INonStandardCertCallback_GetBlobCfm;
using GetBlobReq = ::wifi::supplicant::INonStandardCertCallback_GetBlobReq;
using ListAliasesCfm = ::wifi::supplicant::INonStandardCertCallback_ListAliasesCfm;
using ListAliasesReq = ::wifi::supplicant::INonStandardCertCallback_ListAliasesReq;


bool NonStandardCertSerializeGetBlobReq(const string& alias, vector<uint8_t>& payload)
{
    GetBlobReq msgNonStandardCert;
    msgNonStandardCert.set_alias(alias);
    return ProtoMessageSerialize(&msgNonStandardCert, payload);
}

bool NonStandardCertSerializeGetBlobCfm(const vector<uint8_t>& result, vector<uint8_t>& payload)
{
    GetBlobCfm msgNonStandardCert;
    string resultStr;
    Vector2String(result, resultStr);
    msgNonStandardCert.set_result(resultStr);
    return ProtoMessageSerialize(&msgNonStandardCert, payload);
}

bool NonStandardCertSerializeListAliasesReq(const string& prefix, vector<uint8_t>& payload)
{
    ListAliasesReq msgNonStandardCert;
    msgNonStandardCert.set_prefix(prefix);
    return ProtoMessageSerialize(&msgNonStandardCert, payload);
}

bool NonStandardCertSerializeListAliasesCfm(const vector<string>& result, vector<uint8_t>& payload)
{
    ListAliasesCfm msgNonStandardCert;
    int size_result = result.size();
    if (0 < size_result)
    {
        for (int index = 0; index < size_result; index++)
        {
            msgNonStandardCert.add_result(result[index]);
        }
    }
    return ProtoMessageSerialize(&msgNonStandardCert, payload);
}

bool NonStandardCertParseGetBlobReq(const uint8_t* data, size_t length, string& param)
{
    GetBlobReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.alias();
        return true;
    }
    return false;
}

bool NonStandardCertParseGetBlobCfm(const uint8_t* data, size_t length, vector<uint8_t>& param)
{
    GetBlobCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        String2Vector(msg.result(), param);
        return true;
    }
    return false;
}

bool NonStandardCertParseListAliasesReq(const uint8_t* data, size_t length, string& param)
{
    ListAliasesReq msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        param = msg.prefix();
        return true;
    }
    return false;
}

bool NonStandardCertParseListAliasesCfm(const uint8_t* data, size_t length, vector<string>& param)
{
    ListAliasesCfm msg;
    if (ProtoMessageParse(data, length, &msg))
    {
        int size_result = msg.result_size();
        if (0 < size_result)
        {
            param.resize(size_result);
            for (int index = 0; index < size_result; index++)
            {
                param[index] = msg.result(index);
            }
        }
        return true;
    }
    return false;
}
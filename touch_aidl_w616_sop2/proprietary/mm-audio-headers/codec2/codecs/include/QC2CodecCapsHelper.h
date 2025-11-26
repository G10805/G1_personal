/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef CODEC2_CODEC_CAPS_HELPER_H_
#define CODEC2_CODEC_CAPS_HELPER_H_

#include <util/C2InterfaceHelper.h>

#include "QC2.h"
#include "QC2Constants.h"
#include "QC2Config.h"
#include <unordered_map>
#include <unordered_set>

namespace qc2audio {

//--------------------------------------------------------------------------------------------------
//------------------------ v2 ----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
class IDeviceQuery;
class QC2ParamCapsHelper;

using ParamHelper = C2InterfaceHelper::ParamHelper;

struct paramCapability {
    int32_t nMin;
    int32_t nMax;
    int32_t nStep;
    int32_t nDefault;
};

struct ParamRangeSpecifier {
    int32_t nMin;
    int32_t nMax;
    int32_t nStep;
    int32_t nDefault;
};

/*
 * @brief Device Caps Query class
 *
 * This class defines interface to underlying HW accelerator to query device capabilities.
 * The QC2ParamCapsHelper object for a param can use this to obtain caps from HW during
 * its construction.
 */
class IDeviceQuery {
 public:
    IDeviceQuery() = default;
    virtual ~IDeviceQuery() = default;

    virtual QC2Status queryParamValues(uint32_t paramType,
            std::vector<uint32_t> *values, uint32_t *nDefault) = 0;
    virtual QC2Status queryParamRanges(uint32_t paramType,
            std::vector<ParamRangeSpecifier> *ranges) = 0;
};

/**
 * @brief Codec-variant specifiers
 *
 * Some codecs are merely varaints of the same underlying codec, but listed separately with
 *  a distinguishing name (mostly indicating its unique feature).
 * On Android, these variants map to the corresponding features exposed via
 *   MediaCodecInfo.CodecCapabilities#FEATURE_XYZ
 * Examples:                                                                                      \n
 *      "c2.qti.hevc.encoder"    : regular HEVC encoder component                                 \n
 *      "c2.qti.hevc.encoder.cq" : HEVC encoder variant that supports only ConstantQuality RC mode\n
 *                                                                                                \n
 *      "c2.qti.avc.decoder"            : regular AVC decoder component                           \n
 *      "c2.qti.avc.decoder.secure"     : AVC decoder variant that supports secure                \n
 *      "c2.qti.avc.decoder.low_latency": AVC decoder variant that supports low-latency           \n
 */
enum QC2CodecVariant : uint32_t {
    NONE         = 0,
    SECURE       = 0x1 << 0,     /// Supports secure. CodecCapabilities#FEATURE_SecurePlayback
    CAN_PIPELINE = 0x1 << 1,     /// Codec can be pipelined with other (compatible) codecs
    CQ_MODE      = 0x1 << 2,     /// Supports only Constant-Quality RC mode.
    LOW_LATENCY  = 0x1 << 3,     /// Supports Low-latency. CodecCapabilities#FEATURE_LowLatency
};

/**
 * @brief Max concurrent codec instances
 *
 * Maximum supported concurrent instances for a codec.
 */
enum QC2MaxCodecInstances : uint32_t {
    DEFAULT_MAX = 2,
    HW_MAX = DEFAULT_MAX,
    SW_MAX = DEFAULT_MAX,
};

/*
 * @brief QC Param Caps helper class
 *
 * This class acts as a base for defining parameters in QC Codec 2.0 HAL.
 * This class defines default values, field ranges, flags, attributes, and
 * Setters for given parameter. It also provides method to compose ParamHelper
 * used by interface.
 */
class QC2ParamCapsHelper {
 public:
    enum platform_flags_t : uint64_t {
        DEFAULT_DIFFERENT = 1u << 0,    ///< Parameter's default value doesn't match framework
        STATIC            = 1u << 1,    ///< Parameter doesn't accept values when session is running
        VOLATILE          = 1u << 2,    ///< Parameter's value may change without framework's config
        FRAME_SYNC        = 1u << 3,    ///< Parameter will be applied with the work queued
                                        //    immediately after configuring this parameter
        FRAME_TIME_SYNC   = 1u << 4,    ///< Parameter contains a "timestamp" field and shall
                                        //    be applied with the work matching the timestamp
        CUSTOM_FLAGS      = 1u << 31,   ///< Custom flags defined by platforms
    };

    /// returns bool value based on MimeType/Kind/SecureType
    inline bool isSupported() const { return mSupported; }
    /// returns a copy of this parameter with generic type as C2Param.
    inline std::unique_ptr<C2Param> getParamCopy() const { return C2Param::Copy(*mDefault); }

    /// getters for caps data
    inline uint32_t getIndex() const { return mDefault->index(); }
    inline uint64_t getFlags() const { return static_cast<uint64_t>(mFlags); }
    inline std::vector<C2ParamFieldValues> getRange() const { return mRange; }
    inline std::string getName() const { return std::string(mName); }
    inline const std::vector<uint32_t> getDependencies() const { return mDependencies; }

    /// getters for platform flags
    inline bool isDefaultOverride() const { return (mFlags & DEFAULT_DIFFERENT); }
    inline bool isStatic() const { return (mFlags & STATIC); }
    inline bool isVolatile() const { return (mFlags & VOLATILE); }

    /// returns the C2StructDescriptor for each struct used by this param
    void getStructDescriptors(
            std::unordered_map<uint32_t, C2StructDescriptor>* desc) const {
        if (!desc) {
             return;
        }
        desc->insert(mStructDesc.begin(), mStructDesc.end());
    }

    /// returns ParamHelper for this param by composing caps data defined in constructor
    virtual void getParamHelper(
            std::shared_ptr<C2Param>* param,
            const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps,
            std::vector<std::shared_ptr<ParamHelper>>* paramHelpers) const = 0;

    virtual ~QC2ParamCapsHelper() = default;

 protected:

    inline bool isEncoder() const { return mKind == ComponentKind::KIND_ENCODER;}
    inline bool isDecoder() const { return mKind == ComponentKind::KIND_DECODER;}
    inline bool isSecure() const {return !!(mVariant & QC2CodecVariant::SECURE);}
    inline bool isAac() const { return mCompressedMime == MimeType::AAC; }
    inline bool isAacAdts() const { return mCompressedMime == MimeType::AAC_ADTS; }
    inline bool isALAC() const { return mCompressedMime == MimeType::ALAC; }
    inline bool isEVRC() const { return mCompressedMime == MimeType::EVRC; }
    inline bool isQCELP() const { return mCompressedMime == MimeType::QCELP; }
    inline bool isAmrNb() const { return mCompressedMime == MimeType::AMR_NB; }
    inline bool isAmrWb() const { return mCompressedMime == MimeType::AMR_WB; }
    inline bool isAmrWbPlus() const { return mCompressedMime == MimeType::AMR_WB_PLUS; }
    inline bool isAPE() const { return mCompressedMime == MimeType::APE; }
    inline bool isMPEGH() const { return mCompressedMime == MimeType::MPEGH; }
    inline bool isWmaStd() const { return mCompressedMime == MimeType::WMA; }
    inline bool isWmaPro() const { return mCompressedMime == MimeType::WMA_PRO; }
    inline bool isWmaLossless() const { return mCompressedMime == MimeType::WMA_LOSSLESS; }
    inline bool isFLAC() const { return mCompressedMime == MimeType::FLAC; }
    inline bool isDSD() const { return mCompressedMime == MimeType::DSD; }
    // inline bool isAVC() const {return mCompressedMime == MimeType::AVC;}
    // inline bool isHEVC() const {return mCompressedMime == MimeType::HEVC;}
    // inline bool isVP8() const {return mCompressedMime == MimeType::VP8;}
    // inline bool isVP9() const {return mCompressedMime == MimeType::VP9;}
    // inline bool isHEIC() const {return mCompressedMime == MimeType::HEIC;}
    inline bool isAVC() const {return false;}
    inline bool isHEVC() const {return false;}
    inline bool isVP8() const {return false;}
    inline bool isVP9() const {return false;}
    inline bool isHEIC() const {return false;}

    QC2ParamCapsHelper(
            const std::string& compressedMime,
            uint32_t variant,
            ComponentKind kind)
        : mVariant(variant), mKind(kind), mCompressedMime(compressedMime) {
    }

    const uint32_t mVariant;
    const ComponentKind mKind;
    const std::string mCompressedMime;

    bool mSupported = false;
    C2StringLiteral mName;
    std::unique_ptr<C2Param> mDefault;
    std::vector<C2ParamFieldValues> mRange;
    std::vector<uint32_t> mDependencies;
    uint32_t mAttributes;
    std::unordered_map<uint32_t, C2StructDescriptor> mStructDesc;
    uint64_t mFlags;
};

#define RETURN_IF_NULL(ptr) \
do {   \
    if (!ptr) { \
        QLOGE("nullptr at %s:%s:%d", __FILE__, __func__, __LINE__); \
        return; \
    }   \
} while (0);    \

/// Static method to manufacture QCParamCapsHelpers
template<typename T>
static std::shared_ptr<QC2ParamCapsHelper> CapsHelperFactory(const std::string& codecName,
        const std::string& inputMime, const std::string& outputMime,
        uint32_t variant, ComponentKind kind,
        const std::shared_ptr<IDeviceQuery> device = nullptr) {
    static_assert(std::is_base_of<QC2ParamCapsHelper, T>::value,
            "Factory() can be defined for derived classes of QC2ParamCapsHelper only");
    return std::make_shared<T>(codecName, inputMime, outputMime, variant, kind, device);
}

using ParamCapsHelperFactories = std::vector<std::function<std::shared_ptr<QC2ParamCapsHelper>(
        const std::string&, const std::string&, const std::string&,
        uint32_t, ComponentKind, const std::shared_ptr<IDeviceQuery>)>>;

template<typename T>
T* getTempCopy(std::shared_ptr<T> p) {
    return T::From(C2Param::Copy(*p).release());
}

template<typename T, typename ...Args>
std::shared_ptr<T> GetSharedString(const Args(&... args), const char *str) {
    size_t len = strlen(str) + 1;
    std::shared_ptr<T> ret = T::AllocShared(len, args...);
    snprintf(ret->m.value, len, str);
    return ret;
}

template<typename T, typename ...Args>
std::shared_ptr<T> GetSharedString(const Args(&... args), const std::string &str) {
    size_t len = str.length() + 1;
    std::shared_ptr<T> ret = T::AllocShared(len, args...);
    snprintf(ret->m.value, len, "%s", str.c_str());
    return ret;
}

template <typename T>
struct SetterHelper {
    typedef typename std::remove_reference<T>::type type;

    static C2R NonStrictValueWithNoDeps(
            bool mayBlock, C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        return me.F(me.v.value).validatePossible(me.v.value);
    }

    static C2R NonStrictValuesWithNoDeps(
            bool mayBlock, C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        for (size_t ix = 0; ix < me.v.flexCount(); ++ix) {
            res.plus(me.F(me.v.m.values[ix]).validatePossible(me.v.m.values[ix]));
        }
        return res;
    }

    static C2R StrictValueWithNoDeps(
            bool mayBlock,
            const C2InterfaceHelper::C2P<type> &old,
            C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        if (!me.F(me.v.value).supportsNow(me.v.value)) {
            me.set().value = old.v.value;
        }
        return me.F(me.v.value).validatePossible(me.v.value);
    }

    static C2R EmptySetter(
        bool mayBlock, C2InterfaceHelper::C2P<type> &me) {
        (void)mayBlock;
        (void)me;
        C2R res = C2R::Ok();
        return res;
    }
};

template<typename T>
void applyAttributes(
        C2InterfaceHelper::ParamBuilder<T>* pb, uint32_t attribs) {
    if (!pb) {
        return;
    }
    if (attribs & C2ParamDescriptor::IS_REQUIRED) {
        pb->required();
    }
    if ((attribs & C2ParamDescriptor::IS_PERSISTENT) == 0) {
        pb->transient();
    }
    if (attribs & C2ParamDescriptor::IS_HIDDEN) {
        pb->hidden();
    }
    if (attribs & C2ParamDescriptor::IS_INTERNAL) {
        pb->internal();
    }
}

template<typename T>
std::shared_ptr<T>* getDepRef(const std::unordered_map<uint32_t, std::shared_ptr<C2Param>*>& deps) {
    uint32_t depId = static_cast<uint32_t>(T::PARAM_TYPE);
    auto it = deps.find(depId);
    if (it == deps.end()) {
        QLOGE("Missing dependency %s", DebugString::C2Param(depId).c_str());
        return nullptr;
    }
    auto param = it->second;
    return reinterpret_cast<std::shared_ptr<T>*>(param);
}

};  // namespace qc2audio

#endif  // CODEC2_CODEC_CAPS_HELPER_H_

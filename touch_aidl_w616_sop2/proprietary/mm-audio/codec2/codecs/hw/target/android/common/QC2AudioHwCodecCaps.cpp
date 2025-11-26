/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/

//#include "QC2AudioHwCodec.h"
//#include "QC2CodecCapsHelper.h"
//s#include "QC2TargetSpec.h"
#include "QC2AudioHwCodecCaps_custom.hpp"
#include "QC2AudioHwCodecCaps_standard.hpp"

#undef LOG_TAG
#define LOG_TAG "PlatformCapsHwCodec"

namespace qc2audio {

//--------------------------------------------------------------------------------------------------
//------------------------ v2 ----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

ParamCapsHelperFactories QC2AudioHwCodec::mCapsFactories = {
    CapsHelperFactory<KindHelper>,
    CapsHelperFactory<DomainHelper>,
    CapsHelperFactory<BlockPoolOutputHelper>,
    CapsHelperFactory<BufferTypeInputHelper>,
    CapsHelperFactory<BufferTypeOutputHelper>,
    CapsHelperFactory<MaxBufferSizeInputHelper>,
    CapsHelperFactory<MediaTypeInputHelper>,
    CapsHelperFactory<MediaTypeOutputHelper>,
    CapsHelperFactory<InputSamplingRateHelper>,
    CapsHelperFactory<OutputSamplingRateHelper>,
    CapsHelperFactory<InputChannelCountHelper>,
    CapsHelperFactory<OutputChannelCountHelper>,
    CapsHelperFactory<OutputMaxChannelCountHelper>,
    CapsHelperFactory<InputBitsPerSampleHelper>,
    CapsHelperFactory<OutputBitsPerSampleHelper>,
    CapsHelperFactory<InputBitRateHelper>,
    CapsHelperFactory<OutputBitRateHelper>,
    CapsHelperFactory<AllocatorsOutputHelper>,
    CapsHelperFactory<AllocatorsInputHelper>,
    CapsHelperFactory<InputAacFormatInfoHelper>,
    CapsHelperFactory<InputAacProfileLevelHelper>,
    CapsHelperFactory<ProfileLevelOutputHelper>,
    CapsHelperFactory<OutputStreamAacFormatHelper>,
    CapsHelperFactory<EncoderFrameSizeHelper>,
    CapsHelperFactory<ApiFeaturesHelper>,
    CapsHelperFactory<FrameLengthHelper>,   //vendor custom params START
    CapsHelperFactory<PbTuningHelper>,
    CapsHelperFactory<KbTuningHelper>,
    CapsHelperFactory<MbTuningHelper>,
    CapsHelperFactory<MaxFrameBytesHelper>,
    CapsHelperFactory<MaxRunHelper>,
    CapsHelperFactory<CompressionLevelHelper>,
    CapsHelperFactory<FormatFlagsHelper>,
    CapsHelperFactory<BlocksPerFrameHelper>,
    CapsHelperFactory<FinalFrameBlocksHelper>,
    CapsHelperFactory<TotalFramesHelper>,
    CapsHelperFactory<SeekTableHelper>,
    CapsHelperFactory<MinVocoderRateHelper>,
    CapsHelperFactory<MaxVocoderRateHelper>,
    CapsHelperFactory<WmaVersionHelper>,
    CapsHelperFactory<WmaConfigHelper>,
};

};

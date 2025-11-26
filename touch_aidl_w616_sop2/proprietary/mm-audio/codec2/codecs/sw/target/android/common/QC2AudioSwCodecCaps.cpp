/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#include "QC2AudioSwCodec.h"
#include "QC2CodecCapsHelper.h"
#include "QC2AudioSWCodecCaps_custom.hpp"
#include "QC2AudioSWCodecCaps_standard.hpp"

#undef LOG_TAG
#define LOG_TAG "PlatformCapsSwCodec"

namespace qc2audio {

//--------------------------------------------------------------------------------------------------
//------------------------ v2 ----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

ParamCapsHelperFactories QC2AudioSwCodec::mCapsFactories = {
    CapsHelperFactory<KindHelper>,
    CapsHelperFactory<DomainHelper>,
    CapsHelperFactory<BlockPoolOutputHelper>,
    CapsHelperFactory<BufferTypeInputHelper>,
    CapsHelperFactory<BufferTypeOutputHelper>,
    CapsHelperFactory<MaxBufferSizeInputHelper>,
    CapsHelperFactory<ActualInputDelayHelper>,
    CapsHelperFactory<ActualOutputDelayHelper>,
    CapsHelperFactory<MediaTypeInputHelper>,
    CapsHelperFactory<MediaTypeOutputHelper>,
    CapsHelperFactory<SamplingRateInputHelper>,
    CapsHelperFactory<SamplingRateOutputHelper>,
    CapsHelperFactory<ChannelCountOutputHelper>,
    CapsHelperFactory<BitsPerSampleOutputHelper>,
    CapsHelperFactory<BitRateInputHelper>,
    CapsHelperFactory<AllocatorsOutputHelper>,
    CapsHelperFactory<AllocatorsInputHelper>,
    CapsHelperFactory<FrameLengthHelper>,   //vendor custom params START
    CapsHelperFactory<PbTuningHelper>,
    CapsHelperFactory<KbTuningHelper>,
    CapsHelperFactory<MbTuningHelper>,
    CapsHelperFactory<MinFrameBytesHelper>,
    CapsHelperFactory<MaxFrameBytesHelper>,
    CapsHelperFactory<MaxRunHelper>,
    CapsHelperFactory<CompressionLevelHelper>,
    CapsHelperFactory<FormatFlagsHelper>,
    CapsHelperFactory<BlocksPerFrameHelper>,
    CapsHelperFactory<FinalFrameBlocksHelper>,
    CapsHelperFactory<TotalFramesHelper>,
    CapsHelperFactory<SeekTableHelper>,
    CapsHelperFactory<MinBlockSizeHelper>,
    CapsHelperFactory<MaxBlockSizeHelper>,
};

}; // namespace qc2audio

/*
 * Copyright (c) 2020-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef CHANNEL_REORDER_H_
#define CHANNEL_REORDER_H_

#include <cstring>
#include <inttypes.h>

namespace qc2audio {

/* Maximum number of decoder output channels. */
static const constexpr uint32_t AUDIO_MAX_CHANNELS = 16;

/* PCM Channel map */
enum {
    /* Front left channel. */
    AUDIO_PCM_CHANNEL_L = 1,
    /* Front right channel. */
    AUDIO_PCM_CHANNEL_R = 2,
    /* Front center channel. */
    AUDIO_PCM_CHANNEL_C = 3,
    /* Left surround channel.*/
    AUDIO_PCM_CHANNEL_LS = 4,
    /* Right surround channel.*/
    AUDIO_PCM_CHANNEL_RS = 5,
    /* Low frequency effect channel. */
    AUDIO_PCM_CHANNEL_LFE = 6,
    /* Center surround channel; rear center channel. */
    AUDIO_PCM_CHANNEL_CS = 7,
    /* Center back channel. */
    AUDIO_PCM_CHANNEL_CB = AUDIO_PCM_CHANNEL_CS,
    /* Left back channel; rear left channel. */
    AUDIO_PCM_CHANNEL_LB = 8,
    /* Right back channel; rear right channel. */
    AUDIO_PCM_CHANNEL_RB = 9,
    /* Top surround channel.*/
    AUDIO_PCM_CHANNEL_TS = 10,
    /* Center vertical height channel.*/
    AUDIO_PCM_CHANNEL_CVH = 11,
    /* Top front center channel.*/
    AUDIO_PCM_CHANNEL_TFC = AUDIO_PCM_CHANNEL_CVH,
    /* Mono surround channel.*/
    AUDIO_PCM_CHANNEL_MS = 12,
    /* Front left of center channel. */
    AUDIO_PCM_CHANNEL_FLC = 13,
    /* Front right of center channel. */
    AUDIO_PCM_CHANNEL_FRC = 14,
    /* Rear left of center channel. */
    AUDIO_PCM_CHANNEL_RLC = 15,
    /* Rear right of center channel. */
    AUDIO_PCM_CHANNEL_RRC = 16,
    /* Secondary low frequency effect channel. */
    AUDIO_PCM_CHANNEL_LFE2 = 17,
    /* Side left channel. */
    AUDIO_PCM_CHANNEL_SL = 18,
    /* Side right channel. */
    AUDIO_PCM_CHANNEL_SR = 19,
    /* Top front left channel. */
    AUDIO_PCM_CHANNEL_TFL = 20,
    /* Left vertical height channel. */
    AUDIO_PCM_CHANNEL_LVH = AUDIO_PCM_CHANNEL_TFL,
    /* Top front right channel. */
    AUDIO_PCM_CHANNEL_TFR = 21,
    /* Right vertical height channel. */
    AUDIO_PCM_CHANNEL_RVH = AUDIO_PCM_CHANNEL_TFR,
    /* Top center channel. */
    AUDIO_PCM_CHANNEL_TC = 22,
    /* Top back left channel. */
    AUDIO_PCM_CHANNEL_TBL = 23,
    /* Top back right channel. */
    AUDIO_PCM_CHANNEL_TBR = 24,
    /* Top side left channel. */
    AUDIO_PCM_CHANNEL_TSL = 25,
    /* Top side right channel. */
    AUDIO_PCM_CHANNEL_TSR = 26,
    /* Top back center channel. */
    AUDIO_PCM_CHANNEL_TBC = 27,
    /* Bottom front center channel. */
    AUDIO_PCM_CHANNEL_BFC = 28,
    /* Bottom front left channel. */
    AUDIO_PCM_CHANNEL_BFL = 29,
    /* Bottom front right channel. */
    AUDIO_PCM_CHANNEL_BFR = 30,
    /* Left wide channel. */
    AUDIO_PCM_CHANNEL_LW = 31,
    /* Right wide channel. */
    AUDIO_PCM_CHANNEL_RW = 32,
    /* Left side direct channel. */
    AUDIO_PCM_CHANNEL_LSD = 33,
    /* Right side direct channel. */
    AUDIO_PCM_CHANNEL_RSD = 34
};

class CHReorder
{
public:
    CHReorder();
    CHReorder(uint32_t sampleCount,
              uint8_t channelCount,
              uint32_t bitDepth,
              uint8_t *inChMap,
              uint8_t *outChMap);
    ~CHReorder();

    void setSampleCount(uint32_t sampleCount);
    void setChannelCount(uint8_t channelCount);
    void setBitDepth(uint32_t bitDepth);
    QC2Status setSrcChMap(uint8_t *chMap, bool dft);
    QC2Status setTgtChMap(uint8_t *chMap, bool dft);

    void remap(uint8_t *out, uint8_t *in, uint32_t outSize);

private:
    uint32_t mSampleCount;
    uint8_t mChannels;
    uint32_t mBitDepth;
    uint8_t srcChannelMap[AUDIO_MAX_CHANNELS];
    uint8_t tgtChannelMap[AUDIO_MAX_CHANNELS];

    QC2Status setDefaultChMap(uint8_t *chMap);

    template<int n>
    void _remap(uint8_t *out, uint8_t *in, uint32_t outSize);

    static inline int8_t _find(uint8_t *targets, uint8_t count, uint8_t value);
};

};

#endif

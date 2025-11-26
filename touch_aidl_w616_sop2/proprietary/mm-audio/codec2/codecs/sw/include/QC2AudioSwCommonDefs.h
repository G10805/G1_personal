/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_AUDIO_SW_COMMON_DEFS_H_
#define _QC2_AUDIO_SW_COMMON_DEFS_H_

#include <cstdint>

typedef enum DecErr {
    DEC_EOF = -1,
    DEC_SUCCESS = 0,
    DEC_FAILURE,
    DEC_NOMEMORY_FAILURE,
    DEC_BADPARAM_FAILURE,
    DEC_NEED_MORE,
    DEC_FATAL_FAILURE,
} DecErr_t;

typedef enum InterfaceTypes {
    DEC_DEINIT,  /** Destroys the decoder instance */
    DEC_INIT,    /** Creates and Initializes decoder */
    DEC_RESET,   /** Resets/Flushes the decoder */
} InterfaceTypes_t;

typedef enum InterleavedFmt {
    FMT_INTERLEAVED = 1,
    FMT_DEINTERLEAVED = 2,
} InterleavedFmt_t;

typedef struct BufferMarkers {
    uint32_t mEosFlag : 1;
    uint32_t mReserved : 31;
} BufferMarkers_t;

typedef struct AudioBuf {
    uint8_t* mDataPtr;
    uint32_t mMaxDataLen;

    /* Actual size of data, different meaning in the context of usage.
     * Input buffer to decoder:
     *      Size of actual bitstream data as input to process call.
     *      After process call is done, decoder fills this variable
     *      with how much data is consumed
     * Output buffer to decoder :
     *      Irrelevant as input to process call.
     *      After process call is done, decoder fills this
     *      variable with how much data is produced
     */
    uint32_t mActualDataLen;
    BufferMarkers_t mMarker;
} AudioBuf_t;

typedef struct GenericDecMediaFmt {
    /* Default media format params for all decoders, can be extented
     * with extra payload at end of this structure which has mutual
     * understanding between decoder and client.
     */
    uint32_t mBitsPerSample;
    uint32_t mSampleRate;
    uint32_t mNumChannels;
    uint8_t  mChannelMap[qc2audio::ChannelHelper::MAX_NUM_CHANNELS];
    InterleavedFmt_t mInterleavedType;
    void*    mDecSpecificFmt;
} GenericDecMediaFmt_t;

typedef struct {
    short   tty_dec_flag;
    short   tty_dec_header;
    short   tty_dec_char;
    short   tty_dec_baud_rate;
    short   tty_option;
    short   counter_hist[11];
    short   char_hist[11];
    short   tty_rate_hist[11];
    short   stop_bit_len[2];    // initialized in init_tty_dec() & init_tty_rxtx()
    short   data_bit_len[2];    // initialized in init_tty_dec() & init_tty_rxtx()
    short   bit_num;
    short   bit_size;
    short   current_counter;
    short   current_char;
    short   prev_bit;
    short   tone_param[3];
    short   fFigure;          // flag to indicate FIGS or LTRS mode
    short   AsciiChar;
    short   vocoderType;
    short   TTY_SILENCE_HDR;  // vocoder type dependent macros
    short   TTY_ONSET_HDR;
    short   TTY_CHAR_HDR_STOP;
    short   TTY_COUNTER_STOP;
    short   COUNTER_BETWEEN_START_STOP;
    short   TTY_MIN_INPUT_THRESH;
    short   tty_sin_tab_idx;   // used by tty_continuous_phase_tone_gen()
} TTYDecStruct;

#endif  // _QC2_AUDIO_SW_COMMON_DEFS_H_

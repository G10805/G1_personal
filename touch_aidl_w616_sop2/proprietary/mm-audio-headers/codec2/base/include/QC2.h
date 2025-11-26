/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_H_
#define _QC2AUDIO_H_

#include <C2.h>

// logging
#ifdef __ANDROID__
#include "android/QC2Log.h"
#else  // linux/win
#include "host/QC2Log.h"
#endif

#include "QC2OS.h"

namespace qc2audio {

#define DECLARE_NON_COPYASSIGNABLE(className)        \
    className(const className&) = delete;            \
    className(const className&&) = delete;           \
    className& operator=(const className&) = delete; \
    className& operator=(const className&&) = delete;\

#define DECLARE_NON_CONSTRUCTIBLE(className)       \
    ~className() = delete;                         \

// c2_status_t + internal codes
enum QC2Status : int32_t {
    QC2_OK        = C2_OK,                   ///< operation completed successfully

    // bad input
    QC2_BAD_VALUE = C2_BAD_VALUE,            ///< argument has invalid value (user error)
    QC2_BAD_INDEX = C2_BAD_INDEX,            ///< argument uses invalid index (user error)
    QC2_CANNOT_DO = C2_CANNOT_DO,   ///< argument/index is valid but not possible

    // bad sequencing of events
    QC2_DUPLICATE = C2_DUPLICATE,    ///< object already exists
    QC2_NOT_FOUND = C2_NOT_FOUND,    ///< object not found
    QC2_BAD_STATE = C2_BAD_STATE,    ///< operation is not permitted in the current state

    // bad environment
    QC2_NO_MEMORY = C2_NO_MEMORY,            ///< not enough memory to complete operation
    QC2_REFUSED   = C2_REFUSED,    ///< missing permission to complete operation

    QC2_TIMED_OUT = C2_TIMED_OUT,            ///< operation did not complete within timeout

    // bad versioning
    QC2_OMITTED   = C2_OMITTED,  ///< operation is not implemented/supported (optional only)

    // unknown fatal
    QC2_CORRUPTED = C2_CORRUPTED,   ///< some unexpected error prevented the operation
    QC2_NO_INIT   = C2_NO_INIT,         ///< status has not been initialized

    // internal error codes
    QC2_ERROR = -1,             ///< unknown error
    QC2_BAD_ARG = -2,           ///< invalid argument
};

#define QC2_RETURN_IF(cond, msg) \
    if (cond) {                 \
        QLOGE(#cond msg);       \
        return QC2_ERROR;       \
    }                           \

/**
 * secure mem ops
 */
#ifndef memcpy_s
size_t memcpy_s(void *dst, size_t dst_size, const void *src, size_t src_size);
#endif  // memcpy_s

static constexpr const char *kFileDumpPath = "/data/vendor/media/qc2/";

template <typename T>
inline void InitStruct(T* _t) {
    if (_t) {
        memset(_t, 0x0, sizeof(T));
    }
}

#define ALIGN(num, to) (((num) + (to-1)) & (~(to-1)))

template <typename U, typename T,
    typename =typename std::enable_if<std::is_integral<T>::value>::type,
    typename =typename std::enable_if<std::is_integral<U>::value>::type>
inline T Align(T val, U b) {
    return ((val + (b - 1)) & (~(b - 1)));
}

static inline uint64_t Pack64(uint32_t x, uint32_t y) {
    return (uint64_t)((uint64_t)x << 32 | (uint64_t)y);
}

static inline uint32_t ExtractHigh32(uint32_t x) {
    return (uint32_t)((uint64_t)x >> 32);
}

static inline uint32_t ExtractLow32(uint32_t y) {
    return (uint32_t)((uint64_t)y & 0xFFFFFFFF);
}

static inline uint32_t Pack32(uint16_t x, uint16_t y) {
    return (uint32_t)((uint32_t)x << 16 | (uint32_t)y);
}

static inline uint16_t ExtractHigh16(uint32_t x) {
    return (uint16_t)((uint32_t)x >> 16);
}

static inline uint16_t ExtractLow16(uint32_t y) {
    return (uint16_t)((uint32_t)y & 0xFFFF);
}

};  // namespace qc2audio

#endif  // _QC2AUDIO_H_

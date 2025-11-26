/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_PLATFORM_BUFFER_
#define _QC2AUDIO_PLATFORM_BUFFER_

#include <C2Config.h>
#include "QC2.h"

namespace qc2audio {

/**
 * @brief Platform buffer interface class
 *
 * This class provides the interfaces of platform buffer
 */
class QC2PlatformBuffer {
 public:
    /**
     * @brief create the interface of QC2PlatformBuffer
     *
     * This method will get the interface of QC2PlatformBuffer
     *
     * @param[in]       input       The pointer of C2Handle
     * @return          std::unique_ptr<QC2PlatformBuffer> the instance of QC2PlatformBuffer
     */
    static std::unique_ptr<QC2PlatformBuffer> Create(const C2Handle* hnd);

    virtual ~QC2PlatformBuffer() = default;

    /// get buffer fd
    virtual int fd() = 0;
    /// get allocated size of buffer
    virtual uint32_t allocSize() = 0;
    /// get data offset
    virtual uint32_t offset() = 0;
    /// get (unaligned) width of image in pixels
    virtual uint32_t width() = 0;
    /// get (unaligned) height of image in pixels
    virtual uint32_t height() = 0;
    /// get stride (aligned width) in pixels
    virtual uint32_t stride() = 0;
    /// get slice-height (aligned height) in pixels
    virtual uint32_t sliceHeight() = 0;
    /// get color-format
    virtual uint32_t format() = 0;
    /// get buffer flags
    virtual uint32_t flags() = 0;
    /// get unique id
    virtual uint64_t id() = 0;

    /**
     * @brief Get the metadata from platform buffer
     *
     * This method will get the metadata from platform buffer
     *
     * @param[in]       input       The reference of vector with C2Param::Type
     * @param[in, out]  output      The pointer of the vector with C2Param
     * @return          QC2_OK      Get metadata successfully
     * @return          QC2_ERROR   Failed to get metadata
     */
    virtual QC2Status getMetadata(const std::vector<C2Param::Type>& types,
            std::vector<std::unique_ptr<C2Param>>* infos) = 0;
    virtual QC2Status getMetadata(std::vector<std::unique_ptr<C2Info>>* infos,
            std::vector<std::unique_ptr<C2Param>>* params) = 0;

    /**
     * @brief Set the C2Infos in the metadata of platform buffer
     *
     * This method will set C2Infos in the metadata of platform buffer
     *
     * @param[in]       input       The reference of vector with C2Info
     *
     * @return          QC2_OK      Set metadata successfully
     * @return          QC2_ERROR   Failed to set metadata
     */
    virtual QC2Status setMetadata(const std::vector<std::shared_ptr<C2Info>>& infos) = 0;

 protected:
    explicit QC2PlatformBuffer(const C2Handle* hnd __unused) {}

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2PlatformBuffer);
};  // class QC2PlatformBuffer
};  // namespace qc2audio
#endif  // _QC2AUDIO_PLATFORM_BUFFER_

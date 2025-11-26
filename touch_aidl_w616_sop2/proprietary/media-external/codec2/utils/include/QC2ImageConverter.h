/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_IMAGE_CONVERTER_H_
#define _QC2_IMAGE_CONVERTER_H_

#include <vector>
#include <memory>

#include "QC2.h"
#include "QC2Constants.h"
#include "QC2Buffer.h"

namespace qc2 {

/**
 * @brief Image converter utility
 *
 * This utitlity class provides image conversion routines for pixel buffers.
 * Supported conversions include
 *   -# color-format conversion
 *   -# rotation
 *   -# flip/mirror
 */
class QC2ImageConverter {
 public:

    /**
     * @brief Image converter capabilities
     */
    struct Capabilities {
        /**
         * @brief Query the supported destination pixel formats that the engine can convert to,
         *        given the source pixel-format
         *
         * @param[in] fromFormat source pixel format (one of qc2::PixFormat)
         * @return list of supported destination formats for the specified source format
         */
        const std::vector<uint32_t>& canColorConvertTo(uint32_t fromFormat) const;

        /// Supported rotation angles
        enum RotationAngle : uint32_t {
            ROTATION_0 = 0,     //< no rotation
            ROTATION_90 = 1,    //< rotate clockwise by 90 degrees
            ROTATION_180 = 2,   //< rotate clockwise by 180 degrees
            ROTATION_270 = 3,   //< rotate clockwise by 270 degrees
        };

        /**
         * @brief Query the supported rotation angles
         *
         * @return list of supported rotation angles
         */
        const std::vector<RotationAngle>& canRotate() const;

        /// Supported flip directions
        enum FlipDirection : uint32_t {
            FLIP_NONE = 0,        //< no flip
            FLIP_VERTICAL = 1,    //< flip left to right along the horizontal axis
            FLIP_HORIZONTAL = 2,  //< flip top to bottom along the vertical axis
            FLIP_BOTH = 3,        //< flip horizontal + vertical
        };
        /**
         * @brief Query the supported flip directions
         *
         * @return list of supported flip directions
         */
        const std::vector<FlipDirection>& canFlip() const;
    };

    /**
     * @brief Query the engine's capabilities
     */
    static Capabilities *GetCapabilities();

    /// Format specifier
    struct Format {
        uint32_t pixelFormat = PixFormat::INVALID;   /// one of qc2::PixFormat
        uint32_t width = 0u;                         /// width of the image in pixels
        uint32_t height = 0u;                        /// height of the image in pixels
    };

    /**
     * @brief Converter engine builder
     *
     * Builds a converter engine with the transformations requested
     *
     * @note: The order of image transformations (rotation and flip) matters. They shall be
     *        applied in the same order specified. But please note that we only support setting
     *        rotate and flip once.
     *        Eg: Converters obtained with following build orders produce different results      \n
     *            b1 = Builder(RGBA8888).withRotation(ROTATION_90).withFlip(FLIP_HORIZONTAL)     \n
     *            b2 = Builder(RGBA8888).withFlip(FLIP_HORIZONTAL).withRotation(ROTATION_90)     \n
     *
     */
    struct Builder {
        /// create a builder for specified source format
        explicit Builder(const Format& sourceFormat);

        /// (optional) destination pixel-format. Defaults to source pixel-format
        Builder& withDestinationColor(uint32_t dstPixelFormat);

        /// (optional) rotation(clockwise). Defaults to no rotation
        Builder& withRotation(Capabilities::RotationAngle angle);

        /// (optional) flip. Defaults to no flip
        Builder& withFlip(Capabilities::FlipDirection direction);

        /// (optional) secure. Defaults to false
        Builder& withSecureContext(bool isSecure = false);

        /// (optional) max stash buffer num. Defaults to 0
        Builder& withMaxStashBufferNum(uint32_t maxStashBufferNum = 0);

        /// build the converter for specified transformations
        std::unique_ptr<QC2ImageConverter> build();

    private:
        Format mInputFormat;
        Format mOutputFormat;
        Capabilities::RotationAngle mRotateAngle;
        Capabilities::FlipDirection mFlipDirection;
        bool mIsSecureContext;
        uint32_t mMaxStashBufferNum;
    };

    /**
     * @brief Get the destination (output) format computed based on source and requested
     *        transformations
     *
     * @note: clients may use this to allocate compatible destination buffers
     *
     * @return destination format
     */
    const Format& destinationFormat() const;

    /**
     * @brief Process input (apply transformations) and produce output
     *
     * This method will produce output buffer based on the requested transformations
     * It is imperative that the input and output graphic buffers match the source and
     * destination formats respectively
     *
     * @param[in]       input       the source buffer need to be  transformed
     * @param[in, out]  output      the destination buffer handle
     *
     * @return          QC2_OK      output processed successfully
     * @return          QC2_REFUSED either the source or destination buffer has incompatible format
     * @return          QC2_ERROR   process failed
     */
    QC2Status process(QC2Buffer::Graphic *input, QC2Buffer::Graphic *output);

    ~QC2ImageConverter();

    class Engine {
    public:
        Engine() {}
        virtual ~Engine() {}

        struct Transform {
            enum FlipType : uint32_t {
                NONE,
                VERTICAL,
                HORIZONTAL,
                BOTH,
            };
            uint32_t rotationAngle;
            FlipType flip;
            Transform() : rotationAngle(0), flip(FlipType::NONE) {}
            bool operator == (const Transform &rhs) const {
                return rotationAngle == rhs.rotationAngle && flip == rhs.flip;
            }
            bool operator != (const Transform &rhs) const {
                return !(this == &rhs);
            }
            Transform &operator = (const Transform &rhs) {
                if (this == &rhs) {
                    return *this;
                }
                rotationAngle = rhs.rotationAngle;
                flip = rhs.flip;
                return *this;
            }
            bool isActivated() const {
                return (rotationAngle != 0) || (flip != FlipType::NONE);
            }
        };

        virtual QC2Status convert(QC2Buffer::Graphic *input, QC2Buffer::Graphic *output,
                const Transform &transform = Transform{}) = 0;

    protected:
        struct Format {
            uint32_t mWidth = 0u;
            uint32_t mHeight = 0u;
            uint32_t mFormat = 0u;
            uint32_t mStride = 0u;
            uint32_t mFlags = 0u;

            Format() = default;

            Format(uint32_t width, uint32_t height, uint32_t format,
                    uint32_t stride, uint32_t flags)
                : mWidth(width), mHeight(height), mFormat(format), mStride(stride), mFlags(flags) {
            }

            bool operator == (const Format &rhs) const {
                return mWidth == rhs.mWidth && mHeight == rhs.mHeight && mFormat == rhs.mFormat
                        && mStride == rhs.mStride && mFlags == rhs.mFlags;
            }

            Format &operator = (const Format &rhs) {
                if (this == &rhs) {
                    return *this;
                }
                mWidth = rhs.mWidth;
                mHeight = rhs.mHeight;
                mFormat = rhs.mFormat;
                mStride = rhs.mStride;
                mFlags = rhs.mFlags;
                return *this;
            }
        };

    };

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2ImageConverter);
    friend struct Builder;

    QC2ImageConverter(const Format& inputFormat, const Format& outputFormat,
            Capabilities::RotationAngle rotateAngle, Capabilities::FlipDirection flipDirection, bool isSecureContext, uint32_t maxStashBufferNum);

    std::unique_ptr<Engine> mEngine;
    Format mInputFormat;
    Format mOutputFormat;
    Capabilities::RotationAngle mFinalRotateAngle;
    Capabilities::FlipDirection mFinalFlipDirection;
    bool mIsSecureContext;
    uint32_t mMaxStashBufferNum;
};

};  // namespace qc2

#endif  // _QC2_IMAGE_CONVERTER_H_

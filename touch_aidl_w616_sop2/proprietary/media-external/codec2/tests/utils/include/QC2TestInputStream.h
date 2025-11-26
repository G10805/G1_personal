/*
 **************************************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_TESTUTILS_TEST_INPUTSTREAM_H_
#define _QC2_TESTUTILS_TEST_INPUTSTREAM_H_

#include "QC2Constants.h"
#include "QC2Buffer.h"
//#include "QC2Config.h"

namespace qc2 {

class TestInputSource {
public:
    virtual ~TestInputSource() = default;
    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) = 0;

    void setAuxInfo(std::unique_ptr<Bundle> info) {
        mAuxInfo = std::move(info);
    }

protected:
    std::unique_ptr<Bundle> mAuxInfo;
};

class CPZSession {
 public:
    CPZSession();
    ~CPZSession();

    bool valid();
    QC2Status copyToSecure(
            std::shared_ptr<QC2Buffer> nonSecure,
            std::shared_ptr<QC2Buffer> secure);

 private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

class TestInputStream : public TestInputSource {
public:
    static const size_t kfixedInputBufferSize = 4*1024*1024;
    static QC2Status Create(
            const std::string& mime,
            const std::string& fileName,
            std::shared_ptr<QC2LinearBufferPool> bufPool,
            std::unique_ptr<TestInputStream> *stream);

    TestInputStream(std::shared_ptr<QC2LinearBufferPool> bufPool)
        : mIsValid(false),
          mBufPool(bufPool),
          mSecureBufPool(nullptr),
          mCPZSession(nullptr),
          mAllocSize(0u),
          mSecureAllocSize(0u),
          mRequeueCSD(false) {
    }

    virtual ~TestInputStream() = default;

    bool valid() const {
        return mIsValid;
    }

    void setBufferSize(size_t size) {
        mAllocSize = size;
    }

    // for embedded stream, when looping, CSD is re-queued (as work) when stream rewinds back
    void enableRequeuCSD() {
        mRequeueCSD = true;
    }

    QC2Status enableSecureMode(
            std::shared_ptr<QC2LinearBufferPool> securePool,
            size_t secureAllocSize) {
        mSecureBufPool = securePool;
        mSecureAllocSize = secureAllocSize;
        mCPZSession = std::shared_ptr<CPZSession>(new CPZSession);
        if (!isSecure()) {
            return QC2_ERROR;
        }
        return QC2_OK;
    }
protected:
    bool mIsValid;
    std::shared_ptr<QC2LinearBufferPool> mBufPool;
    std::shared_ptr<QC2LinearBufferPool> mSecureBufPool;
    std::shared_ptr<CPZSession> mCPZSession;
    const size_t kMaxAllocSize = 4 * 1024 * 1024;   // 4 MB
    size_t mAllocSize;
    size_t mSecureAllocSize;
    bool mRequeueCSD;

    static bool FileExists(const std::string& fileName);

    QC2Status allocAndMapBuf(
            uint32_t size,
            std::shared_ptr<QC2Buffer> *buf,
            std::unique_ptr<QC2Buffer::Mapping>* map);

    inline bool isSecure() const {
        QLOGD("isSecure: pool=%p cpz=%p",
                mSecureBufPool.get(), mCPZSession.get());
        if (mCPZSession) {
            QLOGD("cpz-valid=%d", mCPZSession->valid());
        }
        return mSecureBufPool && mCPZSession && mCPZSession->valid();
    }

    QC2Status secureCopy(
            std::shared_ptr<QC2Buffer> nonSecureFrame,
            std::shared_ptr<QC2Buffer> *secureFrame);
};

class QC2GraphicInputSource : public TestInputSource {
public:
    static QC2Status Create(
            uint32_t width,
            uint32_t height,
            uint32_t format,
            const std::string& fileName,
            const std::string& patternName,
            std::shared_ptr<QC2GraphicBufferPool> bufPool,
            std::unique_ptr<QC2GraphicInputSource> *source);

    QC2GraphicInputSource(
            uint32_t width, uint32_t height, uint32_t format,
            std::shared_ptr<QC2GraphicBufferPool> bufPool)
        : mWidth(width), mHeight(height), mFormat(format),
          mFrameDurationUs(33333u),
          mHasColorAspects(false),
          mBufPool(bufPool) {
    };

    virtual void getFrameSize(uint32_t *size) = 0;

    bool isValid();

    void setFrameDuration(uint32_t durationUs) {
        mFrameDurationUs = durationUs;
    }

    void setGraphicBufferPool(std::shared_ptr<QC2GraphicBufferPool> pool) {
        mBufPool = pool;
    }

    void setColorAspects(C2Color::primaries_t primaries, C2Color::range_t range,
            C2Color::matrix_t matrix, C2Color::transfer_t transfer) {
        mHasColorAspects = true;
        mColorPrimaries = primaries;
        mColorRange = range;
        mColorMatrix = matrix;
        mColorTransfer = transfer;
    }

    virtual ~QC2GraphicInputSource() = default;

protected:
    bool mValid;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mFormat;
    uint32_t mFrameSize;
    uint32_t mFrameDurationUs;
    bool mHasColorAspects;
    C2Color::primaries_t mColorPrimaries = C2Color::PRIMARIES_UNSPECIFIED;
    C2Color::range_t mColorRange = C2Color::RANGE_UNSPECIFIED;
    C2Color::matrix_t mColorMatrix = C2Color::MATRIX_UNSPECIFIED;
    C2Color::transfer_t mColorTransfer = C2Color::TRANSFER_UNSPECIFIED;
    std::shared_ptr<QC2GraphicBufferPool> mBufPool;

    QC2Status allocAndMapBuf(
            std::shared_ptr<QC2Buffer> *buf,
            std::unique_ptr<QC2Buffer::Mapping>* map);

    QC2Status updateColorAspects(std::shared_ptr<QC2Buffer> buf);

};

}   // namespace qc2

#endif //  _QC2_TESTUTILS_TEST_INPUTSTREAM_H_


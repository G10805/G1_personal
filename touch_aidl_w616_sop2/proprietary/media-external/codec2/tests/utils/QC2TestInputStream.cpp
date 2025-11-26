/*
 **************************************************************************************************
 * Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#define LOG_TAG "TestInputStream"
#include <iostream>
#include <list>
#include <unordered_map>
#include <string.h>
#include <arpa/inet.h>

#include "QC2V4L2Config.h"
#include "QC2Platform.h"
#include "QC2StreamParser.h"
#include "QC2TestInputStream.h"
#include <dlfcn.h>

namespace qc2 {

/*
static void hexdump(const char* prefix, const uint8_t *m) {
    QLOGD("%s: %02x %02x %02x %02x    %02x %02x %02x %02x", prefix,
            m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7]);
}
*/

//-------------------------------------------------------------------------------------------------
// AVC stream reader : reads .264 (Annex-B) streams
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "AVCFileInputStream"
class AVCFileInputStream : public TestInputStream {
public:
    AVCFileInputStream(
            const std::string& fileName,
            std::shared_ptr<QC2LinearBufferPool> bufPool);

    virtual ~AVCFileInputStream();

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;

private:
    static constexpr const uint32_t kStartCodeSize = 4;
    static constexpr const uint32_t kFetchSize = 1024 * 1024;
    static constexpr const uint64_t kFrameDurationUs = 33333;

    FILE *mFp;
    uint64_t mTimeStamp;
    uint32_t mPos;

    QC2Status searchNextNAL(uint32_t *pos, uint16_t* nalType, uint16_t *nextNalType);

    class Cache {
    public:

        static constexpr const uint32_t kCacheCapacity = 4 * 1024 * 1024;

        Cache()
            : mFp(nullptr),
              mMaxOffset(0),
              mCacheSize(0),
              mCacheOffset(0) {
            mCache = (uint8_t *)malloc(sizeof(uint8_t) * kCacheCapacity);
        }

        void init(FILE *fp) {
            mFp = fp;
            if (!mFp) {
                return;
            }
            fseek(mFp, 0, SEEK_END);
            mMaxOffset = ftell(mFp);
            fseek(mFp, 0, SEEK_SET);
        }

        ~Cache() {
            if (mCache) {
                free(mCache);
            }
        }

        const uint8_t *access(uint32_t start, uint32_t *size) {
            if (!size) {
                return nullptr;
            }
            QLOGI("access in [%x, %u]", start, *size);
            if (!mCache || start >= mMaxOffset) {
                *size = 0;
                return nullptr;
            }
            if (start + *size >= mMaxOffset) {
                *size = mMaxOffset - start;
            }
            // cache hit
            if ((start >= mCacheOffset) &&
                    ((start + *size) <= mCacheOffset + mCacheSize)) {
                QLOGI("access out (%p) [%x, %u]", mCache + start - mCacheOffset, start, *size);
                //hexdump("cache-hit", mCache + start - mCacheOffset);
                return mCache + start - mCacheOffset;
            }

#if 0
            // TODO(PC) optimization

            // partial hit or miss.. fetch more
            uint32_t avail = 0;
            if (mCacheSize + mCacheOffset > start) {
                avail = mCacheSize + mCacheOffset - start;
            }
            uint32_t toFetch = kCacheCapacity - avail;
            memmove(mCache, mCache + (start - mCacheOffset), avail);
            fseek(mFp, start, SEEK_SET);
            auto nFetched = fread(mCache + avail, 1, toFetch, mFp);
            QLOGI("access fetch from file at %u for %u", start, (uint32_t)nFetched);
            mCacheOffset = start;
            mCacheSize = avail + nFetched;
            QLOGI("access out (%p) [%u, %u]", mCache, start, *size);
            hexdump("cache-miss", mCache);
#else
            fseek(mFp, start, SEEK_SET);
            auto nFetched = fread(mCache, 1, kCacheCapacity, mFp);
            mCacheOffset = start;
            mCacheSize = nFetched;
            QLOGI("access out (%p) [%x, %u]", mCache, start, *size);
            //hexdump("cache-miss", mCache);
#endif
            return mCache;
        }

    private:
        FILE *mFp;
        uint8_t *mCache;
        uint32_t mMaxOffset;    // file size
        uint32_t mCacheSize;
        uint32_t mCacheOffset;
    };
    Cache mCache;
};

AVCFileInputStream::AVCFileInputStream(
        const std::string& fileName,
        std::shared_ptr<QC2LinearBufferPool> bufPool)
    : TestInputStream(bufPool) {
    mIsValid = false;
    mFp = fopen(fileName.c_str(), "rb");
    if (!mFp) {
        QLOGE("Failed to open file %s", fileName.c_str());
        return;
    }
    mCache.init(mFp);
    mTimeStamp = 0;
    mPos = 0;
    mIsValid = true;
}

AVCFileInputStream::~AVCFileInputStream() {
    if (mFp) {
        fclose(mFp);
    }
}

QC2Status AVCFileInputStream::searchNextNAL(
        uint32_t *pos, uint16_t *nalType, uint16_t *nextNALType) {
    QLOGI("FindNAL: from %x", *pos);
    for (;;) {
        uint32_t size = kFetchSize;
        auto mem = mCache.access(*pos, &size);
        if (!mem || !size || size <= kStartCodeSize) {
            return QC2_NOT_FOUND;
        }
        uint32_t p = 0;
        if (QC2AVCStreamParser::FindNAL(mem, size, &p, nalType, nextNALType) == QC2_OK) {
            *pos += p;
            QLOGD("StartCode found at %x", *pos);
            return QC2_OK;
        }
        *pos += p;
    }
    return QC2_CANNOT_DO;
}

QC2Status AVCFileInputStream::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!mFp || !mIsValid) {
        return QC2_NO_INIT;
    }
    uint32_t p = mPos;
    uint32_t nalStart = mPos;
    bool eos = false;
    uint64_t flags = 0;

    for (;;) {
        uint32_t size = kStartCodeSize + 1;

        using AVCNALType = QC2AVCStreamParser::NalUnitType;
        uint16_t nalType = AVCNALType::UNKNOWN;
        uint16_t nextNalType = AVCNALType::UNKNOWN;
        if (searchNextNAL(&p, &nalType, &nextNalType) != QC2_OK) {
            QLOGI("no more SCs: p=%x start=%x", p, nalStart);
            if (p <= (nalStart + kStartCodeSize)) {
                QLOGW("AVCFileInputStream: already at end");
            }
            size = 4096 * 1024;
            mCache.access(p, &size);
            p += size;
            eos = true;
            flags |= BufFlag::EOS;
            break;
        }

        QLOGD("NAL TYPE = %u , next = %u", nalType, nextNalType);
        if (nalType == AVCNALType::SPS || nalType == AVCNALType::PPS) {
            flags |= BufFlag::CODEC_CONFIG;
            break;
        } else if (nalType == AVCNALType::IDR || nalType == AVCNALType::NON_IDR) {
            break;
        }
        nalStart = p;
    }
    uint32_t frameSize = p - mPos;
    QLOGD("     frame-sz=%u", frameSize);

    size_t allocSize = mAllocSize ? mAllocSize : frameSize;

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    QC2Status ret = allocAndMapBuf(allocSize, &buf, &mapping);
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("AVCFileInputStream::nextFrame: Failed to allocate/map input buffer!");
        return ret;
    }

    for (uint32_t written = 0; written < frameSize;) {
        uint32_t size = frameSize - written;
        auto mem = mCache.access(mPos, &size);
        if (!mem || !size) {
            QLOGE("AVCFileInputStream:: error fetching from file cache");
            return QC2_CORRUPTED;
        }
        memcpy_s(mapping->baseRW() + written, mapping->capacity(), mem, size);
        QLOGD("writing frame %u bytes", size);
        written += size;
        mPos += size;
    }
    buf->setTimestamp(mTimeStamp);
    buf->setFlags(flags);
    buf->linear().setRange(0, frameSize);
    mTimeStamp += kFrameDurationUs;

#if 0
    // create an embedded stream file
    do {
        static int count = 0;
        const char *streamName = "stream";
        FILE *fp = fopen("/sdcard/stream.cpp", "a");
        if (!fp) {
            break;
        }
        fprintf(fp, "static uint64_t %s__ts_%03d = %lld;\n",
                streamName, count, (long long)mTimeStamp);
        fprintf(fp, "static uint64_t %s__flags_%03d = %lld;\n",
                streamName, count, (long long)flags);
        fprintf(fp, "static uint8_t %s__blob_%03d[] = {\n\t\"", streamName, count);
        auto m = buf->linear().map();
        QLOGD("dumping frame %u bytes", buf->linear().size());
        hexdump("dumping frame", m->base());
        if (buf->linear().size() > 16) {
            hexdump("                frame .. ", m->base() + 8);
        }
        const uint8_t *p = m->base();
        for (int i = 0; i < (int)buf->linear().size(); ++i) {
            fprintf(fp, "\\x%02x", p[i]);
            if (i > 0 && i % 20 == 0) {
                fprintf(fp, "\"\n\t\"");
            }
        }
        fprintf(fp, "\"\n};\n\n");
        ++count;
        fclose(fp);
    } while (false);
#endif

    if (isSecure()) {
        return secureCopy(buf, frame);
    }
    *frame = buf;
    return QC2_OK;
}

//-------------------------------------------------------------------------------------------------
// IVF stream reader
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "IvfFileInputStream"

// TODO(PC): AVC/Ivf stream readers must support wrapping-over to the beginning and play N frames
//  move Cache out of AVC and reuse for all stream parsers
class IvfFileInputStream : public TestInputStream {
public:
    IvfFileInputStream(
            const std::string& fileName,
            std::shared_ptr<QC2LinearBufferPool> bufPool);

    virtual ~IvfFileInputStream();

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;

private:
    static constexpr const uint32_t kFileHeaderSize = 32;
    static constexpr const uint32_t kFrameHeaderSize = 12;
    static constexpr const uint64_t kFrameDurationUs = 33333;
    FILE *mFp;
    size_t mMaxFileOffset = 0u;      // TODO(PC): Use file Cache here from AVCFileInputStream
    uint64_t mTimeStamp = 0llu;
    QC2Status readFileHeader();

};

IvfFileInputStream::IvfFileInputStream( const std::string& fileName,
            std::shared_ptr<QC2LinearBufferPool> bufPool)
            : TestInputStream(bufPool) {
    mIsValid = false;
    mFp = fopen(fileName.c_str(), "rb");
    if (!mFp) {
        QLOGE("Failed to open file %s", fileName.c_str());
        return;
    }
    {
        fseek(mFp, 0, SEEK_END);
        mMaxFileOffset = ftell(mFp);
        fseek(mFp, 0, SEEK_SET);
    }
    mIsValid = true;
    mTimeStamp = 0;
    readFileHeader();
}

IvfFileInputStream::~IvfFileInputStream() {
    if (mFp) {
        fclose(mFp);
    }
}

QC2Status IvfFileInputStream::readFileHeader() {
    uint8_t ivfRaw[sizeof(IVFStreamHeader)];
    if (fread(ivfRaw, 1, sizeof(IVFStreamHeader), mFp) != sizeof(IVFStreamHeader)) {
        QLOGE("Failed to read IVF file header");
        return QC2_ERROR;
    }
    auto ivf = IVFStreamHeader::FromBytes(sizeof(IVFStreamHeader), ivfRaw);
    if (ivf == nullptr) {
        QLOGE("Failed to construct IVF file header");
        return QC2_ERROR;
    }
    auto& f = ivf->fourCC;
    QLOGI("IVF: FourCC(%c%c%c%c) w(%u)xh(%u) fps=%u/%u frames(%u)", f[0], f[1], f[2], f[3],
            ivf->width, ivf->height, ivf->timebaseDenom, ivf->timebaseNom, ivf->numFrames);
    return QC2_OK;
}

QC2Status IvfFileInputStream::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!mFp || !mIsValid || !frame) {
        return QC2_NO_INIT;
    }
    uint8_t ivfRaw[sizeof(IVFFrameHeader)];
    uint64_t flags = 0;

    size_t bytesRead = fread(ivfRaw, 1, sizeof(IVFFrameHeader), mFp);
    if (bytesRead < sizeof(IVFFrameHeader)) {
        QLOGE("Failed to read frame header (read only %zu bytes)", bytesRead);
        if (feof(mFp)) {
            QLOGI("already at EOF.. !!!!");
        }
        return QC2_ERROR;
    }

    auto ivf = IVFFrameHeader::FromBytes(sizeof(IVFFrameHeader), ivfRaw);
    if (ivf == nullptr) {
        QLOGE("Failed to construct IVF frame header");
        return QC2_CORRUPTED;
    }
    QLOGD("frame-sz=%u ts:%lld", ivf->frameSize, (long long)(ivf->timeStampUs));

    size_t allocSize = mAllocSize ? mAllocSize : ivf->frameSize;

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    QC2Status ret = allocAndMapBuf(allocSize, &buf, &mapping);
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("AVCFileInputStream::nextFrame: Failed to allocate/map input buffer!");
        return ret;
    }
    uint32_t filledSize = 0;
    if (fread(mapping->baseRW(), 1, ivf->frameSize, mFp) != ivf->frameSize) {
        QLOGE("Failed reading full frame from file");
        return QC2_CORRUPTED;
    } else {
        filledSize = ivf->frameSize;
        QLOGD("writing frame %u bytes", ivf->frameSize);
    }

    // we've reached the end if there are no more than atleast header-size bytes
    auto pos = ftell(mFp);
    if ((mMaxFileOffset - pos) < sizeof(IVFFrameHeader)) {
    // if (feof(mFp)) {
        QLOGI("Last frame..");
        flags |= BufFlag::EOS;
    }
    buf->setTimestamp(mTimeStamp);
    buf->setFlags(flags);
    buf->linear().setRange(0, filledSize);
    mTimeStamp += kFrameDurationUs;
    *frame = buf;
    return QC2_OK;
}

//-------------------------------------------------------------------------------------------------
// Stream reader from embedded streams
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "EmbeddedInputStream"
// Data and metadata corresponding to each frame
struct EmbeddedFrame {
    uint32_t mLen;          // length of data in bytes
    uint64_t mTimeStampUs;  // timestamp in us
    uint64_t mFlags;        // flags (BufFlag)
    const uint8_t *mData;   // frame-data (bitstream)
    C2VideoCROPStruct mCrop; // Crop info
    C2VideoMISR *mMisr; // Misr info
};

// Stream reader for embedded streams
// Stream reader loops by-default. i.e getNextFrame rewinds back to first frame when frames are
//  exhausted (while skipping the headers - ones marked CODEC_CONFIG)
class EmbeddedInputStream : public TestInputStream {
public:
    // check if there exists an embedded-stream for the given name (resource-id)
    static bool HasStreamForName(const std::string& name) {
        auto itr = mStreamLoaders.find(name);
        return itr != mStreamLoaders.end();
    }

    virtual ~EmbeddedInputStream() = default;

    EmbeddedInputStream(
            const std::string& streamName,
            std::shared_ptr<QC2LinearBufferPool> bufPool)
        : TestInputStream(bufPool) {
        mIsValid = HasStreamForName(streamName);
        if (!mIsValid) {
            return;
        }
        auto& loader = mStreamLoaders[streamName];
        loader(&mFrames);
        QLOGI("Loaded embedded stream [%s] - has %zu frames", streamName.c_str(), mFrames.size());
        mFrameItr = mFrames.begin();
        mLastSegmentTimeBaseUs = 0;
        mLoopCount = 0;
    }

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;

private:
    QC2Status addMetadataToFrame(std::shared_ptr<QC2Buffer> buffer);
    std::list<EmbeddedFrame> mFrames;
    std::list<EmbeddedFrame>::iterator mFrameItr;
    //bool mLooping; default this behavior for now
    uint64_t mLastSegmentTimeBaseUs;
    uint32_t mLoopCount;

    // A Store of all available embedded streams
    using StreamLoaderStore_t = std::unordered_map<std::string,
            std::function<void(std::list<EmbeddedFrame> *)>>;
    static StreamLoaderStore_t mStreamLoaders;
};

QC2Status EmbeddedInputStream::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!mIsValid) {
        QLOGE("nextFrame called on an invalid stream..");
        return QC2_NO_INIT;
    }
    if (!frame) {
        QLOGE("EmbeddedInputStream::nextFrame: Invalid buffer handle passed");
        return QC2_BAD_ARG;
    }

    auto& f = *mFrameItr;

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    size_t allocSize = mAllocSize ? mAllocSize : f.mLen;
    QC2Status ret = allocAndMapBuf(allocSize, &buf, &mapping);
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("EmbeddedInputStream::nextFrame: Failed to allocate/map input buffer!");
        return ret;
    }
    buf->setTimestamp(f.mTimeStampUs);
    auto flags  = f.mFlags;
    // remove CSD flag from CSD buffer when repeating.
    // TODO(PC): reset mLoopCount when seek is implemented and stream is seek'd
    if (mLoopCount > 0 && flags & BufFlag::CODEC_CONFIG) {
        flags &= ~BufFlag::CODEC_CONFIG;
    }
    buf->setFlags(flags);
    memcpy_s(mapping->baseRW(), mapping->capacity(), f.mData, f.mLen);
    QLOGD("nextFrame: base=%p", mapping->baseRW());
    buf->linear().setRange(0, f.mLen);
    addMetadataToFrame(buf);

    ++mFrameItr;
    if (!(mFrameItr != mFrames.end())) {
        // rewind
        ++mLoopCount;
        mFrameItr = mFrames.begin();
        mLastSegmentTimeBaseUs += f.mTimeStampUs;
        // skip over the initial out-of-band codec-configs
        while (!mRequeueCSD &&
                mFrameItr != mFrames.end() && (*mFrameItr).mFlags & BufFlag::CODEC_CONFIG) {
            ++mFrameItr;
        }
    }

    if (isSecure()) {
        return secureCopy(buf, frame);
    }
    *frame = buf;
    return QC2_OK;
}

QC2Status EmbeddedInputStream::addMetadataToFrame(std::shared_ptr<QC2Buffer> frame) {
    auto& f = *mFrameItr;

    // Crop
    frame->bufferInfo().put("crop-left",  f.mCrop.left);
    frame->bufferInfo().put("crop-top", f.mCrop.top);
    frame->bufferInfo().put("crop-right", f.mCrop.displayWidth);
    frame->bufferInfo().put("crop-bottom", f.mCrop.displayHeight);

    // size
    frame->bufferInfo().put("width", f.mCrop.width);
    frame->bufferInfo().put("height", f.mCrop.height);

    // Misr
#if 0
    frame->bufferInfo().put("misr_dpb_luma0", f.mMisr[0].misr_dpb_luma);
    frame->bufferInfo().put("misr_dpb_chroma0", f.mMisr[0].misr_dpb_chroma);
    frame->bufferInfo().put("misr_opb_luma0", f.mMisr[0].misr_opb_luma);
    frame->bufferInfo().put("misr_opb_chroma0", f.mMisr[0].misr_opb_chroma);
    frame->bufferInfo().put("misr_dpb_luma1", f.mMisr[1].misr_dpb_luma);
    frame->bufferInfo().put("misr_dpb_chroma1", f.mMisr[1].misr_dpb_chroma);
    frame->bufferInfo().put("misr_opb_luma1", f.mMisr[1].misr_opb_luma);
    frame->bufferInfo().put("misr_opb_chroma1", f.mMisr[1].misr_opb_chroma);
#endif
    return QC2_OK;
}

#define _SIZE(_stem, _id) (sizeof(_stem##__blob_##_id)/sizeof(uint8_t) - 1)
#define _TS(_stem, _id) (_stem##__ts_##_id)
#define _FLAGS(_stem, _id) (_stem##__flags_##_id)
#define _BLOB(_stem, _id) (_stem##__blob_##_id)
#define _CROP(_stem, _id) (_stem##__crop_##_id)
#define _MISR(_stem, _id) (_stem##__misr_##_id)

#define _FRAME_INITIALIZER(_stem, _id) \
        EmbeddedFrame{_SIZE(_stem, _id), _TS(_stem, _id), _FLAGS(_stem, _id), _BLOB(_stem, _id), \
        _CROP(_stem, _id), _MISR(_stem, _id)}

#include "./res/stream_avc_basic_vga.cpp"   //stream_avc_basic_vga_StreamLoader

// static
EmbeddedInputStream::StreamLoaderStore_t EmbeddedInputStream::mStreamLoaders = {
    {"avc_basic_vga", stream_avc_basic_vga_StreamLoader},
};

//-------------------------------------------------------------------------------------------------
// Container File stream reader : parses streams supported by MediaExtractor
//      uses NDK MediaExtractor
//-------------------------------------------------------------------------------------------------
#include <sys/stat.h>
#include "media/NdkMediaFormat.h"
#include "media/NdkMediaExtractor.h"

#undef LOG_TAG
#define LOG_TAG "ContainerFileInputStream"
class ContainerFileInputStream : public TestInputStream {
public:
    ContainerFileInputStream(
            const std::string& mime,
            const std::string& fileName,
            std::shared_ptr<QC2LinearBufferPool> bufPool);

    virtual ~ContainerFileInputStream();

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;

    static void MediaExtractor_deleter(AMediaExtractor *ext) {
        AMediaExtractor_delete(ext);
    }
    static void MediaFormat_deleter(AMediaFormat *fmt) {
        AMediaFormat_delete(fmt);
    }

private:
    std::shared_ptr<AMediaExtractor> mExtractor;
    std::shared_ptr<AMediaFormat> mMediaFmt;
    int mFd;
    int mTrackId;
    uint8_t *mReadBuffer;
    uint32_t mReadBufferSize;
    struct Header {
        uint8_t *mBuf;
        uint32_t mLen;
        Header(uint32_t len)
            : mLen(len) {
            mBuf = (uint8_t *)malloc(sizeof(uint8_t) * len);
        }
        ~Header() {
            if (mBuf) {
                free(mBuf);
            }
        }
    };
    std::list<std::unique_ptr<Header>> mHeaders;

    QC2Status setDataSource(const std::string& file);

    QC2Status generateFrame(
            uint8_t *data, uint32_t allocSize, uint32_t sampleSize,
            uint64_t sampleFlags, uint64_t timestamp,
            std::shared_ptr<QC2Buffer> *buf);

    void dumpToFile(
            uint8_t *data, uint32_t size,
            uint64_t flags, uint64_t timestamp);
};

ContainerFileInputStream::ContainerFileInputStream(
        const std::string& mime,
        const std::string& fileName,
        std::shared_ptr<QC2LinearBufferPool> bufPool)
    : TestInputStream(bufPool),
      mExtractor(nullptr),
      mMediaFmt(nullptr),
      mFd(-1),
      mTrackId(-1),
      mReadBuffer(nullptr),
      mReadBufferSize(8*1024*1024) {
    mExtractor = std::shared_ptr<AMediaExtractor>(
            AMediaExtractor_new(),
            ContainerFileInputStream::MediaExtractor_deleter);
    if (setDataSource(fileName) != QC2_OK) {
        return;
    }
    const char *mimeType;
    int nTracks = AMediaExtractor_getTrackCount(mExtractor.get());
    for (int i = 0; i < nTracks; i++) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(mExtractor.get(), i);
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mimeType)) {
            QLOGE("ContainerFileInputStream: no mime type detected in %s", fileName.c_str());
            mExtractor = nullptr;
            mIsValid = false;
            AMediaFormat_delete(format);
            return;
        } else if (mime == mimeType) {
            mTrackId = i;
            mMediaFmt = std::shared_ptr<AMediaFormat>(
                    format,
                    ContainerFileInputStream::MediaFormat_deleter);
            QLOGI("ContainerFileInputStream: Found track (%s) in file (%s)",
                    mime.c_str(), fileName.c_str());
            mIsValid = true;
            break;
        } else {
            QLOGI("ContainerFileInputStream: track (%s) in file (%s)",
                    mime.c_str(), fileName.c_str());
            continue;
        }
        AMediaFormat_delete(format);
    }
    if (!mIsValid) {
        QLOGE("ContainerFileInputStream: Could not Find track (%s) in file (%s)",
                mime.c_str(), fileName.c_str());
        mExtractor = nullptr;
    } else {
        mReadBuffer = (uint8_t *)malloc(sizeof(uint8_t) * mReadBufferSize);

        AMediaExtractor_selectTrack(mExtractor.get(), mTrackId);

        // fill headers if present
        for (int i = 0; i < 4; ++i) {
            void *data;
            size_t headerSize = 0;
            char csdName[128];
            snprintf(csdName, 128, "csd-%d", i);
            if (AMediaFormat_getBuffer(mMediaFmt.get(), csdName, &data, &headerSize)) {
                if (data && headerSize) {
                    QLOGI("Found header(%s) of size %zu bytes", csdName, headerSize);
                    auto h = std::make_unique<Header>(headerSize);
                    memcpy_s(h->mBuf, headerSize, data, headerSize);
                    mHeaders.emplace_back(std::move(h));
                }
            }
        }  // </>fill headers if present
    }
}

ContainerFileInputStream::~ContainerFileInputStream() {
    mExtractor = nullptr;
    mMediaFmt = nullptr;
    if (mReadBuffer) {
        free(mReadBuffer);
    }
    if (mFd) {
        close(mFd);
    }
}

QC2Status ContainerFileInputStream::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!mIsValid) {
        QLOGE("nextFrame called on an invalid stream..");
        return QC2_NO_INIT;
    }

    if (!mHeaders.empty()) {
        auto& h = mHeaders.front();
        auto allocSize = mAllocSize ? mAllocSize : h->mLen;
        auto ret = generateFrame(h->mBuf, allocSize, h->mLen, BufFlag::CODEC_CONFIG, 0, frame);
        mHeaders.pop_front();
        return ret;
    }

    bool isEOS = false;
    int trackId = -1;
    do {
        trackId = AMediaExtractor_getSampleTrackIndex(mExtractor.get());
        isEOS = trackId < 0;
        QLOGI("getSampleTrackIndex : %d", trackId);
        if (trackId < 0) {
            isEOS = true;
        } else if (trackId != mTrackId) {
            AMediaExtractor_advance(mExtractor.get());
        }
    } while (trackId >= 0 && trackId != mTrackId);

    ssize_t sampleSize = 0;   // default, if EOS
    uint32_t allocSize = 8;
    uint64_t flags = 0;
    uint64_t timestamp = 0;
    if (!isEOS) {
        sampleSize = AMediaExtractor_readSampleData(
                mExtractor.get(), mReadBuffer, mReadBufferSize);
        QLOGI("AMediaExtractor_readSampleData read %u bytes", (uint32_t)sampleSize);
        if (sampleSize < 0) {
            flags = BufFlag::EOS;
            sampleSize = 0;
        } else {
            uint32_t f = AMediaExtractor_getSampleFlags(mExtractor.get());
            flags |= f & AMEDIAEXTRACTOR_SAMPLE_FLAG_SYNC ? BufFlag::I_FRAME : 0;
            timestamp = (uint64_t)AMediaExtractor_getSampleTime(mExtractor.get());
            allocSize = mAllocSize ? mAllocSize : sampleSize;
        }
        AMediaExtractor_advance(mExtractor.get());
    } else {
        flags = BufFlag::EOS;
    }

    return generateFrame(mReadBuffer, allocSize, sampleSize, flags, timestamp, frame);
}

QC2Status ContainerFileInputStream::generateFrame(
            uint8_t *data, uint32_t allocSize, uint32_t sampleSize,
            uint64_t sampleFlags, uint64_t timestamp,
            std::shared_ptr<QC2Buffer> *frame) {

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    QC2Status ret = allocAndMapBuf(allocSize, &buf, &mapping);
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("ContainerFileInputStream::nextFrame: Failed to allocate/map input buffer!");
        return QC2_NO_MEMORY;
    }

    buf->setTimestamp(timestamp);
    buf->setFlags(sampleFlags);
    memcpy_s(mapping->baseRW(), mapping->capacity(), data, sampleSize);
    buf->linear().setRange(0, sampleSize);

    if (isSecure()) {
        return secureCopy(buf, frame);
    }
    *frame = buf;

#if DUMP_FOR_EMBEDDED_STREAM
    dumpToFile(data, sampleSize, sampleFlags, timestamp);
#endif //  DUMP_FOR_EMBEDDED_STREAM

    return QC2_OK;
}

QC2Status ContainerFileInputStream::setDataSource(const std::string& file) {
    mFd = open(file.c_str(), O_RDONLY);
    if (mFd < 0) {
        QLOGI("ContainerFileInputStream: failed to open file %s", file.c_str());
        return QC2_ERROR;
    }
    off64_t fileSize = 0;
    {
        struct stat st;
        if (fstat(mFd, &st) != 0) {
            QLOGE("ContainerFileInputStream: failed to get file attrs for %s", file.c_str());
            return QC2_ERROR;
        }
        fileSize = st.st_size;
        QLOGI("ContainerFileInputStream: file(%s) has %u bytes", file.c_str(), (uint32_t)fileSize);
    }
    auto res = AMediaExtractor_setDataSourceFd(mExtractor.get(), mFd, 0, fileSize);
    if (res != AMEDIA_OK) {
        QLOGE("Failed to create extractor");
        return QC2_ERROR;
    }
    return QC2_OK;
}

void ContainerFileInputStream::dumpToFile(
        uint8_t *data, uint32_t size,
        uint64_t flags, uint64_t ts) {
    static int count = 0;
    const char *filename = "/sdcard/stream.cpp";
    FILE *fp = fopen(filename, "a");

    if (fp != NULL && data != NULL) {
        int i;
        fprintf(fp, "static uint64_t stream__ts_%03d = %lld;\n", count, (long long)ts);
        fprintf(fp, "static uint64_t stream__flags_%03d = %lld;\n", count, (long long)flags);
        fprintf(fp, "static uint8_t stream__blob_%03d[] = {\n\t\"", count);
        for (i = 0; i < (int)size; ++i) {
            fprintf(fp, "\\x%02x", data[i]);
            if (i > 0 && i % 20 == 0) {
                fprintf(fp, "\"\n\t\"");
            }
        }
        fprintf(fp, "\"\n};\n\n");
        ++count;
    } else if (fp == NULL) {
        QLOGE("Could not write to file %s", filename);
    }
    if (fp) {
        fclose(fp);
    }
}

//-------------------------------------------------------------------------------------------------
// Graphic(YUV/RGB) pattern generator
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "QC2GraphicPatternSource"

class QC2GraphicPatternSource : public QC2GraphicInputSource {
public:
    QC2GraphicPatternSource (uint32_t width, uint32_t height, uint32_t format,
            const std::string& patternName,
            std::shared_ptr<QC2GraphicBufferPool> bufPool)
        : QC2GraphicInputSource(width, height, format, bufPool),
            mFrameIndex(0u), mPatternName(patternName) {
        (void)mFrameIndex;
        (void)mPatternName;
    }

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;
    virtual void getFrameSize(uint32_t *size) override;
    static uint8_t sample_Y;
    static uint8_t sample_U;
    static uint8_t sample_V;

private:
    QC2Status generateVENUS_NV12Frame(uint8_t *data);
    QC2Status generateRGBA8888Frame(uint8_t *data);

    uint32_t mFrameIndex;
    const std::string mPatternName;
};

uint8_t QC2GraphicPatternSource::sample_Y = 120;
uint8_t QC2GraphicPatternSource::sample_U = 160;
uint8_t QC2GraphicPatternSource::sample_V = 200;


void QC2GraphicPatternSource::getFrameSize(uint32_t *size) {
    *size = mFrameSize;
}

QC2Status QC2GraphicPatternSource::generateVENUS_NV12Frame(uint8_t *data) {
    int W = 0; // start width
    int H = 0; // start height
    int uvH = 0; // start uv width
    uint32_t base = 0;

    if (!data) {
         return QC2_BAD_ARG;
    }

    using BufLayout = Platform::VenusBufferLayout;
    int yStride    = BufLayout::Stride(PixFormat::VENUS_NV12, BufLayout::PLANE_Y, mWidth);
    int yScanLines = BufLayout::Scanlines(PixFormat::VENUS_NV12, BufLayout::PLANE_Y, mHeight);
    int uvStride   = BufLayout::Stride(PixFormat::VENUS_NV12, BufLayout::PLANE_UV, mWidth);

    memset(data, 0, mFrameSize);

    int fIndex = mFrameIndex % 4;
    if (fIndex < 2) {
        W = fIndex * (mWidth / 2);
        H = 0;
        uvH = 0;
    } else {
        W = (3 - fIndex) * (mWidth / 2);
        H = mHeight/2;
        uvH = mHeight/4;
    }

    for (int h = H + (mHeight/2) - 1; h >= H; --h) {
        for (int w = W + (mWidth/2) - 1; w >= W; --w) {
            data[h * yStride + w] = sample_Y;
        }
    }

    for (int h = uvH + (mHeight/4) -1 ; h >= uvH; --h) {
        base = (yStride * yScanLines) + (h * uvStride);
        for (int w = W + (mWidth/2) - 1; w >= W ; w = w-2) {
            data[base + w] = sample_V;
            data[base + w - 1] = sample_U;
        }
    }

    return QC2_OK;
}

QC2Status QC2GraphicPatternSource::generateRGBA8888Frame(uint8_t *data) {
    if (!data) {
         return QC2_BAD_ARG;
    }

    auto setColor = [](uint8_t *p, uint8_t r, uint8_t g, uint8_t b) {
        p[0] = r;
        p[1] = g;
        p[2] = b;
        p[3] = 0xff;
    };

    memset(data, 0, mFrameSize);

    uint8_t cLUT[4] = {0xFF, 0xBB, 0x77, 0x44};

    uint32_t x = (mFrameIndex % 4) * (mWidth / 4);
    uint32_t y = ((mFrameIndex / 4) % 2) * (mHeight / 2);

    uint32_t *p = (uint32_t *)data;
    p += ((y * mWidth) + x);
    for (uint32_t i = 0; i < mHeight / 2 ; ++i) {
        for (uint32_t j = 0; j < mWidth / 4 ; ++j) {
            setColor((uint8_t *)(p + j),
                    cLUT[mFrameIndex % 4],
                    cLUT[(mFrameIndex + 1) % 4],
                    cLUT[(mFrameIndex + 2) % 4]);
        }
        p += mWidth;
    }
    return QC2_OK;
}

QC2Status QC2GraphicPatternSource::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!frame) {
        QLOGE("QC2GraphicPatternSource::nextFrame: Invalid buffer handle passed");
        return QC2_BAD_ARG;
    }
    if (!mBufPool) {
        QLOGE("QC2GraphicPatternSource::nextFrame: Buffer pool not initialized");
        return QC2_NO_INIT;
    }

    ++mFrameIndex;

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    QC2Status ret = allocAndMapBuf(&buf, &mapping);
    mFrameSize = buf->graphic().allocSize();
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("QC2GraphicPatternSource::nextFrame: Failed to allocate/map input buffer!");
        return ret;
    }

    switch (mFormat) {
        case PixFormat::VENUS_NV12:
            {
                generateVENUS_NV12Frame(mapping->baseRW());
                break;
            }
        case PixFormat::RGBA8888:
            generateRGBA8888Frame(mapping->baseRW());
            break;
        default:
            break;
    }
    buf->setTimestamp(mFrameIndex * mFrameDurationUs);
    updateColorAspects(buf);
    *frame = buf;
    return QC2_OK;
}

//-------------------------------------------------------------------------------------------------
// raw YUV/RGB reader
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "QC2GraphicRAWSource"
class QC2GraphicRAWSource : public QC2GraphicInputSource {
public:
    QC2GraphicRAWSource(uint32_t width, uint32_t height, uint32_t format,
            const std::string& fileName,
            std::shared_ptr<QC2GraphicBufferPool> bufPool);

    virtual ~QC2GraphicRAWSource();

    virtual QC2Status nextFrame(std::shared_ptr<QC2Buffer> *frame) override;

    virtual void getFrameSize(uint32_t *size) {
        if (!size) {
            return;
        }
        *size = mFrameSize;
    }

private:
    QC2Status readNV12Frame(uint8_t *data);
    QC2Status readCompresedFrame(uint8_t *data);
    std::unique_ptr<C2Info> getROIMapAtFrame(uint32_t frameIndex);
    void init();

    uint32_t mFrameIndex;
    FILE *mFp = nullptr;
    FILE *mROIFp = nullptr;
    uint32_t mFrameSize;
    uint32_t mLcuSize = 16;  // default for AVC
    bool mInitialized = false;
};

QC2GraphicRAWSource::QC2GraphicRAWSource(uint32_t width, uint32_t height, uint32_t format,
        const std::string& fileName,
        std::shared_ptr<QC2GraphicBufferPool> bufPool)
    : QC2GraphicInputSource(width, height, format, bufPool),
        mFrameIndex(0u), mFrameSize(0u) {

    if (format != PixFormat::VENUS_NV12_UBWC && format != PixFormat::YUV420SP
            && format != PixFormat::RGBA8888_UBWC) {
        QLOGE("QC2GraphicRAWSource: supports only VENUS_NV12_UBWC/VENUS_NV12/RGAB8888_UBWC at the "
                "moment");
        mValid = false;
        return;
    }
    mFp = fopen(fileName.c_str(), "rb");
    if (!mFp) {
        QLOGE("Failed to open raw stream %s", fileName.c_str());
        mValid = false;
        return;
    }

    if (format == PixFormat::VENUS_NV12_UBWC || format == PixFormat::RGBA8888_UBWC) {
        mFrameSize = Platform::VenusBufferLayout::CompressedFrameSize(format, width, height);
        mValid = true;
    } else if (format == PixFormat::YUV420SP) {
        uint32_t alignedw = ALIGN(width, 16);
        mFrameSize = (alignedw * height) + (ALIGN(width/2, 16) * (height/2) * 2);
        mValid = true;
    }
}

QC2GraphicRAWSource::~QC2GraphicRAWSource() {
    if (mFp) {
        fclose(mFp);
    }
    if (mROIFp) {
        fclose(mROIFp);
    }
}

QC2Status QC2GraphicRAWSource::nextFrame(std::shared_ptr<QC2Buffer> *frame) {
    if (!mFp || !mValid) {
        return QC2_NO_INIT;
    }
    if (!frame) {
        QLOGE("QC2GraphicRAWSource::nextFrame: Invalid buffer handle passed");
        return QC2_BAD_ARG;
    }
    if (!mBufPool) {
        QLOGE("QC2GraphicRAWSource::nextFrame: Buffer pool not initialized");
        return QC2_NO_INIT;
    }

    if (!mInitialized) {
        init();
    }

    *frame = nullptr;

    std::shared_ptr<QC2Buffer> buf;
    std::unique_ptr<QC2Buffer::Mapping> mapping;
    QC2Status ret = allocAndMapBuf(&buf, &mapping);
    if (buf->graphic().allocSize() < mFrameSize) {
        QLOGE("QC2GraphicRAWSource::nextFrame: insufficient buffer size");
        return QC2_NO_MEMORY;
    }
    if (ret != QC2_OK || !buf || !mapping || !(mapping->baseRW())) {
        QLOGE("QC2GraphicRAWSource::nextFrame: Failed to allocate/map input buffer!");
        return ret;
    }
    if (mFormat == PixFormat::VENUS_NV12_UBWC || mFormat == PixFormat::RGBA8888_UBWC) {
        ret = readCompresedFrame(mapping->baseRW());
        if (ret != QC2_OK) {
            return ret;
        }
    } else if (mFormat == PixFormat::YUV420SP /* aka canonical NV12 */) {
        ret = readNV12Frame(mapping->baseRW());
        if (ret != QC2_OK) {
            return ret;
        }
    } else {
        // should not happen !
        QLOGE("QC2GraphicRAWSource::nextFrame: unsupported format %s", PixFormat::Str(mFormat));
        return QC2_CANNOT_DO;
    }

    // attach infos if available
    {
        std::shared_ptr<C2Info> roiInfo = getROIMapAtFrame(mFrameIndex);
        if (roiInfo != nullptr) {
            buf->addInfo(std::move(roiInfo));
        }
    }

    buf->setTimestamp(mFrameIndex * mFrameDurationUs);
    updateColorAspects(buf);
    *frame = buf;
    ++mFrameIndex;
    QLOGV("QC2GraphicRAWSource::nextFrame: OK");
    return QC2_OK;
}

QC2Status QC2GraphicRAWSource::readNV12Frame(uint8_t *data) {
    auto read = fread(data, 1, mFrameSize, mFp);
    if (read < mFrameSize) {
        fseek(mFp, 0, SEEK_SET);
        auto read = fread(data, 1, mFrameSize, mFp);
        if (read < mFrameSize) {
            QLOGV("QC2GraphicRAWSource::readNV12Frame: failed");
            return QC2_ERROR;
        }
    }
    QLOGV("QC2GraphicRAWSource::readNV12Frame: read %u bytes", (uint32_t)read);
    return QC2_OK;
}

QC2Status QC2GraphicRAWSource::readCompresedFrame(uint8_t *data) {
    auto read = fread(data, 1, mFrameSize, mFp);
    if (read < mFrameSize) {
        fseek(mFp, 0, SEEK_SET);
        auto read = fread(data, 1, mFrameSize, mFp);
        if (read < mFrameSize) {
            QLOGV("QC2GraphicRAWSource::readCompresedFrame: failed");
            return QC2_ERROR;
        }
    }
    QLOGV("QC2GraphicRAWSource::readCompresedFrame: read %u bytes", (uint32_t)read);
    return QC2_OK;
}

std::unique_ptr<C2Info> QC2GraphicRAWSource::getROIMapAtFrame(uint32_t frameIndex) {
    if (!mROIFp) {
        return nullptr;
    }
    size_t numMBRows = (mHeight + mLcuSize - 1) / mLcuSize;
    size_t numMBCols = (mWidth + mLcuSize - 1) / mLcuSize;
    size_t nBytes = numMBCols * numMBRows;

    // fetch frame-ID
    uint32_t frameId = UINT32_MAX;
    {
        auto read = fread(&frameId, 1, sizeof(uint32_t), mROIFp);
        if (read < sizeof(uint32_t)) {
            fseek(mROIFp, 0, SEEK_SET);
            read = fread(&frameId, 1, sizeof(uint32_t), mROIFp);
            if (read < sizeof(uint32_t)) {
                QLOGV("QC2GraphicRAWSource::getROIMapAtFrame failed to read frame-id");
                return nullptr;
            }
        }
        frameId = ntohl(frameId);

        if (frameId > frameIndex) {
            // Not time yet, seek back so we can re-fetch the frame-Id again
            fseek(mROIFp, -sizeof(frameId), SEEK_CUR);
            QLOGD("getROIMapAtFrame: parsed frame-id(%u). still at %u", frameId, frameIndex);
            return nullptr;
        } else if (frameId < frameIndex) {
            // need to discard the subsequent qp data
            fseek(mROIFp, nBytes, SEEK_CUR);
            QLOGE("getROIMapAtFrame: parsed frame-id(%u) is older than current %u",
                    frameId, frameIndex);
            return nullptr;
        }
        QLOGD("getROIMapAtFrame: found ROI for frame %u", frameIndex);
    }

    auto roiInfo = QC2VideoROIMapInfo::output::AllocUnique(nBytes);
    roiInfo->m.mbCols = numMBCols;
    roiInfo->m.mbRows = numMBRows;

    int8_t *pData = (int8_t*)&(roiInfo->m.qpMap[0]);
    auto read = fread(pData, 1, nBytes, mROIFp);
    if (read < (nBytes)) {
        QLOGV("QC2GraphicRAWSource::getROIMapAtFrame failed to read from file");
        return nullptr;
    }
    QLOGD("QC2GraphicRAWSource::getROIMapAtFrame: Read ROI %zux%zu MBs", numMBCols, numMBRows);
    return std::move(roiInfo);
}

void QC2GraphicRAWSource::init() {
    // retrieve LCU size
    if (mAuxInfo) {
        mAuxInfo->get("roi-lcu-size", &mLcuSize);
    }
    if (mLcuSize != 16 && mLcuSize != 32) {
        QLOGE("Invalid LCU size %u", mLcuSize);
        mValid = false;
    }
    // try opening ROI map file
    if (mAuxInfo) {
        std::string roiMbFileName;
        if (mAuxInfo->get("roi-mb-file", &roiMbFileName) == QC2_OK) {
            mROIFp = fopen(roiMbFileName.c_str(), "rb");
            if (mROIFp) {
                QLOGI("Using ROI QP %s", roiMbFileName.c_str());
            }
        }
    }
    mInitialized = true;
}

//-------------------------------------------------------------------------------------------------
// factory
//-------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "TestInputStream"
// static
bool TestInputStream::FileExists(const std::string& fileName) {
    FILE *fp = fopen(fileName.c_str(), "rb");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}


QC2Status TestInputStream::allocAndMapBuf(
        uint32_t size,
        std::shared_ptr<QC2Buffer> *buf,
        std::unique_ptr<QC2Buffer::Mapping>* map) {
    if (!buf || !map) {
        return QC2_BAD_ARG;
    }
    QLOGW("Requested allocation size (%u) v/s max (%zu)", size, kMaxAllocSize);
    if (size > kMaxAllocSize) {
        QLOGE("Requested allocation size is too huge (%u) v/s max (%zu)!", size, kMaxAllocSize);
        return QC2_NO_MEMORY;
    }
    mBufPool->setBufferSize(size);
    QC2Status err = mBufPool->allocate(buf);
    if (err != QC2_OK || *buf == nullptr) {
        QLOGE("allocAndMapBuf: failed to allocate input buffer");
        return QC2_NO_MEMORY;
    }
    (*map) = (*buf)->linear().map();
    return QC2_OK;
}

QC2Status TestInputStream::secureCopy(
        std::shared_ptr<QC2Buffer> nonSecureFrame,
        std::shared_ptr<QC2Buffer> *secureFrame) {
    if (!mSecureBufPool) {
        QLOGE("TestInputStream: getSecureCopy: no secure buf pool !");
        return QC2_CANNOT_DO;
    }
    mSecureBufPool->setUsage(MemoryUsage::HW_CODEC_READ | MemoryUsage::HW_PROTECTED);
    size_t allocSize = mSecureAllocSize ? mSecureAllocSize : nonSecureFrame->linear().capacity();
    mSecureBufPool->setBufferSize(allocSize);
    QLOGV("TestInputStream::getSecureCopy: Requesting secure buf of size=%zu", allocSize);
    std::shared_ptr<QC2Buffer> buf;
    QC2Status ret = mSecureBufPool->allocate(&buf);
    if (ret != QC2_OK || buf == nullptr) {
        QLOGE("TestInputStream::getSecureCopy: failed to allocate secure buffer!");
        return QC2_NO_MEMORY;
    }

    ret = mCPZSession->copyToSecure(nonSecureFrame, buf);
    if (ret != QC2_OK) {
        QLOGE("TestInputStream: Failed preparing secure input frame!");
        return ret;
    }

    // copy buffer flags and timeStamp and range
    buf->setTimestamp(nonSecureFrame->timestamp());
    buf->setFlags(nonSecureFrame->flags());
    buf->linear().setRange(buf->linear().offset(), nonSecureFrame->linear().size());

    *secureFrame = buf;
    QLOGD("TestInputStream: prepared secure input frame");
    return QC2_OK;
}

//static
QC2Status TestInputStream::Create(
        const std::string& mime,
        const std::string& fileName,
        std::shared_ptr<QC2LinearBufferPool> bufPool,
        std::unique_ptr<TestInputStream> *stream) {
    if (!stream || !bufPool) {
        return QC2_BAD_ARG;
    }
    // check if there is an embedded stream for the requested name
    if (EmbeddedInputStream::HasStreamForName(fileName)) {
        QLOGI("Found an embedded stream for : %s", fileName.c_str());
        *stream = std::make_unique<EmbeddedInputStream>(fileName, bufPool);
        return QC2_OK;
    }

    // .. if not, check if the file exists, and create a File-stream in case it does
    if (!TestInputStream::FileExists(fileName)) {
        QLOGE("Input resource not found : %s", fileName.c_str());
        return QC2_NOT_FOUND;
    }
    (*stream).reset();

    if ((mime == MimeType::AVC && fileName.substr(fileName.size() - 3, 3) == "264")
            || (mime == MimeType::HEVC && fileName.substr(fileName.size() - 3, 3) == "265")) {
        *stream = std::make_unique<AVCFileInputStream>(fileName, bufPool);
        return QC2_OK;
    }

    if ((mime == MimeType::VP8 || mime == MimeType::VP9) && fileName.substr(fileName.size() - 3, 3) == "ivf") {
        *stream = std::make_unique<IvfFileInputStream>(fileName, bufPool);
        return QC2_OK;
    }

    // .. try extractor
    {
        auto containerStream = std::make_unique<ContainerFileInputStream>(mime, fileName, bufPool);
        if (containerStream->valid()) {
            *stream = std::move(containerStream);
            return QC2_OK;
        }
    }

    return QC2_CANNOT_DO;
}

//--------------------------------------------------------------------------------------------------
// Factory QC2GraphicInputSource
//--------------------------------------------------------------------------------------------------
#undef LOG_TAG
#define LOG_TAG "QC2GraphicInputSource"
QC2Status QC2GraphicInputSource::Create(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        const std::string& fileName,
        const std::string& patternName,
        std::shared_ptr<QC2GraphicBufferPool> bufPool,
        std::unique_ptr<QC2GraphicInputSource> *source) {

    if (fileName != "") {
        *source = std::make_unique<QC2GraphicRAWSource>(width, height,
                format, fileName, bufPool);
    } else {
        *source = std::make_unique<QC2GraphicPatternSource>(width, height,
                format, patternName, bufPool);
    }
    return QC2_OK;
}

QC2Status QC2GraphicInputSource::allocAndMapBuf(
        std::shared_ptr<QC2Buffer> *buf,
        std::unique_ptr<QC2Buffer::Mapping>* map) {
    if (!buf || !map) {
        return QC2_BAD_ARG;
    }

    mBufPool->setUsage(MemoryUsage::CPU_WRITE_UNCACHED | MemoryUsage::HW_CODEC_READ);
    mBufPool->setResolution(mWidth, mHeight);
    mBufPool->setFormat(mFormat);

    QC2Status err = mBufPool->allocate(buf);
    if (err != QC2_OK || *buf == nullptr) {
        QLOGE("allocAndMapBuf: failed to allocate input buffer");
        return QC2_NO_MEMORY;
    }
    (*map) = (*buf)->graphic().map();
    return QC2_OK;
}

QC2Status QC2GraphicInputSource::updateColorAspects(std::shared_ptr<QC2Buffer> buf) {
    if (!mHasColorAspects) {
        return QC2_OK;
    }
    if (!buf || !buf->isGraphic()) {
        return QC2_BAD_ARG;
    }

    auto colorAspects = std::make_shared<C2StreamColorAspectsInfo::output>();
    colorAspects->range = mColorRange;
    colorAspects->primaries = mColorPrimaries;
    colorAspects->transfer = mColorTransfer;
    colorAspects->matrix = mColorMatrix;
    return buf->graphic().setPrivateMetadata({colorAspects});
}

bool QC2GraphicInputSource::isValid() {
    return mValid;
}

// CP session to copy from non-secure to secure-world
struct CPZSession::Impl {
    Impl();

    ~Impl();

    QC2Status copyToSecure(
            std::shared_ptr<QC2Buffer> nonSecure,
            std::shared_ptr<QC2Buffer> secure);

    inline bool valid() const {
        return mIsValid;
    }

 private:

    typedef uint32_t OEMCrypto_SESSION;
    enum OEMCryptoResult {
        OEMCrypto_SUCCESS = 0,
    };

    typedef enum {
        OEMCrypto_BufferType_Clear = 0,
        OEMCrypto_BufferType_Secure,
        OEMCrypto_BufferType_Unknown = 0x7FFFFFFF,
    } __attribute__ ((packed)) OEMCryptoBufferType;

    typedef struct
    {
        OEMCryptoBufferType type;
        union
        {
            struct      // type == OEMCrypto_BufferType_Secure
            {
                void   *handle;
                size_t  max_length;
                size_t  offset;
            } secure;
        } buffer;
    } OEMCrypto_DestBufferDesc;

    enum {
        OEMCrypto_FirstSubsample = 1,
        OEMCrypto_LastSubsample = 2,
    };

    typedef OEMCryptoResult (*cryptoInit_t)(void);
    typedef OEMCryptoResult (*cryptoTerminate_t)(void);
    typedef OEMCryptoResult (*cryptoCopyBuffer_t)(
            unsigned id, const uint8_t* data_addr, size_t data_length,
            OEMCrypto_DestBufferDesc *outBufDesc, uint8_t subsample_flags);
    typedef OEMCryptoResult(*cryptoOpenSession_t)(unsigned* id);
    typedef OEMCryptoResult(*cryptoCloseSession_t)(unsigned id);

    struct OEMCryptoAPI {
        static std::shared_ptr<OEMCryptoAPI> Get();

        unsigned mSessionId;
        cryptoInit_t mFuncInit;
        cryptoTerminate_t mFuncTerminate;
        cryptoCopyBuffer_t mFuncCopyBuffer;
        cryptoOpenSession_t mFuncOpenSession;
        cryptoCloseSession_t mFuncCloseSession;

     private:
        static bool sAPIInit;
        static std::mutex sAPILock;
        static std::shared_ptr<OEMCryptoAPI> sAPI;
        static void *sLib;

        const char * kLibName                       = "liboemcrypto.so";
        const char * kSymbol_OEMCrypto_Initialize   = "_oecc01";
        const char * kSymbol_OEMCrypto_Terminate    = "_oecc02";
        const char * kSymbol_OEMCrypto_CopyBuffer   = "_oecc93";
        const char * kSymbol_OEMCrypto_OpenSession  = "_oecc09";
        const char * kSymbol_OEMCrypto_CloseSession = "_oecc10";

        OEMCryptoAPI()
            : mFuncInit(nullptr), mFuncTerminate(nullptr),
            mFuncCopyBuffer(nullptr), mFuncOpenSession(nullptr),
            mFuncCloseSession(nullptr) {
            init();
        }
        QC2Status init();
    };

    std::shared_ptr<OEMCryptoAPI> mAPI;
    bool mIsValid;
};

CPZSession::CPZSession()
    : mImpl(std::make_unique<Impl>()) {
}

bool CPZSession::valid() {
    return mImpl->valid();
}

CPZSession::~CPZSession() {
    mImpl.reset(nullptr);
}

QC2Status CPZSession::copyToSecure(
        std::shared_ptr<QC2Buffer> nonSecure,
        std::shared_ptr<QC2Buffer> secure) {
    return mImpl->copyToSecure(nonSecure, secure);
}

CPZSession::Impl::Impl()
    : mAPI(OEMCryptoAPI::Get()), mIsValid(false) {
    if (mAPI) {
        if (mAPI->mFuncInit && mAPI->mFuncOpenSession) {
            if ((*(mAPI->mFuncInit))() != OEMCrypto_SUCCESS) {
                QLOGE("OEMCrypto init failed!");
                return;
            }
            if (mAPI->mFuncOpenSession(&mAPI->mSessionId) != OEMCrypto_SUCCESS) {
                QLOGE("Failed to open Widevine session");
                return;
            }
        }
        QLOGD("OEMCrypto initialized. SessionID:%u", mAPI->mSessionId);
        mIsValid = true;
    }
}

CPZSession::Impl::~Impl() {
    if (mIsValid && mAPI) {
        if (mAPI->mFuncCloseSession && mAPI->mFuncTerminate) {
            if (mAPI->mFuncCloseSession(mAPI->mSessionId) != OEMCrypto_SUCCESS) {
                QLOGE("OEMCrypto Close Session failed!");
            }
            if ((*(mAPI->mFuncTerminate))() != OEMCrypto_SUCCESS) {
                QLOGE("OEMCrypto terminate failed!");
            }
        }
        QLOGD("OEMCrypto terminated");
    }
}

QC2Status CPZSession::Impl::copyToSecure(
        std::shared_ptr<QC2Buffer> nonSecure,
        std::shared_ptr<QC2Buffer> secure) {
    unsigned ret = OEMCrypto_SUCCESS;
    if (!mIsValid || !mAPI) {
        QLOGE("CPZSession: copy: invalid session");
        return QC2_NO_INIT;
    }
    if (!nonSecure || !secure) {
        QLOGE("CPZSession: copy: invalid input buffers");
        return QC2_BAD_ARG;
    }
    // Handles only Linear -> Linear as of now
    if (!nonSecure->isLinear() || !secure->isLinear()) {
        QLOGE("CPZSession: copy: does not copy graphic buffers");
        return QC2_CANNOT_DO;
    }

    // NOTE: there are no checks for buffers being really 'secure'
    OEMCrypto_DestBufferDesc outBufDesc;
    {
        outBufDesc.type = OEMCrypto_BufferType_Secure;
        C2Handle *hnd = const_cast<C2Handle *>(secure->linear().handle());
        outBufDesc.buffer.secure.handle = static_cast<void *>(hnd);
        outBufDesc.buffer.secure.max_length = secure->linear().capacity();
        outBufDesc.buffer.secure.offset = secure->linear().offset();
    }

    const uint8_t *inPtr = nullptr;
    auto mapping = nonSecure->linear().map();
    inPtr = mapping ? mapping->base() : nullptr;
    if (!inPtr) {
        QLOGE("CPZSession: copy: failed to map input buffer");
        return QC2_ERROR;
    }

    QLOGV("CPZSession::CopyBuffer: in=%p [@off=%p] l=%u sz=%u | out-hnd=%p type=%s sz=%zu off=%zu",
            inPtr, inPtr + nonSecure->linear().offset(),
            nonSecure->linear().size(), nonSecure->linear().capacity(),
            outBufDesc.buffer.secure.handle,
            outBufDesc.type == OEMCrypto_BufferType_Secure ? "secure" : "NonSecure",
            outBufDesc.buffer.secure.max_length,
            outBufDesc.buffer.secure.offset);
    if (mAPI->mFuncCopyBuffer)
        ret = mAPI->mFuncCopyBuffer(
                mAPI->mSessionId,
                inPtr + nonSecure->linear().offset(),
                nonSecure->linear().size(),
                &outBufDesc,
                0);
    else {
        ret = -1;
        QLOGE("CPZSession: CopyBuffer function not mapped !!");
    }
    if (ret != OEMCrypto_SUCCESS) {
        QLOGE("CPZSession: copyBuffer failed !! ret:%#x", ret);
        return QC2_ERROR;
    }

    QLOGD("CPZSession: copyBuffer: OK");
    return QC2_OK;
}

bool CPZSession::Impl::OEMCryptoAPI::sAPIInit = false;
std::mutex CPZSession::Impl::OEMCryptoAPI::sAPILock;
std::shared_ptr<CPZSession::Impl::OEMCryptoAPI> CPZSession::Impl::OEMCryptoAPI::sAPI = nullptr;
void * CPZSession::Impl::OEMCryptoAPI::sLib = nullptr;

// static
std::shared_ptr<CPZSession::Impl::OEMCryptoAPI> CPZSession::Impl::OEMCryptoAPI::Get() {
    if (sAPIInit && sAPI) {
        return sAPI;
    }
    std::lock_guard<std::mutex> l(sAPILock);
    sAPI = std::shared_ptr<OEMCryptoAPI>(new OEMCryptoAPI);
    if (!sAPI) {
        return nullptr;
    }
    sAPI->init();
    sAPIInit = true;
    return sAPI;
}

#define LOAD_OR_RETURN_ERROR(method, symbol)                                \
    method = (decltype(method))dlsym(sLib, symbol);                         \
    if (method == nullptr) {                                                \
        QLOGE("OEMCrypto: dlsym %s (for %s) failed", symbol, #symbol);      \
        return QC2_CANNOT_DO;                                               \
    }                                                                       \

QC2Status CPZSession::Impl::OEMCryptoAPI::init() {
    static bool sTriedOnce = false;
    if (sTriedOnce) {
        return QC2_CANNOT_DO;
    }
    sTriedOnce = true;
    sLib = dlopen(kLibName, RTLD_LAZY);
    if (sLib == nullptr) {
        QLOGE("Failed to open %s : %s", kLibName, dlerror());
        return QC2_CANNOT_DO;
    }
    LOAD_OR_RETURN_ERROR(mFuncInit, kSymbol_OEMCrypto_Initialize);
    LOAD_OR_RETURN_ERROR(mFuncTerminate, kSymbol_OEMCrypto_Terminate);
    LOAD_OR_RETURN_ERROR(mFuncCopyBuffer, kSymbol_OEMCrypto_CopyBuffer);
    LOAD_OR_RETURN_ERROR(mFuncOpenSession, kSymbol_OEMCrypto_OpenSession);
    LOAD_OR_RETURN_ERROR(mFuncCloseSession, kSymbol_OEMCrypto_CloseSession);
    return QC2_OK;
}

}   // namespace qc2

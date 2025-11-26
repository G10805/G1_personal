/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_BUFFER_H_
#define _QC2AUDIO_BUFFER_H_

#include <unordered_map>
#include <list>
#include <mutex>
#include <C2Buffer.h>
#include "QC2Event.h"

namespace qc2audio {

class QC2PlatformBuffer;

/// @addtogroup core Core Component
/// @{

/**
 * @brief Abstraction for Linear(bitstream/metadata) or Graphic Buffer.
 *
 * Wraps C2Buffer, C2LinearBlock, C2GraphicBlock, C2ConstLinearBlock, C2ConstGraphicBlock... and
 *  provides an unified buffer abstraction to the Codec and other internal classes\n
 * QC2Buffer can be created with following methods:
 *      -# From a C2Buffer (buffer will be read-only)
 *      -# From a QC2LinearBufferPool (writeable linear buffer)
 *      -# From a QC2GraphicBufferPool\n
 * This provides 2 helper objects (aspects) extract buffer related info
 *      -# Linear (extract fd/capacity/offset/size and set offset/size and map)
 *      -# Graphic (extract fd/size/format/w/h/stride.. and set crop and map) \n
 * Buffer contains BufferInfo to store information for input/output buffers:
 *      -# "timestamp"   : {uint64_t}
 *      -# "frame-index" : {uint64_t}
 *      -# "input-frame-index" : {uint64_t}        * in processed output only
 *  \n and may contain following optional keys:
 *      -# "decode-timestamp" : {uint64_t}
 *      -# "custom-ordinal"   : {uint64_t}
 *
 * @note Although C2Buffer can potentially contain a list of separate blocks of memory, this class
 *  mandates only 1-block per buffer (to keep it real and simple)
 *
 */
class QC2Buffer : public std::enable_shared_from_this<QC2Buffer> {
 public:
    class Mapping;
    class Linear;
    class Graphic;

    virtual ~QC2Buffer() = default;

    /// check if buffer is linear
    bool isLinear() const {
        return mType == C2BufferData::LINEAR;  // LINEAR_CHUNKS are not supported
    }
    /// get handle to Linear aspect
    Linear& linear() {
        return *mLinear;
    }
    /// check if buffer is graphic-buffer
    bool isGraphic() const {
        return mType == C2BufferData::GRAPHIC;  // GRAPHIC_CHUNKS are not supported
    }
    /// get handle to graphic aspect
    Graphic& graphic() {
        return *mGraphic;
    }

    /// create QC2Buffer from C2Buffer shared (by client)
    static std::shared_ptr<QC2Buffer> CreateFromC2Buffer(const std::shared_ptr<C2Buffer>& buf);

    /**
     * @brief Get a shareable (to client) C2Buffer from this buffer
     *
     * @return If created with CreatefromC2Buffer, it returns the original C2Buffer from
     *    which it was constructed
     * @return If created with LienarBlock/GraphicBlock, returns a shared C2Buffer
     * @return nullptr is backing buffer is invalid
     */
    std::shared_ptr<C2Buffer> getSharedBuffer();

    /// get buffer info (const)
    const Bundle& bufferInfo() const {
        return mBufferInfo;
    }
    /// get buffer info (editable)
    Bundle& bufferInfo() {
        return mBufferInfo;
    }

    // helpers to get/set popular buffer-infos
    struct BufInfoKey {
        static constexpr const char * kTimestamp = "timestamp";
        static constexpr const char * kOutIndex = "output-index";
        static constexpr const char * kInputIndex = "input-index";
        static constexpr const char * kFlags = "flags";
        static constexpr const char * kOrdinal = "custom-ordinal";
    };

    bool hasBufferInfo(const std::string& key);
    uint64_t timestamp() const;
    void setTimestamp(uint64_t);
    uint64_t inputIndex() const;
    void setInputIndex(uint64_t);
    uint64_t outputIndex() const;
    void setOutputIndex(uint64_t);
    uint64_t customOrdinal() const;
    void setCustomOrdinal(uint64_t);
    uint64_t flags() const;
    void setFlags(uint64_t);

    const std::vector<std::shared_ptr<C2Info>>& infos() const;
    QC2Status addInfo(std::shared_ptr<C2Info> info);
    void clearInfos();

    /// copy all the bufferinfos (index/flags/timestamps/infos..)
    QC2Status copyBufInfoTo(std::shared_ptr<QC2Buffer> buf);

    // helpers to print buffers details
    static const char *AsString(std::shared_ptr<C2Buffer> buf, char *str, uint32_t strLen);
    static const char *AsString(std::shared_ptr<QC2Buffer> buf, char *str, uint32_t strLen);

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2Buffer);

    explicit QC2Buffer(std::shared_ptr<C2LinearBlock> linearBlock, uint64_t usage);
    explicit QC2Buffer(std::shared_ptr<C2GraphicBlock> graphicBlock);
    explicit QC2Buffer(std::shared_ptr<C2Buffer> buf);  // assumes buf is non-null and no chunks
    friend class QC2GraphicBufferPoolImpl;
    friend class QC2LinearBufferPool;

    C2BufferData::type_t mType;
    Bundle mBufferInfo;
    std::unique_ptr<Linear> mLinear;
    std::unique_ptr<Graphic> mGraphic;

    std::shared_ptr<C2Buffer> mC2Buffer;    // ref to C2Buffer if created via CreateFromC2Buffer

    // TODO(PC): add getter/setter. Rename existing info() to something else
    std::vector<std::shared_ptr<C2Info>> mInfos;
};

/**
 * @brief Linear aspect of a buffer. Contains helpers for linear buffer access/map/modify
 *
 * It is imperative to check for the validity before accessing
 * @todo (PC) How to get secure usage to check if it is secure and is safe to map ?
 * @todo (PC) For C2Buffer shared by Client, how to get usage or secure-flag ?
 */
class QC2Buffer::Linear {
 public:
    /// check if buffer is valid. If invalid, information returned by the other api are un-reliable
    bool valid() const {
        return mValid;
    }
    /// get the buffer's fd
    inline int fd() const {
        return mSharedFd;
    }
    /// get the data offset
    inline uint32_t offset() const {
        return mOffset;
    }
    /// get the size of data in buffer (whence the offset)
    inline uint32_t size() const {
        return mSize;
    }
    /// get the capacity (allocated length) of buffer
    inline uint32_t capacity() const {
        return mCapacity;
    }
    /// get the raw allocation handle
    inline const C2Handle *handle() const {
        return mC2Handle;
    }
    /// set offset and data-size (only allowed for buffers allocated via QC2LinearBufferPool)
    QC2Status setRange(uint32_t offset, uint32_t size);

    /**
     * Map the buffer as READ-WRITE (for its entire capacity) and return a ref-counted mapping
     * @return Ref-counted mapping if successful
     * @return nullptr if buffer cannot be mapped. Eg: secure OR asked writeable mapping for buffer
     *      shared by the client (which is read-only)
     */
    std::unique_ptr<QC2Buffer::Mapping> map();

    /**
     * Map the buffer as READ-ONLY (for its entire capacity) and return a ref-counted mapping
     * @return Ref-counted mapping if successful
     * @return nullptr if buffer cannot be mapped. Eg: secure
     */
    std::unique_ptr<const QC2Buffer::Mapping> mapReadOnly() const;

 private:
    DECLARE_NON_COPYASSIGNABLE(Linear);
    friend class QC2Buffer;     // to construct a valid linear buffer

    // if linearBlock is null, constructs null-object to avoid returning null and doing null-checks
    Linear(QC2Buffer *buf, std::shared_ptr<C2LinearBlock> lb, uint64_t usage);
    Linear(QC2Buffer *buf, const C2ConstLinearBlock& clb);
    void init(const C2ConstLinearBlock& lb);
    std::shared_ptr<C2Buffer> share();
    std::unique_ptr<QC2Buffer::Mapping> mapInternal(bool writeable) const;

    bool mValid;
    int mSharedFd;
    uint32_t mOffset;
    uint32_t mSize;
    uint32_t mCapacity;
    uint64_t mUsage;
    // QC2Buffer& mBuf;    // Linear's lifetime is coupled with QC2Buffer's. Not need to ref-count
    std::shared_ptr<C2LinearBlock> mLinearBlock;
    const C2Handle *mC2Handle;
};

/**
 * @brief graphic aspect of a buffer. Contains helpers for graphic buffer access/map/modify
 *
 * It is imperative to check for the validity before accessing
 */
class QC2Buffer::Graphic {
 public:
    enum ScanType : uint32_t {
        SCAN_PROGRESSIVE = 0,
        SCAN_FIELD_INTERLEAVED = 1,
        SCAN_LINE_INTERLEAVED = 2,
    };

    /// checks if graphic-buffer is valid
    inline bool valid() const {
        return mValid;
    }
    /// get crop
    inline const C2Rect crop() const {
        return mCrop;
    }
    /// set crop
    QC2Status setCrop(const C2Rect &crop);
    /// get buffer fd
    int fd() const;
    /// get allocated size of buffer
    uint32_t allocSize() const;
    /// get data offset
    uint32_t offset() const;
    /// get (unaligned) width of image in pixels
    uint32_t width() const;
    /// get (unaligned) height of image in pixels
    uint32_t height() const;
    /// get aligned width in pixels
    uint32_t alignedWidth() const;
    /// get stride (aligned width * bpp) in bytes
    uint32_t stride() const;
    /// get slice-height (aligned height) in pixels
    uint32_t sliceHeight() const;
    /// get width of area of buffer to be processed in pixels
    uint32_t effectiveWidth() const;
    /// get height of area of buffer to be processed in pixels
    uint32_t effectiveHeight() const;
    /// get color-format
    uint32_t format() const;
    /// get buffer flags
    uint32_t flags() const;
    /// get unique id
    uint64_t id() const;
    /// check if compatible (i.e w/h/f match)
    inline bool compatible(uint32_t w, uint32_t h, uint32_t format) const;
    /// get compressed size (for UBWC)
    uint32_t compressedSize() const;
    /// set compressed size (for UBWC)
    void setCompressedSize(uint32_t);
    /// get scan type
    ScanType scanType() const;
    /// set scan type
    void setScanType(ScanType scanType);

    /**
     * Map the buffer as READ-WRITE (for its entire capacity) and return a ref-counted mapping
     * @return Ref-counted mapping if successful
     * @return nullptr if buffer cannot be mapped. Eg: secure OR asked writeable mapping for buffer
     *      shared by the client (which is read-only)
     */
    std::unique_ptr<QC2Buffer::Mapping> map();

    /**
     * Map the buffer as READ-ONLY (for its entire capacity) and return a ref-counted mapping
     * @return Ref-counted mapping if successful
     * @return nullptr if buffer cannot be mapped. Eg: secure
     */
    std::unique_ptr<const QC2Buffer::Mapping> mapReadOnly() const;

    ~Graphic();

    static uint32_t GetAlignedWidth(uint32_t, uint32_t);
    static uint32_t GetAlignedHeight(uint32_t, uint32_t);

    // TODO(AS): getPrivateMetadata(types, infos) deprecated. Will be removed
    QC2Status getPrivateMetadata(const std::vector<C2Param::Type>& types,
            std::vector<std::unique_ptr<C2Param>>* infos);

    /**
     * Get metadata from the private handle
     */
    QC2Status getPrivateMetadata(std::vector<std::unique_ptr<C2Info>>* infos,
            std::vector<std::unique_ptr<C2Param>>* params);

    /**
     * Set infos as metadata on the private handle
     */
    QC2Status setPrivateMetadata(const std::vector<std::shared_ptr<C2Info>>& infos);

 private:
    DECLARE_NON_COPYASSIGNABLE(Graphic);
    friend class QC2Buffer;     // to construct a valid graphic buffer
    // if GraphicBlock is null, creates a null-object to avoid returning null and doing null-checks
    Graphic(QC2Buffer *buf, std::shared_ptr<C2GraphicBlock> gb);
    Graphic(QC2Buffer *buf, const C2ConstGraphicBlock& cgb);
    void init(const C2ConstGraphicBlock& gb);
    std::shared_ptr<C2Buffer> share();
    std::unique_ptr<QC2Buffer::Mapping> mapInternal(bool writeable) const;

    // QC2Buffer &mBuf;
    bool mValid;
    C2Rect mCrop;
    uint32_t mCompressedSize;
    float mBpp = 1.0;
    std::unique_ptr<QC2PlatformBuffer> mPvtHnd;
    std::shared_ptr<C2GraphicBlock> mGraphicBlock;
    ScanType mScanType;
};

/**
 * @brief Ref-counted container to hold a mapped memory
 *
 * Object holding mapped memory to a (linear/graphic) buffer.
 * This does not hold an explicit reference to the QC2Buffer and nor does the
 * parent QC2Buffer hold a reference to the mapping.
 * It is client's responsibility to hold the mapping and release when done.
 *
 * When deleted, unmaps the memory automatically.
 * @note Mapping cannot be constructed explicitly; can only be acquired via
 *   QC2Buffer::(Linear/Graphic)::map()
 * @note Mappings cannot be copied
 * @note multiple Mappings can be created from the same QC2Buffer
 */
class QC2Buffer::Mapping {
 public:
    /// read-only base to the mapped memory. May be null
    const uint8_t *base() const {
        return mBase;
    }
    /// read-write base to the mapped memory. May be null
    uint8_t *baseRW() {
        if (!mWriteable) {
            return nullptr;
        }
        return mBase;
    }
    /// size of mapped region
    uint32_t capacity() const {
        return mCapacity;
    }

    ~Mapping();

 private:
    DECLARE_NON_COPYASSIGNABLE(Mapping);
    friend class QC2Buffer::Linear;
    friend class QC2Buffer::Graphic;
    Mapping(int fd, uint32_t offset, uint32_t capacity, bool rw);

    uint8_t *map(uint32_t offset, uint32_t length, int prot);
    void unmap(uint8_t *base, uint32_t length);
    void syncCache(uint64_t flag);

    uint8_t *mBase;  // base va mapped by buffer
    uint32_t mCapacity;  // mapped size
    bool mWriteable;
    int mFd;
};

// Allocators supplied to the Codec

/**
 * @brief Buffer-pool for linear buffers
 *
 * Component provides this pool to the Codec in case of encoder to allocate bitstream buffers
 * for uncompressed buffers
 */
class QC2LinearBufferPool {
 public:
    QC2LinearBufferPool(std::shared_ptr<C2BlockPool> pool, uint64_t usage);

    QC2Status setBufferSize(uint32_t bufferSize);

    QC2Status setAlignment(uint32_t alignment);

    // NOTE: secure usage cannot be changed at run-time
    QC2Status setUsage(uint64_t usage);

    uint64_t usage() const {
        return mUsage;
    }

    QC2Status allocate(std::shared_ptr<QC2Buffer> *buf);

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2LinearBufferPool);
    std::shared_ptr<C2BlockPool> mPool;
    uint64_t mUsage;
    uint32_t mBufSize;
    uint32_t mAlignment;
};

/**
 * @brief: Delete callback on QC2Buffer destruction
 *
 * Callback invoked when shared_ptr<QC2Buffer> goes out of
 * scope when callback is passed as argument to the 'allocate'
 * function. Note that 'buf' is for informational/tracking
 * purposes only and should not be used to deference.
 */
class QC2BufferDeleteCallback {
 public:
    virtual ~QC2BufferDeleteCallback() = default;
    // client callback. 'buf' is informational/tracking only and must
    // not be deferenced
    virtual void signalDeleted(void *buf) = 0;
};

class QC2GraphicBufferPool {
 public:
    virtual ~QC2GraphicBufferPool() { }
    /// optionally register callback for notification of released buffer. If
    /// called, must be done/synchronized by client before first call to
    /// allocate buffer.
    virtual QC2Status registerCallback(std::weak_ptr<QC2BufferDeleteCallback> cb) = 0;
    /// set resolution
    virtual QC2Status setResolution(uint32_t width, uint32_t height) = 0;
    /// set pixel format
    virtual QC2Status setFormat(uint32_t pixelFormat) = 0;
    /// NOTE: secure usage cannot be changed at run-time
    virtual QC2Status setUsage(uint64_t usage) = 0;
    /// allocate a buffer
    virtual QC2Status allocate(std::shared_ptr<QC2Buffer> *buf) = 0;
    /// keep reference to a previously allocated buffer (specifically, to its Block).
    /// This will avoid re-allocating a handle that is already being referenced by the allocatee
    virtual QC2Status holdReference(uint64_t bufferId, std::shared_ptr<QC2Buffer> buffer) = 0;

    /// release the reference of a buffer whose reference was held by calling holdReference(..)
    virtual QC2Status dropReference(uint64_t bufferId) = 0;

    /// release references of all buffers (blocks) held
    virtual QC2Status dropAllReferences() = 0;

    /// get count of referenced bufs and prefetched bufs (for debugging)
    virtual void counts(size_t *nRefs, size_t *nPrefetched) const = 0;
};

/**
 * @brief Buffer-pool for graphic buffers
 *
 * Component provides this pool to the Codec in case of decoder/filter to allocate graphic buffers
 * (uncompressed buffers)
 *
 * C2BlockPool's implementation fetches a graphic-handle in an C2GraphicBlock although
 * the block is still held by the component. This can lead to a starvation in the HW
 * unless a 'unreferenced' block is allocated.
 */
class QC2GraphicBufferPoolImpl : public QC2GraphicBufferPool {
 public:
    QC2GraphicBufferPoolImpl(std::shared_ptr<C2BlockPool> pool, bool isSecure);
    virtual ~QC2GraphicBufferPoolImpl() = default;
    /// optionally register callback for notification of released buffer. If
    /// called, must be done/synchronized by client before first call to
    /// allocate buffer.
    virtual QC2Status registerCallback(std::weak_ptr<QC2BufferDeleteCallback> cb);
    /// set resolution
    QC2Status setResolution(uint32_t width, uint32_t height);
    /// set pixel format
    QC2Status setFormat(uint32_t pixelFormat);
    /// NOTE: secure usage cannot be changed at run-time
    QC2Status setUsage(uint64_t usage);
    /// allocate a buffer
    virtual QC2Status allocate(std::shared_ptr<QC2Buffer> *buf);
    /// keep reference to a previously allocated buffer (specifically, to its Block).
    /// This will avoid re-allocating a handle that is already being referenced by the allocatee
    QC2Status holdReference(uint64_t bufferId, std::shared_ptr<QC2Buffer> buffer) {
        return mRefTracker.holdReference(bufferId, buffer);
    }
    /// release the reference of a buffer whose reference was held by calling holdReference(..)
    QC2Status dropReference(uint64_t bufferId) {
        return mRefTracker.dropReference(bufferId);
    }
    /// release references of all buffers (blocks) held
    QC2Status dropAllReferences() {
        return mRefTracker.reset();
    }
    /// get count of referenced bufs and prefetched bufs (for debugging)
    void counts(size_t *nRefs, size_t *nPrefetched) const {
        return mRefTracker.counts(nRefs, nPrefetched);
    }

 private:
    DECLARE_NON_COPYASSIGNABLE(QC2GraphicBufferPoolImpl);
    const bool mSecure;
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mPixelFormat;
    uint64_t mUsage;
    std::shared_ptr<C2BlockPool> mPool;

    /**
     * @brief helper to manage decoder's output references
     *
     * Keeps track of the references held by the decoder, checks and stashes allocations with
     * references held and makes them available after their corresponding reference has been
     * dropped
     */
    struct RefTracker {
        QC2Status holdReference(uint64_t outputIndex, std::shared_ptr<QC2Buffer> buf);
        QC2Status dropReference(uint64_t outputIndex);
        QC2Status reset();
        QC2Status getPreFetchedAndAvailable(std::shared_ptr<QC2Buffer> *buf);
        bool stashIfReferenced(std::shared_ptr<QC2Buffer> buf);
        void counts(size_t *nRefs, size_t *nPrefetched) const {
            if (nRefs) {
                *nRefs = mRefs.size();
            }
            if (nPrefetched) {
                *nPrefetched = mPrefetchedReferenced.size() + mPrefetchedAvailable.size();
            }
        }

     private:
        std::mutex mLock;
        std::unordered_map<uint64_t, std::shared_ptr<QC2Buffer>> mRefs;
        std::list<std::shared_ptr<QC2Buffer>> mPrefetchedReferenced;
        std::list<std::shared_ptr<QC2Buffer>> mPrefetchedAvailable;
    };
    RefTracker mRefTracker;
    std::weak_ptr<QC2BufferDeleteCallback> mDeleteCb;
};

/// @}

};  // namespace qc2audio

#endif  // _QC2AUDIO_BUFFER_H_

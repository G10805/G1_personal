/*
 *******************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************
*/
#ifndef _QC2_FILTER_COMMON_H_
#define _QC2_FILTER_COMMON_H_

#define QC2_META_CHECK_DATA_ACCESS(_capacity, _off, _strukt)                                      \
    if (_off + offsetof(QC2Metadata, data) + sizeof(struct _strukt) > _capacity) {                \
        QLOGE("Cannot parse %s (%zu bytes) starting offset %zu in buf of capacity %u",            \
                #_strukt, sizeof(struct _strukt), _off + offsetof(QC2Metadata, data), _capacity); \
        return QC2_NO_MEMORY;                                                                     \
    }

namespace qc2 {

/*******************************************************************************
                                 QC2FilterBufferList :
*******************************************************************************/
class QC2FilterBufferList {
public:
    QC2FilterBufferList(const std::string& name, int maxBufs);
    QC2Status store(const std::shared_ptr<QC2Buffer>& buf, int& index);
    QC2Status pop(int index, std::shared_ptr<QC2Buffer>& buf);
    size_t listDepth();
    QC2Status waitUntilEmpty(uint32_t timeOutMs);
private:
    std::string mName;
    std::mutex mLock;   //  mutex for the buffer lists
    std::condition_variable mEmptyCondition;
    std::list<std::pair<int, std::shared_ptr<QC2Buffer>>> mUseBufferList;
    std::list<int> mFreeBufferList;
};

/*******************************************************************************
                                 QC2FilterInputTag :
*******************************************************************************/
class QC2FilterInputTag {
 public:
    static constexpr uint64_t INVALID = 0;
    static inline constexpr uint64_t fromId(uint64_t id) {
        return id >= (UINT64_MAX - kOffset) ? INVALID : id + kOffset;
    }
    static inline constexpr uint64_t toId(uint64_t tag) {
        return tag - kOffset;
    }
 private:
    static constexpr uint64_t kOffset = 1;
};

};  // namespace qc2
#endif

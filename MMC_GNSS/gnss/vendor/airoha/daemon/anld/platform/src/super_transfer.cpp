#define LOG_TAG "SuperTransfer"
#include "super_transfer.h"
#include <assert.h>
#include <inttypes.h>
#include <malloc.h>
#include <memory.h>
#include "simulation.h"
using super::SuperCompressVariant;
using super::SuperMessage;
using super::SuperMessageID;
using super::SuperTransfer;
algorithm::CrcCalculator<uint32_t> SuperMessage::sCrcCal(0x04C11DB7, 0xFFFFFFFF,
                                                         0xFFFFFFFF, true,
                                                         true);
SuperMessage::SuperMessage() {
    mMessageId = 0;
    mLength = 0;
    mData = NULL;
    mReserved = 0;
}
SuperMessage::SuperMessage(SuperMessageID id, int32_t length,
                           const void *data) {
    mData = nullptr;
    mLength = 0;
    setMessageId(id);
    setData(data, length);
    return;
}
SuperMessage::SuperMessage(const uint8_t *dataToFormat, int32_t len) {
    int32_t dataLength = len - kMinimalLength;
    setData(dataToFormat + 10, dataLength);
    setMessageId(*((uint16_t *)(dataToFormat + 2)));
}
SuperMessage::~SuperMessage() {
    if (mData) {
        delete[](mData);
    }
}
int32_t SuperMessage::getNeedPackageLength() {
    return kMinimalLength + mLength;
}
int32_t SuperMessage::buildPackage(uint8_t *buffer, int32_t capacity) {
    int32_t tmpSize = kMinimalLength;
    tmpSize += mLength;
    if (tmpSize > capacity) {
        return -1;
    }
    buffer[0] = SMC_PRE1;
    buffer[1] = SMC_PRE2;
    *((uint16_t *)(&(buffer[2]))) = mMessageId;
    buffer[4] = SMC_RESERVED1;
    buffer[5] = SMC_RESERVED2;
    *((uint32_t *)(&(buffer[6]))) = mLength;
    memcpy(&buffer[10], mData, mLength);
    *((uint32_t *)(&(buffer[10 + mLength]))) = 0;
    buffer[10 + mLength + 4] = SMC_END1;
    buffer[10 + mLength + 4 + 1] = SMC_END2;
    // LOG_D("package crc: 0x%08x", sCrcCal.calculateCrc(buffer, tmpSize,
    // sCrcCal.getInitialValue()));
    *((uint32_t *)(&(buffer[10 + mLength]))) = sCrcCal.calculateCrc(
        buffer + 2, tmpSize - kControlDataSize, sCrcCal.getInitialValue());
    return tmpSize;
}
void SuperMessage::setMessageId(uint16_t id) { this->mMessageId = id; }
uint16_t SuperMessage::getMessageId() const { return this->mMessageId; }
void SuperMessage::setData(const void *buf, int32_t length) {
    if (mData) {
        delete[](mData);
    }
    assert((length & 0x80000000) == 0);
    assert((buf && length) || ((buf == NULL) && (length == 0)));
    if (buf == NULL) {
        mData = NULL;
    } else {
        mData = new uint8_t[length];
        memcpy(mData, buf, length);
    }
    mLength = length;
}
const void *SuperMessage::getData() const { return mData; }
int32_t SuperMessage::getLength() const { return mLength; }
bool SuperMessage::isValid(const uint8_t *data, int32_t len) {
    if (len & 0x80000000) {
        // Package too large
        LOG_E("error packet: packet too large");
        return false;
    }
    if (len < kMinimalLength) {
        return false;
    }
    if (data[0] != SMC_PRE1 || data[1] != SMC_PRE2 ||
        data[len - 1] != SMC_END2 || data[len - 2] != SMC_END1) {
        return false;
    }
    if ((len - kMinimalLength) != (*((int32_t *)(data + 6)))) {
        LOG_E("error packet: len not match");
        return false;
    }
    if (sCrcCal.calculateCrc(data + 2, len - kControlDataSize,
                             sCrcCal.getInitialValue()) !=
        *((uint32_t *)(&(data[len - kEndwordSize - kChecksumSize])))) {
        LOG_E("error packet: crc failed");
        return false;
    }
    return true;
}
SuperTransfer::SuperTransfer(size_t rawBufferSize) {
    mRawBuffer = (uint8_t *)malloc(rawBufferSize);
    mRawBufferReadPtr = 0;
    mMaxBufferLength = rawBufferSize;
}
SuperTransfer::~SuperTransfer() { free(mRawBuffer); }
bool SuperTransfer::start() {
    bool ret = connectionSetup();
    return ret;
}
bool SuperTransfer::stop() {
    bool ret = connectionRelease();
    return ret;
}
int32_t SuperTransfer::streamOutput(const uint8_t *buffer, int32_t length) {
    (void)buffer;
    (void)length;
    return true;
}
bool SuperTransfer::connectionSetup() { return true; }
bool SuperTransfer::connectionRelease() { return true; }
bool SuperTransfer::onFilePullRequestReq(const char *filename) {
    (void)filename;
    return true;
}
bool SuperTransfer::onFilePullRequestRsp(const SuperTransferReponse *response) {
    (void)response;
    return true;
}
bool SuperTransfer::onFilePushRequestReq(const char *filename) {
    (void)filename;
    return true;
}
bool SuperTransfer::onFilePushRequestRsp(const SuperTransferReponse *response) {
    (void)response;
    return true;
}
bool SuperTransfer::onFileTransferBeginReq() { return true; }
bool SuperTransfer::onFileTransferBeginRsp(
    const SuperTransferReponse *response) {
    (void)response;
    return true;
}
bool SuperTransfer::onFileTransferFinishReq() { return true; }
bool SuperTransfer::onFileTransferFinishRsp(
    const SuperTransferReponse *response) {
    (void)response;
    return true;
}
bool SuperTransfer::onFileTransferDataReq(const uint8_t *buffer,
                                          int32_t length) {
    (void)buffer;
    (void)length;
    return true;
}
bool SuperTransfer::onFileTransferDataRsp(
    const SuperTransferReponse *response) {
    (void)response;
    return true;
}
bool SuperTransfer::onOtaTriggerReq() { return true; }
bool SuperTransfer::onOtaTriggerRsp() { return true; }
#define PROCESS_STATUS_IDLE 0
int32_t SuperTransfer::streamInput(const uint8_t *buffer, int32_t length) {
    // LOG_D("stream input %p %d wp(%d)", buffer, length, mRawBufferReadPtr);
    int32_t copyLen = 0;
    if ((int32_t)mRawBufferReadPtr + length > mMaxBufferLength) {
        copyLen = mMaxBufferLength - mRawBufferReadPtr;
    } else {
        copyLen = length;
    }
    if (copyLen == 0) {
        return -1;
    }
    memcpy(mRawBuffer + mRawBufferReadPtr, buffer, copyLen);
    mRawBufferReadPtr += copyLen;
    int32_t i = 0;
    int32_t lastErase = -1;
    for (i = 0; i < mRawBufferReadPtr; i++) {
        // LOG_D("[%d] = [0x%02X]", i, mRawBuffer[i]);
        if (mRawBuffer[i] != SMC_PRE1) {
            lastErase = i;
            continue;
        }
        if (mRawBufferReadPtr - i < SuperMessage::kMinimalLength) {
            break;
        }
        SMH_t *hdr = (SMH_t *)&(mRawBuffer[i]);
        int32_t msgLen = hdr->length + SuperMessage::kMinimalLength;
        // LOG_D("msgLen %d", msgLen);
        if ((msgLen & 0x80000000) || (hdr->reserved1 != SMC_RESERVED1) ||
            (hdr->reserved2 != SMC_RESERVED2)) {
            LOG_E("reserved bit not match");
            lastErase = i;
            continue;
        }
        if ((hdr->length + SuperMessage::kMinimalLength) >
            ((int32_t)mRawBufferReadPtr - i)) {
            // LOG_D("msglen %d", hdr->length +
            // SuperMessage::kMinimalLength);
            break;
        }
        if (SuperMessage::isValid(&(mRawBuffer[i]), msgLen)) {
            // handle Message
            if (hdr->length) {
                onSuperMessage(hdr->mid,
                               &(mRawBuffer[i + sizeof(SuperMessageHeader)]),
                               hdr->length);
            } else {
                onSuperMessage(hdr->mid, NULL, 0);
            }
            lastErase = i + msgLen;
            i = lastErase - 1;  // Jump to Correct Position, 1 will be add
        } else {
            LOG_E("STM Error Packet");
            lastErase = i;
            i = lastErase + 1;
            // Error Handle
        }
    }
    if (lastErase == -1) {
        return copyLen;
    }
    i = 0;
    int32_t k = lastErase + 1;
    while (k < mRawBufferReadPtr) {
        mRawBuffer[i] = mRawBuffer[k];
        k++;
        i++;
    }
    mRawBufferReadPtr = i;
    // LOG_D("handle done: , wp : %d", mRawBufferReadPtr);
    return copyLen;
}
bool SuperTransfer::sendSuperMessage(SuperMessageID messageId,
                                     const void *buffer, int32_t length) {
    // LOG_D("send super message(%p) id(%u), buffer (%p), length(%d)", this,
    //       messageId, buffer, length);
    SuperMessage message(messageId, length, buffer);
    int32_t totalLength = message.getNeedPackageLength();
    if (totalLength > 0) {
        uint8_t *buf = (uint8_t *)malloc(totalLength);
        // LOG_D("malloc buffer %p", buf);
        int32_t buildLength = message.buildPackage(buf, totalLength);
        assert(buildLength == totalLength);
        streamOutput(buf, buildLength);
        free(buf);
        return true;
    }
    return false;
}
bool SuperTransfer::onSuperMessage(uint16_t mid, const uint8_t *buffer,
                                   int32_t length) {
    LOG_D("[STM] ins(%p) %d, %p, %d", this, mid, buffer, length);
    switch (mid) {
        case SuperMessageID::SUPER_ID_TEST_REQ: {
            sendSuperMessage(SUPER_ID_TEST_RSP, nullptr, 0);
            break;
        }
        case SuperMessageID::SUPER_ID_PULL_FILE_REQ: {
            SuperMsgFileReq *fileReq = (SuperMsgFileReq *)buffer;
            if (fileReq->fileNameLength <
                (int32_t)sizeof(SuperMsgFileReq::filename)) {
                onFilePullRequestReq(fileReq->filename);
            }
            break;
        }
        case SuperMessageID::SUPER_ID_PUSH_FILE_REQ: {
            SuperMsgFileReq *fileReq = (SuperMsgFileReq *)buffer;
            if (fileReq->fileNameLength <
                (int32_t)sizeof(SuperMsgFileReq::filename)) {
                onFilePushRequestReq(fileReq->filename);
            }
            break;
        }
        case SuperMessageID::SUPER_ID_PULL_FILE_RSP: {
            onFilePullRequestRsp((const SuperTransferReponse *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_PUSH_FILE_RSP: {
            onFilePushRequestRsp((const SuperTransferReponse *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_DATA_REQ: {
            onFileTransferDataReq(buffer, length);
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_DATA_RSP: {
            onFileTransferDataRsp((const SuperTransferReponse *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_FINISH_REQ: {
            onFileTransferFinishReq();
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_FINISH_RSP: {
            onFileTransferFinishRsp((const SuperTransferReponse *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_BEGIN_REQ: {
            onFileTransferBeginReq();
            break;
        }
        case SuperMessageID::SUPER_ID_FILE_TRANSFER_BEGIN_RSP: {
            onFileTransferBeginRsp((const SuperTransferReponse *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_QUERY_FILE_LIST_REQ: {
            onFileListQueryReq((const char *)buffer);
            break;
        }
        case SuperMessageID::SUPER_ID_QUERY_FILE_LIST_RSP: {
            onFileListQueryRsp(buffer, length);
            break;
        }
        case SuperMessageID::SUPER_ID_OTA_TRIGGER_REQ: {
            onOtaTriggerReq();
            break;
        }
        default:
            break;
    }
    return true;
}
int32_t SuperTransfer::testFunctionStreamInput(const uint8_t *buffer,
                                               int32_t length) {
    LOG_D("===test input %p, %d", buffer, length);
    return streamInput(buffer, length);
}
bool SuperTransfer::onFileListQueryReq(const char *dirname) {
    (void)dirname;
    return false;
}
bool SuperTransfer::onFileListQueryRsp(const void *buffer, int32_t length) {
    (void)buffer;
    (void)length;
    return true;
}
SuperCompressVariant::SuperCompressVariant() {
    mBuffer = (uint8_t *)malloc(kDefaultSize);
    mLength = 0;
    mCapacity = kDefaultSize;
    mWp = 0;
}
SuperCompressVariant::~SuperCompressVariant() {
    if (mBuffer) {
        free(mBuffer);
    }
}
int32_t SuperCompressVariant::getLength() const { return mLength; }
void SuperCompressVariant::setData(const void *buffer, int32_t length) {
    free(mBuffer);
    mCapacity = getNecessarySize(length);
    mBuffer = (uint8_t *)malloc(mCapacity);
    memcpy(mBuffer, buffer, length);
    mLength = length;
    mWp = 0;
}
const uint8_t *SuperCompressVariant::getBuffer() const { return mBuffer; }
void SuperCompressVariant::resetWp() { mWp = 0; }
void SuperCompressVariant::pushInt32(int32_t value) {
    pushValue(VT_INT32, sizeof(int32_t), &value);
    return;
}
void SuperCompressVariant::pushString(const char *string) {
    pushValue(VT_STRING, strlen(string) + 1, string);
    return;
}
void SuperCompressVariant::pushBytesArray(const void *data, int32_t length) {
    pushValue(VT_BYTESARRAY, length, data);
    return;
}
void SuperCompressVariant::pushValue(ValueType type, int32_t contentLength,
                                     const void *vp) {
    while (mLength + kMinHeaderSize + contentLength > mCapacity) {
        resize(mCapacity * 2);
    }
    mBuffer[mLength] = type;
    mLength += sizeof(uint8_t);
    int32_t *lenp = (int32_t *)&mBuffer[mLength];
    *lenp = contentLength;
    mLength += sizeof(int32_t);
    memcpy(&mBuffer[mLength], vp, contentLength);
    mLength += contentLength;
    return;
}
void SuperCompressVariant::resize(int32_t toLength) {
    uint8_t *tmp = (uint8_t *)malloc(toLength);
    int32_t copyLen = (toLength > mLength) ? mLength : toLength;
    if (mBuffer) {
        memcpy(tmp, mBuffer, copyLen);
    }
    free(mBuffer);
    mBuffer = tmp;
    mCapacity = toLength;
    mLength = copyLen;
    resetWp();
}
bool SuperCompressVariant::getInt32(int32_t *value) {
    return getValue(VT_INT32, value, sizeof(int32_t), true);
}
bool SuperCompressVariant::getString(char strBuf[], int32_t cap) {
    return getValue(VT_STRING, strBuf, cap, false);
}
bool SuperCompressVariant::getBytesArray(void *data, int32_t length) {
    return getValue(VT_BYTESARRAY, data, length, false);
}
bool SuperCompressVariant::getValue(ValueType type, void *vp, int32_t length,
                                    bool sizeEqual) {
    if (mLength - mWp < kMinHeaderSize) {
        return false;
    }
    if (mBuffer[mWp] != type) {
        return false;
    }
    int32_t *lenIndicator = (int32_t *)&mBuffer[mWp + 1];
    if ((*lenIndicator + kMinHeaderSize) > (mLength - mWp)) {
        return false;
    }
    if (sizeEqual && length != *lenIndicator) {
        return false;
    }
    if (length < *lenIndicator) {
        return false;
    }
    memcpy(vp, &mBuffer[mWp + kMinHeaderSize], *lenIndicator);
    mWp += (*lenIndicator + kMinHeaderSize);
    return true;
}
void SuperCompressVariant::debugDump() {
    std::string tmp;
    char hex[6] = {0};
    LOG_D(">>>>>>>> length: %" PRId32 ", capacity: %" PRId32, mLength,
          mCapacity);
    for (int32_t i = 0; i < mLength; i += 8) {
        if (mLength - i >= 8) {
            char toPrint[50] = {0};
            const char *format =
                "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X";
            snprintf(toPrint, sizeof(toPrint), format, mBuffer[i],
                     mBuffer[i + 1], mBuffer[i + 2], mBuffer[i + 3],
                     mBuffer[i + 4], mBuffer[i + 5], mBuffer[i + 6],
                     mBuffer[i + 7]);
            LOG_D("%s", toPrint);
        } else {
            while (i != mLength) {
                snprintf(hex, 6, "0x%02X ", mBuffer[i]);
                tmp += hex;
                i++;
            }
            LOG_D("%s", tmp.c_str());
        }
    }
}
int32_t SuperCompressVariant::getNecessarySize(int32_t cap) {
    // Max Cap is 1G
    static int32_t MAXIMUM_CAPACITY = 1024 * 1024 * 1024;
    int32_t n = cap - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return (n < 0) ? 1 : (n >= MAXIMUM_CAPACITY) ? MAXIMUM_CAPACITY : n + 1;
}

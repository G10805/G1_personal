#ifndef SUPER_TRANSFER_H
#define SUPER_TRANSFER_H
#include <stdint.h>
#include <stdio.h>
#include "utility/inc/crc_cal.h"
namespace super {
/**
 *   Data Format
 *  =======================
 * || Pre1 | Pre2 | id(uint16_t) | reserved(uint16_t) | length(int32_t) |data  |
 * uint32_t crc |end 1| end 2||
 *
 * ----------------| ------------ CRC ----------------------------------------|
 *
 *
 *
 */
#pragma pack(push, 1)
typedef struct SuperMessageHeader {
    uint8_t pre1;
    uint8_t pre2;
    uint16_t mid;
    uint8_t reserved1;
    uint8_t reserved2;
    int32_t length;
} SMH_t;
#pragma pack(pop)
static_assert(sizeof(SMH_t) == 10, "Header Size Not Match");
enum SuperMessageControl {
    SMC_PRE1 = 0x34,
    SMC_PRE2 = 0x01,
    SMC_END1 = 0x53,
    SMC_END2 = 0x02,
    SMC_RESERVED1 = 0xAB,
    SMC_RESERVED2 = 0xCD,
};
enum SuperMessageID : uint16_t {
    SUPER_ID_TEST_REQ = 1001,
    SUPER_ID_TEST_RSP = 1002,
    SUPER_ID_QUERY_FILE_LIST_REQ = 1003,
    SUPER_ID_QUERY_FILE_LIST_RSP = 1004,
    SUPER_ID_PUSH_FILE_REQ = 1005,
    SUPER_ID_PUSH_FILE_RSP = 1006,
    SUPER_ID_PULL_FILE_REQ = 1007,
    SUPER_ID_PULL_FILE_RSP = 1008,
    SUPER_ID_FILE_TRANSFER_BEGIN_REQ = 1009,
    SUPER_ID_FILE_TRANSFER_BEGIN_RSP = 1010,
    SUPER_ID_FILE_TRANSFER_DATA_REQ = 1011,
    SUPER_ID_FILE_TRANSFER_DATA_RSP = 1012,
    SUPER_ID_FILE_TRANSFER_FINISH_REQ = 1013,
    SUPER_ID_FILE_TRANSFER_FINISH_RSP = 1014,
    SUPER_ID_FILE_DELETE_REQ = 1015,
    SUPER_ID_FILE_DELETE_RSP = 1016,
    SUPER_ID_DATA_LINK_CONNECT = 2000,
    SUPER_ID_DATA_LINK_DISCONNECT = 2001,
    SUPER_ID_OTA_TRIGGER_REQ = 2002,
    SUPER_ID_OTA_TRIGGER_RSP = 2003,
};
class SuperMessage {
 public:
    SuperMessage();
    SuperMessage(SuperMessageID id, int32_t length, const void *data);
    SuperMessage(const uint8_t *dataToFormat, int32_t len);
    ~SuperMessage();
    int32_t getNeedPackageLength();
    int32_t buildPackage(uint8_t *buffer, int32_t capacity);
    static bool isValid(const uint8_t *data, int32_t len);
    void setMessageId(uint16_t id);
    uint16_t getMessageId() const;
    void setData(const void *buf, int32_t length);
    const void *getData() const;
    int32_t getLength() const;
    static const int32_t kMinimalLength = 16;

 private:
    uint16_t mMessageId;
    uint16_t mReserved;
    int32_t mLength;
    uint8_t *mData;
    static algorithm::CrcCalculator<uint32_t> sCrcCal;
    static const int kPremebleSize = 2;
    static const int kEndwordSize = 2;
    static const int kChecksumSize = 4;
    static const int kControlDataSize =
        kPremebleSize + kEndwordSize + kChecksumSize;
};
enum SuperTransferStatus : uint32_t {
    STS_OK,
    STS_FILE_NOT_EXIST,
    STS_FILE_OPEN_FAILED,
    STS_FILE_REQUEST_EMPTY,
    STS_FILE_REQUEST_PARAMETER_ERROR,
    STS_FILE_DIR_OPEN_FAILED,
};
#pragma pack(push, 1)
struct SuperTransferReponse {
    SuperTransferStatus status;
    uint8_t extra[50];
};
enum FileTransferType : uint8_t {
    FILE_REQUEST_TYPE_PULL,
    FILE_REQUEST_TYPE_PUSH,
};
struct SuperMsgFileReq {
    int32_t fileNameLength;
    char filename[256];
};
enum FileType : uint8_t {
    FT_DIR,
    FT_FILE,
    FT_OTHER,
};
struct SuperMsgFileInfo {
    FileType type;
    int32_t filenameLength;
    char filename[0];
};
#pragma pack(pop)
class SuperCompressVariant {
 public:
    // | uint8_t(type) | int32_t length|
    enum ValueType : uint8_t {
        VT_INT32,
        VT_STRING,
        VT_BYTESARRAY,
    };
    SuperCompressVariant();
    SuperCompressVariant(SuperCompressVariant &) = delete;
    SuperCompressVariant &operator=(SuperCompressVariant &) = delete;
    int32_t getLength() const;
    void setData(const void *buffer, int32_t length);
    const uint8_t *getBuffer() const;
    void pushInt32(int32_t value);
    void pushString(const char *string);
    void pushBytesArray(const void *data, int32_t length);
    bool getInt32(int32_t *value);
    bool getString(char strBuf[], int32_t cap);
    bool getBytesArray(void *data, int32_t length);
    void resetWp();
    ValueType getNextValueType();
    void debugDump();
    ~SuperCompressVariant();
 private:
    uint8_t *mBuffer;
    int32_t mWp;
    int32_t mLength;
    int32_t mCapacity;
    const int32_t kDefaultSize = 32;
    const int32_t kMinHeaderSize = sizeof(uint8_t) + sizeof(uint32_t);
    void pushValue(ValueType type, int32_t contentLength, const void *vp);
    bool getValue(ValueType type, void *vp, int32_t length, bool sizeEqual);
    /**
     * Resize the buffer, wp will be reset
     *
     * \param length
     */
    void resize(int32_t length);
    static int32_t getNecessarySize(int32_t origin);
};
struct SuperFileInfo {
    char *filename;
    SuperFileInfo *next;
};

class SuperTransfer {
 public:
    SuperTransfer(size_t rawBufferSize);
    virtual ~SuperTransfer();
    int32_t testFunctionStreamInput(const uint8_t *buffer, int32_t length);
    bool start();
    bool stop();

 protected:
    virtual int32_t streamOutput(const uint8_t *buffer, int32_t length);
    virtual bool onSuperMessage(uint16_t mid, const uint8_t *buffer,
                                int32_t length);
    // should not block here
    virtual bool connectionSetup();
    virtual bool connectionRelease();
    virtual bool onFilePullRequestReq(const char *filename);
    virtual bool onFilePullRequestRsp(const SuperTransferReponse *response);
    virtual bool onFilePushRequestReq(const char *filename);
    virtual bool onFilePushRequestRsp(const SuperTransferReponse *response);
    virtual bool onFileTransferBeginReq();
    virtual bool onFileTransferBeginRsp(const SuperTransferReponse *response);
    virtual bool onFileTransferFinishReq();
    virtual bool onFileTransferFinishRsp(const SuperTransferReponse *response);
    virtual bool onFileTransferDataReq(const uint8_t *buffer, int32_t length);
    virtual bool onFileTransferDataRsp(const SuperTransferReponse *response);
    virtual bool onFileListQueryReq(const char *dirname);
    virtual bool onFileListQueryRsp(const void *buffer, int32_t length);
    virtual bool onOtaTriggerReq();
    virtual bool onOtaTriggerRsp();
    // Sub-Class use api
    int32_t streamInput(const uint8_t *buffer, int32_t length);
    bool sendSuperMessage(SuperMessageID messageId, const void *buffer,
                          int32_t length);
    // bool sendFileContent(const void *buffer, int32_t length);
    // bool sendFileData(const void *buffer, int32_t length);
 private:
    void processData();
    uint8_t *mRawBuffer;
    int32_t mRawBufferReadPtr;
    int32_t mMaxBufferLength;
};
};  // namespace super
#endif
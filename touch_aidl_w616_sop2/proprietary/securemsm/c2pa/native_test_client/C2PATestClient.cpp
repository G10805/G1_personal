/*========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =========================================================================*/

#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <BufferAllocator/BufferAllocatorWrapper.h>

#include "CTC2PAService.hpp"
#include "ITAccessPermissions.h"
#include "ITC2PAService.hpp"
#include "ITC2PAService_invoke.hpp"
#include "IModule.h"
#include "LinkCredentials.h"
#include "config.h"
#include "fdwrapper.h"
#include "minkipc.h"
#include "object.h"
#include "qcbor.h"
#include "utils.h"
#include "vmuuid.h"
#include "xtzdCredentials.h"

#undef LOG_TAG
#define LOG_TAG "C2PA_NATIVE_TEST_APP:"
#define MINK_QRTR_LE_SERVICE 5008
#define MAX_REPORT_SIZE (16*1024) //16KB is max claim report size
#define SIZE_1KB 1024

using namespace std;

typedef struct {
    int32_t dmaFd;
    uint8_t *vaddr;
} data_buf_t;

struct ImageParam
{
    uint8_t type;
    uint32_t height;
    uint32_t width;
    uint32_t stride;
    uint8_t compression;
};

struct GeneralMediaParam
{
    bool locationPermission;
    uint8_t mediaType;
    string inputFile;
    string outputFolder;
    uint32_t maxThumbnailSize;
    uint8_t thumbnailCompression;
};

struct EnrollParam
{
    string licenseFile;
    string apiKey;
    string signTermTee;
    string signTermNonTee;
    string tsaTerm;
};

class InputFiles: public IInputFilesImplBase
{
    map<string, data_buf_t> mFileMap;
   public:
    InputFiles(map<string, data_buf_t> fileMap): mFileMap(fileMap) {};
    int32_t getFile(const void* label, size_t labelLen, IMemObject &fd);
};

int32_t InputFiles::getFile(const void* label, size_t labelLen, IMemObject &fd)
{
    int32_t ret = Object_OK;
    string key;
    Object fdObject;

    T_CHECK_ERR(label != nullptr && labelLen > 0, ERROR_INVALID_PARAMETER);
    key = (char*) label;

    T_CHECK_ERR(key.size() == labelLen-1, ERROR_INVALID_PARAMETER);
    T_CHECK_ERR(mFileMap.find(key) != mFileMap.end(), ERROR_INVALID_PARAMETER);

    fdObject = FdWrapper_new(mFileMap[key].dmaFd);
    T_CHECK_ERR(!Object_isNull(fdObject), Object_ERROR_KMEM);
    fd.consume(fdObject);

exit:
    return ret;
}

class C2PATest: public ::testing::Test
{
   private:
    Object openerObj;
    MinkIPC *minkHub = nullptr;

    int32_t loadApp();
    int32_t unloadApp();
    int32_t getBufHandle(data_buf_t *bufHandle, size_t size);
    int32_t fdToMemObject(data_buf_t bufHandle, IMemObject **fdMemObject);

   protected:
    virtual void SetUp();
    virtual void TearDown();

    int32_t freeHandle(data_buf_t bufHandle);
    int32_t loadFile(string filePath, int32_t &fileSize, data_buf_t *bufHandle);
    int32_t dumpFile(string filename, uint8_t *vaddr, uint32_t size);
    int32_t processEnrollParam(vector<uint8_t> &inputCbor, int32_t fileSize);
    int32_t processImageSignInput(vector<uint8_t> &inputCbor, int32_t fileSize);
    int32_t processCustomAssertion(map<string, string> inputMap, vector<uint8_t> &cbor);
    int32_t processSignOutput(vector<uint8_t> outputCbor, IOutputFiles &files);
    int32_t processValidationInput(std::vector<uint8_t> &inputCbor, int32_t fileSize);
    int32_t processImageThumbnail(QCBORDecodeContext *decodeCxt,
                                  uint8_t arraySize, IOutputFiles &outFiles);
    int32_t processValidationOutput(vector<uint8_t> outputCbor, IOutputFiles &outFiles);
    int32_t enroll();
    int32_t validateMedia();
    int32_t signMedia(vector<uint8_t> customAssert, IInputFiles &customFile);
    TC2PAService *appObj = nullptr;

  public:
    static EnrollParam enrollParam;
    static ImageParam imageParam;
    static GeneralMediaParam mediaParam;
};

EnrollParam C2PATest::enrollParam = {"", "", "", ""};
ImageParam C2PATest::imageParam = {1, 0, 0, 0, 90};
GeneralMediaParam C2PATest::mediaParam = {false, 0, "", "", 1280, 90};

int32_t C2PATest::getBufHandle(data_buf_t *bufHandle, size_t size)
{
    int32_t ret = 0;
    uint32_t len = 0;
    uint32_t flags = 0;
    BufferAllocator *bufferAllocator = CreateDmabufHeapBufferAllocator();

    memset(bufHandle, 0, sizeof(bufHandle));
    bufHandle->dmaFd = -1;

    len = (size + (SIZE_2MB -1)) & (~(SIZE_2MB - 1));

    // Allocate memory for non-contiguous buffer handle
    bufHandle->dmaFd = DmabufHeapAlloc(bufferAllocator, (char *) "qcom,system", len, 0, 0);
    T_CHECK_ERR(bufHandle->dmaFd > 0, -1);

    // map the buffer - only non-secure buffers should be mapped
    bufHandle->vaddr =
        (uint8_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, bufHandle->dmaFd, 0);
    if (bufHandle->vaddr == MAP_FAILED) {
        bufHandle->vaddr = NULL;
        ret = -1;
    }

    T_CHECK_ERR(ret == 0, ret);
    ALOGD("%s::%d getting non-secure buffer handle successfull,"
          "size = %x, buf_fd = %d, vaddr = %x",
          __func__, __LINE__, size,
          bufHandle->dmaFd, bufHandle->vaddr);

exit:
    if (bufferAllocator) {
        FreeDmabufHeapBufferAllocator(bufferAllocator);
    }
    return ret;
}

int32_t C2PATest::freeHandle(data_buf_t bufHandle)
{
    int32_t ret = 0;

    T_CHECK_ERR(bufHandle.dmaFd >= 0, -1);
    ret = close(bufHandle.dmaFd);

    T_CHECK_ERR(ret == 0, ret);
    ALOGD("%s::%d Successfully released DMA fd %d", __func__, __LINE__, bufHandle.dmaFd);
    bufHandle.dmaFd = -1;
exit:
    return ret;
}

int32_t C2PATest::fdToMemObject(data_buf_t bufHandle, IMemObject **fdMemObject)
{
    int32_t ret = Object_OK;
    Object fdObject;

    (*fdMemObject) = new IMemObject();
    T_CHECK_ERR(fdMemObject != nullptr, -1);

    fdObject = FdWrapper_new(bufHandle.dmaFd);
    T_CHECK_ERR(!Object_isNull(fdObject), Object_ERROR_KMEM);

    (*fdMemObject)->consume(fdObject);
exit:
    if (ret) {
        delete fdMemObject;
        Object_ASSIGN_NULL(fdObject);
    }
    return ret;
}

int32_t C2PATest::loadFile(string filePath, int32_t &fileSize,
                           data_buf_t *bufHandle)
{
    int32_t ret = 0;
    int32_t fd = -1;
    struct stat appStat = {};

    fd = open(filePath.c_str(), O_RDONLY);
    T_CHECK_ERR(fd > 0, -1);

    ret = fstat(fd, &appStat);
    T_CHECK_ERR(ret == 0, -1);

    fileSize = appStat.st_size;
    T_CHECK_ERR(fileSize > 0, -1);

    ret = getBufHandle(bufHandle, fileSize);
    T_CHECK_ERR(ret == 0, ret);

    ret = read(fd, bufHandle->vaddr, fileSize);
    T_CHECK_ERR(ret == fileSize, -1);

    ret = 0;
    ALOGD("%s::%d Successfull load file, file size = %d, "
          "buf_fd = %d, vaddr = %x", __func__, __LINE__,
          fileSize, bufHandle->dmaFd,
          bufHandle->vaddr);

exit:
    if (bufHandle->vaddr != nullptr) {
        munmap(bufHandle->vaddr, fileSize);
    }

    if (fd > 0) {
        close(fd);
    }

    return ret;
}

int32_t C2PATest::dumpFile(string filename, uint8_t *vaddr, uint32_t size)
{
    int32_t ret = 0, destFd = -1;
    string filePath;
    T_CHECK_ERR(C2PATest::mediaParam.outputFolder != "" && filename != "", -1);

    filePath = C2PATest::mediaParam.outputFolder + filename;
    destFd = open(filePath.c_str(), O_RDWR | O_CREAT, 0666);
    T_CHECK_ERR(destFd >= 0, -1);

    ret = write(destFd, vaddr, size);
    T_CHECK_ERR(ret == size, -1);
    ret = 0;
    ALOGD("%s::%d Successfully write buffer %x to file %s",
           __func__, __LINE__, vaddr, filePath.c_str());
exit:
    if (destFd >= 0) {
        close(destFd);
    }
    return ret;
}


int32_t C2PATest::loadApp()
{
    int32_t ret = 0;
    Object c2paObject;
    Object linkCredentials = Object_NULL;
    Object xtzdCredentials = Object_NULL;
    CredInfo credInfo = {0, 0, 0};
    credInfo.pid = getpid();
    ITAccessPermissions_rules confRules = {};

    minkHub = MinkIPC_connect_QRTR(MINK_QRTR_LE_SERVICE, &openerObj);
    T_CHECK_ERR(minkHub != nullptr && !Object_isNull(openerObj), Object_ERROR_BADOBJ);

    ret = LinkCred_new(&credInfo, ELOC_LOCAL, UNIX, &linkCredentials);
    T_CHECK_ERR(ret == 0, ret);
    T_CHECK_ERR(!Object_isNull(linkCredentials), Object_ERROR_BADOBJ);

    ret = XtzdCredentials_newFromCBO(linkCredentials, &xtzdCredentials);
    T_CHECK_ERR(ret == 0, ret);
    T_CHECK_ERR(!Object_isNull(xtzdCredentials), Object_ERROR_BADOBJ);

    ret = IModule_open(openerObj, CTC2PAService_UID, linkCredentials, &c2paObject);
    T_CHECK_ERR(ret == 0, ret);
    T_CHECK_ERR(!Object_isNull(c2paObject), Object_ERROR_BADOBJ);

    appObj = new TC2PAService(c2paObject);
    T_CHECK_ERR(appObj != nullptr, -1);
    ALOGD("%s::%d Successfully able to get C2PA sevice object", __func__, __LINE__);

exit:
    Object_ASSIGN_NULL(xtzdCredentials);
    Object_ASSIGN_NULL(linkCredentials);
    return ret;
}

int32_t C2PATest::unloadApp()
{
    if (appObj != nullptr) {
        ALOGD("%s::%d release appObj", __func__, __LINE__);
        delete appObj;
        appObj = nullptr;
    }

    ALOGD("%s::%d release service", __func__, __LINE__);
    Object_ASSIGN_NULL(openerObj);

    if (minkHub != nullptr) {
        MinkIPC_release(minkHub);
        minkHub = nullptr;
    }

    return 0;
}
int32_t C2PATest::processEnrollParam(vector<uint8_t> &inputCbor, int32_t fileSize)
{
    int32_t ret = 0;
    size_t encodedLen = 0;

    QCBOREncodeContext encodeContext;
    QCBOREncode_Init(&encodeContext, inputCbor.data(), inputCbor.size());
    QCBOREncode_OpenMap(&encodeContext);

    QCBOREncode_AddSZStringToMapN(&encodeContext, CLOUD_CREDENTIALS,
                                  C2PATest::enrollParam.apiKey.c_str());
    QCBOREncode_AddUInt64ToMapN(&encodeContext, FEATURE_LICENSE, fileSize);
    QCBOREncode_AddSZStringToMapN(&encodeContext, SIGN_TERM_TEE,
                                  C2PATest::enrollParam.signTermTee.c_str());
    QCBOREncode_AddSZStringToMapN(&encodeContext, SIGN_TERM_NON_TEE,
                                  C2PATest::enrollParam.signTermNonTee.c_str());
    QCBOREncode_AddSZStringToMapN(&encodeContext, TSA_TERM,
                                  C2PATest::enrollParam.tsaTerm.c_str());
    QCBOREncode_CloseMap(&encodeContext);

    ret = QCBOREncode_Finish(&encodeContext, &encodedLen);
    T_CHECK_ERR(ret == 0 && encodedLen > 0, -1);
    inputCbor.resize(encodedLen);
    ALOGD("%s::%d Successfully processed and loaded enroll param to CBOR of size %d",
           __func__, __LINE__, inputCbor.size());
exit:
    return ret;
}
int32_t C2PATest::processImageSignInput(vector<uint8_t> &inputCbor, int32_t fileSize)
{
    int32_t ret = 0;
    size_t encodedLen = 0;

    QCBOREncodeContext encodeContext;
    QCBOREncode_Init(&encodeContext, inputCbor.data(), inputCbor.size());
    QCBOREncode_OpenMap(&encodeContext);

    QCBOREncode_AddUInt64ToMapN(&encodeContext, MEDIA_TYPE, C2PATest::mediaParam.mediaType);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, MEDIA_IS_SECURE, 0);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, LOCATION_PERMISSION,
                                C2PATest::mediaParam.locationPermission);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, IMAGE_TYPE, C2PATest::imageParam.type);

    if (C2PATest::imageParam.type == 0) {
        QCBOREncode_AddUInt64ToMapN(&encodeContext, IMAGE_WIDTH, C2PATest::imageParam.width);
        QCBOREncode_AddUInt64ToMapN(&encodeContext, IMAGE_HEIGHT, C2PATest::imageParam.height);
        QCBOREncode_AddUInt64ToMapN(&encodeContext, IMAGE_STRIDE, C2PATest::imageParam.stride);
        QCBOREncode_AddUInt64ToMapN(&encodeContext, IMAGE_COMPRESSION,
                                    C2PATest::imageParam.compression);
    }

    QCBOREncode_AddUInt64ToMapN(&encodeContext, THUMBNAIL_MAX_SIZE,
                                C2PATest::mediaParam.maxThumbnailSize);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, THUMBNAIL_QUALITY,
                                C2PATest::mediaParam.thumbnailCompression);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, INPUT_MEDIA_FILE, fileSize);
    QCBOREncode_CloseMap(&encodeContext);

    ret = QCBOREncode_Finish(&encodeContext, &encodedLen);
    T_CHECK_ERR(ret == 0 && encodedLen > 0, -1);
    inputCbor.resize(encodedLen);
    ALOGD("%s::%d Successfully processed and loaded sign param to CBOR of size %d",
           __func__, __LINE__, inputCbor.size());
exit:
    return ret;
}

int32_t C2PATest::processCustomAssertion(map<string, string> inputMap, vector<uint8_t> &inputCbor)
{
    int32_t ret = 0;
    size_t encodedLen = 0;

    QCBOREncodeContext encodeContext;
    QCBOREncode_Init(&encodeContext, inputCbor.data(), inputCbor.size());
    QCBOREncode_OpenMap(&encodeContext);
    for (const auto& item : inputMap) {
        QCBOREncode_AddSZStringToMap(&encodeContext, item.first.c_str(), item.second.c_str());
    }
    QCBOREncode_CloseMap(&encodeContext);
    ret = QCBOREncode_Finish(&encodeContext, &encodedLen);

    T_CHECK_ERR(ret == 0 && encodedLen > 0, -1);
    inputCbor.resize(encodedLen);
    ALOGD("%s::%d Successfully processed and loaded custom assertion param to CBOR of size %d",
           __func__, __LINE__, inputCbor.size());
exit:
    return ret;
}

int32_t C2PATest::processSignOutput(vector<uint8_t> outputCbor, IOutputFiles &outFiles)
{
    int32_t ret = Object_OK, itemInMap = 0, iter = 0;
    std::string label;

    uint32_t mediaFdSize = 0;
    data_buf_t bufHandle = {-1, NULL};
    IMemObject *imageFile = nullptr;

    QCBORDecodeContext decodeCxt;
    QCBORItem parent, child;
    UsefulBufC encoded;

    T_CHECK_ERR(!outFiles.isNull(), -1);

    encoded = {outputCbor.data(), outputCbor.size()};
    QCBORDecode_Init(&decodeCxt, encoded, QCBOR_DECODE_MODE_NORMAL);
    ret = QCBORDecode_GetNext(&decodeCxt, &parent);
    itemInMap = parent.val.uCount;

    ALOGD("%s::%d Received CBOR MAP of size %d", __func__, __LINE__, itemInMap);

    for (iter = 0; iter < itemInMap; iter++) {
        ret = QCBORDecode_GetNext(&decodeCxt, &child);
        T_CHECK_ERR(ret == 0, -1);
        T_CHECK_ERR(child.uLabelType == QCBOR_TYPE_INT64, -1);

        switch(child.label.int64) {
            case OUTPUT_MEDIA_FILE:
                T_CHECK_ERR(child.uDataType == QCBOR_TYPE_INT64, -1);
                label = STRINGIFY(OUTPUT_MEDIA_FILE);
                mediaFdSize = (uint32_t) child.val.int64;
                T_CHECK_ERR(mediaFdSize > 0, -1);

                ret = getBufHandle(&bufHandle, mediaFdSize);
                T_CHECK_ERR(ret == 0, ret);

                ret = fdToMemObject(bufHandle, &imageFile);
                T_CHECK_ERR(ret == 0 && imageFile != nullptr, ret);

                ret = outFiles.getFile(label.c_str(), label.size()+1, *imageFile);
                T_CHECK_ERR(ret == 0, ret);

                ret = dumpFile("signed_media", bufHandle.vaddr, mediaFdSize);
                T_CHECK_ERR(ret == 0, ret);
                break;
            default:
                ALOGD("CBOR tag not supported");
        }
    }
    ALOGD("%s::%d Successfully parsed CBOR and extracted output parameter", __func__, __LINE__);

exit:
    if (imageFile != nullptr) {
        delete imageFile;
    }
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, mediaFdSize);
    }
    freeHandle(bufHandle);
    return ret;
}

int32_t C2PATest::processValidationInput(vector<uint8_t> &inputCbor, int32_t fileSize)
{
    int32_t ret = 0;
    size_t encodedLen = 0;

    QCBOREncodeContext encodeContext;
    QCBOREncode_Init(&encodeContext, inputCbor.data(), inputCbor.size());
    QCBOREncode_OpenMap(&encodeContext);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, MEDIA_TYPE, C2PATest::mediaParam.mediaType);
    QCBOREncode_AddUInt64ToMapN(&encodeContext, INPUT_MEDIA_FILE, fileSize);
    QCBOREncode_CloseMap(&encodeContext);
    ret = QCBOREncode_Finish(&encodeContext, &encodedLen);
    T_CHECK_ERR(ret == 0 && encodedLen > 0, -1);
    inputCbor.resize(encodedLen);
    ALOGD("%s::%d Successfully generated validation CBOR of size %d",
           __func__, __LINE__, inputCbor.size());
exit:
    return ret;
}

int32_t C2PATest::processImageThumbnail(QCBORDecodeContext *decodeCxt, uint8_t arraySize,
                                        IOutputFiles &outFiles)
{
    int32_t ret = Object_OK, iter = 0, innerIter = 0;
    std::string label;
    map<int64_t, string> info;

    uint32_t fdSize = 0;
    IMemObject *fileObject = nullptr;
    data_buf_t bufHandle = {-1, NULL};

    QCBORItem parent, child;

    ALOGD("%s::%d Received thumbnail data array of size %d", __func__, __LINE__, arraySize);

    for (iter = 0; iter < arraySize; iter++) {
        ret = QCBORDecode_GetNext(decodeCxt, &parent);
        T_CHECK_ERR(ret == 0 && parent.uDataType == QCBOR_TYPE_MAP, -1);

        info = {
            {THUMBNAIL_URI, ""},
            {THUMBNAIL_ID, ""},
            {THUMBNAIL_MANIFEST_URI, ""},
            {THUMBNAIL_INSTANCE_ID, ""},
            {THUMBNAIL_TITLE, ""},
            {THUMBNAIL_FORMAT, ""}
        };
        for (innerIter = 0; innerIter < parent.val.uCount; innerIter++) {
            ret = QCBORDecode_GetNext(decodeCxt, &child);
            T_CHECK_ERR(ret == 0 && child.uLabelType == QCBOR_TYPE_INT64, -1);

            switch(child.label.int64) {
                case THUMBNAIL_URI:
                case THUMBNAIL_ID:
                case THUMBNAIL_MANIFEST_URI:
                case THUMBNAIL_INSTANCE_ID:
                case THUMBNAIL_TITLE:
                case THUMBNAIL_FORMAT:
                    T_CHECK_ERR(child.uDataType == QCBOR_TYPE_TEXT_STRING, -1);
                    info[child.label.uint64] = std::string((char *) child.val.string.ptr,
                                                           child.val.string.len);
                    break;

                case THUMBNAIL_FILE:
                    T_CHECK_ERR(child.uDataType == QCBOR_TYPE_INT64, -1);
                    label = STRINGIFY(THUMBNAIL_FILE) + std::to_string(iter);
                    fdSize = (uint32_t) child.val.int64;

                    ret = getBufHandle(&bufHandle, fdSize);
                    T_CHECK_ERR(ret == 0, ret);

                    ret = fdToMemObject(bufHandle, &fileObject);
                    T_CHECK_ERR(ret == 0 && fileObject != nullptr, ret);

                    ret = outFiles.getFile(label.c_str(), label.size()+1, *fileObject);
                    T_CHECK_ERR(ret == 0, ret);

                    ret = dumpFile("thumbnail_file_"+std::to_string(iter)+".jpg", bufHandle.vaddr,
                                   fdSize);
                    T_CHECK_ERR(ret == 0, ret);

                    if (bufHandle.vaddr != nullptr) {
                        munmap(bufHandle.vaddr, fdSize);
                        bufHandle.vaddr = nullptr;
                    }

                    if (fileObject != nullptr) {
                        delete fileObject;
                        fileObject = nullptr;
                    }
                    freeHandle(bufHandle);
                    break;

                default:
                    ALOGD("%s::%d CBOR tag not supported");
            }
        }
        ALOGD("%s::%d Out uri - %s Id - %s Manifest uri - %s Instance - %s \
               Title - %s Format - %s", __func__, __LINE__,
               info[THUMBNAIL_URI].c_str(),
               info[THUMBNAIL_ID].c_str(),
               info[THUMBNAIL_MANIFEST_URI].c_str(),
               info[THUMBNAIL_INSTANCE_ID].c_str(),
               info[THUMBNAIL_TITLE].c_str(),
               info[THUMBNAIL_FORMAT].c_str());
    }
    ALOGD("%s::%d Successfully parsed all thumbnail output parameter", __func__, __LINE__);
exit:
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, fdSize);
    }
    if (fileObject != nullptr) {
        delete fileObject;
    }
    freeHandle(bufHandle);
    return ret;
}

int32_t C2PATest::processValidationOutput(vector<uint8_t> outputCbor, IOutputFiles &outFiles)
{
    int32_t ret = Object_OK, itemInMap = 0, iter = 0;
    string label;

    QCBORDecodeContext decodeCxt;
    QCBORItem parent, child;
    UsefulBufC encoded;

    uint32_t fdSize = 0;
    IMemObject *fileObject = nullptr;
    data_buf_t bufHandle = {-1, NULL};

    T_CHECK_ERR(!outFiles.isNull(), -1);

    encoded = {outputCbor.data(), outputCbor.size()};
    QCBORDecode_Init(&decodeCxt, encoded, QCBOR_DECODE_MODE_NORMAL);
    ret = QCBORDecode_GetNext(&decodeCxt, &parent);
    itemInMap = parent.val.uCount;

    for (iter = 0; iter < itemInMap; iter++) {
        ret = QCBORDecode_GetNext(&decodeCxt, &child);
        T_CHECK_ERR(ret == 0, -1);

        T_CHECK_ERR(child.uLabelType == QCBOR_TYPE_INT64, -1);

        switch(child.label.uint64) {
            case THUMBNAIL_ARRAY:
                T_CHECK_ERR(child.uDataType == QCBOR_TYPE_ARRAY, -1);
                ret = processImageThumbnail(&decodeCxt, child.val.uCount, outFiles);
                T_CHECK_ERR(ret == 0, -1);
                break;

            case VALIDATION_REPORT:
                T_CHECK_ERR(child.uDataType == QCBOR_TYPE_INT64, -1);
                label = STRINGIFY(VALIDATION_REPORT);
                fdSize = (uint32_t) child.val.int64;

                ret = getBufHandle(&bufHandle, fdSize);
                T_CHECK_ERR(ret == 0, ret);

                ret = fdToMemObject(bufHandle, &fileObject);
                T_CHECK_ERR(ret == 0 && fileObject != nullptr, ret);

                ret = outFiles.getFile(label.c_str(), label.size()+1, *fileObject);
                T_CHECK_ERR(ret == 0, ret);

                ret = dumpFile("validation_report.txt", bufHandle.vaddr,
                               fdSize);
                T_CHECK_ERR(ret == 0, ret);
                break;

            default:
                ALOGD("%s::%d CBOR tag not supported");
        }
    }
    ALOGD("%s::%d Successfully write report of size %d to file", __func__, __LINE__, fdSize);
exit:
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, fdSize);
        bufHandle.vaddr = nullptr;
    }

    if (fileObject != nullptr) {
        delete fileObject;
        fileObject = nullptr;
    }
    freeHandle(bufHandle);
    return ret;
}

int32_t C2PATest::enroll()
{
    int32_t ret = Object_OK, fileSize = -1;
    vector<uint8_t> inputCbor(SIZE_1KB, 0);

    map<string, data_buf_t> mFileMap;
    data_buf_t bufHandle = {-1, NULL};
    string key = STRINGIFY(FEATURE_LICENSE);
    InputFiles *inputFile = nullptr;
    IInputFiles *inputProxy = nullptr;
    Object inputFileObj;
    IOutputFiles outFiles;

    ret = loadFile(C2PATest::enrollParam.licenseFile, fileSize, &bufHandle);
    T_CHECK_ERR(ret == 0, -1);

    ret = processEnrollParam(inputCbor, fileSize);
    T_CHECK_ERR(ret == 0, -1);

    mFileMap[key] = bufHandle;
    inputFile = new InputFiles(mFileMap);
    T_CHECK_ERR(inputFile != nullptr, -1);

    inputFileObj = (Object){ImplBase::invoke, inputFile};
    inputProxy = new IInputFiles(inputFileObj);
    T_CHECK_ERR(inputProxy != nullptr, -1);

    ret = appObj->enroll(inputCbor.data(), inputCbor.size(), *inputProxy);
    T_CHECK_ERR(ret == 0, ret);

    ALOGD("%s::%d Successfully enrolled device for signing", __func__, __LINE__);
exit:
    if (inputProxy != nullptr) {
        delete inputProxy;
    }
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, fileSize);
    }
    freeHandle(bufHandle);
    return ret;
}

int32_t C2PATest::signMedia(vector<uint8_t> customAssert, IInputFiles &customFile)
{
    int32_t ret = Object_OK, fileSize = -1;
    size_t outSize = 0;
    vector<uint8_t> inputCbor(SIZE_1KB, 0), outputCbor(SIZE_1KB, 0);

    map<string, data_buf_t> mFileMap;
    data_buf_t bufHandle = {-1, NULL};
    string key = STRINGIFY(INPUT_MEDIA_FILE);
    InputFiles *inputFile = nullptr;
    IInputFiles *inputProxy = nullptr;
    Object inputFileObj;
    IOutputFiles outFiles;

    ret = loadFile(C2PATest::mediaParam.inputFile, fileSize, &bufHandle);
    T_CHECK_ERR(ret == 0, ret);

    switch(C2PATest::mediaParam.mediaType)
    {
        case 0:
            T_CHECK_ERR(C2PATest::imageParam.type <= 1, -1);
            ret = processImageSignInput(inputCbor, fileSize);
            break;
        default:
           ret = -1;
           LOGD_PRINT("Media type not supported");
    }
    T_CHECK_ERR(ret == 0, -1);

    mFileMap[key] = bufHandle;
    inputFile = new InputFiles(mFileMap);
    T_CHECK_ERR(inputFile != nullptr, -1);

    inputFileObj = (Object){ImplBase::invoke, inputFile};
    inputProxy = new IInputFiles(inputFileObj);
    T_CHECK_ERR(inputProxy != nullptr, -1);

    ret = appObj->signMedia(inputCbor.data(), inputCbor.size(), *inputProxy,
                            customAssert.data(), customAssert.size(), customFile,
                            outputCbor.data(), outputCbor.size(), &outSize, outFiles);

    T_CHECK_ERR(ret == 0 && !(outFiles.isNull()) && outSize > 0, -1);
    outputCbor.resize(outSize);
    LOGD_PRINT("Sign claim result: %d", ret);

    ret = processSignOutput(outputCbor, outFiles);
    T_CHECK_ERR(ret == 0, -1);
    LOGD_PRINT("Successfully extracted and stored signed file");

exit:
    if (inputProxy != nullptr) {
        delete inputProxy;
    }
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, fileSize);
    }
    freeHandle(bufHandle);
    return ret;
}

int32_t C2PATest::validateMedia()
{
    int32_t ret = 0, fileSize = -1;
    size_t validationOutSize = 0;
    uint8_t claimResult = 0;
    vector<uint8_t> inputCbor(SIZE_1KB, 0), validationCbor(16*SIZE_1KB, 0);

    string key = STRINGIFY(INPUT_MEDIA_FILE);
    data_buf_t bufHandle = {-1, NULL};
    map<string, data_buf_t> mFileMap;
    InputFiles *inputFile = nullptr;
    IInputFiles *inputProxy = nullptr;
    Object inputFileObj;
    IOutputFiles outFiles;

    T_CHECK_ERR(C2PATest::mediaParam.mediaType == 0, -1);

    ret = loadFile(C2PATest::mediaParam.inputFile, fileSize, &bufHandle);
    T_CHECK_ERR(ret == 0, ret);

    ret = processValidationInput(inputCbor, fileSize);
    T_CHECK_ERR(ret == 0, -1);

    mFileMap[key] = bufHandle;

    inputFile = new InputFiles(mFileMap);
    T_CHECK_ERR(inputFile != nullptr, -1);

    inputFileObj = (Object){ImplBase::invoke, inputFile};
    inputProxy = new IInputFiles(inputFileObj);
    T_CHECK_ERR(inputProxy != nullptr, -1);

    ret = appObj->validateMedia(inputCbor.data(), inputCbor.size(), *inputProxy, &claimResult,
                                validationCbor.data(), validationCbor.size(), &validationOutSize,
                                outFiles);
    T_CHECK_ERR(ret == 0, ret);
    T_CHECK_ERR(!(outFiles.isNull()) && validationOutSize > 0, -1);
    validationCbor.resize(validationOutSize);

    LOGD_PRINT("Media validation claim result: %d", claimResult);
    ret = processValidationOutput(validationCbor, outFiles);
    if (claimResult == 0) {
        ret = ITC2PAService::ERROR_INVALID_MEDIA;
    }
exit:
    if (inputProxy != nullptr) {
        delete inputProxy;
    }
    if (bufHandle.vaddr != nullptr) {
        munmap(bufHandle.vaddr, fileSize);
    }
    freeHandle(bufHandle);
    return ret;
}

void C2PATest::SetUp()
{
    int ret = 0;
    ret = loadApp();

    ASSERT_TRUE(ret == 0);
}

void C2PATest::TearDown()
{
    int ret = 0;
    ret = unloadApp();
    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, Enroll)
{
    LOGD_PRINT("Enroll test");
    int32_t ret = 0;

    ret = enroll();
exit:
    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, EnrollWithoutNetwork)
{
    LOGD_PRINT("Enroll without network test");
    int32_t ret = 0;

    ret = enroll();
exit:
    ASSERT_TRUE(ret == Object_OK || ret == ITC2PAService::ERROR_NETWORK_SERVICE);
}

TEST_F(C2PATest, SignMedia)
{
    LOGD_PRINT("Sign Media test");
    int32_t ret = Object_OK;

    vector<uint8_t> customAssert;
    IInputFiles customFile;

    ret = signMedia(customAssert, customFile);

    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, SignJPEGImage)
{
    LOGD_PRINT("Sign JPEG image test");
    int32_t ret = Object_OK;

    vector<uint8_t> customAssert;
    IInputFiles customFile;

    C2PATest::mediaParam.mediaType = 0;
    C2PATest::imageParam.type = 1;

    ret = signMedia(customAssert, customFile);

    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, SignYUVImage)
{
    LOGD_PRINT("Sign YUV Image test");
    int32_t ret = Object_OK;

    vector<uint8_t> customAssert;
    IInputFiles customFile;

    C2PATest::mediaParam.mediaType = 0;
    C2PATest::imageParam.type = 0;

    ret = signMedia(customAssert, customFile);

    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, SignCustomClaimMedia)
{
    LOGD_PRINT("Sign Media test");
    int32_t ret = Object_OK;

    vector<uint8_t> customAssert(4*SIZE_1KB, 0);
    IInputFiles customFile;

    map<string, string> customMap;
    customMap["T1"] = "Test script for custom map 1";
    customMap["T2"] = "Test script for custom map 2";
    customMap["T3"] = "Test script for custom map 3";
    customMap["T4"] = "Test script for custom map 4";

    ret = processCustomAssertion(customMap, customAssert);
    ASSERT_TRUE(ret == Object_OK);

    ret = signMedia(customAssert, customFile);

    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, ValidateMedia)
{
    LOGD_PRINT("Validate media test");
    int32_t ret = 0;

    ret = validateMedia();
    T_CHECK_ERR(ret == 0, -1);

exit:
    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, ValidateValidMedia)
{
    LOGD_PRINT("Validate valid media test");

    int32_t ret = 0;

    ret = validateMedia();

exit:
    ASSERT_TRUE(ret == Object_OK);
}

TEST_F(C2PATest, ValidateModifiedMedia)
{
    LOGD_PRINT("Validate modified media test");

    int32_t ret = 0;

    ret = validateMedia();

exit:
    ASSERT_TRUE(ret == TC2PAService::ERROR_INVALID_MEDIA);
}

TEST_F(C2PATest, ValidateNonC2PAMedia)
{
    LOGD_PRINT("Validate Non C2PA media test");

    int32_t ret = 0;

    ret = validateMedia();

exit:
    ASSERT_TRUE(ret == TC2PAService::ERROR_MANIFEST_CORRUPTED);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    if (argc == 1) goto run;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--inputfile=", sizeof("--inputfile=") - 1 ) == 0) {
            C2PATest::mediaParam.inputFile = argv[i] + sizeof("--inputfile=") - 1;
        } else if (strncmp(argv[i], "--in=", sizeof("--in=") - 1 ) == 0) {
            C2PATest::mediaParam.inputFile = argv[i] + sizeof("--in=") - 1;
        } else if (strncmp(argv[i], "--outputPath=", sizeof("--outputPath=") - 1 ) == 0) {
            C2PATest::mediaParam.outputFolder = argv[i] + sizeof("--outputfile=") - 1;
        } else if (strncmp(argv[i], "--out=", sizeof("--out=") - 1 ) == 0) {
            C2PATest::mediaParam.outputFolder = argv[i] + sizeof("--out=") - 1;
        } else if (strncmp(argv[i], "--mediaType=", sizeof("--mediaType=") - 1 ) == 0) {
            C2PATest::mediaParam.mediaType = atoi(argv[i] + sizeof("--mediaType=") - 1);
        } else if (strcmp(argv[i], "--location") == 0) {
            C2PATest::mediaParam.locationPermission = true;
        } else if (strncmp(argv[i], "--imageType=", sizeof("--imageType=") - 1 ) == 0) {
            C2PATest::imageParam.type = (uint8_t) atoi(argv[i] + sizeof("--imageType=") - 1);
        } else if (strncmp(argv[i], "--compression=", sizeof("--compression=") - 1 ) == 0) {
            C2PATest::imageParam.compression =
                    (uint8_t) atoi(argv[i] + sizeof("--compression=") - 1);
        } else if (strncmp(argv[i], "--height=", sizeof("--height=") - 1 ) == 0) {
            C2PATest::imageParam.height = atoi(argv[i] + sizeof("--height=") - 1);
        } else if (strncmp(argv[i], "--width=", sizeof("--width=") - 1 ) == 0) {
            C2PATest::imageParam.width = atoi(argv[i] + sizeof("--width=") - 1);
        } else if (strncmp(argv[i], "--stride=", sizeof("--stride=") - 1 ) == 0) {
            C2PATest::imageParam.stride = atoi(argv[i] + sizeof("--stride=") - 1);
        } else if (strncmp(argv[i], "--thumbnailSize=", sizeof("--thumbnailSize=") - 1 ) == 0) {
            C2PATest::mediaParam.maxThumbnailSize = atoi(argv[i] + sizeof("--thumbnailSize=") - 1);
        } else if (strncmp(argv[i], "--thumbnailCompression=",
                   sizeof("--thumbnailCompression=") - 1 ) == 0) {
            C2PATest::mediaParam.thumbnailCompression =
                    atoi(argv[i] + sizeof("--thumbnailCompression=") - 1);
        } else if (strncmp(argv[i], "--licenseFile=", sizeof("--licenseFile=") - 1 ) == 0) {
            C2PATest::enrollParam.licenseFile = argv[i] + sizeof("--licenseFile=") - 1;
        } else if (strncmp(argv[i], "--apiKey=", sizeof("--apiKey=") - 1 ) == 0) {
            C2PATest::enrollParam.apiKey = argv[i] + sizeof("--apiKey=") - 1;
        } else if (strncmp(argv[i], "--signTermTee=", sizeof("--signTermTee=") - 1 ) == 0) {
            C2PATest::enrollParam.signTermTee = argv[i] + sizeof("--signTermTee=") - 1;
        } else if (strncmp(argv[i], "--signTermNonTee=", sizeof("--signTermNonTee=") - 1 ) == 0) {
            C2PATest::enrollParam.signTermNonTee = argv[i] + sizeof("--signTermNonTee=") - 1;
        } else if (strncmp(argv[i], "--tsaTerm=", sizeof("--tsaTerm=") - 1 ) == 0) {
            C2PATest::enrollParam.tsaTerm = argv[i] + sizeof("--tsaTerm=") - 1;
        } else {
            return 0;
        }
    }
run:
    return RUN_ALL_TESTS();
}
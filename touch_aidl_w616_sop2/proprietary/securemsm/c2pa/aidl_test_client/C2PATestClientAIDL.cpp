/*========================================================================
  Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =========================================================================*/

#include <aidl/android/hardware/common/Ashmem.h>
#include <aidl/vendor/qti/hardware/c2pa/BnC2PA.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cutils/ashmem.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <utils/Log.h>

#include "memscpy.h"
#include "utils.h"

#undef LOG_TAG
#define LOG_TAG "C2PA_TEST_APP_AIDL:"
#define MAX_REPORT_SIZE (16*1024) //16KB is max claim report size

using ::aidl::android::hardware::common::Ashmem;
using ::ndk::ScopedAIBinder_DeathRecipient;
using ::ndk::ScopedAStatus;
using ::ndk::SpAIBinder;

using namespace std;
using namespace ::aidl::vendor::qti::hardware::c2pa;

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

class C2PATest: public ::testing::Test
{
   private:
    SpAIBinder c2paBinder;
    ScopedAIBinder_DeathRecipient deathRecipient;

   protected:
    virtual void SetUp();
    virtual void TearDown();
    int32_t loadFile(string imagePath, Ashmem& ashmemFile);
    int32_t dumpFile(string filename, uint8_t *vaddr, uint32_t size);
    int32_t dumpAshmemFile(string filename, Ashmem &inFile);
    int32_t processStringOutput(C2PADataTypePair &item, string key, string &value);
    int32_t processEnrollParam(vector<C2PADataTypePair> &param);
    int32_t processImageParam(vector<C2PADataTypePair> &imageParam);
    int32_t processImageValidationOutput(vector<C2PADataTypePair> &output);
    int32_t enroll(EnrollResponse &result);
    int32_t signMedia(vector<C2PADataTypePair> &customAssertion);
    int32_t validateMedia(ValidateResponse &claimResult, vector<C2PADataTypePair> &output);

   public:
    static shared_ptr<IC2PA> c2paService;
    static EnrollParam enrollParam;
    static ImageParam imageParam;
    static GeneralMediaParam mediaParam;
    static void serviceDied(void* cookie);
};

EnrollParam C2PATest::enrollParam = {"", "", "", ""};
ImageParam C2PATest::imageParam = {1, 0, 0, 0, 90};
GeneralMediaParam C2PATest::mediaParam = {0, "", "", 1280, 90};

shared_ptr<IC2PA> C2PATest::c2paService = nullptr;

void C2PATest::serviceDied(void* cookie) {
    ALOGI("C2PA AIDL died");
    C2PATest::c2paService = nullptr;
}

void C2PATest::SetUp()
{
    int32_t ret = -1;
    ScopedAStatus status = ScopedAStatus::ok();
    const string instance = string() + IC2PA::descriptor +"/default";

    if (!AServiceManager_isDeclared(instance.c_str())) {
        LOGE_PRINT("%s:%d AIDL service is not declared in VINTF manifest", __func__, __LINE__);
        goto exit;
    }

    c2paBinder = ::ndk::SpAIBinder(AServiceManager_waitForService(instance.c_str()));
    T_CHECK_ERR(c2paBinder.get() != nullptr, -1);

    deathRecipient = ScopedAIBinder_DeathRecipient(AIBinder_DeathRecipient_new(&serviceDied));
    status = ScopedAStatus::fromStatus(AIBinder_linkToDeath(c2paBinder.get(),
                                       deathRecipient.get(),(void*) serviceDied));

    T_CHECK_ERR(status.isOk(), -1);

    C2PATest::c2paService = IC2PA::fromBinder(c2paBinder);

    T_CHECK_ERR (C2PATest::c2paService != nullptr, -1);

    LOGD_PRINT("Connected to C2PA AIDL service");
    ret = 0;

exit:
    ASSERT_TRUE(ret == 0);
}

void C2PATest::TearDown()
{
    ScopedAStatus status = ScopedAStatus::ok();
    if(C2PATest::c2paService != nullptr) {
        status = ScopedAStatus::fromStatus(AIBinder_unlinkToDeath(c2paBinder.get(),
                                           deathRecipient.get(), (void*) serviceDied));
        if (!status.isOk()) {
            LOGD_PRINT("Failed to unlinking from death recipient %d: %s",
                        status.getStatus(), status.getMessage());
        }
        C2PATest::c2paService = nullptr;
    }
    ASSERT_TRUE(status.isOk());
}

int32_t C2PATest::dumpFile(string filename, uint8_t *vaddr, uint32_t size)
{
    int32_t ret = 0, destFd = -1;
    string filePath;
    T_CHECK_ERR(C2PATest::mediaParam.outputFolder != "" && filename != "", -1);

    filePath = C2PATest::mediaParam.outputFolder + filename;

    ALOGD("%s::%d Write buffer at vaddr = %x to file %s", __func__, __LINE__,
           vaddr, filePath.c_str());

    destFd = open(filePath.c_str(), O_RDWR | O_CREAT, 0666);
    T_CHECK_ERR(destFd >= 0, -1);

    ret = write(destFd, vaddr, size);
    T_CHECK_ERR(ret == size, -1);
    ret = 0;

    ALOGD("%s::%d Successfully wrote %d bytes to file", __func__, __LINE__, size);
exit:
    if (destFd >= 0) {
        close(destFd);
    }
    return ret;
}

int32_t C2PATest::dumpAshmemFile(string filename, Ashmem &inFile)
{
    int32_t ret = 0, destFd = -1;
    uint8_t *data = nullptr;
    string filePath;
    size_t fileSize;

    fileSize = (size_t) inFile.size;
    T_CHECK_ERR(fileSize > 0, -1);

    data = (uint8_t*) mmap(NULL, fileSize, PROT_READ, MAP_SHARED, inFile.fd.get(), 0);
    if (data == MAP_FAILED) {
        ALOGE("%s::%d mmap of buffer fd failed with error: %s", __func__, __LINE__,
               strerror(errno));
        ret = -1;
        goto exit;
    }

    ALOGD("Succefully loaded ashmem file %d at vaddr = %x", inFile.fd.get(), data);
    ret = dumpFile(filename, data, fileSize);

exit:
    if (destFd >= 0) {
        close(destFd);
    }
    return ret;
}

int32_t C2PATest::loadFile(string filePath, Ashmem& ashmemFile)
{
    int32_t ret = 0;
    int32_t imageFd = -1, ashmemFd = -1;
    struct stat appStat = {};
    void *imageBuff = nullptr, *outImageBuffer = nullptr;
    size_t copied = 0;
    size_t imageFileSize = 0;

    imageFd = open(filePath.c_str(), O_RDONLY);
    T_CHECK_ERR(imageFd > 0, imageFd);

    ret = fstat(imageFd, &appStat);
    T_CHECK_ERR(ret == 0, -1);

    imageFileSize = appStat.st_size;
    T_CHECK_ERR(imageFileSize > 0, -1);

    imageBuff = malloc(imageFileSize);
    T_CHECK_ERR(imageBuff != nullptr, -1);

    ret = read(imageFd, imageBuff, imageFileSize);
    T_CHECK_ERR(ret == imageFileSize, -1);
    ret = 0;

    LOGD_PRINT("Load image of size :%d", imageFileSize);
    ashmemFd = ashmem_create_region("C2PA_ADIL_Client", imageFileSize);
    T_CHECK_ERR(ashmemFd > 0, imageFd);

    outImageBuffer = mmap(NULL, imageFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, ashmemFd, 0);
    T_CHECK_ERR(outImageBuffer != MAP_FAILED, -1);

    copied =  memscpy(outImageBuffer, imageFileSize, imageBuff, imageFileSize);
    T_CHECK_ERR(copied == imageFileSize, -1);
    ashmemFile.fd.set(ashmemFd);
    ashmemFile.size = imageFileSize;

    LOGD_PRINT("Successfully loaded image into ashmem memory");

exit:
    if (outImageBuffer != nullptr) {
        munmap(outImageBuffer, imageFileSize);
    }
    if (ret != 0 && ashmemFd >= 0) {
        close(ashmemFd);
    }
    if (imageBuff != NULL) {
        free(imageBuff);
    }
    if (imageFd > 0) {
        close(imageFd);
    }
    return ret;
}

int32_t C2PATest::processStringOutput(C2PADataTypePair &item, string key, string &value)
{
    int32_t ret = 0;
    T_CHECK_ERR(item.key == key && item.value.getTag() == C2PADataType::stringValue, -1);
    value = item.value.get<C2PADataType::stringValue>();
exit:
    return ret;
}

int32_t C2PATest::processEnrollParam(vector<C2PADataTypePair> &param)
{
    int32_t ret = 0;
    C2PADataTypePair outPair;

    outPair.key = "CLOUD_CREDENTIALS";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>(C2PATest::enrollParam.apiKey);
    param.push_back(std::move(outPair));

    outPair.key = "FEATURE_LICENSE";
    outPair.value = C2PADataType::make<C2PADataType::fdValue>();
    ret = loadFile(C2PATest::enrollParam.licenseFile, outPair.value.get<C2PADataType::fdValue>());
    T_CHECK_ERR(ret == 0, -1);
    param.push_back(std::move(outPair));

    outPair.key = "SIGN_TERM_TEE";
    outPair.value =
        C2PADataType::make<C2PADataType::stringValue>(C2PATest::enrollParam.signTermTee);
    param.push_back(std::move(outPair));

    outPair.key = "SIGN_TERM_NON_TEE";
    outPair.value =
        C2PADataType::make<C2PADataType::stringValue>(C2PATest::enrollParam.signTermNonTee);
    param.push_back(std::move(outPair));

    outPair.key = "TSA_TERM";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>(C2PATest::enrollParam.tsaTerm);
    param.push_back(std::move(outPair));

exit:
    return ret;
}

int32_t C2PATest::processImageParam(vector<C2PADataTypePair> &param)
{
    int32_t ret = 0;
    C2PADataTypePair outPair;

    outPair.key = "IMAGE_TYPE";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::imageParam.type);
    param.push_back(std::move(outPair));

    outPair.key = "IMAGE_HEIGHT";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::imageParam.height);
    param.push_back(std::move(outPair));

    outPair.key = "IMAGE_WIDTH";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::imageParam.width);
    param.push_back(std::move(outPair));

    outPair.key = "IMAGE_STRIDE";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::imageParam.stride);
    param.push_back(std::move(outPair));

    outPair.key = "IMAGE_COMPRESSION";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::imageParam.compression);
    param.push_back(std::move(outPair));

    return ret;
}

int32_t C2PATest::processImageValidationOutput(vector<C2PADataTypePair> &output)
{
    int32_t ret = 0;
    uint8_t listSize = 0, iter = 0, iter2 = 0, iter3 = 0;

    vector<string> thumbnailLabel = {"THUMBNAIL_URI", "THUMBNAIL_ID",
                                     "THUMBNAIL_MANIFEST_URI", "THUMBNAIL_INSTANCE_ID",
                                     "THUMBNAIL_TITLE", "THUMBNAIL_FORMAT"};
    vector<string> thumbnailValue;
    string label, report;

    for (iter = 0; iter < output.size(); iter++)
    {
        if (output[iter].key == "VALIDATION_REPORT") {
            T_CHECK_ERR(output[iter].value.getTag() == C2PADataType::fdValue, -1);
            ret = dumpAshmemFile("validation_report.txt",
                                 output[iter].value.get<C2PADataType::fdValue>());
            if (ret != 0) {
                LOGD_PRINT("Unable to write thumbnail image data to file");
                ret = 0;
            }
        } else if (output[iter].key == "THUMBNAIL_ARRAY" &&
                   output[iter].value.getTag() == C2PADataType::intValue) {
            listSize = output[iter].value.get<C2PADataType::intValue>();

            for (iter2 = 0; iter2 < listSize; iter2++) {
                thumbnailValue = {"", "", "", "", "", ""};
                iter++;
                ALOGD("%s:: Process thumbnail at index: %d", __func__, iter);
                for (iter3 = 0; iter3 < thumbnailLabel.size() && iter < output.size(); iter3++) {
                    label = thumbnailLabel[iter3] + to_string(iter2);
                    processStringOutput(output[iter], label, thumbnailValue[iter3]);
                    iter++;
                }

                ALOGD("%s::%d Out uri - %s Id - %s Manifest uri - %s Instance - %s Title - %s\
                      Format - %s", __func__, __LINE__,
                      thumbnailValue[0].c_str(),
                      thumbnailValue[1].c_str(),
                      thumbnailValue[2].c_str(),
                      thumbnailValue[3].c_str(),
                      thumbnailValue[4].c_str(),
                      thumbnailValue[5].c_str());

                label = "THUMBNAIL_FILE" + to_string(iter2);
                T_CHECK_ERR(output[iter].key == label &&
                            output[iter].value.getTag() == C2PADataType::fdValue, -1);
                ret = dumpAshmemFile("thumbnail_file_"+std::to_string(iter2)+".jpg",
                                      output[iter].value.get<C2PADataType::fdValue>());
                if (ret != 0) {
                    LOGD_PRINT("Unable to write thumbnail image data to file");
                    ret = 0;
                }
            }
        }
    }
    LOGD_PRINT("Successfully parsed thumbnail information");
exit:
    return ret;
}

int32_t C2PATest::enroll(EnrollResponse &result)
{
    int32_t ret = 0;
    ScopedAStatus status = ScopedAStatus::ok();
    vector<C2PADataTypePair> enrollParam;

    ret = processEnrollParam(enrollParam);
    T_CHECK_ERR(ret == 0, -1);

    status = c2paService->enroll(enrollParam, &result);
    T_CHECK_ERR(status.isOk() &&
                result == EnrollResponse::ENROLL_RESPONSE_SUCCESS, (int32_t) result);
exit:
    return ret;
}

int32_t C2PATest::signMedia(vector<C2PADataTypePair> &customAssertion)
{
    int32_t ret = 0;
    SignResponse result = SignResponse::SIGN_RESPONSE_FAILED;
    ScopedAStatus status = ScopedAStatus::ok();
    Ashmem imageFd, outImageFd;
    vector<C2PADataTypePair> inputParam;

    C2PADataTypePair outPair;

    outPair.key = "MEDIA_TYPE";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::mediaParam.mediaType);
    inputParam.push_back(std::move(outPair));

    outPair.key = "THUMBNAIL_MAX_SIZE";
    outPair.value =
        C2PADataType::make<C2PADataType::intValue>(C2PATest::mediaParam.maxThumbnailSize);
    inputParam.push_back(std::move(outPair));

    outPair.key = "THUMBNAIL_QUALITY";
    outPair.value =
        C2PADataType::make<C2PADataType::intValue>(C2PATest::mediaParam.thumbnailCompression);
    inputParam.push_back(std::move(outPair));

    if (C2PATest::mediaParam.mediaType == 0) {
        ret = processImageParam(inputParam);
    } else {
        ret = -1;
        LOGD_PRINT("Media format is currently not supported");
    }

    ret = loadFile(C2PATest::mediaParam.inputFile, imageFd);
    T_CHECK_ERR(ret == 0 && (imageFd.fd.get()) >= 0, -1);

    status = c2paService->signMedia(imageFd, inputParam, customAssertion, &outImageFd, &result);
    T_CHECK_ERR(status.isOk() && result == SignResponse::SIGN_RESPONSE_SUCCESS &&
                (outImageFd.fd.get()) > 0, (int32_t) result);
    ret = 0;

    LOGD_PRINT("Successfully signed media using AIDL service");
    ret = dumpAshmemFile("signed_media", outImageFd);
    T_CHECK_ERR(ret == 0, -1)

    LOGD_PRINT("Successfully wrote signed media to file");
exit:
    return ret;
}

int32_t C2PATest::validateMedia(ValidateResponse &result, vector<C2PADataTypePair> &output)
{
    int32_t ret = 0;
    ScopedAStatus status = ScopedAStatus::ok();
    Ashmem imageFd, outImageFd;
    vector<C2PADataTypePair> inputParam;

    C2PADataTypePair outPair;

    outPair.key = "MEDIA_TYPE";
    outPair.value = C2PADataType::make<C2PADataType::intValue>(C2PATest::mediaParam.mediaType);
    inputParam.push_back(std::move(outPair));

    if (C2PATest::mediaParam.mediaType != 0) {
        LOGD_PRINT("Media type is currently not supported");
        ret = -1;
    }
    T_CHECK_ERR(ret == 0, -1);

    ret = loadFile(C2PATest::mediaParam.inputFile, imageFd);
    T_CHECK_ERR(ret == 0 && (imageFd.fd.get()) >= 0, -1);

    status = c2paService->validateMedia(imageFd, inputParam, &output, &result);
    LOGD_PRINT("Result of validate image output: %d", result);
    T_CHECK_ERR(result != ValidateResponse::VALIDATION_RESPONSE_FAILED, (uint32_t) result);
    ret = 0;

exit:
    return ret;
}

TEST_F(C2PATest, Enroll)
{
    LOGD_PRINT("Enrollment test");
    int32_t ret = 0;
    EnrollResponse result = EnrollResponse::ENROLL_RESPONSE_FAILED;

    ret = enroll(result);
    T_CHECK(ret == 0 && result == EnrollResponse::ENROLL_RESPONSE_SUCCESS);

    LOGD_PRINT("Enrollment successfull: %d", ret);
exit:
    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, EnrollWithoutNetwork)
{
    LOGD_PRINT("Enrollment test without internet");
    int32_t ret = 0;
    EnrollResponse result = EnrollResponse::ENROLL_RESPONSE_FAILED;

    ret = enroll(result);
    T_CHECK(ret == 0 && result == EnrollResponse::ENROLL_RESPONSE_SUCCESS);
    LOGD_PRINT("Enrollment successfull: %d", ret);

exit:
    ASSERT_TRUE(result == EnrollResponse::ENROLL_RESPONSE_SUCCESS ||
                result == EnrollResponse::ENROLL_RESPONSE_NETWORK_ERROR);
}

TEST_F(C2PATest, SignMedia)
{
    LOGD_PRINT("Sign Media test");

    int32_t ret = 0;
    vector<C2PADataTypePair> customAssertion;

    ret = signMedia(customAssertion);

    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, SignJPEGImage)
{
    LOGD_PRINT("Sign JPEG image test");

    int32_t ret = 0;
    C2PATest::mediaParam.mediaType = 0;
    C2PATest::imageParam.type = 1;

    vector<C2PADataTypePair> customAssertion;

    ret = signMedia(customAssertion);

    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, SignYUVImage)
{
    LOGD_PRINT("Sign YUV image test");

    int32_t ret = 0;
    C2PATest::mediaParam.mediaType = 0;
    C2PATest::imageParam.type = 0;
    vector<C2PADataTypePair> customAssertion;

    ret = signMedia(customAssertion);

    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, SignCustomClaimMedia)
{
    LOGD_PRINT("Sign media with custom claim test");

    int32_t ret = 0;
    C2PADataTypePair outPair;
    vector<C2PADataTypePair> customAssertion;

    outPair.key = "T1";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>("Test script for custom map 1");
    customAssertion.push_back(std::move(outPair));

    outPair.key = "T2";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>("Test script for custom map 2");
    customAssertion.push_back(std::move(outPair));

    outPair.key = "T3";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>("Test script for custom map 3");
    customAssertion.push_back(std::move(outPair));

    outPair.key = "T4";
    outPair.value = C2PADataType::make<C2PADataType::stringValue>("Test script for custom map 4");
    customAssertion.push_back(std::move(outPair));

    ret = signMedia(customAssertion);

    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, ValidateMedia)
{
    LOGD_PRINT("Validate media test");

    int32_t ret = 0;
    ValidateResponse claimResult = ValidateResponse::VALIDATION_RESPONSE_FAILED;
    vector<C2PADataTypePair> validationOutput;

    ret = validateMedia(claimResult, validationOutput);
    T_CHECK(claimResult == ValidateResponse::VALIDATION_RESPONSE_VALID ||
            claimResult == ValidateResponse::VALIDATION_RESPONSE_INVALID);

    ret = processImageValidationOutput(validationOutput);
    T_CHECK_ERR(ret == 0, -1);

    LOGD_PRINT("Validation successful: %d", ret);
exit:
    ASSERT_TRUE(ret == 0);
}

TEST_F(C2PATest, ValidateValidMedia)
{
    LOGD_PRINT("Validate valid media test");

    int32_t ret = 0;
    ValidateResponse claimResult;
    vector<C2PADataTypePair> validationOutput;

    ret = validateMedia(claimResult, validationOutput);
exit:
    ASSERT_TRUE(ret == 0 && claimResult == ValidateResponse::VALIDATION_RESPONSE_VALID);
}

TEST_F(C2PATest, ValidateModifiedMedia)
{
    LOGD_PRINT("Validate modified media test");

    int32_t ret = 0;
    ValidateResponse claimResult;
    vector<uint8_t> claimReport;
    vector<C2PADataTypePair> validationOutput;

    ret = validateMedia(claimResult, validationOutput);

exit:
    ASSERT_TRUE(claimResult == ValidateResponse::VALIDATION_RESPONSE_INVALID);
}

TEST_F(C2PATest, ValidateNonC2PAMedia)
{
    LOGD_PRINT("Validate non C2PA media test");

    int32_t ret = 0;
    ValidateResponse claimResult;
    vector<uint8_t> claimReport;
    vector<C2PADataTypePair> validationOutput;

    ret = validateMedia(claimResult, validationOutput);

exit:
    ASSERT_TRUE(claimResult == ValidateResponse::VALIDATION_RESPONSE_MANIFEST_CORRUPTED);
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
            C2PATest::mediaParam.mediaType = (uint8_t) atoi(argv[i] + sizeof("--mediaType=") - 1);
        } else if (strncmp(argv[i], "--imageType=", sizeof("--imageType=") - 1 ) == 0) {
            C2PATest::imageParam.type = (uint8_t) atoi(argv[i] + sizeof(argv[i]) - 1);
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
                   sizeof("--thumbnailCompression=") - 1) == 0) {
            C2PATest::mediaParam.thumbnailCompression =
                    (uint8_t) atoi(argv[i] + sizeof("--thumbnailCompression=") - 1);
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

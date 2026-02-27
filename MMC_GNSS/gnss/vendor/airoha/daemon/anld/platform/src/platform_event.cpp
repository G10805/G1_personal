#define LOG_TAG "PEVT"
#include "platform_event.h"
#include <assert.h>
#include <inttypes.h>
#include <memory.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#ifdef __ANDROID__
#include <cutils/properties.h>
#endif
#include "EpoBase.h"
#include "airo_gps.h"
#include "anld_service_interface.h"
#include "default_platform.h"
#include "download_manager.h"
#include "native_time.h"
#include "network_util.h"
#include "simulation.h"
#define EPO_FILE_PERMISSION (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
// clang-format off
#define TEST_FUNC_ENABLE
IfEpoFile_t gEpoFileList[] = {
#ifndef PLATFORM_SUPPORT_ELPO
    {
        .constellation = airoha::epo::EC_GPS,
        .fileType = airoha::epo::EFT_QEPO,
        .filename = "QGPS.DAT"
    },
    {
        .constellation = airoha::epo::EC_GPS,
        .fileType = airoha::epo::EFT_EPO,
        .filename = "EPO_GPS_3_1.DAT"
    },
    {
        .constellation = airoha::epo::EC_GLONASS,
        .fileType = airoha::epo::EFT_QEPO,
        .filename = "QG_R.DAT"
    },
    {
        .constellation = airoha::epo::EC_GLONASS,
        .fileType = airoha::epo::EFT_EPO,
        .filename = "EPO_GR_3_1.DAT"
    },
    {
        .constellation = airoha::epo::EC_GALILEO,
        .fileType = airoha::epo::EFT_QEPO,
        .filename = "QGA.DAT"},
    {
        .constellation = airoha::epo::EC_GALILEO,
        .fileType = airoha::epo::EFT_EPO,
        .filename = "EPO_GAL_3.DAT"
    },
    {
        .constellation = airoha::epo::EC_BEIDOU,
        .fileType = airoha::epo::EFT_QEPO,
        .filename = "QBD2.DAT"
    },
    {
        .constellation = airoha::epo::EC_BEIDOU,
        .fileType = airoha::epo::EFT_EPO,
        .filename = "EPO_BDS_3.DAT"
    },
#else
    // ELPO struct
    {
        .constellation = airoha::epo::EC_GPS,
        .fileType = airoha::epo::EFT_ELPO_3,
        .filename = "ELPO_GPS_3.DAT",
        .tmpFilename = "ELPO_GPS_3.DAT.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_GLONASS,
        .fileType = airoha::epo::EFT_ELPO_3,
        .filename = "ELPO_GLO_3.DAT",
        .tmpFilename = "ELPO_GLO_3.DAT.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_BEIDOU,
        .fileType = airoha::epo::EFT_ELPO_3,
        .filename = "ELPO_BDS_3.DAT",
        .tmpFilename = "ELPO_BDS_3.DAT.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_GALILEO,
        .fileType = airoha::epo::EFT_ELPO_3,
        .filename = "ELPO_GAL_3.DAT",
        .tmpFilename = "ELPO_GAL_3.DAT.waitcopy"
    },
    // QELPO Struct
    {
        .constellation = airoha::epo::EC_GPS,
        .fileType = airoha::epo::EFT_QELPO,
        .filename = "QELPO_GPS.DAT",
        .tmpFilename = "QELPO_GPS.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_GLONASS,
        .fileType = airoha::epo::EFT_QELPO,
        .filename = "QELPO_GLO.DAT",
        .tmpFilename = "QELPO_GLO.DAT.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_BEIDOU,
        .fileType = airoha::epo::EFT_QELPO,
        .filename = "QELPO_BDS.DAT",
        .tmpFilename = "QELPO_BDS.DAT.waitcopy"
    },
    {
        .constellation = airoha::epo::EC_GALILEO,
        .fileType = airoha::epo::EFT_QELPO,
        .filename = "QELPO_GAL.DAT",
        .tmpFilename = "QELPO_GAL.DAT.waitcopy"
    },
#endif
};
// clang-format on
PlatformEventLoop::PMessage::~PMessage() {
    if (buffer) {
        free(buffer);
    }
}
PlatformEventLoop::PMessage::PMessage() {
    evt = EVENT_INVALID;
    length = 0;
    buffer = nullptr;
}
PlatformEventLoop::EpoDownloadResult::EpoDownloadResult() {
    status = US_LAST_UNKNOWN;
    filter.constellation = airoha::epo::EC_INVALID;
    lastSuccessMs = -1;
    lastDownloadSystemMs = -1;
}
std::string PlatformEventLoop::EpoDownloadResult::toString() {
    std::string tmp = "epo download result: ";
    switch (filter.constellation) {
        case airoha::epo::EC_GPS:
            tmp += " EC_GPS ";
            break;
        case airoha::epo::EC_GLONASS:
            tmp += " EC_GLONASS ";
            break;
        case airoha::epo::EC_GALILEO:
            tmp += " EC_GALILEO ";
            break;
        case airoha::epo::EC_BEIDOU:
            tmp += " EC_BEIDOU ";
            break;
        case airoha::epo::EC_INVALID:
            tmp += " EC_INVALID ";
            break;
        default:
            break;
    }
    switch (filter.type) {
        case airoha::epo::EFT_EPO:
            tmp += " EFT_EPO ";
            break;
        case airoha::epo::EFT_QEPO:
            tmp += " EFT_QEPO ";
            break;
        case airoha::epo::EFT_ELPO_3:
            tmp += " EFT_ELPO_3";
            break;
        default:
            break;
    }
    switch (status) {
        case US_LAST_FAIL:
            tmp += " US_LAST_FAIL ";
            break;
        case US_LAST_PROCESSING:
            tmp += " US_LAST_PROCESSING ";
            break;
        case US_LAST_SUCCESS:
            tmp += " US_LAST_SUCCESS ";
            break;
        case US_LAST_UNKNOWN:
            tmp += " US_LAST_UNKNOWN";
            break;
        default:
            break;
    }
    char t[64] = {0};
    snprintf(t, sizeof(t), "last update(mono_ms): %" PRId64, lastSuccessMs);
    tmp += t;
    return tmp;
}
void PlatformEventLoop::PMessage::setData(const void *tbuffer, size_t tlength) {
    if (tlength) {
        this->buffer = malloc(tlength);
        memcpy(this->buffer, tbuffer, tlength);
        this->length = tlength;
    }
}
PlatformEventLoop *PlatformEventLoop::instance = new PlatformEventLoop();
PlatformEventLoop *PlatformEventLoop::getInstance() { return instance; }
PlatformEventLoop::PlatformEventLoop() {
    running = false;
    mDebugPortOn = false;
    mAutoDlEpo = true;
    mElpoDownloadDelayMs = 0;
}
bool PlatformEventLoop::init() {
    mMutex.lock();
    running = true;
    mMutex.unlock();
    LOG_D("-----event loop init done------");
    isAnldRunning = false;
    mIsEpoDownloading = false;
    mEpoUpdateInterval = kUpdateEpoInitialIntervalMs;
#ifndef PLATFORM_SUPPORT_ELPO
    setEpo3DayDownloadStatus(airoha::epo::EC_GPS,
                             EpoDownloadResult::US_LAST_FAIL);
    setEpo3DayDownloadStatus(airoha::epo::EC_GLONASS,
                             EpoDownloadResult::US_LAST_FAIL);
    setEpo3DayDownloadStatus(airoha::epo::EC_BEIDOU,
                             EpoDownloadResult::US_LAST_FAIL);
    setEpo3DayDownloadStatus(airoha::epo::EC_GALILEO,
                             EpoDownloadResult::US_LAST_FAIL);
#else
    setElpo3DayDownloadStatus(airoha::epo::EC_GPS,
                              EpoDownloadResult::US_LAST_UNKNOWN);
    setElpo3DayDownloadStatus(airoha::epo::EC_GLONASS,
                              EpoDownloadResult::US_LAST_UNKNOWN);
    setElpo3DayDownloadStatus(airoha::epo::EC_BEIDOU,
                              EpoDownloadResult::US_LAST_UNKNOWN);
    setElpo3DayDownloadStatus(airoha::epo::EC_GALILEO,
                              EpoDownloadResult::US_LAST_UNKNOWN);
    updateVendorProperty();
#endif
#ifdef TEST_FUNC_ENABLE
    test();
#endif
    // fill default epo config
    setEpoConfig("xxx", "xxx", "xxx");
    setEpoDataPath("/data/vendor/airoha");
    return true;
}
bool PlatformEventLoop::loop() {
    for (;;) {
        mMutex.lock();
        while (mMsgQueue.size() == 0) {
            LOG_D("wait for empty");
            mMsgCond.wait(mMutex);
        }
        PMessage *msg = mMsgQueue.front();
        mMsgQueue.pop_front();
        mMutex.unlock();
        // handle message
        LOG_D("platform event: evt(%d)", msg->evt);
        if (msg->evt == Event::EVENT_EXIT) {
            assert(mMsgQueue.size() == 0);
            delete (msg);
            break;
        }
        handleMessage(msg);
        // a new struct should be delete with delete
        delete (msg);
    }
    return true;
}
bool PlatformEventLoop::sendMessage(Event evt, const void *buffer,
                                    size_t length, bool acquireWakeLock) {
    mMutex.lock();
    if (evt == Event::EVENT_EXIT) {
        // make sure that event_exit is the last message
        running = false;
    }
    if (running || evt == Event::EVENT_EXIT) {
        PMessage *msg = new PMessage();
        msg->evt = evt;
        msg->buffer = nullptr;
        msg->length = 0;
        msg->setData(buffer, length);
        if (acquireWakeLock) {
            msg->wakeSource = std::make_shared<WakeSource>("PLEVT");
        }
        mMsgQueue.push_back(msg);
    }
    mMsgCond.notify_one();
    mMutex.unlock();
    return true;
}
bool PlatformEventLoop::sendDownloadEpoFileMsg(
    airoha::epo::EpoConstellation constellation,
    airoha::epo::EpoFileType type) {
    airoha::epo::EpoFileFilter filter;
    filter.constellation = constellation;
    filter.type = type;
    return sendMessage(PlatformEventLoop::EVENT_DOWNLOAD_EPO_FILE, &filter,
                       sizeof(filter), true);
}
bool PlatformEventLoop::sendNetworkConnectedMessage() {
    sendMessage(PlatformEventLoop::EVENT_SYSTEM_NETWORK_CONNECTED, nullptr, 0);
    return true;
}
void PlatformEventLoop::flushFilesIfNeed() {
#ifdef PLATFORM_SUPPORT_ELPO
    EpoDownloadResult *resArr = m3DayElpoUpdateResult;
#else
    EpoDownloadResult *resArr = m3DayEpoUpdateResult;
#endif
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        if (resArr[i].status == EpoDownloadResult::US_WAIT_FOR_COPY) {
            flushElpoFile(resArr[i].filter.constellation,
                          resArr[i].filter.type);
            resArr[i].status = EpoDownloadResult::US_LAST_SUCCESS;
        }
    }
    updateVendorProperty();
}
void PlatformEventLoop::checkIfNeedCopy() {
    bool inProcess = false;
    int needRetryNum = 0;
    (void)inProcess;
    (void)needRetryNum;
#ifdef PLATFORM_SUPPORT_ELPO
    EpoDownloadResult *resArr = m3DayElpoUpdateResult;
#else
    EpoDownloadResult *resArr = m3DayEpoUpdateResult;
#endif
    int waitForCopy = 0;
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        if (resArr[i].status == EpoDownloadResult::US_WAIT_FOR_COPY) {
            waitForCopy++;
        }
    }
    if (waitForCopy == 0) {
        if (mIsEpoFileLocked) {
            Airoha::anldSendMessage2Service(
                Airoha::AnldServiceMessage::GNSS_EPO_FILE_UNLOCK, nullptr, 0);
            mIsEpoFileLocked = false;
        }
    } else {
        if (!mIsEpoFileLocked) {
            Airoha::anldSendMessage2Service(
                Airoha::AnldServiceMessage::GNSS_EPO_FILE_LOCK, nullptr, 0);
        } else {
            flushFilesIfNeed();
            sendMessage(PlatformEventLoop::EVENT_UPDATE_EPO_CHECKLIST, nullptr,
                        0, true);
        }
    }
}
void PlatformEventLoop::flushElpoFile(
    airoha::epo::EpoConstellation constellation,
    airoha::epo::EpoFileType epoFileType) {
    size_t i = 0;
    bool found = false;
    for (i = 0; i < sizeof(gEpoFileList) / sizeof(IfEpoFile_t); i++) {
        if (gEpoFileList[i].constellation != constellation) {
            continue;
        }
        if (gEpoFileList[i].fileType != epoFileType) {
            continue;
        }
        found = true;
        break;
    }
    if (found) {
        char dst[kEpoMaxPathSize] = {0};
        char src[kEpoMaxPathSize] = {0};
        int pRet = snprintf(dst, sizeof(dst), "%s/%s", mEpoDataPath,
                            gEpoFileList[i].filename);
        if (pRet == -1 || pRet == sizeof(dst)) {
            LOG_E("combine dst path %s error.", gEpoFileList[i].filename);
            return;
        }
        pRet = snprintf(src, sizeof(src), "%s/%s", mEpoDataPath,
                        gEpoFileList[i].tmpFilename);
        if (pRet == -1 || pRet == sizeof(src)) {
            LOG_E("combine src path %s error.", gEpoFileList[i].tmpFilename);
            return;
        }
        int unlinkRet = unlink(dst);
        // if (unlinkRet == -1) {
        //     LOG_E("unlink %s error, %d, %s", gEpoFileList[i].filename, errno,
        //           strerror(errno));
        // }
        int renameRet = rename(src, dst);
        if (renameRet == -1) {
            LOG_E("rename %s->%s error, %d, %s", src, dst, errno,
                  strerror(errno));
        }
        LOG_I("flush: %s, unlink=%d, rename=%d", gEpoFileList[i].filename,
              unlinkRet, renameRet);
        TRACE_D("flush: %s, unlink=%d, rename=%d", gEpoFileList[i].filename,
              unlinkRet, renameRet);
    } else {
        LOG_E("Flush elpo file with unknown constellation and epo file type");
    }
}
void PlatformEventLoop::handleMessage(PMessage *msg) {
    TRACE_D("platform event message: %d", msg->evt);
    switch (msg->evt) {
        case EVENT_STOP_ANLD_APP: {
            anldCoreStop();
            break;
        }
        case EVENT_START_ANLD_APP: {
            anldCoreStart();
            break;
        }
        case EVENT_TEST: {
            LOG_D("---------------------");
            LOG_D("---EVENT_TEST---");
            LOG_D("_________________________-");
            break;
        }
        case EVENT_OTA_DOWNLOAD_FIRMWARE: {
            // write an empty file to trigger ota download
            anldCoreStop();
            FILE *f = fopen(USE_OTA_TO_DOWNLOAD_FILE, "wb+");
            if (f) {
                fclose(f);
            }
            portDownloadChip();
            unlink(USE_OTA_TO_DOWNLOAD_FILE);
            initValueRelatedToService();
            anldCoreStart();
            break;
        }
        case EVENT_DOWNLOAD_EPO_FILE: {
            handleEpoFileDownload(msg->buffer, msg->length);
            break;
        }
        case EVENT_UPDATE_EPO_CHECKLIST: {
            TRACE_D("EPO Update Request:");
            for (const auto u : mEpoUpdateList) {
                TRACE_D("---> %d,%d", (int)u.constellation, (int)u.type);
            }
            if (mEpoUpdateList.size() > 0) {
#if 0
                if (!mIsEpoFileLocked) {
                    // currently we lock all epo file.
                    Airoha::anldSendMessage2Service(
                        Airoha::AnldServiceMessage::GNSS_EPO_FILE_LOCK, nullptr,
                        0);
                } else {
#endif
                if (!mIsEpoDownloading) {
                    airoha::epo::EpoFileFilter filter = mEpoUpdateList.front();
                    sendDownloadEpoFileMsg(filter.constellation, filter.type);
                    mIsEpoDownloading = true;
                }
#if 0
            }
#endif
            } else {
#if 0
                Airoha::anldSendMessage2Service(
                    Airoha::AnldServiceMessage::GNSS_EPO_FILE_UNLOCK, nullptr,
                    0);
                mIsEpoFileLocked = false;
#endif
                checkIfNeedCopy();
            }
            break;
        }
        case EVENT_ANLD_EPO_FILE_EXPIRE_NOTIFY: {
            handleEpoFileExpireNotify(msg->buffer, msg->length);
            break;
        }
        case EVENT_ANLD_EPO_FILE_LOCK_NOTIFY: {
            mIsEpoFileLocked = true;
            sendMessage(EVENT_UPDATE_EPO_CHECKLIST, nullptr, 0);
            break;
        }
        case EVENT_ANLD_EPO_FILE_UNLOCK_NOTIFY: {
            mIsEpoFileLocked = false;
            break;
        }
        case EVENT_3DAYS_EPO_MONITOR_START: {
            if (!mAutoDlEpo) {
                LOG_I("No need START 3-day EPO monitor due to configuration.");
                break;
            }
            // Here we should use a suspend-ware timer.
            // If the timer expired when system suspend,
            // it will pending until system resume.
            mEpoDlTmr = SysTimer::createTimer(epoTimerCallback,
                                              mEpoUpdateInterval, this, true);
            mEpoDlTmr->start();
            break;
        }
        case EVENT_3DAYS_EPO_MONITOR_STOP: {
            if (!mAutoDlEpo) {
                LOG_I("No need STOP 3-day EPO monitor due to configuration.");
                break;
            }
            mEpoDlTmr->stop();
            SysTimer::removeTimer(mEpoDlTmr);
            break;
        }
        case EVENT_UPDATE_EPO_TIMEOUT: {
            handleEpoUpdateTimeout();
            break;
        }
        case EVENT_SYSTEM_NETWORK_MIGHT_CONNECTED: {
            uint32_t remainExpireMs = 0;
            bool ret = mEpoDlTmr->getNextExpireMs(&remainExpireMs);
            if (!ret) {
                LOG_E("get net expire ms error! %p", mEpoDlTmr);
                break;
            }
            TRACE_D("next epo download remain %u (ms)", remainExpireMs);
            // Here we multiply the kUpdateEpoWhenNetworkConnectedMs by 2 to
            // avoid resetting the timer frequently.
            if (remainExpireMs > kUpdateEpoWhenNetworkConnectedMs * 2 ||
                remainExpireMs == 0) {
                mEpoDlTmr->stop();
                mEpoDlTmr->setDuration(kUpdateEpoWhenNetworkConnectedMs);
                mEpoDlTmr->start();
            }
            break;
        }
        case EVENT_SYSTEM_TRIGGER_3_DAYS_EPO_DOWNLOAD: {
            console("Trigger 3-Day EPO Download...\n");
            uint32_t remainExpireMs = 0;
            bool ret = mEpoDlTmr->getNextExpireMs(&remainExpireMs);
            if (!ret) {
                LOG_E("get net expire ms error! %p", mEpoDlTmr);
                break;
            }
            TRACE_D("next epo download remain %u (ms)", remainExpireMs);
            // Here we multiply the kUpdateEpoWhenNetworkConnectedMs by 2 to
            // avoid resetting the timer frequently.
            if (remainExpireMs > kUpdateEpoWhenUserTrigger * 2 ||
                remainExpireMs == 0) {
                mEpoDlTmr->stop();
                mEpoDlTmr->setDuration(kUpdateEpoWhenUserTrigger);
                mEpoDlTmr->start();
            }
            console("Please wait at least %d second and restart\n",
                    kUpdateEpoWhenUserTrigger * 2);
            break;
        }
        case EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_ON: {
            mDebugPortOn = true;
            console("Debug Message ON\n");
            break;
        }
        case EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_OFF: {
            console("Debug Message OFF\n");
            mDebugPortOn = false;
            break;
        }
        case EVENT_SYSTEM_QUERY_EPO_FILE_STATUS: {
            dumpEpoFileState();
            dump3DaysEpoFileDownloadState();
            break;
        }
        case EVENT_SYSTEM_NETWORK_CONNECTED: {
            handleNetworkConnected();
            break;
        }
        default:
            break;
    };
    return;
}
void PlatformEventLoop::initValueRelatedToService() {
    mIsEpoFileLocked = false;
    return;
}
void PlatformEventLoop::anldCoreStart() {
    if (!isAnldRunning) {
        Airoha::anldServiceInit();
        isAnldRunning = true;
    }
}
void PlatformEventLoop::anldCoreStop() {
    if (isAnldRunning) {
        Airoha::anldServiceSendStopSignal();
        Airoha::anldRun();  // wait return
        Airoha::anldServiceDeinit();
        isAnldRunning = false;
    }
}
void PlatformEventLoop::setEpoConfig(const char *vendor, const char *project,
                                     const char *device) {
    strncpy(mEpoVendor, vendor, sizeof(mEpoVendor) - 1);
    strncpy(mEpoProject, project, sizeof(mEpoProject) - 1);
    strncpy(mEpoDevice, device, sizeof(mEpoDevice) - 1);
    return;
}
void PlatformEventLoop::setEpoVendor(const std::string &vendor) {
    if (vendor.size() == 0) {
        LOG_W("[EPO] Invalid vendor id");
        return;
    }
    strncpy(mEpoVendor, vendor.c_str(), sizeof(mEpoVendor) - 1);
}
void PlatformEventLoop::setEpoProject(const std::string &project) {
    if (project.size() == 0) {
        LOG_W("[EPO] Invalid project id");
        return;
    }
    strncpy(mEpoProject, project.c_str(), sizeof(mEpoProject) - 1);
}
void PlatformEventLoop::setEpoDevice(const std::string &device) {
    if (device.size() == 0) {
        LOG_W("[EPO] Invalid device id");
        return;
    }
    strncpy(mEpoDevice, device.c_str(), sizeof(mEpoDevice) - 1);
}
void PlatformEventLoop::setEpoDataPath(const char *epoDataPath) {
    strncpy(mEpoDataPath, epoDataPath, sizeof(mEpoDataPath) - 1);
    LOG_D("set epo data path: %s", mEpoDataPath);
    return;
}
void PlatformEventLoop::setEpoDataPath(const std::string &epoPath) {
    strncpy(mEpoDataPath, epoPath.c_str(), sizeof(mEpoDataPath) - 1);
    LOG_D("set epo data path: %s", mEpoDataPath);
    return;
}
void PlatformEventLoop::genRandomDevice(char *p, size_t n) {
    if (n == 0) {
        LOG_E("gen random device failed!");
        return;
    }
    for (size_t i = 0; i < n - 1; i++) {
        p[i] = rand() % 26 + 'A';
        if (rand() % 2) {
            p[i] |= (1 << 5);
        }
    }
    p[n - 1] = 0;
}
void PlatformEventLoop::test() {
    LOG_D("*************** PlatformEventLoop Test *******************");
    char random[20];
    genRandomDevice(random, 20);
    LOG_D("Test Random DEV1: %s", random);
    genRandomDevice(random, 20);
    LOG_D("Test Random DEV2: %s", random);
    genRandomDevice(random, 20);
    LOG_D("Test Random DEV3: %s", random);
}
std::string PlatformEventLoop::queryLastEpoUpdateResult() {
    std::string result;
#ifdef PLATFORM_SUPPORT_ELPO
    result += "=== Use ELPO === \r\n";
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        result += m3DayElpoUpdateResult[i].toString();
        result += "\r\n";
    }
#else
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        result += m3DayEpoUpdateResult[i].toString();
        result += "\r\n";
    }
#endif
    return result;
}
void PlatformEventLoop::handleEpoFileDownload(void *buffer, size_t len) {
    assert(len == sizeof(airoha::epo::EpoFileFilter));
    const airoha::epo::EpoFileFilter *p =
        (const airoha::epo::EpoFileFilter *)buffer;
    LOG_D("handle epo file download: %d, %d", p->constellation, p->type);
    bool found = false;
    size_t i = 0;
    for (i = 0; i < sizeof(gEpoFileList) / sizeof(IfEpoFile_t); i++) {
        if (gEpoFileList[i].constellation != p->constellation) {
            continue;
        }
        if (gEpoFileList[i].fileType != p->type) {
            continue;
        }
        found = true;
        break;
    }
    if (found) {
        LOG_D("handle epo file download cons(%d), type(%d), name(%s)",
              p->constellation, p->type, gEpoFileList[i].filename);
        PStatus ret = downloadEpoByFilename(gEpoFileList[i].filename,
                                            gEpoFileList[i].tmpFilename);
        // notify service if qepo is download
        if (ret == PStatus::PSTATUS_OK && p->type == airoha::epo::EFT_QEPO) {
            Airoha::anldSendMessage2Service(
                Airoha::AnldServiceMessage::GNSS_EPO_FILE_UPDATE_FINISH, p,
                sizeof(airoha::epo::EpoFileFilter));
            // if QEPO download success, the network may be connected.
            sendMessage(PlatformEventLoop::EVENT_SYSTEM_NETWORK_MIGHT_CONNECTED,
                        nullptr, 0);
        }
#ifdef PLATFORM_SUPPORT_ELPO
        if (ret == PStatus::PSTATUS_OK && p->type == airoha::epo::EFT_ELPO_3) {
            Airoha::anldSendMessage2Service(
                Airoha::AnldServiceMessage::GNSS_EPO_FILE_UPDATE_FINISH, p,
                sizeof(airoha::epo::EpoFileFilter));
        }
        if (p->type == airoha::epo::EFT_ELPO_3) {
            if (ret == PStatus::PSTATUS_OK) {
                setElpo3DayDownloadStatus(p->constellation,
                                          EpoDownloadResult::US_WAIT_FOR_COPY);
            } else if (ret == PStatus::PSTATUS_NETWORK_FAIL) {
                setElpo3DayDownloadStatus(p->constellation,
                                          EpoDownloadResult::US_LAST_FAIL);
            } else if (ret == PStatus::PSTATUS_DOWNLOAD_TOO_MUCH) {
                setElpo3DayDownloadStatus(
                    p->constellation,
                    EpoDownloadResult::US_LAST_FAIL_BUT_NO_RETRY);
            }
            checkEpo3DaysDownloadResult();
        }
#else
        if (p->type == airoha::epo::EFT_EPO) {
            setEpo3DayDownloadStatus(p->constellation,
                                     ret ? EpoDownloadResult::US_LAST_SUCCESS
                                         : EpoDownloadResult::US_LAST_FAIL);
            checkEpo3DaysDownloadResult();
        }
#endif
    } else {
        LOG_E("epo file not found in list %d, %d", p->constellation, p->type);
    }
    // Check if this request is in update list
    LOG_D("epo download from internet finish %d, %d, queue:%zu",
          p->constellation, p->type, mEpoUpdateList.size());
    if (mEpoUpdateList.size()) {
        airoha::epo::EpoFileFilter &f = mEpoUpdateList.front();
        if ((f.constellation == p->constellation) && (f.type == p->type)) {
            mEpoUpdateList.pop_front();
        }
    }
    // Send this message to async check list
    sendMessage(PlatformEventLoop::EVENT_UPDATE_EPO_CHECKLIST, nullptr, 0,
                true);
    mIsEpoDownloading = false;
}
bool PlatformEventLoop::checkAllowElpoDownload(const char *filename) const {
    bool allow = false;
    int64_t ms = getSystemRunningTimeMs();
    if (mEpoFileDownloadRecord.count(filename) == 0) {
        allow = true;
    } else {
        int64_t oldMs = mEpoFileDownloadRecord.at(filename);
        if (oldMs > ms) {
            allow = true;
        } else if (ms - oldMs > kDownloadEpoAllowInterval) {
            allow = true;
        } else {
            allow = false;
        }
    }
    return allow;
}
void PlatformEventLoop::recordElpoDownload(const char *filename) {
    int64_t ms = getSystemRunningTimeMs();
    mEpoFileDownloadRecord[filename] = ms;
}
PStatus PlatformEventLoop::downloadEpoByFilename(const char *filename,
                                                 const char *localFilename) {
    if (checkAllowElpoDownload(filename) == false) {
        LOG_E("Download EPO too freqently.");
        TRACE_D("Download EPO too freqently. [%s]", filename);
        return PStatus::PSTATUS_DOWNLOAD_TOO_MUCH;
    }
    char url[256] = {0};
    char localPath[kEpoMaxPathSize] = {0};
    char tmpEpoDevice[kEpoMaxConfigNameLength] = {0};
    if (strncmp(mEpoDevice, "_RANDOM_", sizeof(mEpoDevice)) == 0) {
        genRandomDevice(tmpEpoDevice, 20);
    } else {
        strncpy(tmpEpoDevice, mEpoDevice, sizeof(tmpEpoDevice));
    }
    size_t ret = 0;
    ret = snprintf(url, sizeof(url), PLATFORM_EPO_URL_PATTERN, filename,
                   mEpoVendor, mEpoProject, tmpEpoDevice);
    if (ret == sizeof(url)) {
        LOG_W("url maybe overflow");
    }
    ret = snprintf(localPath, sizeof(localPath), "%s/%s", mEpoDataPath,
                   localFilename);
    if (ret == sizeof(localPath)) {
        LOG_W("local path maybe overflow");
    }
    NetworkStatus status = Network::download(url, localPath);
    if (mElpoDownloadDelayMs > 0 && mElpoDownloadDelayMs <= 1000) {
        usleep(mElpoDownloadDelayMs * 1000);
    }
    TRACE_D("[EPO] download epo file [%s] ret = %d", filename, status);
    console("[EPO] download epo file [%s] ret = %d\n", filename, status);
    if (status != NET_STATUS_OK) {
        LOG_E("download epo file [%s] error %d", filename, status);
        return PStatus::PSTATUS_NETWORK_FAIL;
    }
    Airoha::anldReportFormatCriticalMessage("$EPO_SUCCESS_[%s]", filename);
    LOG_D("download epo file [%s] success", filename);
    chmod(localPath, EPO_FILE_PERMISSION);
#ifndef AIROHA_SIMULATION
    // 1021 = gps, 1000 = system
    chown(localPath, 1021, 1000);
#endif
    recordElpoDownload(filename);
    return PStatus::PSTATUS_OK;
}
void PlatformEventLoop::checkEpo3DaysDownloadResult() {
    bool inProcess = false;
    int needRetryNum = 0;
#ifdef PLATFORM_SUPPORT_ELPO
    EpoDownloadResult *resArr = m3DayElpoUpdateResult;
#else
    EpoDownloadResult *resArr = m3DayEpoUpdateResult;
#endif
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        if (resArr[i].status == EpoDownloadResult::US_LAST_PROCESSING) {
            // Wait for download done;
            inProcess = true;
        } else if (resArr[i].status == EpoDownloadResult::US_LAST_FAIL) {
            needRetryNum++;
        }
    }
    if (inProcess) {
        return;
    }
    if (needRetryNum == 0) {
        // Get random hour
        uint32_t randomSec =
            (uint32_t)((uint32_t)rand() % kEpoUpdateRandomScaleSecond);
        mEpoUpdateInterval = kEpoRegularUpdateIntervalMs + randomSec * 1000;
        mEpoDlTmr->setDuration(mEpoUpdateInterval);
        mEpoDlTmr->start();
        LOG_D("all epo pass, restart epo query at %" PRIu32,
              mEpoUpdateInterval);
    } else {
        uint32_t randomSec = (uint32_t)((uint32_t)rand() % 60);
        mEpoUpdateInterval = kEpoRetryUpdateIntervalMs + randomSec * 1000;
        mEpoDlTmr->setDuration(mEpoUpdateInterval);
        mEpoDlTmr->start();
        LOG_D("epo fail, restart epo query at %" PRIu32, mEpoUpdateInterval);
    }
    TRACE_D("next epo file will be update at %" PRIu32, mEpoUpdateInterval);
    updateVendorProperty();
    // If there is any download failed in system, we restart at 5 Second;
}
const char *PlatformEventLoop::getEpoDownloadStatusMarker(
    EpoDownloadResult::UpdateStatus status) {
    switch (status) {
        case EpoDownloadResult::US_LAST_UNKNOWN:
            return "U";
        case EpoDownloadResult::US_LAST_SUCCESS:
            return "O";
        case EpoDownloadResult::US_LAST_FAIL:
            return "X";
        case EpoDownloadResult::US_LAST_PROCESSING:
            return "P";
        case EpoDownloadResult::US_LAST_FAIL_BUT_NO_RETRY:
            return "B";
        case EpoDownloadResult::US_WAIT_FOR_COPY:
            return "C";
    }
    return "";
}
static void ctime_r_no_newline(const time_t *t, char *buf) {
    struct tm tm_result;
    localtime_r(t, &tm_result);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d %3s", tm_result.tm_year + 1900,
            tm_result.tm_mon + 1, tm_result.tm_mday, tm_result.tm_hour,
            tm_result.tm_min, tm_result.tm_sec, tm_result.tm_zone);
}
void PlatformEventLoop::updateVendorProperty() {
    const char *VENDOR_PROPERTY_NAME = "vendor.gnss.psds";
    char buffer[200] = {0};
    char ctimeGps[40] = {0};
    time_t t = airoha::native::nativeRealtime() / 1000;
    ctime_r_no_newline(&t, ctimeGps);
    std::string existString = "";
    for (const IfEpoFile_t &f : gEpoFileList) {
        char represent = 'N';
        char fullFile[256] = {0};
        snprintf(fullFile, sizeof(fullFile), "%s/%s",
                 Airoha::Configuration::INSTANCE->dataPath(), f.filename);
        LOG_D("Check: %s", fullFile);
        bool skipHeader = false;
        if (f.constellation == airoha::epo::EC_BEIDOU ||
            f.constellation == airoha::epo::EC_GALILEO) {
            skipHeader = true;
        }
        base::EpoReader::Result res =
            base::EpoReader::decodeEpoFile(fullFile, 18, skipHeader);
        if (res.valid) {
            represent = 'E';
        }
        if (res.valid && t >= res.start.secondSinceEpoch &&
            t <= res.end.secondSinceEpoch) {
            represent = 'V';
        }
        existString += represent;
    }
    snprintf(buffer, sizeof(buffer), "%s,%s,%s,%s,%s,%s",
             getEpoDownloadStatusMarker(
                 m3DayElpoUpdateResult[airoha::epo::EC_GPS].status),
             getEpoDownloadStatusMarker(
                 m3DayElpoUpdateResult[airoha::epo::EC_GLONASS].status),
             getEpoDownloadStatusMarker(
                 m3DayElpoUpdateResult[airoha::epo::EC_GALILEO].status),
             getEpoDownloadStatusMarker(
                 m3DayElpoUpdateResult[airoha::epo::EC_BEIDOU].status),
             existString.c_str(), ctimeGps);
#ifdef __ANDROID__
    LOG_D("Property [%s]:[%s]", VENDOR_PROPERTY_NAME, buffer);
    TRACE_D("Property [%s]:[%s]", VENDOR_PROPERTY_NAME, buffer);
    property_set(VENDOR_PROPERTY_NAME, buffer);
#endif
}
#ifndef PLATFORM_SUPPORT_ELPO
void PlatformEventLoop::setEpo3DayDownloadStatus(
    airoha::epo::EpoConstellation c, EpoDownloadResult::UpdateStatus ret) {
    m3DayEpoUpdateResult[c].filter.constellation = c;
    m3DayEpoUpdateResult[c].filter.type = airoha::epo::EFT_EPO;
    m3DayEpoUpdateResult[c].status = ret;
    if (ret == EpoDownloadResult::US_LAST_SUCCESS) {
        m3DayEpoUpdateResult[c].lastSuccessMs = getSystemRunningTimeMs();
    }
}
#else
void PlatformEventLoop::setElpo3DayDownloadStatus(
    airoha::epo::EpoConstellation c, EpoDownloadResult::UpdateStatus ret) {
    m3DayElpoUpdateResult[c].filter.constellation = c;
    m3DayElpoUpdateResult[c].filter.type = airoha::epo::EFT_ELPO_3;
    m3DayElpoUpdateResult[c].status = ret;
    if (ret == EpoDownloadResult::US_LAST_SUCCESS) {
        m3DayElpoUpdateResult[c].lastSuccessMs = getSystemRunningTimeMs();
    }
    m3DayElpoUpdateResult[c].lastDownloadSystemMs =
        airoha::native::nativeRealtime();
}
PlatformEventLoop::EpoDownloadResult::UpdateStatus
PlatformEventLoop::getElpo3DayDownloadStatus(
    airoha::epo::EpoConstellation constellation) {
    return m3DayElpoUpdateResult[constellation].status;
}
#endif
void PlatformEventLoop::epoTimerCallback(SysTimer *timer_ptr, void *vp) {
    (void)timer_ptr;
    LOG_D("platform epo update timer timeout");
    PlatformEventLoop *ins = (PlatformEventLoop *)vp;
    ins->sendMessage(PlatformEventLoop::EVENT_UPDATE_EPO_TIMEOUT, nullptr, 0);
}
int64_t PlatformEventLoop::getSystemRunningTimeMs() {
    int64_t timeMs;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    timeMs = (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
    return timeMs;
}
void PlatformEventLoop::handleEpoFileExpireNotify(void *data, size_t length) {
    assert(length == sizeof(airoha::epo::EpoFileFilter));
    airoha::epo::EpoFileFilter *filter = (airoha::epo::EpoFileFilter *)data;
    TRACE_D("handleEpoFileExpireNotify %d %d", (int)filter->constellation,
            (int)filter->type);
    // Check if file request exist
    bool exist = false;
    for (auto f : mEpoUpdateList) {
        if (f.constellation == filter->constellation &&
            f.type == filter->type) {
            LOG_W("epo request exist");
            exist = true;
            break;
        }
    }
#if 1
    EpoDownloadResult::UpdateStatus result =
        getElpo3DayDownloadStatus(filter->constellation);
    if (result == EpoDownloadResult::US_WAIT_FOR_COPY ||
        result == EpoDownloadResult::US_LAST_PROCESSING) {
        LOG_I("Elpo Download is in procedure(%d), Skip req (%d)", result,
              filter->constellation);
        return;
    }
#endif
    if (!exist) {
        mEpoUpdateList.push_back(*filter);
        sendMessage(PlatformEventLoop::EVENT_UPDATE_EPO_CHECKLIST, nullptr, 0);
        setElpo3DayDownloadStatus(
            (airoha::epo::EpoConstellation)filter->constellation,
            EpoDownloadResult::US_LAST_PROCESSING);
    }
}
void PlatformEventLoop::handleEpoFileExpireNotify(
    airoha::epo::EpoConstellation c, airoha::epo::EpoFileType t) {
    airoha::epo::EpoFileFilter filter;
    filter.constellation = c;
    filter.type = t;
    handleEpoFileExpireNotify(&filter, sizeof(filter));
}
#ifdef UPDATE_ELPO_EVERY_HALF_OF_DAY
#pragma message("Update ELPO every half of day.")
#endif
void PlatformEventLoop::handleEpoUpdateTimeout() {
    int64_t runningMs = getSystemRunningTimeMs();
    int expireNum = 0;
    TRACE_D("EPO Timeout");
#ifndef PLATFORM_SUPPORT_ELPO
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        if (m3DayEpoUpdateResult[i].status == EpoDownloadResult::US_LAST_FAIL) {
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_EPO);
            expireNum++;
        } else if ((m3DayEpoUpdateResult[i].status ==
                    EpoDownloadResult::US_LAST_SUCCESS) &&
                   (runningMs - m3DayEpoUpdateResult[i].lastSuccessMs >
                        kSingleDayMs ||
                    m3DayEpoUpdateResult[i].lastSuccessMs == -1)) {
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_EPO);
            expireNum++;
        } else if (!checkEpoFileExist(
                       m3DayEpoUpdateResult[i].filter.constellation,
                       m3DayEpoUpdateResult[i].filter.type)) {
            LOG_W("EPO file is delete, %d, %d, redownload it",
                  m3DayEpoUpdateResult[i].filter.constellation,
                  m3DayEpoUpdateResult[i].filter.type);
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_EPO);
            expireNum++;
        } else {
            LOG_I("Nothing to do");
        }
    }
#else
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        TRACE_D("timeout:elpo_sta %zu=%d", i,
                (int)m3DayElpoUpdateResult[i].status);
        if (m3DayElpoUpdateResult[i].status ==
                EpoDownloadResult::US_LAST_FAIL ||
            m3DayElpoUpdateResult[i].status ==
                EpoDownloadResult::US_LAST_UNKNOWN) {
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_ELPO_3);
            expireNum++;
        } else if ((m3DayElpoUpdateResult[i].status ==
                        EpoDownloadResult::US_LAST_SUCCESS ||
                    m3DayElpoUpdateResult[i].status ==
                        EpoDownloadResult::US_LAST_FAIL_BUT_NO_RETRY) &&
                   (runningMs - m3DayElpoUpdateResult[i].lastSuccessMs >=
#ifdef UPDATE_ELPO_EVERY_HALF_OF_DAY
                        (kSingleDayMs / 2)
#else
                        (kSingleDayMs)
#endif
                    || m3DayElpoUpdateResult[i].lastSuccessMs == -1)) {
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_ELPO_3);
            expireNum++;
        } else if (m3DayElpoUpdateResult[i].status ==
                       EpoDownloadResult::US_LAST_PROCESSING ||
                   m3DayElpoUpdateResult[i].status ==
                       EpoDownloadResult::US_WAIT_FOR_COPY) {
            // Do nothing
        } else if (!checkEpoFileExist(
                       m3DayElpoUpdateResult[i].filter.constellation,
                       m3DayElpoUpdateResult[i].filter.type)) {
            LOG_W("EPO file is delete, %d, %d, redownload it",
                  m3DayElpoUpdateResult[i].filter.constellation,
                  m3DayElpoUpdateResult[i].filter.type);
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_ELPO_3);
            expireNum++;
        } else {
            LOG_I("Nothing to do");
        }
    }
#endif
    if (expireNum == 0) {
        LOG_D("no epo expire, should reset timer");
        uint32_t randomSec =
            (uint32_t)((uint32_t)rand() % kEpoUpdateRandomScaleSecond);
        mEpoUpdateInterval = kEpoRegularUpdateIntervalMs + randomSec * 1000;
        mEpoDlTmr->setDuration(mEpoUpdateInterval);
        mEpoDlTmr->start();
        LOG_D("restart epo query at %" PRIu32, mEpoUpdateInterval);
    }
}
void PlatformEventLoop::handleNetworkConnected() {
    if (!mAutoDlEpo) {
        LOG_D("Not care about network connected.");
        return;
    }
    int expireNum = 0;
#ifdef PLATFORM_SUPPORT_ELPO
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        TRACE_D("network:elpo_sta %d=%d", i,
                (int)m3DayElpoUpdateResult[i].status);
        if (m3DayElpoUpdateResult[i].status ==
            EpoDownloadResult::US_LAST_FAIL) {
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_ELPO_3);
            expireNum++;
        } else if (m3DayElpoUpdateResult[i].status ==
                       EpoDownloadResult::US_WAIT_FOR_COPY ||
                   m3DayElpoUpdateResult[i].status ==
                       EpoDownloadResult::US_LAST_PROCESSING) {
            // do nothing
        } else if (!checkEpoFileExist(
                       m3DayElpoUpdateResult[i].filter.constellation,
                       m3DayElpoUpdateResult[i].filter.type)) {
            LOG_W("EPO file is delete, %d, %d, redownload it",
                  m3DayElpoUpdateResult[i].filter.constellation,
                  m3DayElpoUpdateResult[i].filter.type);
            handleEpoFileExpireNotify((airoha::epo::EpoConstellation)i,
                                      airoha::epo::EFT_ELPO_3);
            expireNum++;
        } else {
            LOG_I("Nothing to do");
        }
    }
#endif
    if (expireNum > 0) {
        mEpoDlTmr->stop();
    }
}
void PlatformEventLoop::setPlatformInstance(DefaultPlatform *i) {
    mPlatformInstance = i;
}
void PlatformEventLoop::console(const char *format, ...) {
    if (!mDebugPortOn) {
        return;
    }
    char output[1600];
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(output, sizeof(output), format, ap);
    va_end(ap);
    if (ret > 0) {
        mPlatformInstance->sendDebugPortMessage(output, ret);
    }
}
void PlatformEventLoop::dumpEpoFileState() {
    LOG_D("dump Epo File State >>>>");
    console("Dump EPO File:\n");
    for (const IfEpoFile_t &f : gEpoFileList) {
        char fullFile[256] = {0};
        snprintf(fullFile, sizeof(fullFile), "%s/%s",
                 Airoha::Configuration::INSTANCE->dataPath(), f.filename);
        LOG_D("Check: %s", fullFile);
        bool skipHeader = false;
        if (f.constellation == airoha::epo::EC_BEIDOU ||
            f.constellation == airoha::epo::EC_GALILEO) {
            skipHeader = true;
        }
        base::EpoReader::Result res =
            base::EpoReader::decodeEpoFile(fullFile, 18, skipHeader);
        console("Filename: %s\n", fullFile);
        LOG_D("\t is_valid: %d", res.valid);
        console("\t is_valid: %d\n", res.valid);
        LOG_D("\t start: %d-%d-%d %d:%d:%d", res.start.year, res.start.month,
              res.start.day, res.start.hour, res.start.minute,
              res.start.second);
        console("\t start: %d-%d-%d %d:%d:%d\n", res.start.year,
                res.start.month, res.start.day, res.start.hour,
                res.start.minute, res.start.second);
        LOG_D("\t end: %d-%d-%d %d:%d:%d", res.end.year, res.end.month,
              res.end.day, res.end.hour, res.end.minute, res.end.second);
        console("\t end: %d-%d-%d %d:%d:%d\n", res.end.year, res.end.month,
                res.end.day, res.end.hour, res.end.minute, res.end.second);
        LOG_D("===============");
    }
}
void PlatformEventLoop::dump3DaysEpoFileDownloadState() {
    console("Dump 3-Day EPO Download State");
#ifdef PLATFORM_SUPPORT_ELPO
    EpoDownloadResult *arr = m3DayElpoUpdateResult;
    console("> ELPO In Use");
#else
    EpoDownloadResult *arr = m3DayEpoUpdateResult;
#endif
    uint32_t nextTrigger = 0;
    if (!mEpoDlTmr->getNextExpireMs(&nextTrigger)) {
        nextTrigger = 0;
    }
    console("> Next Trigger(ms): %" PRId32 "\n", nextTrigger);
    for (size_t i = 0; i < kMaxSupportEpoSystemNum; i++) {
        console("--- Constellation: %d\n", arr[i].filter.constellation);
        if (arr[i].status == EpoDownloadResult::US_LAST_PROCESSING) {
            console("--- \t Last Result: US_LAST_PROCESSING\n");
        } else if (arr[i].status == EpoDownloadResult::US_LAST_FAIL) {
            console("--- \tLast Result: US_LAST_FAIL\n");
        } else if (arr[i].status == EpoDownloadResult::US_LAST_SUCCESS) {
            console("--- \tLast Result: US_LAST_SUCCESS\n");
        } else if (arr[i].status == EpoDownloadResult::US_LAST_UNKNOWN) {
            console("--- \tLast Result: US_LAST_UNKNOWN\n");
        }
    }
}
bool PlatformEventLoop::checkEpoFileExist(
    airoha::epo::EpoConstellation constellation,
    airoha::epo::EpoFileType fileType) {
    bool found = false;
    size_t i = 0;
    for (i = 0; i < sizeof(gEpoFileList) / sizeof(IfEpoFile_t); i++) {
        if (gEpoFileList[i].constellation != constellation) {
            continue;
        }
        if (gEpoFileList[i].fileType != fileType) {
            continue;
        }
        found = true;
        break;
    }
    if (!found) {
        return false;
    }
    char fullname[256] = {0};
    snprintf(fullname, sizeof(fullname), "%s/%s", mEpoDataPath,
             gEpoFileList[i].filename);
    FILE *f = fopen(fullname, "rb");
    if (!f) {
        return false;
    }
    fclose(f);
    return true;
}
void PlatformEventLoop::setAutoDownloadEpo(bool value) { mAutoDlEpo = value; }
void PlatformEventLoop::setElpoDownloadDelay(int value) {
    mElpoDownloadDelayMs = value;
}
void PlatformEventLoop::clearElpoDownloadCache() {
    mEpoFileDownloadRecord.clear();
}

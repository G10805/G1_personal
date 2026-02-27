#pragma once
#include <stdint.h>
#include <unistd.h>
#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include "airo_gps.h"
#include "system_timer.h"
#include "wakelock/air_wakelock.h"
using airoha::WakeSource;
// #define UPDATE_ELPO_EVERY_HALF_OF_DAY
#define PLATFORM_SUPPORT_ELPO
#ifdef PLATFORM_SUPPORT_ELPO
#define PLATFORM_EPO_URL_PATTERN \
    "https://elpo.airoha.com/%s?vendor=%s&project=%s&device_id=%s"
#else
#define PLATFORM_EPO_URL_PATTERN \
    "wpepodownload.mediatek.com/%s?vendor=%s&project=%s&device_id=%s"
#endif
typedef struct IfEpoFile {
    airoha::epo::EpoConstellation constellation;
    airoha::epo::EpoFileType fileType;
    const char *filename;
    const char *tmpFilename;
} IfEpoFile_t;
enum class PStatus {
    PSTATUS_OK = 0,
    PSTATUS_NETWORK_FAIL = 100,
    PSTATUS_DOWNLOAD_TOO_MUCH = 101,
};
class DefaultPlatform;
class PlatformEventLoop : public airoha::SysTimer {
 public:
    enum Event {
        EVENT_TEST = 0,
        EVENT_START_ANLD_APP = 1,
        EVENT_STOP_ANLD_APP = 2,
        EVENT_OTA_DOWNLOAD_FIRMWARE = 3,
        EVENT_DOWNLOAD_EPO_FILE = 4,
        EVENT_UPDATE_EPO_TIMEOUT = 5,
        EVENT_ANLD_EPO_REQUEST = 6,
        EVENT_EXIT = 7,
        EVENT_ANLD_EPO_FILE_LOCK_NOTIFY = 8,
        EVENT_ANLD_EPO_FILE_UNLOCK_NOTIFY = 9,
        EVENT_ANLD_EPO_FILE_EXPIRE_NOTIFY = 10,
        EVENT_UPDATE_EPO_CHECKLIST = 11,
        EVENT_3DAYS_EPO_MONITOR_START = 12,
        EVENT_3DAYS_EPO_MONITOR_STOP = 13,
        EVENT_SYSTEM_NETWORK_MIGHT_CONNECTED = 14,
        EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_ON = 15,
        EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_OFF = 16,
        EVENT_SYSTEM_TRIGGER_3_DAYS_EPO_DOWNLOAD = 17,
        EVENT_SYSTEM_QUERY_EPO_FILE_STATUS = 18,
        EVENT_SYSTEM_NETWORK_CONNECTED = 19,
        EVENT_UPDATE_EPO_DO_COPY = 20,
        EVENT_INVALID = 0xFFFF,
    };
    struct PMessage {
        Event evt;
        void *buffer;
        size_t length;
        std::shared_ptr<WakeSource> wakeSource;
        PMessage();
        ~PMessage();
        void setData(const void *buffer, size_t length);
    };
    struct EpoDownloadResult {
        EpoDownloadResult();
        airoha::epo::EpoFileFilter filter;
        enum UpdateStatus {
            US_LAST_UNKNOWN,
            US_LAST_FAIL,
            US_LAST_SUCCESS,
            US_LAST_PROCESSING,
            US_LAST_FAIL_BUT_NO_RETRY,
            US_WAIT_FOR_COPY,
        };
        UpdateStatus status;
        int64_t lastSuccessMs;
        int64_t lastDownloadSystemMs;
        std::string toString();
    };
 public:
    static PlatformEventLoop *getInstance();
    void setPlatformInstance(DefaultPlatform *);
    bool init();
    bool loop();
    bool sendMessage(Event, const void *buffer, size_t length,
                     bool acquireWakeLock = false);
    bool sendDownloadEpoFileMsg(airoha::epo::EpoConstellation,
                                airoha::epo::EpoFileType);
    bool sendNetworkConnectedMessage();
    void setEpoConfig(const char *vendor, const char *project,
                      const char *device);
    void setEpoVendor(const std::string &vendor);
    void setEpoProject(const std::string &project);
    void setEpoDevice(const std::string &device);
    void setEpoDataPath(const char *epoDataPath);
    void setEpoDataPath(const std::string &epoPath);
    void setAutoDownloadEpo(bool value);
    void setElpoDownloadDelay(int value);
    void clearElpoDownloadCache();
    void test();
    std::string queryLastEpoUpdateResult();

 protected:
    PlatformEventLoop();
    PlatformEventLoop(PlatformEventLoop &) = delete;
 private:
    const static size_t kEpoMaxPathSize = 256;
    const static size_t kEpoMaxConfigNameLength = 128;
    const static size_t kEpoUpdateBaseDurationMs =
        (1000 * 60 * 60 * 24);  // 1 day
    void handleMessage(PMessage *msg);
    void anldCoreStart();
    void anldCoreStop();
    void genRandomDevice(char *p, size_t n);
    // event handler
    void handleEpoFileDownload(void *, size_t buffer);
    void handleEpoFileExpireNotify(void *data, size_t length);
    void handleEpoFileExpireNotify(airoha::epo::EpoConstellation,
                                   airoha::epo::EpoFileType);
    void handleEpoUpdateTimeout();
    void handleNetworkConnected();
    void checkIfNeedCopy();
    void flushFilesIfNeed();
    void flushElpoFile(airoha::epo::EpoConstellation constellation,
                       airoha::epo::EpoFileType epoFileType);
    bool checkEpoFileExist(airoha::epo::EpoConstellation,
                           airoha::epo::EpoFileType);
    PStatus downloadEpoByFilename(const char *filename,
                                  const char *localFilename);
    void checkEpo3DaysDownloadResult();
    void updateVendorProperty();
    const char *getEpoDownloadStatusMarker(
        EpoDownloadResult::UpdateStatus status);
#ifndef PLATFORM_SUPPORT_ELPO
    void setEpo3DayDownloadStatus(airoha::epo::EpoConstellation,
                                  EpoDownloadResult::UpdateStatus ret);
#else
    void setElpo3DayDownloadStatus(airoha::epo::EpoConstellation,
                                   EpoDownloadResult::UpdateStatus ret);
    EpoDownloadResult::UpdateStatus getElpo3DayDownloadStatus(airoha::epo::EpoConstellation constellation);
#endif
    void dumpEpoFileState();
    void dump3DaysEpoFileDownloadState();
    void console(const char *format, ...) __attribute__((format(printf, 2, 3)));
    bool checkAllowElpoDownload(const char *filename) const;
    void recordElpoDownload(const char *filename);
    static void epoTimerCallback(SysTimer *timer_ptr, void *vp);
    static int64_t getSystemRunningTimeMs();
    /**
     * If you call anld stop, please call this function before anldStart.
     * Because anld service is total restart.
     *
     */
    void initValueRelatedToService();
    static PlatformEventLoop *instance;
    std::condition_variable_any mMsgCond;
    std::mutex mMutex;
    std::list<PMessage *> mMsgQueue;
    bool running;
    bool isAnldRunning;
    char mEpoVendor[kEpoMaxConfigNameLength];
    char mEpoProject[kEpoMaxConfigNameLength];
    char mEpoDevice[kEpoMaxConfigNameLength];
    char mEpoDataPath[kEpoMaxPathSize];
    std::list<airoha::epo::EpoFileFilter> mEpoUpdateList;
    bool mIsEpoFileLocked;
    bool mIsEpoDownloading;
    airoha::SysTimer *mEpoDlTmr;
// #define TEST_EPO_TIMER_ENABLE
#ifdef TEST_EPO_TIMER_ENABLE
    static const int kMaxSupportEpoSystemNum = 4;
    static const uint32_t kUpdateEpoInitialIntervalMs = 60000;
    static const uint32_t kEpoRegularUpdateIntervalMs = 60000;  // 2min
    static const uint32_t kEpoRetryUpdateIntervalMs = (30000);
    static const uint32_t kSingleDayMs = (60000);
    static const uint32_t kEpoUpdateRandomScaleSecond = (120);  // 120s
    static const uint32_t kUpdateEpoWhenNetworkConnectedMs = (30 * 1000);
    static const uint32_t kUpdateEpoWhenUserTrigger = (3 * 1000);
    static const int64_t kDownloadEpoAllowInterval = (60 * 1000);
#else
    static const int kMaxSupportEpoSystemNum = 4;
    static const uint32_t kUpdateEpoInitialIntervalMs = 60000;
#ifdef UPDATE_ELPO_EVERY_HALF_OF_DAY
    static const uint32_t kEpoRegularUpdateIntervalMs = (43200000);
#else
    static const uint32_t kEpoRegularUpdateIntervalMs = (86400000);
#endif
    static const uint32_t kEpoRetryUpdateIntervalMs = (60 * 20 * 1000);
    static const uint32_t kSingleDayMs = (86400000);
    static const uint32_t kEpoUpdateRandomScaleSecond = (60 * 60 * 5);
    static const uint32_t kUpdateEpoWhenNetworkConnectedMs = (30 * 1000);
    static const uint32_t kUpdateEpoWhenUserTrigger = (3 * 1000);
#ifdef UPDATE_ELPO_EVERY_HALF_OF_DAY
    static const int64_t kDownloadEpoAllowInterval = (6 * 3600 * 1000);
#else
    static const int64_t kDownloadEpoAllowInterval = (12 * 3600 * 1000);
#endif
#endif
#ifndef PLATFORM_SUPPORT_ELPO
    EpoDownloadResult m3DayEpoUpdateResult[kMaxSupportEpoSystemNum];
#else
    EpoDownloadResult m3DayElpoUpdateResult[kMaxSupportEpoSystemNum];
#endif
    uint32_t mEpoUpdateInterval;
    bool mDebugPortOn;
    DefaultPlatform *mPlatformInstance;
    bool mAutoDlEpo;
    std::map<std::string, int64_t> mEpoFileDownloadRecord;
    int mElpoDownloadDelayMs = 0;
};

#include <assert.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include "anld_service_interface.h"
#include "default_platform.h"
#include "platform_virtual_driver.h"
#include "simulation.h"
PlatformComStatus IComVirtualDriver::onExtraCommand(ExtraCommand command) {
    (void)command;
    return PlatformComStatus::PSC_OK;
}
SpiVirtualDriver::SpiVirtualDriver(const char *devName,
                                   DefaultPlatform *platform)
    : SpiDriver(devName) {
    mParent = platform;
}
PlatformComStatus SpiVirtualDriver::connect() {
    if (SpiDriver::open()) {
        return PlatformComStatus::PSC_OK;
    }
    return PlatformComStatus::PSC_IO_ERROR;
}
PlatformComStatus SpiVirtualDriver::disconnect() {
    if (SpiDriver::close()) {
        return PlatformComStatus::PSC_OK;
    }
    return PlatformComStatus::PSC_IO_ERROR;
}
PlatformComStatus SpiVirtualDriver::sendData(const void *buf, size_t length) {
    if (length == 0) {
        return PlatformComStatus::PSC_OK;
    }
    SpiDriverStatus status = SpiDriver::asyncWrite(buf, length);
    if (status == SpiDriverStatus::SPI_STATUS_OK) {
        return PlatformComStatus::PSC_OK;
    }
    return PlatformComStatus::PSC_IO_ERROR;
}
PlatformComStatus SpiVirtualDriver::powerSuspend() {
    LOG_D("Visteon: SpiVirtualDriver::powerSuspend");
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus SpiVirtualDriver::powerResume() {
    LOG_D("Visteon: SpiVirtualDriver::powerResume");
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus SpiVirtualDriver::consecutiveDataOpen() {
    LOG_D("Visteon: SpiVirtualDriver::consecutiveDataOpen");
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus SpiVirtualDriver::consecutiveDataClose() {
    LOG_D("Visteon: SpiVirtualDriver::consecutiveDataClose");
    return PlatformComStatus::PSC_OK;

}
PlatformComStatus SpiVirtualDriver::interrupt() {
    triggerReadOnce();
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus SpiVirtualDriver::onExtraCommand(ExtraCommand command) {
    if (command == ExtraCommand::EXTRA_COMMAND_PORT_POWER_OFF) {
        notifySlavePowerState(false);
    } else if (command == ExtraCommand::EXTRA_COMMAND_PORT_POWER_ON) {
        notifySlavePowerState(true);
    }
    return PlatformComStatus::PSC_OK;
}
void SpiVirtualDriver::onDataIn(const uint8_t *data, spi_size_t length) {
    if (length == 0) {
        return;
    }
    mParent->sendPowerGPSMessage(data, length);
    recordLog(data, length);
    Airoha::anldSendMessage2Service(Airoha::AnldServiceMessage::GNSS_DATA_INPUT,
                                    data, length);
}
void SpiVirtualDriver::triggerReadOnce() {
    // SpiDriver::asyncRead();
    SpiDriver::triggerRead(5, 10);
    return;
}
// Function for UartComVirtualDriver
UartVirtualDriver::UartVirtualDriver(DefaultPlatform *parent,
                                     const char *devName, int baudrate,
                                     FlowControl fc)
    : UartDriver(devName, baudrate, fc) {
        LOG_D("Visteon: UartVirtualDriver::UartVirtualDriver, devName: %s, baudrate: %d",
             devName, baudrate);
    mIsUartLock = false;
    mIsSwFlowControl = false;
    mEscapeCharMark = false;
    mParent = parent;
    mLockFlags = 0;
    pthread_mutex_init(&mMutex, nullptr);
    if (fc == FlowControl::FC_SOFTWARE) {
        mIsSwFlowControl = true;
    }
#ifdef UART_SANITY_CHECK
    mSanityLocker = false;
#endif
    mIsQcPlatform = false;
}
PlatformComStatus UartVirtualDriver::connect() {
    LOG_D("Visteon: UartVirtualDriver::connect");
    pthread_mutex_lock(&mMutex);
    LOG_D("Visteon: UartVirtualDriver open ::connect, mLockFlags: %d", mLockFlags);
    UartDriver::open();
    LOG_D("Visteon: UartVirtualDriver done");
    handleUartLock();
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::disconnect() {
    LOG_D("UartVirtualDriver::disconnect");
    pthread_mutex_lock(&mMutex);
    // unlock uart before close uart
    unlockUart();
    UartDriver::close();
    LOG_D("Visteon: UartVirtualDriver closing done");
#ifdef UART_SANITY_CHECK
    mSanityLocker = false;
#endif
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::sendData(const void *buf, size_t length) {
#ifdef UART_SANITY_CHECK
    if (!mSanityLocker) {
        LOG_E("====Sanity Check Failed!!! LINE: %d====", __LINE__);
        LOG_E("Reason: uart must be LOCK when data out.");
        assert(0);
    }
#endif
    std::string toSend;
    if (mIsSwFlowControl) {
        toSend = uartSwFcTxConvert(buf, length);
        // toSend = std::string((char *)buf, length);
        UartDriver::sendData(reinterpret_cast<const uint8_t *>(toSend.data()),
                             toSend.size());
    } else {
        UartDriver::sendData(static_cast<const uint8_t *>(buf), length);
    }
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::powerSuspend() {
    LOG_D("UartVirtualDriver::powerSuspend");
    pthread_mutex_lock(&mMutex);
    mLockFlags &= (~UartLockFlag::ULF_POWER_RESUME_LOCK);
    handleUartLock();
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::powerResume() {
    LOG_D("UartVirtualDriver::powerResume");
    pthread_mutex_lock(&mMutex);
    mLockFlags |= UartLockFlag::ULF_POWER_RESUME_LOCK;
    handleUartLock();
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::consecutiveDataOpen() {
    LOG_D("UartVirtualDriver::consecutiveDataOpen");
    pthread_mutex_lock(&mMutex);
    mLockFlags |= UartLockFlag::ULF_CONSECUTIVE_DATA_LOCK;
    handleUartLock();
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::consecutiveDataClose() {
        LOG_D("UartVirtualDriver::consecutiveDataClose");
    pthread_mutex_lock(&mMutex);
    mLockFlags &= (~UartLockFlag::ULF_CONSECUTIVE_DATA_LOCK);
    handleUartLock();
    pthread_mutex_unlock(&mMutex);
    return PlatformComStatus::PSC_OK;
}
PlatformComStatus UartVirtualDriver::interrupt() {
    return PlatformComStatus::PSC_OK;
}
void UartVirtualDriver::handleUartLock() {
    LOG_D("Visteon: handle uart lock: %d, %d", mLockFlags, mIsUartLock);
    if (mLockFlags) {
        lockUart();
    } else {
        unlockUart();
    }
}
void UartVirtualDriver::lockUart() {
    // lock uart
    if (!mIsUartLock && getUartFd() > 0) {
        mIsUartLock = true;
#ifdef UART_SANITY_CHECK
        mSanityLocker = true;
#endif
        if (mIsQcPlatform) {
            ioctl(getUartFd(), 0x544D, 0);
        }
    }
    return;
}
void UartVirtualDriver::unlockUart() {
    // unlock uart
    if (mIsUartLock && getUartFd() > 0) {
        // platform unlock uart
        mIsUartLock = false;
#ifdef UART_SANITY_CHECK
        mSanityLocker = false;
#endif
        if (mIsQcPlatform) {
            ioctl(getUartFd(), 0x544E, 0);
        }
    }
    return;
}
void UartVirtualDriver::onDataIn(const void *buffer, uart_size_t length) {
#ifdef UART_SANITY_CHECK
    if (!mSanityLocker) {
        LOG_E("====Sanity Check Failed!!! LINE: %d====", __LINE__);
        LOG_E("Reason: uart must be LOCK when data in.");
        assert(0);
    }
#endif
    if (length == 0) {
        return;
    }
    std::string raw((char *)buffer, length);
    if (mIsSwFlowControl) {
        uartSwFcRxConvert(raw);
    }
    mParent->sendPowerGPSMessage(raw.data(), raw.size());
    recordLog(raw.data(), raw.size());
    Airoha::anldSendMessage2Service(Airoha::AnldServiceMessage::GNSS_DATA_INPUT,
                                    raw.data(), raw.size());
}
std::string UartVirtualDriver::uartSwFcTxConvert(const void *data,
                                                 size_t length) {
    assert(length != 0);
    size_t total_length = length;
    const char *p = (char *)data;
    size_t i = 0;
    size_t j = 0;
    for (i = 0; i < length; i++) {
        if (p[i] == 0x77 || p[i] == 0x13 || p[i] == 0x11) {
            total_length++;
        }
    }
    char *res = (char *)malloc(total_length);
    j = 0;
    for (i = 0; i < length; i++) {
        if (p[i] == 0x77) {
            res[j] = 0x77;
            res[j + 1] = 0x88;
            j += 2;
        } else if (p[i] == 0x13) {
            res[j] = 0x77;
            res[j + 1] = 0xEC;
            j += 2;
        } else if (p[i] == 0x11) {
            res[j] = 0x77;
            res[j + 1] = 0xEE;
            j += 2;
        } else {
            res[j] = p[i];
            j++;
        }
    }
    assert(j == total_length);  // check if any invalid case exist
    std::string r(res, total_length);
    free(res);
    return r;
}
void UartVirtualDriver::uartSwFcRxConvert(std::string &arr) {
    size_t j = 0;
    // char *p = (char *)malloc(arr.size());
    // memcpy(p,)
    for (size_t i = 0; i < arr.size(); i++) {
        if (arr[i] == 0x77) {
            mEscapeCharMark = true;
            continue;
        }
        if (mEscapeCharMark) {
            if (arr[i] == '\x88') {
                arr[j] = '\x77';
            } else if (arr[i] == '\xee') {
                arr[j] = '\x11';
            } else if (arr[i] == '\xec') {
                arr[j] = '\x13';
            } else {
                LOG_E("unrecognized char 0x%" PRIx8, (uint8_t)arr[i]);
            }
            mEscapeCharMark = false;
        } else {
            arr[j] = arr[i];
        }
        j++;
    }
    arr = arr.substr(0, j);
}
void UartVirtualDriver::setQcPlatformEnable(bool enable) {
    mIsQcPlatform = enable;
}
// Function for IComVirtualDriver
IComVirtualDriver::IComVirtualDriver() {
    relayer = nullptr;
    pthread_mutex_init(&relayMutex, nullptr);
}
void IComVirtualDriver::logRecordOpen(const char *userLogPath,
                                      const char *userPrefix, size_t maxFileNum,
                                      size_t maxSingleFileSize) {
    pthread_mutex_lock(&relayMutex);
    relayer =
        new LogRelay(userLogPath, userPrefix, maxFileNum, maxSingleFileSize);
    pthread_mutex_unlock(&relayMutex);
    // Record Log Open.
    logMessage("START");
}
#define MAX_MESSAGE_LEN (128)
#define NMEA_CONTROL_BYTE_NUM (7)
void IComVirtualDriver::logMessage(const char *message) {
    struct tm localTime;
    time_t timeUTC = time(NULL);
    gmtime_r(&timeUTC, &localTime);
    char msgFormat[] = "PAIRANL,%d,%d,%d,%d,%d,%d,%s,%s";
    char msg[MAX_MESSAGE_LEN - NMEA_CONTROL_BYTE_NUM] = {0};
    char fullData[MAX_MESSAGE_LEN] = {0};
    snprintf(msg, sizeof(msg), msgFormat, localTime.tm_year + 1900,
             localTime.tm_mon + 1, localTime.tm_mday, localTime.tm_hour,
             localTime.tm_min, localTime.tm_sec, localTime.tm_zone, message);
    PAIRData::getFullPairCommand(msg, strlen(msg), fullData, sizeof(fullData));
    recordLog(fullData, strlen(fullData));
}
void IComVirtualDriver::logRecordClose() {
    logMessage("END");
    pthread_mutex_lock(&relayMutex);
    if (relayer) {
        delete (relayer);
        relayer = nullptr;
    }
    pthread_mutex_unlock(&relayMutex);
}
void IComVirtualDriver::recordLog(const void *buffer, size_t length) {
    pthread_mutex_lock(&relayMutex);
    if (relayer) {
        relayer->relayLog(buffer, length);
    }
    pthread_mutex_unlock(&relayMutex);
}
PlatformComStatus IComVirtualDriver::prepareSession() {
    pthread_mutex_lock(&relayMutex);
    if (relayer) {
        relayer->restartLog();
    }
    pthread_mutex_unlock(&relayMutex);
    return PlatformComStatus::PSC_OK;
}

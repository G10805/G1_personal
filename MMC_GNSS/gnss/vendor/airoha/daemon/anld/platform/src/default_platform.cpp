/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#define LOG_TAG "ANLD_Platform"
#include "default_platform.h"
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <algorithm>
#include <mutex>
#include <string>
#include "AdvancedLogger.h"
#include "GPIOControl.h"
#include "PairUtil.h"
#include "SocketBase.h"
#include "anld_service_interface.h"
#include "download_interface.h"
#include "driver/wakelock/air_wakelock.h"
#include "main/mini_loader.h"
#include "native_call.h"
#include "platform_event.h"
#include "simulation.h"
using airoha::WakeLock;
using airoha::communicator::uart_size_t;
/** Airoha Static Config Print **/
#pragma message("==== Airoha Static Config =======")
#if OPEN_UART_TIMING == OPEN_UART_BEFORE_POWER_ON
#pragma message("OPEN_UART_TIMING : OPEN_UART_BEFORE_POWER_ON")
#endif
#if OPEN_UART_TIMING == OPEN_UART_BEFORE_OPEN_DSP
#pragma message("OPEN_UART_TIMING : OPEN_UART_BEFORE_OPEN_DSP")
#endif
#pragma message("uart has active: " STR1(UART_HAS_ACTIVE_FUNCTION))
#pragma message("============= END ==============")
#pragma message("====================================")
// must include config file
#ifndef ANLD_CUSTOMER_CONFIG_INCLUDED
#error "Must Include Config File"
#endif
/** Airoha Static Config Print End **/
static DownloadStatus sDownloadStatus;
static pthread_t sHostDownloadThread;  // download thread
#define MAX_UART_LOG_FILE_NUM (20)
#define MAX_UART_LOG_SINGLE_FILE_SIZE (5 * 1024 * 1024)
extern DownloadConfig gConfigList[];
// #define TEST_HOST_DOWNLOAD
static void *downloadWrapperThread(void *argv) {
    (void)argv;
    DownloadManager dlManager;
#ifdef TEST_HOST_DOWNLOAD
    dlManager.setDownloadRound(100);
#endif
#if COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_SPI
    SpiDownloadInterface sdi(AG3335_SPI_DEV);
    dlManager.registerInterface(&sdi);
    dlManager.setIoType(DownloadManager::DM_SPI);
#elif COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_UART
    UartDownloadInterface udi(Airoha::Configuration::INSTANCE->uartConfig().nodeName);
    udi.setQcPlatfrom(Airoha::Configuration::INSTANCE->supportQcPlatform());
    dlManager.registerInterface(&udi);
    dlManager.setIoType(DownloadManager::DM_UART);
#else
#error "No communication interface select"
#endif
    if (access(USE_OTA_TO_DOWNLOAD_FILE, F_OK | R_OK) == 0) {
        LOG_D("[Check Download] OTA File Exist, use ota bin [%s]",
              USE_OTA_TO_DOWNLOAD_FILE);
        dlManager.setUseOtaBin(true);
    }
    int retryTimes =
        Airoha::Configuration::INSTANCE->firmwareDownloadSetting().retryTimes;
    retryTimes = retryTimes > 0 ? retryTimes : 0;
    sDownloadStatus = dlManager.start(retryTimes);
    return nullptr;
}
LogRelay *DefaultPlatform::getLogRelayer() { return comRelayer; }
int DefaultPlatform::onServiceMessage(AnldServiceMessage message, void *data,
                                      size_t length) {
    if (message != AnldServiceMessage::ANLD_MESSAGE_NMEA_CALLBACK) {
        LOG_D("[DefaultPlatform]:%s(%d),data:%p,len %zu",
              platformMessageEnumToString(message), message, data, length);
    }
    switch (message) {
        case AnldServiceMessage::GNSS_CHIP_DATA_CONNECTION_CLOSE:
#ifdef TRACE_ENABLE
            if (traceLogger) {
                delete (traceLogger);
                traceLogger = nullptr;
            }
#endif
            mComVirtualDriver->disconnect();
            if (logEnable) {
                mComVirtualDriver->logRecordClose();
            }
            break;
        case AnldServiceMessage::GNSS_CHIP_DATA_CONNECTION_OPEN:
            if (logEnable) {
                int fileNum =
                    Airoha::Configuration::INSTANCE->uartLogConfig().maxFileNum;
                int fileSize = Airoha::Configuration::INSTANCE->uartLogConfig()
                                   .maxFileSize;
                mComVirtualDriver->logRecordOpen(mLogPath.c_str(), "relay_",
                                                 fileNum, fileSize);
            }
            mComVirtualDriver->connect();
            break;
        case AnldServiceMessage::GNSS_CHIP_POWER_ON: {
            showOpenInfo();
            onPowerOn();
            mComVirtualDriver->onExtraCommand(
                IComVirtualDriver::EXTRA_COMMAND_PORT_POWER_ON);
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_POWER_OFF: {
            mComVirtualDriver->onExtraCommand(
                IComVirtualDriver::EXTRA_COMMAND_PORT_POWER_OFF);
            onPowerOff();
            showCloseInfo();
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_RESET: {
            Airoha::GPIO::portResetChip();
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_RTC_WAKEUP: {
            onRTCWakeup();
            break;
        }
        case AnldServiceMessage::GNSS_DATA_OUTPUT: {
            // onDataOut(data, length);
            // for test spi, do not send
            mComVirtualDriver->sendData(data, length);
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_DATA_TRIGGER_DOWNLOAD_FIRMWARE: {
            // Not Used
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_WAKE_UP_GPIO26: {
            onWakeUpChip();
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_PREPARE_DATA_CONNECTION: {
            // prepareDataConn();
            mComVirtualDriver->consecutiveDataOpen();
            break;
        }
        case AnldServiceMessage::GNSS_CHIP_SUSPEND_DATA_CONNECTION: {
            // suspendDataConn();
            mComVirtualDriver->consecutiveDataClose();
            break;
        }
        case AnldServiceMessage::FACTORY_MESSAGE: {
            onFactoryMessage((FactoryMessage *)data);
            break;
        }
        case AnldServiceMessage::ANLD_MESSAGE_NMEA_CALLBACK: {
            onNmea((const char *)data, length);
            break;
        }
        case AnldServiceMessage::HOST_DRIVER_WAKE_LOCK: {
            LOG_D("set kernel LOCK");
            WakeLock::getInstnace()->acquireLock(
                WakeLock::WAKELOCK_USER_ANLD_SERVICE);
            break;
        }
        case AnldServiceMessage::HOST_DRIVER_WAKE_UNLOCK: {
            LOG_D("set kernel UNLOCK");
            WakeLock::getInstnace()->releaseLock(
                WakeLock::WAKELOCK_USER_ANLD_SERVICE);
            break;
        }
        case AnldServiceMessage::HOST_POWER_SUSPEND_CALLBACK: {
            LOG_D("host power suspend handle");
            // onPowerSuspend();
            mComVirtualDriver->powerSuspend();
            break;
        }
        case AnldServiceMessage::HOST_POWER_RESUME_CALLBACK: {
            LOG_D("host power resume handle");
            // onPowerResume();
            mComVirtualDriver->powerResume();
            break;
        }
        case AnldServiceMessage::GNSS_EPO_FILE_LOCK_SUCCESS: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_ANLD_EPO_FILE_LOCK_NOTIFY, nullptr, 0);
            break;
        }
        case AnldServiceMessage::GNSS_EPO_FILE_UNLOCK_SUCCESS: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_ANLD_EPO_FILE_UNLOCK_NOTIFY, nullptr,
                0);
            break;
        }
        case AnldServiceMessage::GNSS_EPO_FILE_EXPIRE: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_ANLD_EPO_FILE_EXPIRE_NOTIFY, data,
                length);
            break;
        }
        case AnldServiceMessage::ANLD_SERVICE_MSG_NETWORK_CONNECTED: {
            PlatformEventLoop::getInstance()->sendNetworkConnectedMessage();
            break;
        }
        case AnldServiceMessage::GNSS_RELAY_PROPRIETARY_MESSAGE: {
            handlePropMsgRelay((const PropMessageRelayPayload *)data);
            break;
        }
        case AnldServiceMessage::ANLD_SERVICE_MSG_REQUEST_LOCATION_AIDING: {
            if (mPlatformLocation.flags & Airoha::Gnss::GPS_LOCATION_HAS_LAT_LONG) {
                anldSendMessage2Service(
                    AnldServiceMessage::
                        ANLD_SERVICE_MSG_INJECT_LOCATION_BY_PLATFORM,
                    &mPlatformLocation, sizeof(mPlatformLocation));
            }
            break;
        }
        case AnldServiceMessage::ANLD_SERVICE_MSG_DELETE_AIDING_DATA: {
            LOG_D("Platform delete aiding data.");
            onDeleteAidingData();
            break;
        }
        case AnldServiceMessage::ANLD_SERVICE_PREPARE_SESSION: {
            // Restart log
            mComVirtualDriver->prepareSession();
            LogUtil::LogRelay::restartEvent();
            LogUtil::LogRelay::restartTrace();
            break;
        }
        default:
            break;
    }
    return 0;
}
int DefaultPlatform::onPowerOn() {
    Airoha::GPIO::portPowerOnChip();
    return 0;
}
int DefaultPlatform::onPowerOff() {
    Airoha::GPIO::portPowerOffChip();
    // make sure that chip is power off
    return 0;
}
int DefaultPlatform::onRTCWakeup() { return 0; }
int DefaultPlatform::onWakeUpChip() {
    Airoha::GPIO::portGenerateInterruptHigh();
    usleep(5000);
    Airoha::GPIO::portGnerateInterruptLow();
    usleep(10000);
    Airoha::GPIO::portGenerateInterruptHigh();
    LOG_D("Notify chip success");
    return 0;
}
#define ENUMS(x)       \
    {                  \
        case x:        \
            return #x; \
    }
const char *platformMessageEnumToString(AnldServiceMessage m) {
    switch (m) {
        ENUMS(AnldServiceMessage::GNSS_DATA_INPUT)
        ENUMS(AnldServiceMessage::GNSS_CHIP_DATA_DOWNLOAD_FIRMWARE_SUCCESS);
        // service => 3335
        ENUMS(AnldServiceMessage::GNSS_DATA_OUTPUT);
        ENUMS(AnldServiceMessage::GNSS_CHIP_POWER_ON);
        ENUMS(AnldServiceMessage::GNSS_CHIP_POWER_OFF);
        ENUMS(AnldServiceMessage::GNSS_CHIP_RTC_WAKEUP);
        ENUMS(AnldServiceMessage::GNSS_CHIP_WAKE_UP_GPIO26);
        ENUMS(AnldServiceMessage::GNSS_CHIP_DATA_CONNECTION_OPEN);
        ENUMS(AnldServiceMessage::GNSS_CHIP_DATA_CONNECTION_CLOSE);
        ENUMS(AnldServiceMessage::GNSS_CHIP_DATA_TRIGGER_DOWNLOAD_FIRMWARE);
        ENUMS(AnldServiceMessage::GNSS_HOST_WAKEUP_GPIO24);
        ENUMS(AnldServiceMessage::HOST_DRIVER_WAKE_LOCK);
        ENUMS(AnldServiceMessage::HOST_DRIVER_WAKE_UNLOCK);
        ENUMS(AnldServiceMessage::HOST_POWER_RESUME_CALLBACK);
        ENUMS(AnldServiceMessage::HOST_POWER_SUSPEND_CALLBACK);
        ENUMS(AnldServiceMessage::GNSS_EPO_FILE_UNLOCK_SUCCESS);
        ENUMS(AnldServiceMessage::GNSS_EPO_FILE_LOCK_SUCCESS);
        ENUMS(AnldServiceMessage::ANLD_SERVICE_PREPARE_SESSION);
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
static bool checkBinFile(const char *sourcefile, const char *destfile) {
    LOG_D("%s...sourcefile %s  destfile %s ", __FUNCTION__, sourcefile,
          destfile);
    uint8_t sourcebuf[4096];
    uint8_t destbuf[4096];
    FILE *source = NULL;
    FILE *dest = NULL;
    source = fopen(sourcefile, "rb");
    if (source == NULL) {
        LOG_E("can not open %s,no need download", sourcefile);
        return false;
    }
    dest = fopen(destfile, "rb");
    if (dest == NULL) {
        LOG_D("can not open %s", destfile);
        fclose(source);
        return true;
    }
    if (getFileSize(sourcefile) != getFileSize(destfile)) {
        LOG_D("%s file size not same,need download", sourcefile);
        fclose(source);
        fclose(dest);
        return true;
    }
    uint32_t readnummux = (getFileSize(sourcefile)) / 4096 + 1;
    while (readnummux--) {
        memset(sourcebuf, 0, sizeof(sourcebuf));
        memset(destbuf, 0, sizeof(destbuf));
        if (fread(sourcebuf, sizeof(sourcebuf), 1, source) == 0) {
            if (feof(source)) LOG_D("READ END OF FILE");
        }
        fread(destbuf, sizeof(destbuf), 1, dest);
        if (memcmp(sourcebuf, destbuf, 4096) != 0) {
            fclose(source);
            fclose(dest);
            return true;
        };
    }
    fclose(source);
    fclose(dest);
    LOG_D("%s same bin ,no need download", sourcefile);
    return false;
}
static bool checkBinFileModify() {
    int i = 0;
    while (i++ < 4) {
        if (checkBinFile(gConfigList[i - 1].fileName.c_str(),
            gConfigList[i - 1].backupfileName.c_str())) {
            LOG_D("%s has changed,need download",
                  gConfigList[i - 1].fileName.c_str());
            return true;
        }
    }
    return false;
}
#define BIN_EXIST_RETURN(x)                            \
    {                                                  \
        if (access(x, F_OK | R_OK) != 0) {             \
            LOG_W("[Check Download]" #x " Not Exist"); \
            return false;                              \
        }                                              \
    }
static bool loadDownloadConfig() {
    if (DownloadManager::loadDownloadConfig(
            IMAGE_GNSS_FLASH_DOWNLOAD_CFG_PATH) == false) {
        LOG_W("[Check Download] loadDownloadCfgFile error");
        return false;
    }
    return true;
}
static bool checkVendorBinExistAndDiff() {
    BIN_EXIST_RETURN(IMAGE_GNSS_FLASH_DOWNLOAD_CFG_PATH);
    for (int i = 0; i < 4; i++)
        BIN_EXIST_RETURN(gConfigList[i].fileName.c_str());
    if (checkBinFileModify() == false) {
        LOG_W("[Check Download] BIN FILE ARE SAME,NO NEED DOWNLOAD");
        return false;
    }
    return true;
}
static bool checkOtaBinExist() {
    BIN_EXIST_RETURN(IMAGE_GNSS_OTA_BOOTLOADER_PATH);
    BIN_EXIST_RETURN(IMAGE_GNSS_OTA_GNSS_DEMO_PATH);
    BIN_EXIST_RETURN(IMAGE_GNSS_OTA_CONFIG_PATH);
    BIN_EXIST_RETURN(IMAGE_GNSS_OTA_PARTITION_PATH);
    BIN_EXIST_RETURN(USE_OTA_TO_DOWNLOAD_FILE);
    LOG_D("Check OTA Condition Pass!!!");
    return true;
}
static std::string readVersionFromFile(const char *versionFile) {
    FILE *f = nullptr;
    f = fopen(versionFile, "rb");
    if (f == nullptr) {
        return "FILE_NOT_EXIST";
    }
    char buf[512] = {0};
    fread(buf, 1, sizeof(buf), f);
    std::string ver = buf;
    while (ver.back() == 'r' || ver.back() == 'n') ver.pop_back();
    LOG_D("Visteon: readVersionFromFile %s, ver:%s", versionFile, ver.c_str());
    fclose(f);
    return ver;
}
static bool firmwareVersionChange() {
    LOG_D("Visteon: Check Chip Firmware Version Change");
    int uartBaudrate = 921600;
#if COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_UART
    UartDriver::FlowControl uartFlowControl = UartDriver::FlowControl::FC_NONE;
    if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
        Airoha::UartConfigNode::SOFTWARE_FLOW_CONTROL) {
        uartFlowControl = UartDriver::FlowControl::FC_SOFTWARE;
    } else if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
               Airoha::UartConfigNode::HARDWARE_FLOW_CONTROL) {
        uartFlowControl = UartDriver::FlowControl::FC_HARDWARE;
    } else if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
               Airoha::UartConfigNode::SOFTWARE_FLOW_CONTROL_EXT) {
        uartFlowControl = UartDriver::FlowControl::FC_SOFTWARE_EXT;
    }
    LOG_D("Visteon: uartFlowControl:%d",
          static_cast<int>(Airoha::Configuration::INSTANCE->uartConfig().fc));
#ifdef LOW_BAUD
#pragma message "Low Baud Compile"
    uartBaudrate = 115200;
#endif
#endif
    LOG_D("Visteon: uartBaudrate:%d", uartBaudrate);
    MiniLoader miniLoader(
        Airoha::Configuration::INSTANCE->uartConfig().nodeName, uartBaudrate,
        uartFlowControl);
    miniLoader.runUntil(5);
    std::string ver = miniLoader.getVersion();
    std::string verFromFile = readVersionFromFile(ANLD_FIRMWARE_VERSION_FILE);
    LOG_I("Chip firmware: %s, Firmware in Host: %s, NMEA:%d", ver.c_str(),
          verFromFile.c_str(), miniLoader.isNmeaReceived());
    TRACE_D("Chip firmware: %s, Firmware in Host: %s, NMEA:%d", ver.c_str(),
            verFromFile.c_str(), miniLoader.isNmeaReceived());
    return ver != verFromFile || (!miniLoader.isNmeaReceived());
}
static bool backupFileChange() {
    if (checkOtaBinExist() == false && checkVendorBinExistAndDiff() == false) {
        return false;
    }
    return true;
}
int downloadChipByConfig() {
    DownloadManager::setFwBackupPath(Airoha::Configuration::INSTANCE->dataPath());
    if (!loadDownloadConfig()) {
        LOG_E("Load DL config error, please check cfg file");
        return -1;
    }
    bool download =
        Airoha::Configuration::INSTANCE->firmwareDownloadSetting().download;
    if (!download) {
        LOG_D("no need download due to config");
        return 0;
    }
    bool needDownload = false;
    bool backupFirmwareChange =
        Airoha::Configuration::INSTANCE->firmwareDownloadSetting()
                .backupFileChange
            ? backupFileChange()
            : false;
    // For Fast check
    if (backupFirmwareChange &&
        Airoha::Configuration::INSTANCE->firmwareDownloadSetting()
                .triggerMethod == Airoha::FirmwareDownloadSetting::TRIGGER_OR) {
        needDownload = true;
        LOG_D(
            "firmwareInChipChange=NoCheck, backupFirmwareChange=%d, needDL=%d",
            backupFirmwareChange, needDownload);
        TRACE_D(
            "firmwareInChipChange=NoCheck, backupFirmwareChange=%d, needDL=%d",
            backupFirmwareChange, needDownload);
    } else {
        bool firmwareInChipChange =
            Airoha::Configuration::INSTANCE->firmwareDownloadSetting()
                    .chipFirmwareChange
                ? firmwareVersionChange()
                : false;
        if (Airoha::Configuration::INSTANCE->firmwareDownloadSetting()
                .triggerMethod ==
            Airoha::FirmwareDownloadSetting::TRIGGER_AND) {
            needDownload = firmwareInChipChange && backupFirmwareChange;
        } else if (Airoha::Configuration::INSTANCE->firmwareDownloadSetting()
                       .triggerMethod ==
                   Airoha::FirmwareDownloadSetting::TRIGGER_OR) {
            needDownload = firmwareInChipChange || backupFirmwareChange;
        }
        LOG_D("firmwareInChipChange=%d, backupFirmwareChange=%d, needDL=%d",
              firmwareInChipChange, backupFirmwareChange, needDownload);
    }
    if (needDownload) {
        portDownloadChip();
    }
    return 0;
}
int portDownloadChip() {
    // if (access(SPI_DA_PATH, F_OK | R_OK) != 0) {
    //     LOG_W("[Check Download] DA Not exist [%s][%s]", strerror(errno),
    //           DA_PATH);
    //     return -1;
    // }
    // Step 1: Check if ota file is in

    LOG_D("[Check Download] Start Download...");
    // g_dthread = std::thread(downloadWrapperThread);
    pthread_create(&sHostDownloadThread, NULL, &downloadWrapperThread, NULL);
    // g_pwrThread = std::thread(powerOnThread);
    pthread_join(sHostDownloadThread, NULL);
    // g_pwrThread.join();
    if (sDownloadStatus == DownloadStatus::DL_STATUS_FINISH) {
        LOG_D("[Check Download] Download Pass");
    } else {
        LOG_E("[Check Download] Download FAILED, ret = %d", sDownloadStatus);
    }
    return sDownloadStatus;
}
int DefaultPlatform::openDebugPort() {
    epollFd = epoll_create(10);
    // Create signal socket
    if (debugPort > 0) {
        debugServerFd = SocketBase::createTCPSocket("0.0.0.0", debugPort, 1);
        debugClientFd = -1;
        SocketBase::addSocketDescriporInEpoll(epollFd, debugServerFd);
    }
    if (powerGPSPort > 0) {
        powerGPSServerFd =
            SocketBase::createTCPSocket("0.0.0.0", powerGPSPort, 1);
        powerGPSClientFd = -1;
        SocketBase::addSocketDescriporInEpoll(epollFd, powerGPSServerFd);
    }
    return 0;
}
int DefaultPlatform::closeDebugPort() {
    LOG_I("close debug port");
    close(debugServerFd);
    close(powerGPSServerFd);
    return 0;
}
void *DefaultPlatform::platformThreadFunc(void *argv) {
    DefaultPlatform *instance = (DefaultPlatform *)argv;
    epoll_event eventList[50];
    LOG_I("platform thread enter...");
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    instance->signalFd = signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);
    // Create debug socket
    SocketBase::addSocketDescriporInEpoll(instance->epollFd,
                                          instance->signalFd);
    bool breakSignal = false;
    while (!breakSignal) {
        int k = epoll_wait(instance->epollFd, eventList, 50, -1);
        if (k < 0) {
            // LOG_D("Epoll Error -1");
            if (errno == EINTR) {
                continue;
            }
            LOG_E("epoll_wait error=[%s]", strerror(errno));
            break;
        } else if (k > 0) {
            LOG_D("platform thread %d", k);
            for (int i = 0; i < k; i++) {
                if (eventList[i].events | EPOLLIN) {
                    if (eventList[i].data.fd == instance->debugServerFd) {
                        instance->handleDebugServerMsg(true);
                    } else if (eventList[i].data.fd ==
                               instance->debugClientFd) {
                        instance->handleDebugServerMsg(false);
                    } else if (eventList[i].data.fd ==
                               instance->powerGPSServerFd) {
                        instance->handlePowerGPSMsg(true);
                    } else if (eventList[i].data.fd ==
                               instance->powerGPSClientFd) {
                        instance->handlePowerGPSMsg(false);
                    } else if (eventList[i].data.fd == instance->signalFd) {
                        breakSignal = true;
                        break;
                    } else if (eventList[i].data.fd == instance->gpsIntFd) {
                        instance->handleGpsInterrupt();
                    } else {
                        LOG_W("unregonized event fd");
                    }
                }
            }
        }
    }
    LOG_I("platformThreadFunc Exit");
    return NULL;
}
void DefaultPlatform::handleDebugServerMsg(bool isClientIn) {
    if (isClientIn) {
        struct sockaddr_in ipv4_addr;
        memset(&ipv4_addr, 0, sizeof(ipv4_addr));
        socklen_t len = 0;
        int c_fd = accept(debugServerFd, (struct sockaddr *)&ipv4_addr, &len);
        if (debugClientFd > 0) {
            LOG_E("[Debug Port]Previous Connection is Running");
            close(c_fd);
            return;
        }
        SocketBase::addSocketDescriporInEpoll(epollFd, c_fd);
        debugClientFd = c_fd;
    } else {
        char tmpbuffer[513] = {0};
        char buffer[513] = {0};
        std::string cmdRsp;
        int k = read(debugClientFd, tmpbuffer, 512);
        if (k == 0) {
            LOG_D("[app connect] client leave");
            SocketBase::removeEpollSocket(epollFd, debugClientFd);
            close(debugClientFd);
            debugClientFd = -1;
        } else if (k > 0) {
            buffer[k] = 0;
            // LOG_D("[app PAIR] msg: %s", buffer);
            // remove \r, \n
            size_t j = 0;
            for (size_t i = 0; tmpbuffer[i] != 0 && i < 513; i++) {
                if (tmpbuffer[i] != '\r' && tmpbuffer[i] != '\n') {
                    buffer[j] = tmpbuffer[i];
                    j++;
                }
            }
            // std::string data(buffer, j);
            TRACE_D("data.c_str(): %s, size:%zu", (char *)buffer, j);
            LOG_D("data.c_str(): %s, size:%zu", (char *)buffer, j);
            if (strstr(buffer, "PANL") != NULL) {
                handleDebugMessage(DebugMessageType::TYPE_PANL, buffer, k + 1);
            } else if (strstr(buffer, "PAIR") != NULL) {
                handleDebugMessage(DebugMessageType::TYPE_PAIR, buffer, k + 1);
            } else {
                const char cmdRsp[] =
                    "[ANLD] APP PAIR/PANL ERROR,please check cmd... \r\n";
                sendDebugPortMessage(cmdRsp, sizeof(cmdRsp));
            }
        }
    }
}
void DefaultPlatform::handlePowerGPSMsg(bool isClientIn) {
    if (isClientIn) {
        struct sockaddr_in ipv4_addr;
        memset(&ipv4_addr, 0, sizeof(ipv4_addr));
        socklen_t len = 0;
        int c_fd =
            accept(powerGPSServerFd, (struct sockaddr *)&ipv4_addr, &len);
        if (powerGPSClientFd > 0) {
            LOG_E("[PowerGPS Port]Previous Connection is Running");
            close(c_fd);
            return;
        }
        SocketBase::addSocketDescriporInEpoll(epollFd, c_fd);
        powerGPSClientFd = c_fd;
    } else {
        // handle message
        char raw[512];
        int ret = read(powerGPSClientFd, raw, 512);
        if (ret > 0) {
            anldSendMessage2Service(AnldServiceMessage::ANLD_MESSAGE_SEND_DATA,
                                    raw, ret);
        } else {
            LOG_I("Power GPS Exit");
            SocketBase::removeEpollSocket(epollFd, powerGPSClientFd);
            close(powerGPSClientFd);
            powerGPSClientFd = -1;
        }
    }
}
DefaultPlatform::DefaultPlatform(int userdebugPort, int userPowerGPSPort) {
    LOG_D("Visteon: DefaultPlatform Constructor %d, %d", userdebugPort,
          userPowerGPSPort);
    uart_lock_status = false;
    data_conn_status = false;
    uart_pwr_owner = 0;
    debugPort = userdebugPort;
    powerGPSPort = userPowerGPSPort;
    mUartComm = nullptr;
    mSpiComm = nullptr;
    gpsIntFd = -1;
    debugServerFd = -1;
    debugClientFd = -1;
    powerGPSServerFd = -1;
    powerGPSClientFd = -1;
    openDebugPort();
    openGpsInterrupt();
    comRelayer = nullptr;
    mSimCmdHdr = new SimpleCommandHandler();
#if COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_UART
    UartDriver::FlowControl uartFlowControl = UartDriver::FlowControl::FC_NONE;
    if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
        Airoha::UartConfigNode::SOFTWARE_FLOW_CONTROL) {
        uartFlowControl = UartDriver::FlowControl::FC_SOFTWARE;
    } else if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
               Airoha::UartConfigNode::HARDWARE_FLOW_CONTROL) {
        uartFlowControl = UartDriver::FlowControl::FC_HARDWARE;
    } else if (Airoha::Configuration::INSTANCE->uartConfig().fc ==
               Airoha::UartConfigNode::SOFTWARE_FLOW_CONTROL_EXT) {
        uartFlowControl = UartDriver::FlowControl::FC_SOFTWARE_EXT;
    }
    LOG_D("Visteon: uartFlowControl:%d",
          static_cast<int>(Airoha::Configuration::INSTANCE->uartConfig().fc));
    int baudrate = 921600;
#ifdef LOW_BAUD
#pragma message "Low Baud Compile"
    baudrate = 115200;
#endif
    LOG_D("Visteon: Defaultplatform Construtor baudrate:%d", baudrate);
    mUartComm = new UartVirtualDriver(
        this, Airoha::Configuration::INSTANCE->uartConfig().nodeName, baudrate,
        uartFlowControl);
    bool isQc = Airoha::Configuration::INSTANCE->supportQcPlatform();
    mUartComm->setQcPlatformEnable(isQc);
    mComVirtualDriver = mUartComm;
#elif COMMUNICATION_INTERFACE_SELECE == COMMUNICATION_INTERFACE_SPI
    mSpiComm = new SpiVirtualDriver(AG3335_SPI_DEV, this);
    // mSpiInst = new SpiDriver(AG3335_SPI_DEV);
    mSimCmdHdr->mSpiInst = mSpiComm;
    mComVirtualDriver = mSpiComm;
#endif
    pthread_create(&platformThread, 0, platformThreadFunc, this);
    // Instance value init
    mInstClockDrift = 99999.99;
    initPlatformLocation();
    loadPlatformLocationFromConfig();
}
void DefaultPlatform::setLogPath(const std::string &path) { mLogPath = path; }
DefaultPlatform::~DefaultPlatform() {
    pthread_kill(platformThread, SIGUSR1);
    pthread_join(platformThread, NULL);
    closeDebugPort();
    closeGpsInterrupt();
    // delete (mUartInstance);
    delete (mComVirtualDriver);
    delete (mSimCmdHdr);
}
extern const char *ANLD_BUILD_VERSION;
void DefaultPlatform::handleDebugMessage(DebugMessageType type, char *buf,
                                         int len) {
    std::string command(buf, len);
    if (type == DebugMessageType::TYPE_PAIR) {
        rawPAIRHandler(buf, len);
    } else if (type == DebugMessageType::TYPE_PANL) {
        userSimpleCmdHandler(buf, len);
    }
}
void DefaultPlatform::userSimpleCmdHandler(char *buf, int len) {
    SimpleCommand simCmd(buf, len);
    std::string response = SimpleCommand::getReason(simCmd.getError());
    std::string cmdRsp;
    LOG_D("simple command Error:%d", static_cast<int>(simCmd.getError()));
    sendDebugPortMessage(response.data(), response.size());
    cmdRsp += "\r\n";
    if (simCmd.getError() == SimpleCommand::ErrorCode::NO_ERROR) {
        switch (simCmd.getNumber()) {
            case 1: {
                cmdRsp = "Airoha Navigation Library Daemon(ANLD) \r\n";
                cmdRsp += "Build Version:";
                cmdRsp += ANLD_BUILD_VERSION;
                cmdRsp += "\r\n";
                break;
            }
            case 2: {
                FactoryMessage message;
                message.msgid = FactoryMessageID::FACTORY_SET_POWER_DSP_ON;
                anldSendMessage2Service(AnldServiceMessage::FACTORY_MESSAGE,
                                        &message, sizeof(message));
                cmdRsp = "Power On DSP, please Wait.... \r\n";
                break;
            }
            case 3: {
                FactoryMessage message;
                message.msgid = FactoryMessageID::FACTORY_SET_POWER_POWER_OFF;
                anldSendMessage2Service(AnldServiceMessage::FACTORY_MESSAGE,
                                        &message, sizeof(message));
                cmdRsp = "Power off, please Wait.... \r\n";
                break;
            }
            case 4: {
                FactoryMessage message;
                message.msgid = FactoryMessageID::FACTORY_SET_POWER_POWER_ON;
                anldSendMessage2Service(AnldServiceMessage::FACTORY_MESSAGE,
                                        &message, sizeof(message));
                cmdRsp = "Power off, please Wait.... \r\n";
                break;
            }
            case PL_CMD_INST_GET_LAST_LOCATION: {
                char posStr[64] = {0};
                snprintf(posStr, sizeof(posStr), "PANL%d,%d,%.7f,%.7f,%.7f\r\n",
                         PL_CMD_INST_GET_LAST_LOCATION,
                         mInstPosition.fix_quality, mInstPosition.latitude,
                         mInstPosition.longitude, mInstPosition.altitude);
                cmdRsp += posStr;
                break;
            }
            case PL_CMD_INST_GET_LAST_CLOCK_DRIFT: {
                char ckDftStr[50] = {0};
                snprintf(ckDftStr, sizeof(ckDftStr), "PANL%d,%d,%.3f\r\n",
                         PL_CMD_INST_GET_LAST_CLOCK_DRIFT,
                         mInstPosition.fix_quality, mInstClockDrift);
                cmdRsp += ckDftStr;
                break;
            }
            case 100: {
                // Message::globalSendNormalMessage(GnssMessageType::GNSS_COMMON_MESSAGE,
                //    GnssMessageID::GNSS_MSG_APP_QUERY_CMD_NUM, NULL, 0);
                break;
            }
            case 666: {
                assert(0);
                break;
            }
            case PL_CMD_FACTORY_SET_SILENT: {
                FactoryMessage message;
                message.msgid = FactoryMessageID::FACTORY_SET_SILENT_MODE;
                anldSendMessage2Service(AnldServiceMessage::FACTORY_MESSAGE,
                                        &message, sizeof(message));
                break;
            }
            case PL_CMD_FACTORY_UNSET_SILENT: {
                FactoryMessage message;
                message.msgid = FactoryMessageID::FACTORY_SET_NORMAL_MODE;
                anldSendMessage2Service(AnldServiceMessage::FACTORY_MESSAGE,
                                        &message, sizeof(message));
                break;
            }
            default: {
                bool ret = mSimCmdHdr->handleMessage(simCmd, &cmdRsp);
                if (ret) {
                    cmdRsp += "handle comand ok \r\n";
                } else {
                    cmdRsp += "command not found \r\n";
                }
                break;
            }
                // case 4:
                // {
                //     Airoha::Factory::Factory
                // }
        }
    }
    sendDebugPortMessage(cmdRsp.data(), cmdRsp.size());
}
void DefaultPlatform::rawPAIRHandler(char *buff, int len) {
    int pairnum = 0;
    char temp[4] = {0};
    char pairFull[513] = {0};
    int app_pair_found = 0;
    std::vector<int>::iterator iter;
    LOG_D("sendAPPPair:%s   length %d", buff, len);
    PAIRData::getFullPairCommand(buff, strlen(buff) - 2, pairFull, 512);
    memcpy(temp, buff + 4, 3);
    pairnum = atoi(temp);
    LOG_D("PAIR NUM:%d", pairnum);
    iter = std::find(pairWaitAckList.begin(), pairWaitAckList.end(), pairnum);
    if (iter == pairWaitAckList.end()) {
        pairWaitAckList.push_back(pairnum);
    }
    if (buff[strlen(buff) - 1] == '1')  // need response
    {
        for (iter = pairlist.begin(); iter != pairlist.end();) {
            if (*iter == pairnum) {
                LOG_D("already save pair");
                app_pair_found = 1;
                break;
            } else {
                ++iter;
            }
        }
        if (app_pair_found == 0) pairlist.push_back(pairnum);
    } else if (buff[strlen(buff) - 1] == '2')  // delete response
    {
        for (iter = pairlist.begin(); iter != pairlist.end();) {
            if (*iter == pairnum) {
                iter = pairlist.erase(iter);
                break;
            } else
                ++iter;
        }
    }
    anldSendMessage2Service(AnldServiceMessage::ANLD_MESSAGE_SEND_DATA,
                            pairFull, strlen(pairFull) + 1);
}
void DefaultPlatform::checkAppPairResponse(const std::string &data) {
    int pairnum = 0;
    std::vector<int>::iterator iter;
    std::string pair = (data.substr(5, 3));
    pairnum = atoi(pair.c_str());
    // LOG_D("checkAppPairResponse pairnum:%d", pairnum);
    for (iter = pairlist.begin(); iter != pairlist.end(); iter++) {
        // LOG_D("pairlis:%d", *iter);
        if (*iter == pairnum) {
            LOG_D("sendDebugResponse:%s", data.c_str());
            sendDebugPortMessage(data.c_str(), data.size());
            break;
        }
    }
}
void DefaultPlatform::checkAppPairAck(std::string &data, int ackCmd,
                                      int ackStatus) {
    // int pairnum=0;
    std::vector<int>::iterator iter;
    LOG_D("checkAppPairAck : %s, ack for :%d", data.c_str(), ackCmd);
    for (iter = pairWaitAckList.begin(); iter != pairWaitAckList.end();) {
        LOG_D("pairlis:%d", *iter);
        if (*iter == ackCmd) {
            LOG_D("sendDebugResponse:%s", data.c_str());
            sendDebugPortMessage(data.c_str(), data.size());
            if (ackStatus == 0 || ackStatus == 2 || ackStatus == 4 ||
                ackStatus == 5) {
                iter = pairWaitAckList.erase(iter);
            }
            break;
        } else {
            iter++;
        }
    }
}
int DefaultPlatform::onNmea(const char *buffer, ANL_SIZE_T length) {
    std::string pairStr(buffer, length);
    if (strstr(buffer, "PAIR001") != NULL) {
        PAIRData *pair = new PAIRData();
        bool ret = pair->setCommand(pairStr.c_str());
        if (!ret) {
            LOG_E("nmea ack error");
        } else {
            checkAppPairAck(pairStr, pair->getParamInt(1),
                            pair->getParamInt(2));
        }
        delete (pair);
    }
    checkAppPairResponse(pairStr);
    return 0;
}
void DefaultPlatform::sendDebugPortMessage(const void *buffer,
                                           ANL_SIZE_T length) {
    char tmp[512] = {0};
    ANL_SIZE_T copyLen = length > 511 ? 511 : length;
    memcpy(tmp, buffer, copyLen);
    TRACE_D("Send Debug(%d): %s", debugClientFd, tmp);
    if (debugClientFd > 0) {
        native_safe_write(debugClientFd, buffer, length);
    }
}
ssize_t DefaultPlatform::sendPowerGPSMessage(const void *buffer,
                                             ANL_SIZE_T length) {
    ssize_t ret = -1;
    if (powerGPSClientFd) {
        ret = write(powerGPSClientFd, buffer, length);
        if (ret < 0) {
            // LOG_E("send to PowerGPS Error,%d,%s", errno, strerror(errno));
        }
    }
    return ret;
}
int DefaultPlatform::onFactoryMessage(FactoryMessage *msg) {
    std::string response;
    switch (msg->status) {
        case FactoryStatus::FACTORY_STATUS_DSP_ON: {
            response += "[Factory] Gnss DSP on";
            break;
        }
        case FactoryStatus::FACTORY_STATUS_POWER_OFF: {
            response += "[Factory] Gnss Power Off";
            break;
        }
        default:
            break;
    }
    response += "\r\n";
    sendDebugPortMessage(response.data(), response.size());
    return 0;
}
int DefaultPlatform::onDeleteAidingData() {
    fillPlatformLocation(0.0, 0.0, 0.0, false);
    return 0;
}
void DefaultPlatform::openGpsInterrupt() {
    gpsIntFd = open("/dev/airoha_gps", O_RDWR);
    SocketBase::addSocketDescriporInEpoll(epollFd, gpsIntFd);
}
void DefaultPlatform::closeGpsInterrupt() {
    SocketBase::removeEpollSocket(epollFd, gpsIntFd);
    close(gpsIntFd);
    gpsIntFd = -1;
}
void DefaultPlatform::handleGpsInterrupt() {
    char msg[20] = {0};
    read(gpsIntFd, msg, 20);
    LOG_D("gps driver message %s", msg);
    anldSendMessage2Service(
        AnldServiceMessage::HOST_DRIVER_RECEIVED_WAKEUP_MESSAGE, NULL, 0);
    mComVirtualDriver->interrupt();
    EVENT_I("WakeupHost: %s", msg);
}
void DefaultPlatform::lockLinuxPower() {
    if (gpsIntFd > 0) {
        write(gpsIntFd, "LOCKWA", sizeof("LOCKWA"));
    }
}
void DefaultPlatform::unlockLinuxPower() {
    if (gpsIntFd > 0) {
        write(gpsIntFd, "UNLOCK", sizeof("UNLOCK"));
    }
}
void DefaultPlatform::showOpenInfo() {
    struct tm localTime;
    time_t timeUTC = time(NULL);
    gmtime_r(&timeUTC, &localTime);
    char dateStr[] = "PAIRANL,START,%d,%d,%d,%d,%d,%d,%s";
    char date[100] = {0};
    char fullData[120] = {0};
    snprintf(date, 60, dateStr, localTime.tm_year + 1900, localTime.tm_mon + 1,
             localTime.tm_mday, localTime.tm_hour, localTime.tm_min,
             localTime.tm_sec, localTime.tm_zone);
    PAIRData::getFullPairCommand(date, 100, fullData, 120);
    // AdvancedLogger::advanceUart((uint8_t *)fullData, 120);
    // AdvancedLogger::advanceRawData((uint8_t *)fullData, 120);
    LOG_I("%s", fullData);
    return;
}
void DefaultPlatform::showCloseInfo() {
    struct tm localTime;
    time_t timeUTC = time(NULL);
    gmtime_r(&timeUTC, &localTime);
    char dateStr[] = "PAIRANL,STOP,%d,%d,%d,%d,%d,%d,%s";
    char date[100] = {0};
    char fullData[120] = {0};
    snprintf(date, 60, dateStr, localTime.tm_year + 1900, localTime.tm_mon + 1,
             localTime.tm_mday, localTime.tm_hour, localTime.tm_min,
             localTime.tm_sec, localTime.tm_zone);
    PAIRData::getFullPairCommand(date, 100, fullData, 120);
    AdvancedLogger::advanceUart((uint8_t *)fullData, 120);
    AdvancedLogger::advanceRawData((uint8_t *)fullData, 120);
    LOG_I("%s", fullData);
    return;
}
void DefaultPlatform::setUartLogRecord(bool enable) { logEnable = enable; }
void DefaultPlatform::platformSetUartActive(bool enable) {
    // to be implement
    LOG_D("Platform Set Uart :%d", enable);
}
void DefaultPlatform::handlePropMsgRelay(
    const PropMessageRelayPayload *payload) {
    using airoha::proprietary::PropPvt;
    using airoha::proprietary::PropTimeInfo;
    switch (payload->type) {
        case airoha::proprietary::PROP_MSG_TIME_INFO: {
            assert(sizeof(PropTimeInfo) == payload->length);
            const PropTimeInfo *tmInfo = (const PropTimeInfo *)payload->data;
            mInstClockDrift = tmInfo->clockDrift * 1e-6;  // convert to PPM
            break;
        }
        case airoha::proprietary::PROP_MSG_TIME_PVT: {
            assert(sizeof(PropPvt) == payload->length);
            const PropPvt *pvt = (const PropPvt *)payload->data;
            mInstPosition.fix_quality = pvt->fixQuality;
            mInstPosition.latitude = (double)pvt->latitude * 1e-7;
            mInstPosition.longitude = (double)pvt->longitude * 1e-7;
            mInstPosition.altitude =
                (double)pvt->hMsl * 1e-3 + (double)pvt->mslCorr * 1e-3;
            break;
        }
        default:
            break;
    }
    return;
}
void DefaultPlatform::initPlatformLocation() {
    mPlatformLocation.flags = 0;
    mPlatformLocation.latitudeDegrees = 0.0;
    mPlatformLocation.longitudeDegrees = 0.0;
    mPlatformLocation.altitudeMeters = 0.0;
    mPlatformLocation.horizontalAccuracyMeters = 0.0;
    mPlatformLocation.verticalAccuracyMeters = 0.0;
    mPlatformLocation.speedAccuracyMetersPerSecond = 0.0;
    mPlatformLocation.bearingAccuracyDegrees = 0.0;
    mPlatformLocation.timestamp = 0.0;
    mPlatformLocation.elapsedTime.flags = 0;
}
void DefaultPlatform::loadPlatformLocationFromConfig() {
    const Airoha::InitLocationConfiguration &locCfg =
        Airoha::Configuration::INSTANCE->initLocation();
    if (!locCfg.enable) {
        return;
    }
    LOG_D("Load init location: %f, %f, %f", locCfg.latitude, locCfg.longitude,
          locCfg.altitude);
    fillPlatformLocation(locCfg.latitude, locCfg.longitude, locCfg.altitude);
}
void DefaultPlatform::fillPlatformLocation(double latitude, double longitude,
                                           double altitude, bool valid) {
    if (!valid) {
        mPlatformLocation.flags = 0;
        return;
    }
    mPlatformLocation.latitudeDegrees = latitude;
    mPlatformLocation.longitudeDegrees = longitude;
    mPlatformLocation.altitudeMeters = altitude;
    mPlatformLocation.flags |= Airoha::Gnss::GPS_LOCATION_HAS_LAT_LONG;
    mPlatformLocation.flags |= Airoha::Gnss::GPS_LOCATION_HAS_ALTITUDE;
}
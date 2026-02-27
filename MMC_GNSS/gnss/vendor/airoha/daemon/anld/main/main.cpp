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
#include <errno.h>
#include <error.h>
#include "AdvancedLogger.h"
#include "ConfigLoader.h"
#include "LogRelayer.h"
#include "anld_customer_config.h"
#include "anld_service_interface.h"
#include "default_platform.h"
#include "environment.h"
#include "linux_transfer.h"
#include "mini_loader.h"
#include "network_util.h"
#include "platform_event.h"
#include "signal.h"
#include "simulation.h"
#include "unistd.h"
using namespace Airoha;
using Airoha::Configuration;
/** Private Function **/
static void setAnldConfig();
static int g_sigal_is_handle = 0;
static void sigHandler(int signal) {
    switch (signal) {
        case SIGTERM:
        case SIGINT:
            LOG_D("Bye!");
            if (g_sigal_is_handle == 0) {
                g_sigal_is_handle = 1;
                PlatformEventLoop::getInstance()->sendMessage(
                    PlatformEventLoop::EVENT_3DAYS_EPO_MONITOR_STOP, nullptr,
                    0);
                PlatformEventLoop::getInstance()->sendMessage(
                    PlatformEventLoop::EVENT_STOP_ANLD_APP, nullptr, 0);
                PlatformEventLoop::getInstance()->sendMessage(
                    PlatformEventLoop::EVENT_EXIT, nullptr, 0);
            }
            break;
    }
    ::signal(SIGINT, SIG_DFL);
    ::signal(SIGTERM, SIG_DFL);
}
static AdvancedLogger *globalLogger = NULL;
static void loopCreateLogFile() {
    while (1) {
        usleep(6000000);
        globalLogger =
            new AdvancedLogger("/storage/3963-3132/", "gnss_",
                               AdvancedLogger::LogMode::MODE_ANLD_LOG);
        if (globalLogger->isValidLogger()) {
            LOG_D("====== Logger Start ======");
            break;
        }
        delete (globalLogger);
        usleep(1000000);
    }
}
static void platformEventLoopInit() {
    PlatformEventLoop::getInstance()->init();
    PlatformEventLoop::getInstance()->setAutoDownloadEpo(
        Airoha::Configuration::INSTANCE->autoDownloadEPO());
    PlatformEventLoop::getInstance()->setEpoDataPath(
        (strlen(Airoha::Configuration::INSTANCE->dataPath()) > 0)
            ? Airoha::Configuration::INSTANCE->dataPath()
            : "/data/vendor/airoha/");
    PlatformEventLoop::getInstance()->setEpoVendor(Airoha::Configuration::INSTANCE->epoVendorId());
    PlatformEventLoop::getInstance()->setEpoProject(
        Airoha::Configuration::INSTANCE->epoProjectId());
    PlatformEventLoop::getInstance()->setEpoDevice(Airoha::Configuration::INSTANCE->epoDeviceId());
    // Test Message
    PlatformEventLoop::getInstance()->sendMessage(PlatformEventLoop::EVENT_TEST,
                                                  nullptr, 0);
    PlatformEventLoop::getInstance()->sendMessage(
        PlatformEventLoop::EVENT_START_ANLD_APP, nullptr, 0);
    PlatformEventLoop::getInstance()->sendMessage(
        PlatformEventLoop::EVENT_3DAYS_EPO_MONITOR_START, nullptr, 0);
}
static void mainSetConfigLevel(const std::string &level) {
    LOG_D("User Set Level to: %s", level.c_str());
    if (level == "none") {
        AdvancedLogger::setLogLevel(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_NONE);
    } else if (level == "error") {
        AdvancedLogger::setLogLevel(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_ERROR);
    } else if (level == "debug") {
        AdvancedLogger::setLogLevel(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_DEBUG);
    } else if (level == "warning") {
        AdvancedLogger::setLogLevel(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_WARNING);
    } else if (level == "info") {
        AdvancedLogger::setLogLevel(AdvancedLogger::LogLevel::ADV_LOG_LEVEL_INFO);
    } else {
        LOG_E("unrecognized log level");
    }
};


int main(int argc, const char *argv[]) {
    signal(SIGTERM, sigHandler);
    signal(SIGINT, sigHandler);
    signal(SIGPIPE, SIG_IGN);
    LOG_E("ANLD Running, sizeof(size) = %zu ", sizeof(size_t));
    LOG_E("This is %zu bit system", sizeof(size_t) * 8);
    srand(time(NULL));
#ifdef SUPERIOR_DEBUG_ENABLE
    LinuxTransfer *trans = new LinuxTransfer("0.0.0.0", 5566, true);
    trans->start();
#endif
    Environment::getInstance()->parserCommandLine(argc, argv);
    Network::init();
    (void)loopCreateLogFile;
    if (Environment::getInstance()->hasVariable(ENVIRONMENT_KEY_CONFIG_PATH)) {
        LOG_D("Found config path in argument, use it...");
        Airoha::Configuration::INSTANCE->loadConfigureFile(
            Environment::getInstance()->variable(ENVIRONMENT_KEY_CONFIG_PATH).c_str());
    } else {
        LOG_D("Use default configuration file.");
        Airoha::Configuration::INSTANCE->loadConfigureFile(
            ANLD_CONFIG_XML_PATH);
    }
    // First use Configuration in this place, and then other places that use
    // ConfigLoader will also be replaced with Configuration.
    bool needTrace = Airoha::Configuration::INSTANCE->traceLogRecord();
    int traceLogNum =
        Airoha::Configuration::INSTANCE->traceLogConfig().maxFileNum;
    int traceLogSize =
        Airoha::Configuration::INSTANCE->traceLogConfig().maxFileSize;
    if (needTrace) {
        LogUtil::LogRelay::initTrace(
            Airoha::Configuration::INSTANCE->dataPath(), traceLogNum,
            traceLogSize);
    }
    // init event log
    const auto &eventLogConfig =
        Airoha::Configuration::INSTANCE->eventLogConfig();
    if (eventLogConfig.enable) {
        LogUtil::LogRelay::initEventLog(
            Airoha::Configuration::INSTANCE->dataPath(),
            eventLogConfig.maxFileNum, eventLogConfig.maxFileSize);
    }
    downloadChipByConfig();
    // Download finish, to init platform.
    LOG_D("Visteon: DefaultPlatform init");
    DefaultPlatform *dplat = NULL;
    
    mainSetConfigLevel(Airoha::Configuration::INSTANCE->logLevel());
    int debugPort = Airoha::Configuration::INSTANCE->debugPort();
    int powerGPSPort = Airoha::Configuration::INSTANCE->powerGPSPort();
    bool logRecord = Airoha::Configuration::INSTANCE->uartLogSupport() >
                     Airoha::Configuration::UART_LOG_RECORD_DISABLED;
    const char *logPath = Airoha::Configuration::INSTANCE->dataPath();
    LOG_D("Visteon: Debug Port: %d, Power GPS Port: %d, Log Record: %d, Log Path: %s",
          debugPort, powerGPSPort, logRecord, logPath);
    dplat = new DefaultPlatform(debugPort, powerGPSPort);
    dplat->setUartLogRecord(logRecord);
    setAnldConfig();
    dplat->setLogPath(logPath);
    LOG_D("Visteon: ANLD Service Register Handler");
    anldServiceRegisterHandler(dplat);

    platformEventLoopInit();
    PlatformEventLoop::getInstance()->setPlatformInstance(dplat);
    PlatformEventLoop::getInstance()->loop();
    PlatformEventLoop::getInstance()->setPlatformInstance(nullptr);
    delete (dplat);
#ifdef SUPERIOR_DEBUG_ENABLE
    trans->stop();
    delete (trans);
#endif
    Network::deinit();
    if (eventLogConfig.enable) {
        LogUtil::LogRelay::deinitEventLog();
    }
    if (needTrace) {
        LogUtil::LogRelay::deinitTrace();
    }
    LOG_D("ANLD control thread exit");
    LOG_D("Main System Exit");
}
void setAnldConfig() {
    Airoha::ANLCustomerConfig anldConf = {
        .outputDebugLog = Airoha::Configuration::INSTANCE->outputDebugLog(),
        .outputBinary = Airoha::Configuration::INSTANCE->outputBinary(),
        .epoVendorID = Airoha::Configuration::INSTANCE->epoVendorId(),
        .epoProjectName = Airoha::Configuration::INSTANCE->epoProjectId(),
        .epoDeviceID = Airoha::Configuration::INSTANCE->epoProjectId(),
        .rawMeasurementSupport =
            Airoha::Configuration::INSTANCE->rawMeasurementSupport(),
        .autoLockSleep = Airoha::Configuration::INSTANCE->autoLockSleep(),
        .autoDLEpo = Airoha::Configuration::INSTANCE->autoDownloadEPO(),
        .supportHighFixRate =
            Airoha::Configuration::INSTANCE->supportHighFixRate(),
        .wakeDuration = Airoha::Configuration::INSTANCE->wakeDuration(),
        .wakeupBeforeSendData =
            Airoha::Configuration::INSTANCE->wakeupBeforeSendData(),
        .suspendImplementVersion =
            Airoha::Configuration::INSTANCE->suspendImplement(),
        .dataPath = Airoha::Configuration::INSTANCE->dataPath(),
    };
    anldServiceSetConfig(anldConf);
}


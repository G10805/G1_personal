#include "platform_command.h"
#include <string.h>
#include "GPIOControl.h"
#include "airo_gps.h"
#include "anld_service_interface.h"
#include "platform_event.h"
#include "simulation.h"
using airoha::epo::EpoConstellation;
using airoha::epo::EpoFileType;
#define REPLAY_LOG_FILE_NAME "UART0.LOG"
#ifdef SUPPORT_SENSOR_FEATURE
#include "driver/sensor/sensor.h"
using airoha::communicator::Sensor;
#endif
SimpleCommandHandler::SimpleCommandHandler() {
    mSpiInst = nullptr;
#ifdef SUPPORT_SENSOR_FEATURE
    mSensorInst = nullptr;
#endif
}
bool SimpleCommandHandler::handleMessage(const SimpleCommand &cmd,
                                         std::string *rsp) {
    bool ret = true;
    switch (cmd.getNumber()) {
        case PL_CMD_TEST_SPI_Q_RX_BUFFER_SIZE: {
            uint32_t rxFree = mSpiInst->querySlaveRxBufferSize();
            LOG_D("PL_CMD_TEST_SPI_Q_RX_BUFFER_SIZE %u", rxFree);
            break;
        }
        case PL_CMD_TEST_SPI_Q_TX_BUFFER_SIZE: {
            uint32_t txFree = mSpiInst->querySlaveTxBufferSize();
            LOG_D("PL_CMD_TEST_SPI_Q_TX_BUFFER_SIZE %u", txFree);
            break;
        }
        case PL_CMD_TEST_SPI_Q_ALL_TX_BUFFER: {
            std::string str = mSpiInst->readAll();
            LOG_D("%s", str.c_str());
            break;
        }
        case PL_CMD_FACTORY_GPIO26_INT: {
            Airoha::GPIO::portGenerateInterruptHigh();
            usleep(5000);
            Airoha::GPIO::portGnerateInterruptLow();
            usleep(10000);
            Airoha::GPIO::portGenerateInterruptHigh();
            LOG_D("Notify chip success");
            break;
        }
        case PL_CMD_OPEN_SD_CARD_LOG: {
            LogUtil::LogRelay::initSdLog();
            break;
        }
        case PL_CMD_CLOSE_SD_CARD_LOG: {
            LogUtil::LogRelay::deinitSdLog();
            break;
        }
        case PL_CMD_OTA_DOWNLOAD: {
            break;
        }
        case PL_CMD_DOWNLOAD_EPO_QGPS: {
            PlatformEventLoop::getInstance()->sendDownloadEpoFileMsg(
                EpoConstellation::EC_GPS, EpoFileType::EFT_QEPO);
            break;
        }
        case PL_CMD_DOWNLOAD_EPO_QGR: {
            PlatformEventLoop::getInstance()->sendDownloadEpoFileMsg(
                EpoConstellation::EC_GLONASS, EpoFileType::EFT_QEPO);
            break;
        }
        case PL_CMD_DOWNLOAD_EPO_QBD2: {
            PlatformEventLoop::getInstance()->sendDownloadEpoFileMsg(
                EpoConstellation::EC_BEIDOU, EpoFileType::EFT_QEPO);
            break;
        }
        case PL_CMD_DOWNLOAD_EPO_QGA: {
            PlatformEventLoop::getInstance()->sendDownloadEpoFileMsg(
                EpoConstellation::EC_GALILEO, EpoFileType::EFT_QEPO);
            break;
        }
        case PL_CMD_QUERY_EPO_UPDATE_RESULT: {
            rsp->append(
                PlatformEventLoop::getInstance()->queryLastEpoUpdateResult());
            break;
        }
        case PL_CMD_CONSOLE_GET_3DAY_EPO_STATE: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_SYSTEM_QUERY_EPO_FILE_STATUS, nullptr, 0);
            break;
        }
        case PL_CMD_CONSOLE_TRIGGER_3DAYS_EPO_CHECK: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_SYSTEM_TRIGGER_3_DAYS_EPO_DOWNLOAD, nullptr,
                0);
            break;
        }
        case PL_CMD_ENABLE_NETWORK_LOW_SPEED: {
            PlatformEventLoop::getInstance()->setElpoDownloadDelay(1000);
            break;
        }
        case PL_CMD_DISABLE_NETWORK_LOW_SPEED: {
            PlatformEventLoop::getInstance()->setElpoDownloadDelay(0);
            break;
        }
        case PL_CMD_CLEAR_NET_DL_CACHE: {
            PlatformEventLoop::getInstance()->clearElpoDownloadCache();
            break;
        }
        case PL_CMD_TRIGGER_NETWORK_CONNECTED: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_SYSTEM_NETWORK_CONNECTED, nullptr, 0);
            break;
        }
        case PL_CMD_CONSOLE_SWITCH_ON: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_ON,
                nullptr, 0);
            break;
        }
        case PL_CMD_CONSOLE_SWITCH_OFF: {
            PlatformEventLoop::getInstance()->sendMessage(
                PlatformEventLoop::EVENT_SYSTEM_SET_DEBUG_PORT_SWITCH_OFF,
                nullptr, 0);
            break;
        }
#ifndef AIROHA_SIMULATION
#ifdef SUPPORT_SENSOR_FEATURE
        case PL_CMD_TEST_SENSOR: {
            Sensor::TEST(5);
            break;
        }
        case PL_CMD_TEST_SENSOR_LONG: {
            Sensor::TEST(5 * 60);
            break;
        }
#endif // SUPPORT_SENSOR_FEATURE
#endif
#ifdef SUPPORT_SENSOR_FEATURE
        case PL_CMD_SENSOR_INIT: {
            break;
        }
        case PL_CMD_SENSOR_START: {
            break;
        }
        case PL_CMD_SENSOR_STOP: {
            break;
        }
        case PL_CMD_SENSOR_CLEANUP: {
            break;
        }
        case PL_CMD_SENSOR_TEST_CASE: {
            LOG_D("=== PL_CMD_SENSOR_TEST_CASE ===");
            airoha::communicator::Sensor *sensor =
                new airoha::communicator::Sensor("TEST");
            sensor->setSampleRateHz(100);
            sensor->setJitter(3000);
            bool status = sensor->start();
            if (!status) {
                LOG_E("sensor start error");
                break;
            }
            usleep(5000000);
            status = sensor->stop();
            if (!status) {
                LOG_E("sensor stop error");
                break;
            }
            delete (sensor);
            LOG_D("=== PL_CMD_SENSOR_TEST_CASE Done ===");
            break;
        }
        case PL_CMD_SENSOR_SET_SAMPLE_RATE_50Hz: {
            airoha::communicator::Sensor::setDebugSampleRateHz(50);
            *rsp += "Set Sample Rate 50hz\n";
            break;
        }
        case PL_CMD_SENSOR_SET_SAMPLE_RATE_100Hz: {
            airoha::communicator::Sensor::setDebugSampleRateHz(100);
            *rsp += "Set Sample Rate 100hz\n";
            break;
        }
#endif
        case PL_CMD_LOG_RELAY_INPUT: {
            relayFileLogToService(REPLAY_LOG_FILE_NAME);
            break;
        }
        case PL_CMD_TEST_MOCK_SEND_COMMAND_AIDING_EPO: {
            mockSendCommand("$PAIR010,0,1,2261,270930*32\r\n");
            break;
        }
        case PL_CMD_SEND_TEST_ASSERT_SERVICE_MESSAGE: {
            struct tm t;
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            time_t sec = ts.tv_sec;
            gmtime_r(&sec, &t);
            Airoha::anldReportFormatCriticalMessage(
                "Test Service Assert Message 1 %04d-%02d-%02d "
                "%02d:%02d:%02d.%03d",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
                t.tm_sec, ts.tv_nsec / 1000000);
            break;
        }
        case PL_CMD_SEND_TEST_CRITICAL_SERVICE_MESSAGE: {
            struct tm t;
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            time_t sec = ts.tv_sec;
            gmtime_r(&sec, &t);
            Airoha::anldReportFormatCriticalMessage(
                "$Test Service Assert Message 1 %04d-%02d-%02d "
                "%02d:%02d:%02d.%03d",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
                t.tm_sec, ts.tv_nsec / 1000000);
            break;
        }
        default: {
            ret = false;
            break;
        }
    }
    return ret;
}
void SimpleCommandHandler::relayFileLogToService(const char *filename) {
    LOG_D("===== Relay File (%s) =====", filename);
    FILE *f = fopen(filename, "rb");
    if (!f) {
        LOG_E("Open %s failed, %d, %s", filename, errno, strerror(errno));
        return;
    }
    uint8_t data[4096];
    size_t hasRead = 0;
    while (1) {
        size_t ret = fread(data, 1, sizeof(data), f);
        if (ret == 0) break;
        Airoha::anldSendMessage2Service(
            Airoha::AnldServiceMessage::GNSS_DATA_INPUT, data, ret);
        hasRead += ret;
        LOG_D("Relay %zu bytes", hasRead);
        usleep(10000);
    }
    fclose(f);
}
void SimpleCommandHandler::mockSendCommand(const char *cmd) {
    Airoha::anldSendMessage2Service(Airoha::AnldServiceMessage::GNSS_DATA_INPUT,
                                    cmd, strlen(cmd));
}

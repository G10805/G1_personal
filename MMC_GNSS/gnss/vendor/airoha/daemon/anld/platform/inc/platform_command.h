#pragma once
#include "PairUtil.h"
#include "platform_virtual_driver.h"
#include "driver/sensor/sensor.h"
enum PlatformCommand {
    PL_CMD_QUERY_BUILD_VERSION = 1,
    PL_CMD_FACTORY_DSP_ON = 2,
    PL_CMD_FACTORY_POWER_OFF = 3,
    PL_CMD_FACTORY_POWER_ON = 4,
    PL_CMD_FACTORY_GPIO26_INT = 5,
    PL_CMD_OPEN_SD_CARD_LOG = 6,
    PL_CMD_CLOSE_SD_CARD_LOG = 7,
    PL_CMD_OTA_DOWNLOAD = 8,
    PL_CMD_GEN_NEW_DEVICE_ID = 9,
    PL_CMD_DOWNLOAD_EPO_QGPS = 10,
    PL_CMD_DOWNLOAD_EPO_QGR = 11,
    PL_CMD_DOWNLOAD_EPO_QGA = 12,
    PL_CMD_DOWNLOAD_EPO_QBD2 = 13,
    PL_CMD_DOWNLOAD_EPO_GPS_3_1 = 14,
    PL_CMD_DOWNLOAD_EPO_GR_3_1 = 15,
    PL_CMD_DOWNLOAD_EPO_GA_3_1 = 16,
    PL_CMD_DOWNLOAD_EPO_BD_3_1 = 17,
    PL_CMD_QUERY_EPO_UPDATE_RESULT = 18,
    PL_CMD_TEST_SENSOR = 19,
    PL_CMD_TEST_SENSOR_LONG = 20,
    PL_CMD_LOG_RELAY_INPUT = 21,
#ifdef SUPPORT_SENSOR_FEATURE
    PL_CMD_SENSOR_INIT = 30,
    PL_CMD_SENSOR_START = 31,
    PL_CMD_SENSOR_STOP = 32,
    PL_CMD_SENSOR_CLEANUP = 33,
    PL_CMD_SENSOR_TEST_CASE = 34,
    PL_CMD_SENSOR_SET_SAMPLE_RATE_50Hz = 35,
    PL_CMD_SENSOR_SET_SAMPLE_RATE_100Hz = 36,
#endif
    PL_CMD_SEND_TEST_ASSERT_SERVICE_MESSAGE = 37,
    PL_CMD_SEND_TEST_CRITICAL_SERVICE_MESSAGE = 38,
    PL_CMD_QUERY_PAIR_COMMAND_LISTEN_LIST = 100,
    PL_CMD_LOC_ENABLE_LOCATION_DEBUG_OUTPUT = 101,
    PL_CMD_TEST_SPI_Q_RX_BUFFER_SIZE = 600,
    PL_CMD_TEST_SPI_Q_TX_BUFFER_SIZE = 601,
    PL_CMD_TEST_SPI_Q_ALL_TX_BUFFER = 602,
    PL_CMD_TEST_ASSERT = 666,
    PL_CMD_TEST_MOCK_SEND_COMMAND_AIDING_EPO = 667,
    /**
     * @brief Get Last Location
     *
     *
     * @code
     * send: PANL700
     * response: PANL700,<fix_quality>,<latitude>,<longitude>,<altitude>
     * <fix_quality>:
     *      0: no fix
     *      1: sta
     *      2:
     * <value>:
     *      latitude: WGS84 latitude
     *      longitude: WGS84 Longitude
     *      Altitude: WGS84 Height
     *
     * @endcode
     *
     * @note Please refer to this value only when clock_drift_valid is 1
     */
    PL_CMD_INST_GET_LAST_LOCATION = 700,
    /**
     * @brief Get Last Clock Drift
     * Platform can send this command to query last clock drift
     *
     * @code
     * send: PANL701
     * response: PANL701,<fix_quality>,<value>
     * <fix_quality>:
     *      0: invalid
     *      1 or 2: valid
     *      other: to be check
     * <value>:
     *      Clock Drift Value
     * @endcode
     *
     * @note Please refer to this value only when clock_drift_valid is 1
     */
    PL_CMD_INST_GET_LAST_CLOCK_DRIFT = 701,
    PL_CMD_CONSOLE_BASE = 800,
    /**
     * @brief Switch on console
     *
     *
     * @code
     * send: PANL801
     * response: <TEXT>
     * @endcode
     *
     */
    PL_CMD_CONSOLE_SWITCH_ON = 801,
    /**
     * @brief Switch off console
     *
     *
     * @code
     * send: PANL802
     * response: <TEXT>
     * @endcode
     *
     */
    PL_CMD_CONSOLE_SWITCH_OFF = 802,
    /**
     * @brief Dump EPO Download State
     *
     *
     * @code
     * send: PANL801
     * response: <TEXT>
     * @endcode
     *
     */
    PL_CMD_CONSOLE_GET_3DAY_EPO_STATE = 803,
    /**
     * @brief Trigger 3-day epo check. It will try to re-download 3-day epo if
     * last download is failed.
     *
     *
     * @code
     * send: PANL802
     * response: <TEXT>
     * @endcode
     *
     */
    PL_CMD_CONSOLE_TRIGGER_3DAYS_EPO_CHECK = 804,
    PL_CMD_ENABLE_NETWORK_LOW_SPEED = 805,
    PL_CMD_DISABLE_NETWORK_LOW_SPEED = 806,
    PL_CMD_CLEAR_NET_DL_CACHE = 807,
    PL_CMD_TRIGGER_NETWORK_CONNECTED = 808,
    PL_CMD_FACTORY_SET_SILENT = 900,
    PL_CMD_FACTORY_UNSET_SILENT = 901,
};
struct SimpleCommandHandler {
 public:
    SimpleCommandHandler();
    SpiVirtualDriver *mSpiInst;
    bool handleMessage(const SimpleCommand &cmd, std::string *rsp);
 private:
    void relayFileLogToService(const char *filename);
    void mockSendCommand(const char *cmd);
#ifdef SUPPORT_SENSOR_FEATURE
    airoha::communicator::Sensor *mSensorInst;
#endif
};

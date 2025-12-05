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
#ifdef SUPPORT_SENSOR_FEATURE
#ifndef AIROHA_SIMULATION
#define LOG_TAG "Sensor"
#include "sensor.h"
#include <android/looper.h>
#include <android/sensor.h>
#include <inttypes.h>
#include <string.h>
#include "simulation.h"
using airoha::communicator::Sensor;
#define CUSTOMER_ACC ASENSOR_TYPE_ACCELEROMETER_UNCALIBRATED
#define CUSTOMER_GYRO ASENSOR_TYPE_GYROSCOPE_UNCALIBRATED
#define CUSTOMER_BARO ASENSOR_TYPE_PRESSURE
#define SENSOR_LOG_FREQ 100
const char* Sensor::kDefaultPackageName = "Airoha-Sensor";
int Sensor::sDebugSampleHz = -1;
int Sensor::sDebugJiffer = -1;
// static int looperCallback(int fd, int events, void* data) {
//     LOG_D("");
//     retu
// }
Sensor::Sensor() {
    mSampleRateMicroSecond = kMicroSecondPerSecond;
    mSampleRateHz = 1;
    mIsSensorRunning = false;
    mEventQueue = nullptr;
    mSensorAcc = nullptr;
    mSensorGyro = nullptr;
    mIsThreadRunning = false;
    mSensorList = nullptr;
    mSensorCount = 0;
    mLooper = nullptr;
    mTimestampJitterNs = 0;
    memset(mPackageName, 0, sizeof(mPackageName));
    strcat(mPackageName, kDefaultPackageName);
}
Sensor::Sensor(const char* packageName) {
    mSampleRateMicroSecond = kMicroSecondPerSecond;
    mSampleRateHz = 1;
    mIsSensorRunning = false;
    mEventQueue = nullptr;
    mSensorAcc = nullptr;
    mSensorGyro = nullptr;
    mIsThreadRunning = false;
    mSensorList = nullptr;
    mSensorCount = 0;
    mLooper = nullptr;
    mTimestampJitterNs = 0;
    memset(mPackageName, 0, sizeof(mPackageName));
    strcat(mPackageName, kDefaultPackageName);
    strcat(mPackageName, packageName);
}
Sensor::~Sensor() {
    if (mIsSensorRunning) {
        stop();
    }
}
bool Sensor::setSampleRateHz(int hz) {
    if (kMicroSecondPerSecond % hz != 0) {
        LOG_E("sample rate hz error: %d", hz);
        return false;
    }
    mSampleRateHz = hz;
    mSampleRateMicroSecond = kMicroSecondPerSecond / hz;
    return true;
}
bool Sensor::setBarometerSampleRate(int microsecond) {
    mSampleRateBarometerMicroSecond = microsecond;
    return true;
}
void Sensor::setJitter(int microSecond) { mTimestampJitterNs = microSecond * 1000; }
void Sensor::TEST(int64_t second) {
    LOG_D("======= SENSOR_TEST =======");
    ASensorManager* sensorMgrInstance =
        ASensorManager_getInstanceForPackage(kDefaultPackageName);
    if (!sensorMgrInstance) {
        LOG_E("Could not get sensor instance");
        return;
    }
    ASensorList sensorList = nullptr;
    int sensorCount =
        ASensorManager_getSensorList(sensorMgrInstance, &sensorList);
    LOG_D("Find %d support sensors", sensorCount);
    const int kLooperId = 1;
    ASensorEventQueue* queue = ASensorManager_createEventQueue(
        sensorMgrInstance, ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS),
        kLooperId, NULL, /* no callback */
        NULL /* no private data for a callback  */);
    if (!queue) {
        fprintf(stderr, "Failed to create a sensor event queue");
        return;
    }
    // Find the first sensor of the specified type that can be opened
    const int kTimeoutMicroSecs = 10000;
    const int kTimeoutMilliSecs = 1000;
    ASensorRef sensorAcc = nullptr;
    ASensorRef sensorGyr = nullptr;
    ASensorRef sensor = nullptr;
    bool sensor_found = false;
    for (int i = 0; i < sensorCount; i++) {
        sensor = sensorList[i];
        LOG_D("Sensor Type: %d, %s", ASensor_getType(sensor),
              ASensor_getName(sensor));
        if (ASensor_getType(sensor) != CUSTOMER_ACC &&
            ASensor_getType(sensor) != CUSTOMER_GYRO)
            continue;
        if (ASensorEventQueue_enableSensor(queue, sensor) < 0) continue;
        if (ASensorEventQueue_setEventRate(queue, sensor, kTimeoutMicroSecs) <
            0) {
            LOG_E("Failed to set the %s sample rate\n",
                  ASensor_getName(sensor));
            return;
        }
        if (ASensor_getType(sensor) == CUSTOMER_ACC) {
            sensorAcc = sensor;
        }
        if (ASensor_getType(sensor) == CUSTOMER_GYRO) {
            sensorGyr = sensor;
        }
        // Found an equipped sensor of the specified type.
        sensor_found = true;
    }
    if (!sensor_found) {
        LOG_E("No sensor of the specified type found\n");
        int ret = ASensorManager_destroyEventQueue(sensorMgrInstance, queue);
        if (ret < 0)
            LOG_E("Failed to destroy event queue: %s\n", strerror(-ret));
        return;
    }
    printf("\nSensor %s activated\n", ASensor_getName(sensor));
    // Init file
    FILE* fileAcc = fopen("/data/vendor/airoha/acc.csv", "wb+");
    FILE* fileGyro = fopen("/data/vendor/airoha/gyro.csv", "wb+");
    if (!(fileAcc && fileGyro)) {
        LOG_E("open file error: %p, %p", fileAcc, fileGyro);
        return;
    }
    const char* accHeader = "timestamp,acc_x,acc_y,acc_z\n";
    fwrite(accHeader, 1, strlen(accHeader), fileAcc);
    const char* gyroHeader =
        "timestamp,gyro_x,gyro_y,gyro_z,gyro_pitch,gyro_roll,gyro_azimuth\n";
    fwrite(gyroHeader, 1, strlen(gyroHeader), fileGyro);
    const int kNumEvents = 10;
    const int kNumSamples = second * 1000 / 10 * 2;
    for (int i = 0; i < kNumSamples; i++) {
        ASensorEvent data[kNumEvents];
        memset(data, 0, sizeof(data));
        int ident = ALooper_pollOnce(
            kTimeoutMilliSecs, NULL /* no output file descriptor */,
            NULL /* no output event */, NULL /* no output data */);
        // if (ident != kLooperId) {
        //     LOG_E("Incorrect Looper ident read from poll. %d, %d\n", ident,
        //           kLooperId);
        //     continue;
        // }
        if (ident != ALOOPER_POLL_CALLBACK) {
            continue;
        }
        // LOG_D("Looper ident read from poll. %d\n", ident);
        int eventCount = ASensorEventQueue_getEvents(queue, data, kNumEvents);
        if (eventCount <= 0) {
            LOG_E("Failed to read data from the sensor.");
            continue;
        }
        // LOG_D("receive event: %d", eventCount);
        char buf[512] = {0};
        const char* formatAcc = "%" PRId64 ",%f,%f,%f\n";
        const char* formatGyro = "%" PRId64 ",%f,%f,%f,%f,%f,%f\n";
        for (int j = 0; j < eventCount; j++) {
            switch (data[j].type) {
                case ASENSOR_TYPE_ACCELEROMETER: {
                    LOG_D("Acceleration[%d][%" PRId64
                          "]: x = %f, y = %f, z = %f",
                          i, data[j].timestamp, data[j].acceleration.x,
                          data[j].acceleration.y, data[j].acceleration.z);
                    int ret =
                        snprintf(buf, sizeof(buf), formatAcc, data[j].timestamp,
                                 data[j].acceleration.x, data[j].acceleration.y,
                                 data[j].acceleration.z);
                    fwrite(buf, 1, ret, fileAcc);
                    break;
                }
                case ASENSOR_TYPE_GYROSCOPE: {
                    LOG_D("GYR[%d][%" PRId64
                          "]: x=%f, y=%f, z=%f, x_b=%f, y_b=%f, z_b=%f",
                          i, data[j].timestamp,
                          data[j].uncalibrated_gyro.x_uncalib,
                          data[j].uncalibrated_gyro.y_uncalib,
                          data[j].uncalibrated_gyro.z_uncalib,
                          data[j].uncalibrated_gyro.x_bias,
                          data[j].uncalibrated_gyro.y_bias,
                          data[j].uncalibrated_gyro.z_bias);
                    int ret = snprintf(buf, sizeof(buf), formatGyro,
                                       data[j].timestamp,
                                       data[j].uncalibrated_gyro.x_uncalib,
                                       data[j].uncalibrated_gyro.y_uncalib,
                                       data[j].uncalibrated_gyro.z_uncalib,
                                       data[j].uncalibrated_gyro.x_bias,
                                       data[j].uncalibrated_gyro.y_bias,
                                       data[j].uncalibrated_gyro.z_bias);
                    fwrite(buf, 1, ret, fileGyro);
                    break;
                }
            }
        }
        // DisplaySensorData(ASENSOR_TYPE_ACCELEROMETER, data);
    }
    fclose(fileAcc);
    fclose(fileGyro);
    int ret = ASensorEventQueue_disableSensor(queue, sensorAcc);
    if (ret < 0) {
        LOG_E("Failed to disable %s: %s\n", ASensor_getName(sensor),
              strerror(-ret));
    }
    ret = ASensorEventQueue_disableSensor(queue, sensorGyr);
    if (ret < 0) {
        LOG_E("Failed to disable %s: %s\n", ASensor_getName(sensor),
              strerror(-ret));
    }
    ret = ASensorManager_destroyEventQueue(sensorMgrInstance, queue);
    if (ret < 0) {
        LOG_E("Failed to destroy event queue: %s\n", strerror(-ret));
        return;
    }
    return;
}
bool Sensor::setDebugSampleRateHz(int hz) {
    sDebugSampleHz = hz;
    return true;
}
bool Sensor::sensorSetup() {
    LOG_D("environment setup with package: %s", mPackageName);
    mSensorMgrInstance = ASensorManager_getInstanceForPackage(mPackageName);
    if (!mSensorMgrInstance) {
        LOG_E("Could not get sensor instance");
        return false;
    }
    mSensorCount =
        ASensorManager_getSensorList(mSensorMgrInstance, &mSensorList);
    LOG_D("Find %d support sensors", mSensorCount);
    const int kLooperId = 1;
    mLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    mEventQueue = ASensorManager_createEventQueue(
        mSensorMgrInstance, mLooper, kLooperId, NULL, /* no callback */
        NULL /* no private data for a callback  */);
    if (!mEventQueue) {
        LOG_E("Failed to create a sensor event queue");
        return false;
    }
    return true;
}
bool Sensor::enableSensor() {
    bool sensorFound = true;
    ASensorRef sensor = nullptr;
    if (sDebugJiffer > 0) {
        setJitter(sDebugJiffer);
    }
    if (sDebugSampleHz > 0) {
        LOG_D("Sendor Sample Rate Debug Mode: %d", sDebugSampleHz);
        setSampleRateHz(sDebugSampleHz);
    }
    LOG_D("enable sensor: rate(%d), microSec(%d)", mSampleRateHz, mSampleRateMicroSecond);
    for (int i = 0; i < mSensorCount; i++) {
        sensor = mSensorList[i];
        LOG_D("Sensor Type: %d, %s", ASensor_getType(sensor),
              ASensor_getName(sensor));
        if (ASensor_getType(sensor) == CUSTOMER_ACC &&
            (mEnableSensors & SENSOR_TYPE_ACCELEROMETER)) {
            mSensorAcc = sensor;
        }
        if (ASensor_getType(sensor) == CUSTOMER_GYRO &&
            (mEnableSensors & SENSOR_TYPE_GYROSCOPE)) {
            mSensorGyro = sensor;
        }
        if (ASensor_getType(sensor) == CUSTOMER_BARO &&
            (mEnableSensors & SENSOR_TYPE_BAROMETER)) {
            mSensorBaro = sensor;
        }
        // Found an equipped sensor of the specified type.
        sensorFound = true;
    }
    if ((mEnableSensors & SENSOR_TYPE_ACCELEROMETER) && mSensorAcc == nullptr) {
        LOG_E("enable acc sensor failed");
        return false;
    }
    if ((mEnableSensors & SENSOR_TYPE_GYROSCOPE) && mSensorGyro == nullptr) {
        LOG_E("enable gyro sensor failed");
        return false;
    }
    if ((mEnableSensors & SENSOR_TYPE_BAROMETER) && mSensorBaro == nullptr) {
        LOG_E("enable baro sensor failed");
        return false;
    }
    if (mEnableSensors & SENSOR_TYPE_ACCELEROMETER) {
        if (ASensorEventQueue_enableSensor(mEventQueue, mSensorAcc) < 0) {
            LOG_E("Failed to enable %s \n", ASensor_getName(mSensorAcc));
        }
        if (ASensorEventQueue_setEventRate(mEventQueue, mSensorAcc,
                                           mSampleRateMicroSecond) < 0) {
            LOG_E("Failed to set the %s sample rate\n",
                  ASensor_getName(mSensorAcc));
            return false;
        }
    }
    if (mEnableSensors & SENSOR_TYPE_GYROSCOPE) {
        if (ASensorEventQueue_enableSensor(mEventQueue, mSensorGyro) < 0) {
            LOG_E("Failed to enable %s \n", ASensor_getName(mSensorGyro));
        }
        if (ASensorEventQueue_setEventRate(mEventQueue, mSensorGyro,
                                           mSampleRateMicroSecond) < 0) {
            LOG_E("Failed to set the %s sample rate\n",
                  ASensor_getName(mSensorGyro));
            return false;
        }
    }
    if (mEnableSensors & SENSOR_TYPE_BAROMETER) {
        if (ASensorEventQueue_enableSensor(mEventQueue, mSensorBaro) < 0) {
            LOG_E("Failed to enable %s \n", ASensor_getName(mSensorBaro));
        }
        if (ASensorEventQueue_setEventRate(mEventQueue, mSensorBaro,
                                           mSampleRateBarometerMicroSecond) <
            0) {
            LOG_E("Failed to set the %s sample rate\n",
                  ASensor_getName(mSensorBaro));
            return false;
        }
    }
    if (!sensorFound) {
        LOG_E("No sensor of the specified type found\n");
        int ret =
            ASensorManager_destroyEventQueue(mSensorMgrInstance, mEventQueue);
        mEventQueue = nullptr;
        if (ret < 0)
            LOG_E("Failed to destroy event queue: %s\n", strerror(-ret));
        return false;
    }
    return true;
}
bool Sensor::disableSensor() {
    int ret = 0;
    if (mSensorAcc) {
        ret = ASensorEventQueue_disableSensor(mEventQueue, mSensorAcc);
        if (ret < 0) {
            LOG_E("Failed to disable %s: %s\n", ASensor_getName(mSensorAcc),
                  strerror(-ret));
        }
        mSensorAcc = nullptr;
    }
    if (mSensorGyro) {
        ret = ASensorEventQueue_disableSensor(mEventQueue, mSensorGyro);
        if (ret < 0) {
            LOG_E("Failed to disable %s: %s\n", ASensor_getName(mSensorGyro),
                  strerror(-ret));
        }
        mSensorGyro = nullptr;
    }
    if (mSensorBaro) {
        ret = ASensorEventQueue_disableSensor(mEventQueue, mSensorBaro);
        if (ret < 0) {
            LOG_E("Failed to disable %s: %s\n", ASensor_getName(mSensorBaro),
                  strerror(-ret));
        }
        mSensorBaro = nullptr;
    }
    ret = ASensorManager_destroyEventQueue(mSensorMgrInstance, mEventQueue);
    if (ret < 0) {
        LOG_E("Failed to destroy event queue: %s\n", strerror(-ret));
    }
    return true;
}
bool Sensor::start() {
    bool ret = true;
    LOG_D("setup environment:%d", mIsSensorRunning);
    if (mIsSensorRunning) {
        LOG_W("duplicate start sensor");
        ret = false;
    }
    if (ret) {
        ret = sensorSetup();
    }
    int pthreadRet = 0;
    if (ret) {
        clearImuCache();
        mIsThreadRunning = true;
        // Create Thread
        pthreadRet =
            pthread_create(&mCallbackThread, NULL, dataPollingThread, this);
    } else {
        LOG_E("sensor setup error");
    }
    if (pthreadRet != 0) {
        LOG_E("create thread fail");
        ret = false;
    }
    if (ret) {
        mIsSensorRunning = true;
    }
    return ret;
}
bool Sensor::stop() {
    bool ret = true;
    if (!mIsSensorRunning) {
        LOG_E("sensor not runnning");
        ret = false;
    }
    LOG_D("stop sensor");
    if (ret) {
        mIsThreadRunning = false;
        ALooper_wake(mLooper);
        pthread_join(mCallbackThread, nullptr);
        mIsSensorRunning = false;
        LOG_D("stop sensor ok");
    }
    mLooper = nullptr;
    mEnableSensors = 0;
    mLastBarometerTimestamp = -1;
    return ret;
}
void Sensor::enableSensorType(SensorTypes types) { mEnableSensors = types; }
void* Sensor::dataPollingThread(void* argv) {
    Sensor* instance = (Sensor*)argv;
    LOG_D("data polling thread run: %p", argv);
    instance->enableSensor();
    while (instance->mIsThreadRunning) {
        instance->pollingData();
    }
    instance->disableSensor();
    LOG_D("data polling thread exit: %p", argv);
    return nullptr;
}
int maxWait = 10;
Sensor::PollingDataResult Sensor::pollingData() {
    const int kNumEvents = 5;
    const int kTimeoutMilliSecs = 1000;
    ASensorEvent data[kNumEvents];
    memset(data, 0, sizeof(data));
    int ident = ALooper_pollOnce(
        kTimeoutMilliSecs, NULL /* no output file descriptor */,
        NULL /* no output event */, NULL /* no output data */);
    // if (ident != kLooperId) {
    //     LOG_E("Incorrect Looper ident read from poll. %d, %d\n", ident,
    //           kLooperId);
    //     continue;
    // }
    if (maxWait == 0) {
        TRACE_D("Looper ident=%d\n", ident);
        maxWait = 10;
    } else {
        maxWait--;
    }
    if (ident == ALOOPER_POLL_TIMEOUT) {
        return POLLING_RESULT_NO_DATA;
    }
    if (ident == ALOOPER_POLL_WAKE) {
        return POLLING_RESULT_WAKEUP;
    }
    if (ident != ALOOPER_POLL_CALLBACK) {
        return POLLING_RESULT_NO_DATA;
    }
    int eventCount = ASensorEventQueue_getEvents(mEventQueue, data, kNumEvents);
    if (eventCount <= 0) {
        // LOG_E("Failed to read data from the sensor.");
        return POLLING_RESULT_NO_DATA;
    }
    // LOG_D("receive event: %d", eventCount);
    for (int j = 0; j < eventCount; j++) {
        switch (data[j].type) {
            case CUSTOMER_ACC: {
                updateAcc(data[j].timestamp, data[j].acceleration.x,
                          data[j].acceleration.y, data[j].acceleration.z);
                break;
            }
            case CUSTOMER_GYRO: {
                float xUnCali = data[j].uncalibrated_gyro.x_uncalib;
                float yUnCali = data[j].uncalibrated_gyro.y_uncalib;
                float zUnCali = data[j].uncalibrated_gyro.z_uncalib;
                updateGyro(data[j].timestamp, xUnCali, yUnCali, zUnCali);
                break;
            }
            case CUSTOMER_BARO: {
                updateBaro(data[j].timestamp, data[j].pressure);
                break;
            }
        }
    }
    return POLLING_RESULT_OK;
    // DisplaySensorData(ASENSOR_TYPE_ACCELEROMETER, data);
}
void Sensor::clearImuCache() {
    mImuData.timestamp = 0;
    mImuDataQueue.clear();
}
void Sensor::updateAcc(int64_t timestamp, float x, float y, float z) {
    static uint8_t cnt = 0;
    if ((cnt++ % SENSOR_LOG_FREQ) == 0) {
        TRACE_D("ACC: %" PRId64 ", x=%f,y=%f,z=%f", timestamp, x, y, z);
        LOG_D("ACC: %" PRId64 ", x=%f,y=%f,z=%f", timestamp, x, y, z);
    }
    cnt %= SENSOR_LOG_FREQ;
    StandaloneImuData imuData;
    imuData.timestamp = timestamp;
    imuData.acc.x = x;
    imuData.acc.y = y;
    imuData.acc.z = z;
    imuData.type = StandaloneImuData::DATA_TYPE_ACCELERATION;
    pushImuData(imuData);
}
void Sensor::updateGyro(int64_t timestamp, float x, float y, float z) {
    static uint8_t cnt = 0;
    if ((cnt++ % SENSOR_LOG_FREQ) == 0) {
        TRACE_D("Gyro: %" PRId64 ", x=%f,y=%f,z=%f", timestamp, x, y, z);
        LOG_D("Gyro: %" PRId64 ", x=%f,y=%f,z=%f", timestamp, x, y, z);
    }
    cnt %= SENSOR_LOG_FREQ;
    StandaloneImuData imuData;
    imuData.timestamp = timestamp;
    imuData.gyro.x = x;
    imuData.gyro.y = y;
    imuData.gyro.z = z;
    imuData.type = StandaloneImuData::DATA_TYPE_GYROSCOPE;
    pushImuData(imuData);
}
void Sensor::updateBaro(int64_t timestamp, float baro) {
    static uint8_t cnt = 0;
    if ((cnt++ % SENSOR_LOG_FREQ) == 0) {
        TRACE_D("Barometer: %" PRId64 ", baro=%f", timestamp, baro);
        LOG_D("Barometer: %" PRId64 ", baro=%f", timestamp, baro);
    }
    cnt %= SENSOR_LOG_FREQ;
    StandaloneImuData imuData;
    imuData.timestamp = timestamp;
    imuData.pressure = baro;
    if (mLastBarometerTimestamp == -1 ||
        (timestamp - mLastBarometerTimestamp >
         mSampleRateBarometerMicroSecond * 0.75)) {
        onBarometerData(imuData);
        mLastBarometerTimestamp = timestamp;
    }
}
void Sensor::pushImuData(const StandaloneImuData& data) {
    if (mImuDataQueue.size() == 0) {
        mImuDataQueue.push_back(data);
        return;
    }
    int64_t kMaxInvalidDelta = 20 * 1000000000LL;
    if (absTimestampDelta(data.timestamp, mImuDataQueue.back().timestamp) >
        kMaxInvalidDelta) {
        mImuDataQueue.clear();
        mImuDataQueue.push_back(data);
        return;
    }
    std::list<StandaloneImuData>::iterator it;
    for (it = mImuDataQueue.begin(); it != mImuDataQueue.end(); it++) {
        if (it->timestamp > data.timestamp) break;
    }
    mImuDataQueue.insert(it, data);
    while(checkAndCallback())
        ;
}
bool Sensor::checkAndCallback() {
    // we allow 50 data cache in queue, if exceed, pop old data from front.
    const size_t kMaxAllowDataInQueue = 50;
    TRACE_D("IMUQ: %zu", mImuDataQueue.size());
    if (mImuDataQueue.size() == 0) {
        return false;
    }
    while (mImuDataQueue.size() > kMaxAllowDataInQueue) {
        mImuDataQueue.pop_front();
    }
    // Find a GYRO and ACC
    std::list<StandaloneImuData>::iterator it;
    std::list<StandaloneImuData>::iterator
        dataIt[StandaloneImuData::DATA_TYPE_MAX] = {mImuDataQueue.end(),
                                                    mImuDataQueue.end()};
    bool found = false;
    for (it = mImuDataQueue.begin(); it != mImuDataQueue.end(); it++) {
        dataIt[it->type] = it;
        if (dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION] ==
            mImuDataQueue.end()) {
            continue;
        }
        if (dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE] ==
            mImuDataQueue.end()) {
            continue;
        }
        if (absTimestampDelta(
                dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION]->timestamp,
                dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE]->timestamp) <
            mTimestampJitterNs) {
            found = true;
            break;
        }
    }
    if (found == true) {
        MergedImuData imuData;
        imuData.timestamp =
            max(dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION]->timestamp,
                dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE]->timestamp);
        imuData.accX = dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION]->acc.x;
        imuData.accY = dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION]->acc.y;
        imuData.accZ = dataIt[StandaloneImuData::DATA_TYPE_ACCELERATION]->acc.z;
        imuData.gyroX = dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE]->gyro.x;
        imuData.gyroY = dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE]->gyro.y;
        imuData.gyroZ = dataIt[StandaloneImuData::DATA_TYPE_GYROSCOPE]->gyro.z;
        onImuData(imuData);
        it++;
        mImuDataQueue.erase(mImuDataQueue.begin(), it);
        return true;
    }
    return false;
}
void Sensor::onImuData(const MergedImuData& imuData) {
    TRACE_D("Sensor Data: [%" PRId64
          "](ms) %f,%f,%f,%f,%f,%f",
          imuData.timestamp / 1000000, imuData.accX, imuData.accY, imuData.accZ,
          imuData.gyroX, imuData.gyroY, imuData.gyroZ);
}
void Sensor::onBarometerData(const StandaloneImuData& barometerData) {
    (void)barometerData;
}
int64_t Sensor::absTimestampDelta(int64_t t1, int64_t t2) {
    return (t1 > t2) ? (t1 - t2) : (t2 - t1);
}
#endif
#endif  // #ifdef SUPPORT_SENSOR_FEATURE

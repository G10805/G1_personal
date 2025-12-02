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
#pragma once
#include <stdint.h>
#include <pthread.h>
#include <list>
/* NOTE: changes to these structs have to be backward compatible */
#include <android/sensor.h>
namespace airoha {
namespace communicator {
struct MergedImuData {
    int64_t timestamp;
    float accX;
    float accY;
    float accZ;
    float gyroX;
    float gyroY;
    float gyroZ;
};
struct SensorDataVector {
    float x;
    float y;
    float z;
};
struct StandaloneImuData {
    enum DataType {
        DATA_TYPE_ACCELERATION,
        DATA_TYPE_GYROSCOPE,
        DATA_TYPE_MAX,
    };
    union {
        SensorDataVector acc;
        SensorDataVector gyro;
        float pressure;
    };
    int type;
    int64_t timestamp;
    int64_t timestampSystemNano;
};
class Sensor {
 public:
    enum SensorType : uint8_t {
        SENSOR_TYPE_ACCELEROMETER = 1,
        SENSOR_TYPE_GYROSCOPE = 2,
        SENSOR_TYPE_BAROMETER = 4
    };
    typedef uint8_t SensorTypes;
    bool setSampleRateHz(int hz);
    bool setBarometerSampleRate(int microsecond);
    void setJitter(int microSecond);
    Sensor(const char* packageName);
    Sensor();
    virtual ~Sensor();
    bool start();
    bool stop();
    void enableSensorType(SensorTypes types);
    virtual void onImuData(const MergedImuData& imuData);
    virtual void onBarometerData(const StandaloneImuData& barometerData);
    static void TEST(int64_t second);
    static bool setDebugSampleRateHz(int hz);

 private:
    static const char* kDefaultPackageName;
    char mPackageName[64];
    MergedImuData mImuData;
    ASensorList mSensorList;
    ALooper* mLooper;
    int mSensorCount;
    int mSampleRateHz;
    int mSampleRateMicroSecond;
    int mSampleRateBarometerMicroSecond = 80000;
    int mTimestampJitterNs;
    const int kMicroSecondPerSecond = 1000000;
    bool mIsSensorRunning;
    bool mIsThreadRunning;
    ASensorEventQueue* mEventQueue;
    ASensorRef mSensorAcc = nullptr;
    ASensorRef mSensorGyro = nullptr;
    ASensorRef mSensorBaro = nullptr;
    ASensorManager* mSensorMgrInstance;
    pthread_t mCallbackThread;
    static void* dataPollingThread(void* argv);
    enum PollingDataResult {
        POLLING_RESULT_OK,
        POLLING_RESULT_NO_DATA,
        POLLING_RESULT_WAKEUP,
    };
    PollingDataResult pollingData();
    void clearImuCache();
    void updateAcc(int64_t timestamp, float x, float y, float z);
    void updateGyro(int64_t timestamp, float x, float y, float z);
    void updateBaro(int64_t timestamp, float baro);
    void pushImuData(const StandaloneImuData&);
    bool checkAndCallback();
    bool sensorSetup();
    bool disableSensor();
    bool enableSensor();
    int64_t absTimestampDelta(int64_t t1, int64_t t2);
    constexpr int64_t max(int64_t t1, int64_t t2) {
        return (t1 > t2) ? t1 : t2;
    }
    std::list<StandaloneImuData> mImuDataQueue;
    static int sDebugSampleHz;
    static int sDebugJiffer;
    SensorTypes mEnableSensors = 0;
    int64_t mLastBarometerTimestamp = -1;
};
}  // namespace communicator
}  // namespace airoha
#endif
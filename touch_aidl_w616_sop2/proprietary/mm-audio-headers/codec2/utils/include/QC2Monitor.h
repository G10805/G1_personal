/*
 **************************************************************************************************
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_MONITOR_H_
#define _QC2AUDIO_MONITOR_H_

#include "QC2Thread.h"

namespace qc2audio {

class QC2MonitorThread;

class QC2Monitor {

public:
    enum MonitorEvent : uint32_t {
        EVT_TIMEOUT = 1,
        EVT_ACTION = 2,
    };

    typedef QC2Status (*clientCallback_t)(uint32_t event, void *client, bool *wait);
    static constexpr uint32_t kDefaultTimeOutMs = 5000;

    QC2Monitor(const std::string name,
               clientCallback_t cbf = nullptr,
               void *client = nullptr,
               uint32_t timeoutMs = kDefaultTimeOutMs)
            : mCallback(cbf),
            mClient(client),
            mInstName(name),
            mTimeOutMs(timeoutMs) { };
    virtual ~QC2Monitor();

    QC2Status create();
    QC2Status reset();
    QC2Status stop();
    QC2Status notify();
    uint32_t getState() { return mState; }

    struct MonitorStates {
        static constexpr const uint32_t IDLE = 0x1;
        static constexpr const uint32_t RUNNING = 0x2;
        static constexpr const uint32_t TIMED_OUT = 0x4;
        static constexpr const uint32_t ACTING = 0x8;
        static const char* Str(uint32_t);
    };
    bool isIdle() { return mState == MonitorStates::IDLE; }
    bool isRunning() { return mState == MonitorStates::RUNNING; }
    bool isActing() { return mState == MonitorStates::ACTING; }
    bool isTimedOut() { return mState == MonitorStates::TIMED_OUT; }

protected:
    void setState(uint32_t state) { mState = state; }

    class QC2MonitorThread : public QC2Thread {
    public:
        QC2MonitorThread(const std::string name,
                         QC2Monitor *monitor)
            : QC2Thread("Monitor_"+ name),
              mMonitor(*monitor) { }
        virtual ~QC2MonitorThread();
        virtual void reset();
        virtual void stop();
        virtual void notify();
    private:
        void threadLoop();
        std::mutex mLock;
        std::condition_variable mCondition;
        QC2Monitor& mMonitor;
    };

    clientCallback_t mCallback = nullptr;
    void *mClient = nullptr;
    std::string mInstName;
    uint32_t mTimeOutMs;
    std::atomic<uint32_t> mState;
    std::unique_ptr<QC2MonitorThread> mMonitorThread = nullptr;
};

};  // namespace qc2audio

#endif  // _QC2AUDIO_MONITOR_H_

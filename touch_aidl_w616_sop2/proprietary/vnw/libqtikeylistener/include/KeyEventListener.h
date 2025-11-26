/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef KEY_EVENT_LISTENER_H
#define KEY_EVENT_LISTENER_H
#include <linux/input.h>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace android {
namespace qti {
namespace testing {
class KeyEventListenerTest;
}
class KeyEventCallback {
   public:
    virtual ~KeyEventCallback();
    virtual void onKeyPressed(int keyCode, uint64_t durationMs) = 0;
};
class KeyEventListener {
   public:
    /**
     * Default constructor for KeyEventListener.
     * It initializes KeyEventListener instance with autoStartStop set to true.
     */
    KeyEventListener() : KeyEventListener(true){};
    /**
     * Overloaded constructor for KeyEventListener.
     * @param autoStartStop: A boolean value to control the automatic start and
     * stop of the monitor and handler threads.
     */
    KeyEventListener(bool autoStartStop);
    ~KeyEventListener() {
        stopListener();
        mLastKeyInfoStore.clear();
        mKeyCodeClientsMap.clear();
    }
    /**
     * Starts the monitor thread if not already started by autoStartStop.
     * @return status_t: OK on success.
     */
    status_t startListener();
    /**
     * Stops the monitor thread.
     * @return status_t: OK on success.
     */
    status_t stopListener();
    /**
     * Registers a key event listener for a specific key code.
     * @param keyCode: The key code to listen for.
     * @param callback: The callback function to be called when the key event
     * occurs.
     * @return status_t: return OK on success
     */
    status_t registerKeyEventListener(
        int keyCode, std::shared_ptr<KeyEventCallback> callback);
    /**
     * Unregisters a key event listener for a specific key code.
     * @param keyCode: The key code to stop listening for.
     * @param callback: The callback function to be removed from the listeners.
     * @return status_t: return OK on success.
     */
    status_t unregisterKeyEventListener(
        int keyCode, std::shared_ptr<KeyEventCallback> callback);
    /**
     * Triggers a mock key event.
     * This function is used for testing purposes to simulate key events.
     * @param ev: A reference to an input_event structure that contains the key
     * event data.
     */
    void mockKeyTrigger(struct input_event &ev);
   private:
    int openNode();
    void monitorThread();
    void processQueue();
    friend class testing::KeyEventListenerTest;
   private:
    std::mutex mLock;
    std::condition_variable mEventQueueWait;
    std::map<int, std::vector<std::shared_ptr<KeyEventCallback>>>
        mKeyCodeClientsMap;
    std::queue<struct input_event> mEventQueue;
    std::map<int, struct input_event> mLastKeyInfoStore;
    std::atomic<bool> mRequestExit;
    bool mStarted;
    bool mAutoStartStop;
    int mExitMonitorFd[2];
    std::thread mMoniterThread, mProcessThread;
};
}  // namespace qti
}  // namespace android
#endif

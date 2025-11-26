/*
 **************************************************************************************************
 * Copyright (c) 2020-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2AUDIO_UTILS_EVENTQUEUE_H_
#define _QC2AUDIO_UTILS_EVENTQUEUE_H_

#include <thread>
#include <memory>
#include <list>
#include <mutex>
#include <condition_variable>
#include <limits>

#include "QC2Event.h"

namespace qc2audio {

/// @addtogroup utils Utilities
/// @{

class EventQueue;
/**
 * @brief Interface which handles events and to which events are posted
 *
 * An EventHandler works in conjunction with an EventQueue\n
 * Events posted to the handler are added to the EventQueue (in the caller's context)\n
 * Events are handled (based on added order and priority) in the EventQueue's thread-context\n
 * Events can be posted asynchronously - in which case, the caller does not wait for the response\n
 * Events can be posted synchronously - in which case, the caller is blocked until the event is
 * handled and replied\n
 */
class EventHandler : public virtual std::enable_shared_from_this<EventHandler> {
 public:
    EventHandler();
    virtual ~EventHandler() = default;

    /**
     * @brief Associate the handler with an EventQueue
     *
     * Sets the backing EventQueue. Can be set only once.
     * @param[in] eventQueue reference to the backing event-queue
     * @return QC2_REFUSED attempt to update an already attached event-queue
     * @return QC2_BAD_VALUE event-queue reference is invalid
     * @return QC2_OK event-queue was attached successfully
     */
    QC2Status attach(std::shared_ptr<EventQueue> eventQueue);

    /**
     * @brief post an event asynchronously
     *
     * post an Event without waiting for the response
     * @param[in] event event to be posted
     * @return QC2_OK event was posted successfully
     * @return QC2_BAD_VALUE invalid event
     * @return QC2_CANNOT_DO no backing event-queue to handle events
     * @todo (SK) check if unique_ptr<Event> works better
     */
    QC2Status postAsync(std::shared_ptr<Event> event);

    /**
     * @brief post an event asynchronously with a delay specified in milliseconds
     *
     * post an Event to be scheduled after a specified delay without waiting for the response
     * @param[in] event event to be posted
     * @param[in] delayMs delay in milliseconds
     * @return QC2_OK event was posted successfully
     * @return QC2_BAD_VALUE invalid event
     * @return QC2_CANNOT_DO no backing event-queue to handle events
     */
    QC2Status postAsyncDelayed(std::shared_ptr<Event> event, uint32_t delayMs);

    /**
     * @brief post an event synchronously and wait till it is handled
     *
     * Post an event synchronously and wait for the event to be handled.
     * Handler may (optionally) update the event's reply
     * @param[in] event event to be posted
     * @return QC2_OK event was posted successfully
     * @return QC2_BAD_VALUE invalid event
     * @return QC2_CANNOT_DO no backing event-queue to handle events
     */
    QC2Status postSyncAndWait(std::shared_ptr<Event> event);

    // TODO(PC):  Handling timeout can be tricky.. inclined not to support that
    /*
    //            For sync-event, notify_all() and waiting thread can check if event-id matches
    //            it's event id, else pends again
    //            When waiting thread times-out
    //              if event is still in queue, remove it since waiter is gone
    //              if event is currently being handled, skip waking up (anyone) as waiter is gone
    QC2Status postSyncAndWait(
            std::shared_ptr<Event> event,
            uint32_t timeOutMs = std::numeric_limits<uint32_t>::max());*/

 protected:
    /**
     * @brief callback invoked in event-thread's context to handle an event
     *
     * @param[in] event event to be handled
     * @return QC2_OK if the event was handled
     */
    virtual QC2Status onEvent(std::shared_ptr<Event> event) = 0;

 private:
    DECLARE_NON_COPYASSIGNABLE(EventHandler);
    friend class EventQueue;
    std::shared_ptr<EventQueue> mEventQueue;    ///< backing event-queue reference

    QC2Status post(std::shared_ptr<Event> event, bool sync, uint32_t delayMs = 0);
};

/**
 * @brief Schedules and dispatches queued events
 *
 * EventQueue has following components:
 * -# A queue where the posted-events are added \n
 *    (Event is associated with its corresponding Handler)
 * -# A thread (aka Event-Thread) that provides the context in which the events are handler
 * -# Synchronization variables to handle syncronously posted events\n
 * EventQueue does the following:
 * -# adds event to event-queue (blocks caller if event is synchronous)
 * -# pops the event to be handled (based on order and priority)
 * -# dispatches the event to the associated handler via EventHandler::onEvent(..)
 * -# wakes up the corresponding caller if event is synchronous\n
 */
class EventQueue {
 public:
    explicit EventQueue(const std::string& name);
    ~EventQueue();

    /**
     * @brief start the event-thread synchronously
     *
     * EventQueue accepts and scheduled events after starting
     * @return QC2_BAD_STATE could not start the event-thread
     * @return QC2_OK event-queue started
     */
    QC2Status start();

    /**
     * @brief stop the event-thread synchronously
     *
     * Stops the event thread. Finishes current event being handled and discard the rest
     * TODO(PC): when discarding, callers waiting on synchronous events must be woken up
     * Will block momentarily (or forever) if the handler is stuck handling the current event.
     * No messages can be posted to the event-queue after this call returns
     */
    QC2Status stop();

    static QC2Status threadWrapper(EventQueue&);

 private:
    DECLARE_NON_COPYASSIGNABLE(EventQueue);

    struct EventItem {
        EventItem(std::shared_ptr<Event> e, std::shared_ptr<EventHandler> h, bool isSync,
                uint64_t scheduledTimeUs, uint64_t delayTimeUs)
            : mEvent(e), mHandler(h), mIsSynchronous(isSync), mScheduledTimeUs(scheduledTimeUs),
              mDelayTimeUs(delayTimeUs), mPostedThreadId(std::this_thread::get_id()) {
        }
        std::shared_ptr<Event> mEvent;
        std::weak_ptr<EventHandler> mHandler;
        bool mIsSynchronous;
        uint64_t mScheduledTimeUs;
        uint64_t mDelayTimeUs;
        std::thread::id mPostedThreadId;
    };

    std::string mName;
    std::mutex mLock;                           ///< lock guarding the event-queue
    std::list<EventItem> mQueue;                ///< queue holding {event, handler}
    std::condition_variable mQueueCondition;    ///< condition to signal filled/empty events

    std::mutex mSyncWaitLock;                   ///< lock to block caller posting synchronously
    std::condition_variable mSyncWaitCondition;     ///< signal to wake up blocked caller

    std::shared_ptr<std::thread> mThread;       ///< event-thread
    enum class ThreadState : uint32_t {
        UNINIT = 0,
        RUNNING = 1,
        STOPPING = 2,   // thread is joinable and is currently waiting to finish
    };
    ThreadState mThreadState;
    std::thread::id mSelfThreadId;

    friend class EventHandler;

    QC2Status postEvent(
            std::shared_ptr<Event> event,
            std::shared_ptr<EventHandler> handler,
            bool isSynchronous,
            uint32_t delayMs);

    struct Stats {
        Stats()
            : mStartTimeUs(0ull), mNumPostedTotal(0ull), mNumWakeupsTotal(0ull),
              mTotalWaitMs(0ull), mWorstWaitMs(0ull) {
        }
        uint64_t mStartTimeUs;
        uint64_t mNumPostedTotal;
        uint64_t mNumWakeupsTotal;
        uint64_t mTotalWaitMs;
        uint64_t mWorstWaitMs;
        std::string summary();
    };
    Stats mStats;

    // thread loop and synchronization
    QC2Status threadLoop();
    std::atomic_bool mStopRequest;
};

/// @}

};  // namespace qc2audio

#endif  // _QC2AUDIO_UTILS_EVENTQUEUE_H_

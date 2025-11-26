#include "Touch.h"
#include "DeathRecipient.h"
#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <log/log.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <linux/input.h>
#include <chrono>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

std::shared_ptr<Touch> Touch::sInstance = nullptr;

std::shared_ptr<Touch> Touch::getInstance() {
    if (!sInstance) {
        sInstance = ndk::SharedRefBase::make<Touch>();
    }
    return sInstance;
}

Touch::Touch() {
    ALOGD("Touch constructor called");
    display0_thread = std::thread(&Touch::Display_0, this, "/dev/input/event0", 0);
    display1_thread = std::thread(&Touch::Display_1, this, "/dev/input/event1", 1);
}

Touch::~Touch() {
    if (display0_thread.joinable()) display0_thread.join();
    if (display1_thread.joinable()) display1_thread.join();
}

ndk::ScopedAStatus Touch::registerCallback(
    const std::shared_ptr<aidl::vendor::visteon::hardware::interfaces::touch::ITouchCallback>& callback,
    int32_t clientID) {

    ALOGD("registerCallback called for client ID %d", clientID);
    mCallbacks[clientID] = callback;

    auto deathRecipient = std::make_shared<TouchDeathRecipient>(callback, this);
    return ndk::ScopedAStatus::ok();
}

void Touch::notifyClients(int id) {
    for (auto& [clientID, callback] : mCallbacks) {
        if (callback) {
            callback->onTouchReceived(id);
        }
    }
}

void Touch::handleClientDeath() {
    mCallbacks.clear();
    ALOGI("Client died. Cleared callbacks.");
}

void Touch::Display_0(std::string path, int id) {
    ALOGD("[%s] : Waiting for display node %s", __func__, path.c_str());

    int inputFd = -1;
    while (true) {
        inputFd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (inputFd >= 0) {
            ALOGD("[%s] : Connection to display node success", __func__);
            break;
        } else {
            ALOGE("[%s] : Failed to open %s, retrying...", __func__, path.c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }

    struct pollfd fds[1];
    fds[0].fd = inputFd;
    fds[0].events = POLLIN;

    ALOGD("[%s] : Monitoring input device...", __func__);

    while (true) {
        int ret = poll(fds, 1, -1);
        if (ret < 0) {
            ALOGE("[%s] : poll failed", __func__);
            break;
        }

        if (fds[0].revents & POLLIN) {
            struct input_event ev;
            while (read(inputFd, &ev, sizeof(ev)) == sizeof(ev)) {
                if (ev.type == EV_ABS) {
                    notifyClients(id);
                    ALOGD("[%s] : Touch event notified to id = %d", __func__, id);
                }
            }
        }
    }

    close(inputFd);
}

void Touch::Display_1(std::string path, int id) {
        ALOGD("[%s] : Waiting for display node %s", __func__, path.c_str());

    int inputFd = -1;
    while (true) {
        inputFd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (inputFd >= 0) {
            ALOGD("[%s] : Connection to display node success", __func__);
            break;
        } else {
            ALOGE("[%s] : Failed to open %s, retrying...", __func__, path.c_str());
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    }

    struct pollfd fds[1];
    fds[0].fd = inputFd;
    fds[0].events = POLLIN;

    ALOGD("[%s] : Monitoring input device...", __func__);

    while (true) {
        int ret = poll(fds, 1, -1);
        if (ret < 0) {
            ALOGE("[%s] : poll failed", __func__);
            break;
        }

        if (fds[0].revents & POLLIN) {
            struct input_event ev;
            while (read(inputFd, &ev, sizeof(ev)) == sizeof(ev)) {
                if (ev.type == EV_ABS) {
                    notifyClients(id);
                    ALOGD("[%s] : Touch event notified to id = %d", __func__, id);
                }
            }
        }
    }

    close(inputFd);
}

} // namespace

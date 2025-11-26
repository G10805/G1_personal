// FIXME: your file license if you have one
#include <hidl/HidlTransportSupport.h>


#include "Touch.h"
#include "TouchCallback.h"
#include "DeathRecipient.h"
#include <log/log.h>
#include <map>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/inotify.h>
#include <linux/input.h>
#include <cstring>
#include <chrono>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

using android::hardware::hidl_death_recipient;
using android::hardware::Return;
using android::sp;

android::sp<Touch> Touch::sInstance = nullptr;

android::sp<Touch> Touch::getInstance() {
    if (sInstance == nullptr) {
        sInstance = new Touch();
    }
    return sInstance;
}

Touch::Touch(){
  ALOGD("Constructor called");
    devices = retrieveEventIdForEachDisplay(device_file);
    int i = 0;
    for(auto device : devices){
        std::string path = driverPath + device["Handlers"];
        //display_thread.emplace_back(&Touch::Display, this, path, i++);
    }

    ALOGD("Display threads started");
    display0_thread = std::thread(&Touch::Display_0, this, DISPLAY_1, 0);
    display1_thread = std::thread(&Touch::Display_1, this, DISPLAY_0, 1);
    mCallbacks.clear();

}

Touch::~Touch(){    

     if (display0_thread.joinable()) {
         display0_thread.join();  // Ensure thread finishes before destruction
     }
     if (display1_thread.joinable()) {
         display1_thread.join();  // Ensure thread finishes before destruction
     }
}

std::string Touch::get_device_name(int fd) {
    char name[256] = "Unknown";
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    return std::string(name);
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

void Touch::notifyClients(int id) {
    if (mCallbacks.empty()) {
        ALOGW("notifyClients: No callbacks registered");
        return;
    }
    for (auto& iter : mCallbacks) {
        const int clientID = iter.first;
        sp<ITouchCallback>& callback = iter.second;
        if (callback == nullptr) {
            ALOGE("notifyClients: Null callback for client ID %d", clientID);
            continue;
        }
        //ALOGD("notifyClients: Notifying client ID %d", clientID);
        Return<void> ret = callback->onTouchReceived(id);
        if (!ret.isOk()) {
            ALOGE("notifyClients: Failed to notify client ID %d", clientID);
        }
    }
}

void Touch::handleClientDeath(){
    mCallbacks.clear();
    ALOGI("Client has died. Cleaned up resources.");
}

// Methods from ::vendor::visteon::hardware::interfaces::touch::V1_0::ITouch follow.
Return<void> Touch::registerCallback(const sp<::vendor::visteon::hardware::interfaces::touch::V1_0::ITouchCallback>& obj, int32_t clientID) {
    ALOGD("[%s] called ",__func__);
mCallbacks.insert(std::make_pair(clientID, obj));
	ALOGD(" Touch::registerCallback called mCallbacks.size(): %d", mCallbacks.size());
	vendor::visteon::hardware::interfaces::touch::implementation::TouchDeathRecipient* deathRecipient =
    new vendor::visteon::hardware::interfaces::touch::implementation::TouchDeathRecipient(obj, this);
   	ALOGD(" Touch::registerCallback for clients created ");
    return Void();
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

//ITouch* HIDL_FETCH_ITouch(const char* /* name */) {
    //return new Touch();
//}
//
}  // namespace vendor::visteon::hardware::interfaces::touch::implementation

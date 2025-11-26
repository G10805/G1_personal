/*
 * Copyright (c) 2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#define LOG_NDEBUG 0
#define LOG_TAG "CarPowerUsbService"

#include "CarPowerUsbService.h"

#include <android/binder_process.h>
#include <android/binder_manager.h>
#include <android-base/logging.h>
#include <cutils/properties.h>
#include <utils/Log.h>
#include <dirent.h>
#include <list>
#include <cutils/android_reboot.h>

using aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy;
using aidl::android::frameworks::automotive::powerpolicy::PowerComponent;
using ::android::frameworks::automotive::powerpolicy::hasComponent;
using ::ndk::ScopedAStatus;

#define BOARD_PLATFORM_PROP             "ro.board.platform"
#define USB_MODE_NONE                   "none"
#define USB_MODE_HOST                   "host"
#define LPM_STATE                       "persist.vendor.vehicle.lpm"
#define DISK                            "disk"
#define USB_MODE_BIND                   "/sys/bus/platform/drivers/xhci-hcd/bind"
#define USB_MODE_UNBIND                 "/sys/bus/platform/drivers/xhci-hcd/unbind"
#define USB_PATH                        "/sys/bus/platform/drivers/xhci-hcd/"

constexpr PowerComponent kCpuComponent     = PowerComponent::CPU;
constexpr PowerComponent kDisplayComponent = PowerComponent::DISPLAY;

CarPowerUsbService::CarPowerUsbService() {
    initUsbDevice();
    mKeyListener = std::make_shared<KeyEventListener>();
    mkeyCallback = std::make_shared<LocalKeyEventCallback>();
    mKeyListener->registerKeyEventListener(KEY_AGAIN, mkeyCallback);
    mKeyListener->registerKeyEventListener(KEY_STOP, mkeyCallback);
}

CarPowerUsbService::~CarPowerUsbService() {
    mKeyListener->unregisterKeyEventListener(KEY_AGAIN, mkeyCallback);
    mKeyListener->unregisterKeyEventListener(KEY_STOP, mkeyCallback);
}

void CarPowerUsbService::LocalKeyEventCallback::onKeyPressed(int keyCode, uint64_t durationMs) {
    ALOGD("Keyevent reached: %d, duration: %lld ms", keyCode, durationMs);
    int ret=0;
    if (keyCode == KEY_AGAIN) {
        //TODO: @asmemoha add boot reason
        ret = android_reboot(ANDROID_RB_RESTART2, 0, nullptr);
    } else if (keyCode == KEY_STOP) {
        ret = android_reboot(ANDROID_RB_POWEROFF, 0, nullptr);
    } else {
        // Shouldnt come here
        ALOGD("unknown Keyevent reached: %d, duration: %lld ms", keyCode, durationMs);
    }
    if (ret != 0)
        ALOGE("Failed android_reboot call. Error code:%d err:%s", ret, strerror(ret));

}

void CarPowerUsbService::onInitFailed() {
    ALOGE("Initializing power policy client failed");
}

std::vector<PowerComponent> CarPowerUsbService::getComponentsOfInterest() {
    std::vector<PowerComponent> components{kCpuComponent, kDisplayComponent};
    return components;
}

ScopedAStatus CarPowerUsbService::onPolicyChanged(const CarPowerPolicy& powerPolicy) {
    ALOGD("onPower Policy");
    return ScopedAStatus::ok();
}

void CarPowerUsbService::initUsbDevice() {
    char platform[256];
    const char *usbDevice = "";
    const char *usbDevice2 = "";
    const char *usbDevice3 = "";

    mUsbDevice.clear();
    mUsbDevice2.clear();
    mUsbDevice3.clear();

    property_get(BOARD_PLATFORM_PROP, platform, "");
    std::string platform_str(platform);
    ALOGD("Board platform '%s'",  platform);
    if ((platform_str.compare("msmnile") == 0)) {
        // Hana, Poipu
        usbDevice  = "/sys/devices/platform/soc/a600000.ssusb/mode";
        usbDevice2 = "/sys/devices/platform/soc/a800000.ssusb/mode";
        usbDevice3 = "/sys/devices/platform/soc/a400000.ssusb/mode";
    } else if ((platform_str.compare("sm6150") == 0)) {
        // Talos
        usbDevice  = "/sys/bus/platform/devices/a600000.ssusb/mode";
        usbDevice2 = "/sys/bus/platform/devices/a800000.hsusb/mode";
    } else if ((platform_str.compare("gen4") == 0)) {
        // gen4
        usbDevice  = "/sys/devices/platform/soc/a600000.ssusb/mode";
        usbDevice2 = "/sys/devices/platform/soc/a800000.ssusb/mode";
        usbDevice3 = "/sys/devices/platform/soc/a400000.hsusb/mode";
    } else {
        ALOGE("Unknown platform '%s'",  platform);
        return;
    }

    ALOGD("usbDevice  '%s', usbDevice2 '%s', usbDevice3 '%s' ",usbDevice,usbDevice2,usbDevice3);
    mUsbDevice.assign(usbDevice);
    mUsbDevice2.assign(usbDevice2);
    mUsbDevice3.assign(usbDevice3);
}

void CarPowerUsbService::handleDeepSleepEntry() {
    ALOGD(" deep sleep entry ");
    listDirectory();
    detachUsb(mUsbDevice.c_str(), mUsbModeSaved);
    detachUsb(mUsbDevice2.c_str(), mUsbModeSaved2);
    if (!isHibernation()) {
        detachUsb(mUsbDevice3.c_str(), mUsbModeSaved3);
    }
}

void CarPowerUsbService::handleDeepSleepExit() {
    ALOGD("deep sleep exit");
    attachUsb(mUsbDevice.c_str(), mUsbModeSaved);
    attachUsb(mUsbDevice2.c_str(), mUsbModeSaved2);
    if (!isHibernation()) {
        attachUsb(mUsbDevice3.c_str(), mUsbModeSaved3);
    }
    for (auto file : mMultiPortUsbDevice) {
        ALOGD(" write data  : %s", file.c_str());
        attachOtherUsb(file);
    }
}

void CarPowerUsbService::listDirectory() {
    mMultiPortUsbDevice.clear();
    struct dirent *file;
    DIR *dir = opendir(USB_PATH);
    if (dir != NULL ) {
        while ((file = readdir(dir)) != NULL) {
            string fileName = file->d_name;
            if (fileName.find("xhci-hcd") != string::npos) {
                mMultiPortUsbDevice.push_back(fileName);
                ALOGD("usb file : %s", fileName.c_str());
                detachOtherUsb(fileName);
            }
        }
        closedir(dir);
    } else {
        ALOGE("Directory is not present");
    }
}

void CarPowerUsbService::attachOtherUsb(string &usbMode) {
    ALOGD("%s write %s into fd : %s",__func__, usbMode.c_str(), USB_MODE_BIND);
    int fd = openUsbMode(USB_MODE_BIND);
    if (fd >= 0 ) {
        writeUsbMode(fd, usbMode);
        close(fd);
    } else {
        ALOGE("open usb failed '%s' into fd %d ('%s')", USB_MODE_BIND, fd, usbMode.c_str());
    }
}

void CarPowerUsbService::detachOtherUsb(string &usbMode) {
    ALOGD("%s write %s into fd : %s",__func__, usbMode.c_str(),USB_MODE_UNBIND);
    int fd = openUsbMode(USB_MODE_UNBIND);
    if (fd >= 0 ) {
        writeUsbMode(fd, usbMode);
        close(fd);
    } else {
        ALOGE("open usb failed '%s' into fd %d ('%s')", USB_MODE_UNBIND, fd, usbMode.c_str());
    }
}

bool CarPowerUsbService::isUsbDetached(string &usbMode) {
    const char *str = usbMode.c_str();
    return str && !strncmp(str, USB_MODE_NONE, strlen(USB_MODE_NONE));
}

int CarPowerUsbService::openUsbMode(const char *device) {
    int fd = open(device, O_RDWR);
    if (fd < 0) {
        ALOGE("open usb mode fail");
    }
    return fd;
}

bool CarPowerUsbService::isHibernation() {
    char suspend[100] = "";
    property_get(LPM_STATE, suspend, "N/A");
    if (!strncmp(suspend, DISK, strlen(DISK))) {
        return true;
    }
    return false;
}

bool CarPowerUsbService::writeUsbMode(int fd, string &usbMode) {
    const char *str = usbMode.c_str();
    size_t len = strlen(str);
    ssize_t ret = write(fd, str, len);
    if (ret != (ssize_t) len) {
        ALOGE("write usb mode fail");
    }
    return (ret == (ssize_t) len);
}

bool CarPowerUsbService::readUsbMode(int fd, string &usbMode) {
    char buf[256];
    ssize_t ret;
    ret = read(fd, buf, sizeof(buf) - 1);
    if((ret >=0) && ((unsigned long) ret < sizeof(buf))){
        buf[ret] = '\0';
        ALOGD("read usb mode '%s' ", buf);
        usbMode.assign(buf);
        return true;
    }
    return false;
}

void CarPowerUsbService::detachUsb(const char *device, std::string &usbMode) {
    int fd;
    string cmd = USB_MODE_NONE;
    if (!usbMode.empty()) {
        /* Ignore because usb mode has been stored. */
        return;
    }

    fd = openUsbMode(device);
    if (fd >= 0 ) {
        if (!readUsbMode(fd, usbMode) || isUsbDetached(usbMode)) {
            close(fd);
            return;
        }
    } else {
        ALOGE("open usb mode failed '%s'", device);
        return;
    }
    ALOGD("detach usb, write '%s' into fd %d ('%s')", cmd.c_str(), fd, device);
    writeUsbMode(fd, cmd);
    close(fd);
}

void CarPowerUsbService::attachUsb(const char *device, string &usbMode) {
    int fd;
    if (isUsbDetached(usbMode)) {
        /* Ignore because usb isn't attached when to suspend. */
        return;
    }

    if (usbMode.empty()) {
        /* Ignore if its already Attached */
        return;
    }

    fd = openUsbMode(device);
    if (fd >= 0) {
        ALOGD("attach usb, write '%s' into fd %d ('%s')", usbMode.c_str(), fd, device);
        writeUsbMode(fd, usbMode);
        close(fd);
    }
    usbMode.clear();
}

/* ------------------------------------------------------------------------------- */
int main() {
    ALOGI(LOG_TAG " is starting");
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    std::shared_ptr<CarPowerUsbService> carPowerUsbService = ndk::SharedRefBase::make<CarPowerUsbService>();
    if(carPowerUsbService == NULL)
        ALOGD("carpower is null");
    carPowerUsbService->init();
    ALOGD("Power Policy class inited, joining threadpool");
    ABinderProcess_joinThreadPool();
    ALOGE(LOG_TAG " carPower is null Error in client binder thread");
    return EXIT_FAILURE;
}

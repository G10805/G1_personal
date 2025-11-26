/*==============================================================================
 *
 *  Copyright (c) 2022 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 *===============================================================================
 */

#ifndef power_policy_CarPowerusbService_H_
#define power_policy_CarPowerusbService_H_

#include <PowerPolicyClientBase.h>
#include <KeyEventListener.h>

using ::std::string;
using android::qti::KeyEventCallback;
using android::qti::KeyEventListener;

class CarPowerUsbService : public ::android::frameworks::automotive::powerpolicy::PowerPolicyClientBase {
  public:
    CarPowerUsbService();
    ~CarPowerUsbService();
    std::vector<::aidl::android::frameworks::automotive::powerpolicy::PowerComponent>
            getComponentsOfInterest() override;
    void onInitFailed();
    ::ndk::ScopedAStatus onPolicyChanged(
            const ::aidl::android::frameworks::automotive::powerpolicy::CarPowerPolicy&) override;

    void initVehicle();
    void initUsbDevice();
    void subscribeVehicle();
    bool isPropertyTrue(const char *prop, bool def);
    bool isCarPowerManagerEnabled();
    void handleDeepSleepEntry();
    void handleDeepSleepExit();
    void detachUsb(const char *device, string &usbMode);
    void attachUsb(const char *device, string &usbMode);
    int openUsbMode(const char *device);
    bool isUsbDetached(string &usbMode);
    bool readUsbMode(int fd, string &usbMode);
    bool writeUsbMode(int fd, string &usbMode);
    bool isHibernation();
    // list all the multiport usb connected to the device
    void listDirectory();
    // detach multiport usb during suspend
    void detachOtherUsb(string &usbMode);
    // attach multiport usb during resume
    void attachOtherUsb(string &usbMode);

    string mUsbDevice, mUsbDevice2, mUsbDevice3;
    string mUsbModeSaved, mUsbModeSaved2, mUsbModeSaved3;
    std::vector<string> mMultiPortUsbDevice;

  private:
    class LocalKeyEventCallback : public KeyEventCallback {
        public:
            void onKeyPressed(int, uint64_t) override;
    };

    std::shared_ptr<KeyEventListener> mKeyListener;
    std::shared_ptr<LocalKeyEventCallback> mkeyCallback;
};

#endif  // power_policy_CarPowerusbService_H_

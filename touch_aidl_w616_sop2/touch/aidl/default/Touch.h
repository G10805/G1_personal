#pragma once

#include <aidl/vendor/visteon/hardware/interfaces/touch/BnTouch.h>
#include <aidl/vendor/visteon/hardware/interfaces/touch/ITouchCallback.h>
#include <map>
#include <thread>
#include <string>
#include <memory>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

class Touch : public aidl::vendor::visteon::hardware::interfaces::touch::BnTouch {
public:
    static std::shared_ptr<Touch> getInstance();
    Touch();
    ~Touch();

    ndk::ScopedAStatus registerCallback(
        const std::shared_ptr<aidl::vendor::visteon::hardware::interfaces::touch::ITouchCallback>& callback,
        int32_t clientID) override;

    void handleClientDeath();
    void notifyClients(int id);

private:
    static std::shared_ptr<Touch> sInstance;
    std::map<int, std::shared_ptr<aidl::vendor::visteon::hardware::interfaces::touch::ITouchCallback>> mCallbacks;

    std::thread display0_thread;
    std::thread display1_thread;

    void Display_0(std::string path, int id);
    void Display_1(std::string path, int id);
};

} // namespace
#ifndef READVHALPROP_H
#define READVHALPROP_H

#include <cutils/properties.h>
#include <utils/Log.h>

#include <AidlVhalClient.h>
#include <IVhalClient.h>
#include <aidl/android/hardware/automotive/vehicle/IVehicle.h>
#include <aidl/vendor/visteon/infocore/hardware/automotive/vehicle/VehicleProperty.h>


#include "VSTRvcService.h"

#define TIMEOUT_40SEC 200
#define TIMEOUT_20SEC 150

using ::aidl::android::hardware::automotive::vehicle::VehiclePropertyStatus;
using ::aidl::android::hardware::automotive::vehicle::VehiclePropValue;
using ::aidl::vendor::visteon::infocore::hardware::automotive::vehicle::VehicleProperty;
using ::android::frameworks::automotive::vhal::ISubscriptionCallback;
using ::android::frameworks::automotive::vhal::IVhalClient;

int timeout_count = TIMEOUT_40SEC;
int hmi_ready = 0, sysui_flag = 0, launcher_flag = 0, rvc_flag = 0, popup_flag = 0, hmi_animating = 0;
int akg_sent = 0, timeout_akg_sent = 0, s2r_hmiReady = 0, power_mode = 0, rvc_thread = 0;
char animdone[92];
int gpio_value_rvc = -1;

class VhalCallback : public ISubscriptionCallback {
public:
    explicit VhalCallback(VSTRvc::VSTRvcService* vstInst);
    void onPropertyEvent(const std::vector<std::unique_ptr<android::frameworks::automotive::vhal::IHalPropValue>>& values) override;
    void onPropertySetError([[maybe_unused]] const std::vector<android::frameworks::automotive::vhal::HalPropError>& errors) override;
    static void handleVhalDeath();
};

std::shared_ptr<::android::frameworks::automotive::vhal::IVhalClient> pVHalService;
std::shared_ptr<VhalCallback> mVehicleCallback;
std::unique_ptr<android::frameworks::automotive::vhal::ISubscriptionClient> mSubscriptionClient;
std::shared_ptr<android::frameworks::automotive::vhal::IVhalClient::OnBinderDiedCallbackFunc>
            mOnBinderDiedCallback;

void subscribetovhalprop(VSTRvc::VSTRvcService* vst);
void set_hmi_ready_ack();
void set_ivi_service_ready();
void set_hmi_timeout_ready();
void set_fastrvcexit();
void reset_hmi_ready();
void* check_rvc_gpio(void* data);
void* monitor_rvc(void* data);
int read_sysfs();
int write_sysfs();

#endif

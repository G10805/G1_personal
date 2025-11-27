#include "readVhalProp.h"
#include <cutils/properties.h>
#include "libvstRvc/osal.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#define LOG_TAG "VSTRvcService"
#pragma GCC diagnostic pop

#include <android/binder_process.h>

THREAD_HANDLE(rvc_gpio_monitor);
THREAD_HANDLE(rvc);

bool binder_died = false;

VSTRvc::VSTRvcService* localVst;
VhalCallback::VhalCallback(VSTRvc::VSTRvcService* vst)

{
    ALOGE("Entering [%s]", __func__);
}

void VhalCallback::onPropertyEvent(const std::vector<std::unique_ptr<android::frameworks::automotive::vhal::IHalPropValue>>& values)
{
    for (const auto& value : values) {
        VehiclePropValue* parameter = (VehiclePropValue*)value->toVehiclePropValue();
        if (parameter->prop != 0) {
            if (parameter->prop == (int32_t)VehicleProperty::VHAL_HMI_READY) {
                hmi_ready = parameter->value.int32Values[0];
                ALOGE("HMI READY received value is : %d", hmi_ready);
                if (hmi_ready==1) sysui_flag=1;
                else if (hmi_ready==2) launcher_flag=1;
                else if (hmi_ready==4) rvc_flag=1;

                // To be enabled later
                /*if(timeout_count>TIMEOUT_20SEC){
                        timeout_count =TIMEOUT_20SEC -(TIMEOUT_40SEC-timeout_count);
                        ALOGE("Wait for timeout reduced to 20sec");
                }
                */
                if (sysui_flag && launcher_flag /*&& rvc_flag*/ /*&& popup_flag*/) {
                    if (akg_sent == 0) {
                        ALOGE("All HMI ready received");
                        property_set("vendor.vstsysui.ready", "1");
                    }
                }
                if (!rvc_thread && rvc_flag && sysui_flag && launcher_flag) {
                    THREAD_CREATE(rvc, monitor_rvc, NULL);
                    rvc_thread = 1;
                }
            }
            if (parameter->prop == (int32_t)VehicleProperty::VHAL_ANIMATING_ACK) {
                hmi_animating = parameter->value.int32Values[0];
                ALOGE("------VSTRVC:onPropertyEvent for VHAL_ANIMATING_ACK property val is %d -------", hmi_animating);
                property_get("vendor.animation.done", animdone, "0");
                if ((atoi(animdone) || (power_mode == 2)) && hmi_animating) {
                    ALOGE("VSTRVC stopping the animation now");
                    property_set("vendor.rvc.home_ready", "1");
                    // property_set("vendor.vstsysui.ready", "1");
                }
            }
            // Do nothing - just to confirm HMI_READY_ACK is set successfully
            if (parameter->prop == (int32_t)VehicleProperty::VHAL_HMI_READY_ACK) {
                ALOGE("------VSTRVC:onPropertyEvent for VHAL_HMI_READY_ACK property set %d ,%d-------", parameter->value.int32Values[0], parameter->value.int32Values[1]);
            }
            if (parameter->prop == (int32_t)VehicleProperty::IVI_POWER_STATE_REPORT) {
                power_mode = parameter->value.int32Values[0];
                if ((power_mode == 7) || (power_mode == 2)) {
                    ALOGE("------VSTRVC:onPropertyEvent for IVI_POWER_STATE_REPORT is %d", power_mode);
                }
            }

            if (parameter->prop == (int32_t)VehicleProperty::VHAL_EANABLE_LATE_RVC_FOR_HMI) {
                ALOGE("------VSTRVC:onPropertyEvent for VHAL_EANABLE_LATE_RVC_FOR_HMI is %d", parameter->value.int32Values[0]);
            }
        }
    }
}
void VhalCallback::onPropertySetError([[maybe_unused]] const std::vector<android::frameworks::automotive::vhal::HalPropError>& errors)
{
    ALOGE("[%s] onPropertySetError", __func__);
    return;
}
void VhalCallback::handleVhalDeath(){
    ALOGE("VHAL binder died. Attempting to reconnect...");
    binder_died = true;
    //mOnBinderDiedCallback.reset();
    pVHalService.reset();
    subscribetovhalprop(localVst);
    ALOGE("Resubscribe done");
    ABinderProcess_joinThreadPool();
}

void subscribetovhalprop(VSTRvc::VSTRvcService* vst)
{
    ALOGI("VSTRvcService::Subscribe to vhal called");
    localVst=vst;
    pVHalService = ::android::frameworks::automotive::vhal::IVhalClient::tryCreate();
    while (pVHalService == nullptr) {
        ALOGE("Vhalservice is NULL!!!!!!! Retrying.......");
        (void)usleep(static_cast<uint32_t>(250 * 1000));
        pVHalService = ::android::frameworks::automotive::vhal::IVhalClient::tryCreate();
    }
    ALOGI("Subscribe to vhal successfull");
    if (!binder_died){
    	mVehicleCallback = std::make_unique<VhalCallback>(vst);
    	mOnBinderDiedCallback = std::make_shared<IVhalClient::OnBinderDiedCallbackFunc>([] { VhalCallback::handleVhalDeath(); });
    }
    mSubscriptionClient = pVHalService->getSubscriptionClient(mVehicleCallback);
    pVHalService->addOnBinderDiedCallback(mOnBinderDiedCallback);

    // subscribe vehicle HAL property
    std::vector<aidl::android::hardware::automotive::vehicle::SubscribeOptions> options;
    aidl::android::hardware::automotive::vehicle::SubscribeOptions optionsData[] = {
        { .propId = static_cast<int32_t>(VehicleProperty::VHAL_HMI_READY), .sampleRate = 1.0 },
        { .propId = static_cast<int32_t>(VehicleProperty::VHAL_ANIMATING_ACK), .sampleRate = 1.0 },
        { .propId = static_cast<int32_t>(VehicleProperty::VHAL_HMI_READY_ACK), .sampleRate = 1.0 },
        { .propId = static_cast<int32_t>(VehicleProperty::IVI_POWER_STATE_REPORT), .sampleRate = 1.0 },
    };

    options.push_back(optionsData[0]);
    options.push_back(optionsData[1]);
    options.push_back(optionsData[2]);
    options.push_back(optionsData[3]);

    if (auto result = mSubscriptionClient->subscribe(options); !result.ok()) {
        ALOGW("Failed to subscribe to vehicle property error: %s",
            result.error().message().c_str());
    } else {
        ALOGI("VHAL subscription Done ");
	if (!binder_died){
        	THREAD_CREATE(rvc_gpio_monitor, check_rvc_gpio, NULL);
	 }
    }
}
void set_ivi_service_ready()
{
    // send VHAL_IVISERVICEREADYMESSAGE
    ALOGE("------VSTRVC sending VHAL_HMISERVICEREADYMESSAGE -------");

    int propId = static_cast<int32_t>(VehicleProperty::VHAL_HMISERVICEREADYMESSAGE);
    std::vector<int32_t> dataVec = std::vector<int32_t> { 1 };
    auto propToSet = pVHalService->createHalPropValue(propId);
    propToSet->setInt32Values(dataVec);
    auto setValueResult = pVHalService->setValueSync(*propToSet);

    if (setValueResult.ok()) {
        ALOGE(" VHAL_HMISERVICEREADYMESSAGE property Written to VHAL");
    } else {
        ALOGE("Failed to set value for property VHAL_HMISERVICEREADYMESSAGE");
    }
}

void set_hmi_ready_ack()
{
    if (akg_sent == 0) {
        ALOGE("------VSTRVC sending VHAL_HMI_READY_ACK -------");
        int propId = static_cast<int32_t>(VehicleProperty::VHAL_HMI_READY_ACK);
        std::vector<int32_t> dataVec = std::vector<int32_t> { 1, 1 };
        auto propToSet = pVHalService->createHalPropValue(propId);
        propToSet->setInt32Values(dataVec);
        auto setValueResult = pVHalService->setValueSync(*propToSet);

        if (setValueResult.ok()) {
            ALOGE("VHAL_HMI_READY_ACK property Written to VHAL");
            akg_sent = 1;
            set_ivi_service_ready();
        } else {
            ALOGE("Failed to set value for property VHAL_HMI_READY_ACK");
        }
    } else {
        ALOGE("------VSTRVC VHAL_HMI_READY_ACK already sent-------");
    }
}
void set_hmi_timeout_ready()
{
    if (timeout_akg_sent == 0) {
        ALOGE("------VSTRVC sending VHAL_HMI_READY_ACK timeout 2,0-------");
        int propId = static_cast<int32_t>(VehicleProperty::VHAL_HMI_READY_ACK);
        std::vector<int32_t> dataVec = std::vector<int32_t> { 2, 0 };
        auto propToSet = pVHalService->createHalPropValue(propId);
        propToSet->setInt32Values(dataVec);
        auto setValueResult = pVHalService->setValueSync(*propToSet);

        if (setValueResult.ok()) {
            ALOGE("VHAL_HMI_READY_ACK property Written to VHAL");
            timeout_akg_sent = 1;
        } else {
            ALOGE("Failed to set value for property VHAL_HMI_READY_ACK");
        }
    } else {
        ALOGE("------VSTRVC VHAL_HMI_READY_ACK timeout already sent-------");
    }
}
void set_fastrvcexit()
{
    ALOGE("------VSTRVC sending VHAL_EANABLE_LATE_RVC_FOR_HMI-------");
    int propId = static_cast<int32_t>(VehicleProperty::VHAL_EANABLE_LATE_RVC_FOR_HMI);
    std::vector<int32_t> dataVec = std::vector<int32_t> { 1 };
    auto propToSet = pVHalService->createHalPropValue(propId);
    propToSet->setInt32Values(dataVec);
    auto setValueResult = pVHalService->setValueSync(*propToSet);

    if (setValueResult.ok()) {
        ALOGE("VHAL_EANABLE_LATE_RVC_FOR_HMI property Written to VHAL");
    } else {
        ALOGE("Failed to set value for property VHAL_EANABLE_LATE_RVC_FOR_HMI");
    }
}

void reset_hmi_ready()
{
    sysui_flag = 0;
    launcher_flag = 0;
    rvc_flag = 0;
    popup_flag = 0;
    akg_sent = 0;
    ALOGE("VSTRVC HMI ready flag reset done");
}

void* monitor_rvc(void* data)
{
    /*
    while (gpio_value_rvc != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (write_sysfs()) {
        ALOGE("Write to sysfs successful");
        set_fastrvcexit();
        property_set("vendor.fastrvc.exit", "1");

        if (read_sysfs()) {
            ALOGE("sysfs read ack success");
        } else {
            ALOGE("Timeout done - sysfs ack not received");
        }
    } else {
        ALOGE("Write to sysfs failed");
    }
    */
    return NULL;
}
int write_sysfs()
{
    char file_path[256];
    char buf[5] = { 0 };
    int sysfs_file;
    ssize_t count;

    sprintf(file_path, "/sys/kernel/ufrvc/rvc_hmi_ready");
    sprintf(buf, "%d", 1);

    sysfs_file = open(file_path, O_WRONLY);
    if (sysfs_file == -1) {
        ALOGE("open %s fail, return", file_path);
        return false;
    }
    count = write(sysfs_file, buf, strlen(buf));
    if (static_cast<size_t>(count) != strlen(buf)) {
        ALOGE("write %s to %s fail, return", buf, file_path);
        close(sysfs_file);
        return false;
    }
    close(sysfs_file);
    return true;
}

int read_sysfs()
{
    FILE* fd = NULL;
    char file_path[256];
    int ack_status;
    int count = 50;

    sprintf(file_path, "/sys/kernel/ufrvc/rvc_hmi_ready_ack");

    fd = fopen(file_path, "r");
    if (fd == NULL) {
        ALOGE("read_sysfs file open failed");
        return 0;
    }
    while (1) {
        rewind(fd);
        fflush(fd);
        fscanf(fd, "%d", &ack_status);
        if (ack_status == 1) {
            ALOGE("Ack received value is : %d", ack_status);
            fclose(fd);
            return 1;
        }
        ALOGE("sysfs ack not set retry : %d", ack_status);
        count--;
        if (count == 0) {
            fclose(fd);
            break;
        }
        usleep(10000);
    }
    return 0;
}

int write_gpio_file(char* file_path, char* buf)
{
    int gpio_file;
    size_t count;
    gpio_file = open(file_path, O_WRONLY | O_CREAT, S_IWUSR | S_IWGRP | S_IWOTH);
    if (gpio_file == -1) {
        ALOGE("open %s fail, return", file_path);
        // close(gpio_file);
        return false;
    }
    count = write(gpio_file, buf, strlen(buf));
    if (count != strlen(buf)) {
        ALOGE("write %s to %s fail, return", buf, file_path);
        close(gpio_file);
        return false;
    }
    close(gpio_file);
    return true;
}

void* check_rvc_gpio(void* data)
{
    FILE* fd = NULL;
    char file_path[256];
    int gpio_rvc;
    char buf[5];

    gpio_rvc = 340;
    ALOGE("RVC gpio_number is %d", gpio_rvc);
    sprintf(file_path, "/sys/class/gpio/gpio%d", gpio_rvc);
    if (access(file_path, F_OK)) {
        sprintf(file_path, "/sys/class/gpio/export");
        sprintf(buf, "%d", gpio_rvc);
        if (!write_gpio_file(file_path, buf)) {
            return 0;
        }
    }
    sprintf(file_path, "/sys/class/gpio/gpio%d/direction", gpio_rvc);
    sprintf(buf, "%s", "in");
    if (!write_gpio_file(file_path, buf)) {
        return 0;
    }
    sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_rvc);

    fd = fopen(file_path, "r");
    if (fd == NULL) {
        ALOGE("Failed to open gpio path");
        return 0;
    }
    ALOGE("Start monitoring RVC gpio");
    // return 0;
    char fastrvcexit[92];
    int exit_done = -1, gpio_value_rvc_old = -1;
    while (1) {
        rewind(fd);
        fflush(fd);
        fscanf(fd, "%d", &gpio_value_rvc);
        if (gpio_value_rvc != gpio_value_rvc_old) {
            ALOGE("RVC gpio value changed to: %d ", gpio_value_rvc);
            gpio_value_rvc_old = gpio_value_rvc;
        }
        property_get("vendor.fastrvc.exit", fastrvcexit, "0");
        exit_done = atoi(fastrvcexit);
        if (exit_done) {
            ALOGE("Ufastrvc exited -stop gpio monitor");
            fclose(fd);
            break;
        }
        usleep(10000);
    }
    return NULL;
}

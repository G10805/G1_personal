/**
*
**/

#include <stdint.h>
#include <math.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Singleton.h>
#include <utils/String16.h>

#include <binder/BinderService.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <map>

#include "libvstRvc/osal.h"
#include <libxml/parser.h>
//#include "IVSTRvcService.h"
#include "VSTRvcService.h"
#include "service/VST_rvc_impl.h"

#include <chrono>
#include <thread>
#include <string>
#include <cutils/properties.h>
#include <sys/inotify.h>

//DTC headers
#include <log/log_dtc.h>
#define DTC_STATUS_LOG (2)
#define DTC_STATUS_CLEAR (1)
#define DTC_VIDEOLOSS "0x001A40:2"
#define DTC_NOVIDEOLOSS "0x001A40:1"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"

#define LOG_NDEBUG 0

#pragma GCC diagnostic pop
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmacro-redefined"

#define LOG_TAG "VSTRvcService"

#pragma GCC diagnostic pop

extern void set_fastrvcexit();
extern void subscribetovhalprop(VSTRvc::VSTRvcService *vst);
extern THREAD_HANDLE(rvc);
extern "C" {
    int start_camera_rvc(display_parameters *mDisplayParameters);
    int stop_camera_rvc();
    int clean_logo_thread();
    void* xml_open(char *path);
    char* xml_get_setting(void *handle, char *name);
    //int check_logo_gpio( void *data);
}

typedef enum {
    AP_POWER_MODE_CMD_OFF = 0x0,
    AP_POWER_MODE_CMD_EMERGENCY_OFF =  0x1,
    AP_POWER_MODE_CMD_DISP_OFF = 0x2,
    AP_POWER_MODE_CMD_FULL_RUN = 0x3,
    AP_POWER_MODE_CMD_SYSTEM_IDLE = 0x4,
    AP_POWER_MODE_CMD_REFLASH = 0x5,
    AP_POWER_MODE_CMD_DISP_ON = 0x6,
    AP_POWER_MODE_CMD_SOFT_RESET = 0x7,      //Soft reset command  from VIP for Diag 11 02
    AP_POWER_MODE_CMD_QCRITICAL = 0x8,
    AP_POWER_MODE_CMD_SHOW_ANIM = 0x31
}AP_POWER_MODE_CMD;

int check_logo_gpio(char *data){
        void *xml_handle = NULL;
        FILE *fd = NULL;
        char file_path[256];
        char *tmp;
        int gpio_number_logo;
        int gpio_level_logo;
        int gpio_value_logo;
        char buf[5];

//TBD once anim gpio enabled
#if 1
        xml_handle = xml_open(data);
        if (!xml_handle) {
          ALOGE("open setting file fail");
          return -1;
         }
	char gpio_num[] = "gpio_number_logo";
        tmp = xml_get_setting(xml_handle, gpio_num);
        ALOGE("Logo tmp gpio number %s\n", tmp);
        if(tmp) {
                gpio_number_logo = atoi(tmp);
                ALOGE("Logo gpio_number is %d", gpio_number_logo);
                xmlFree(tmp);
                sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_number_logo);
        } else {
                ALOGE("Can't find gpio_number in config file, return");
                return 0;
        }
#else
#endif
        while (!fd) {
                fd = fopen(file_path, "r");
                usleep(50000);
		ALOGE("GPIO open status is  %s ",strerror(errno));
        }
	char gpio_level[] ="gpio_level_logo";
        tmp = xml_get_setting(xml_handle, gpio_level);
        if(tmp) {
                gpio_level_logo = atoi(tmp);
                ALOGE("Animation gpio_level is %d", gpio_level_logo);
                xmlFree(tmp);
                if (gpio_level_logo){
                       ALOGE("Animation -GPIO level 1 -display anim");
                       fclose(fd);
                       return 1;
               }
               else{
                       ALOGE("Animation -GPIO level 0 -no anim -monitor gpio");
                       //return 0;
                       while(1){
                               rewind(fd);
                               fflush(fd);
                               fscanf(fd, "%d", &gpio_value_logo);
                               if(gpio_value_logo ==1){
                                       ALOGE("--Logo gpio val set to : %d",gpio_value_logo);
                                       fclose(fd);
                                       return 1;
                               }
                               usleep(100000);
                       }
               }
        } else {
                ALOGE("can't find gpio_level_logo in config file, return");
                return 0;
        }
}


namespace VSTRvc {
    using namespace android;
    using namespace std;

// ---------------------------------------------------------------------------

//    static void* notify_thread(void *data) {
//        ALOGD(" Enter [%s]",__FUNCTION__);
//        int *state = (int *)data;
//        if (*state == RVC_STATE_RUNNING) {
//            system("am broadcast -a android.visteon.intent.action.REAR_CAMERA_STATE_CHANGED --ez state 1 --user 0");
//        } else {
//            system("am broadcast -a android.visteon.intent.action.REAR_CAMERA_STATE_CHANGED --ez state 0 --user 0");
//        }
//        ALOGD(" Exit [%s]",__FUNCTION__);
//        return NULL;
//    }
    static char bootAnimStatus [20];

    VSTRvcService::VSTRvcService() {
        ALOGV(" Enter [%s]", __FUNCTION__);
        mRvcState = RVC_STATE_IDLE;
        mDisplayParameters.initialized = false;
        mBootAnimStopped = false;
        mDisplayParameters.mDisplayWidth = 0; //statements added 
        mDisplayParameters.mDisplayHeight = 0;
        mDisplayParameters.mXStart = 0;
        mDisplayParameters.mYStart = 0; //statements over
        ALOGV(" Exit [%s]", __FUNCTION__);
    }
    
    void VSTRvcService::setRvcBounds(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom) {

        ALOGV(" Enter [%s]",__FUNCTION__);

        mDisplayParameters.initialized = true;
        mDisplayParameters.mDisplayWidth = right - left;
        mDisplayParameters.mDisplayHeight = bottom - top;
        mDisplayParameters.mXStart = left;
        mDisplayParameters.mYStart = top;

        ALOGI("###########Starting Camera with########### ");
        ALOGI("mDisplayParameters.mDisplayWidth [%d]",
                mDisplayParameters.mDisplayWidth);
        ALOGI("mDisplayParameters.mDisplayHeight [%d]",
                mDisplayParameters.mDisplayHeight);
        ALOGI("mDisplayParameters.mXStart [%d]",
                mDisplayParameters.mXStart);
        ALOGI("mDisplayParameters.mYStart [%d]",
                mDisplayParameters.mYStart);

        ALOGV(" Exit [%s]", __FUNCTION__);
    }

    void VSTRvcService::startRvcCamera() {
        ALOGV(" Enter [%s]", __FUNCTION__);
        if (mRvcState == RVC_STATE_RUNNING) {
            ALOGE("RVC is already Running");
            return;

        }
	char xml_config_file_path_tmp[] = "/vendor/fastlogo_file/fastlogo_config_s2r.xml";
	if (!check_logo_gpio(xml_config_file_path_tmp)){
                ALOGE("Error in logo gpio ");
        }
        property_set("ctl.start", "s2ranim"); 	
	//start_camera_rvc(&mDisplayParameters);
        mRvcState = RVC_STATE_RUNNING;
//        THREAD_HANDLE(notify_handle);
//        THREAD_CREATE(notify_handle, notify_thread, &mRvcState);
        ALOGV(" Exit [%s]",__FUNCTION__);
        return;
    }

    void VSTRvcService::stopRvcCamera() {
        ALOGV(" Enter [%s]",__FUNCTION__);
        if (mRvcState == RVC_STATE_IDLE) {
            ALOGE("RVC is already RVC_STATE_IDLE");
            return;
        }
        //stop_camera_rvc();
        mRvcState = RVC_STATE_IDLE;
//        THREAD_HANDLE(notify_handle);
//        THREAD_CREATE(notify_handle, notify_thread, &mRvcState);
        ALOGV(" Exit [%s]",__FUNCTION__);
        return;
    }

    bool VSTRvcService::hasStarted() {
        ALOGV(" Enter [%s]",__FUNCTION__);
        return (mRvcState == RVC_STATE_RUNNING);
    }

    status_t VSTRvcService::dump(int fd, const Vector <String16> &args) {
    (void)fd;
    (void)args;
        ALOGV(" Enter [%s]",__FUNCTION__);
        return NO_ERROR;
    }

    void VSTRvcService::cleanUp() {
        //clean_camera_rvc();
    }

void VSTRvcService::setAnimStatus(std::string state) {
    char cmd_anim[200];
    snprintf(cmd_anim, sizeof(cmd_anim), "echo %s > /mnt/bootanim ",
            state.c_str());
    system(cmd_anim);
}

char const*const BOOT_ANIMATION_STATUS
        = "/mnt/bootanim";
int
read_bootAnim_status()
{
    FILE *fd_bl = NULL;
	char status [20] ="";
	fd_bl = fopen(BOOT_ANIMATION_STATUS, "r");
	//fscanf(fd_bl, "%s", status);fclose(fd_bl);
	if (fd_bl != NULL) {
	    if(fscanf(fd_bl, "%s", status) == EOF){
	    ALOGE("Error in opening file\n");   
	}
	fclose(fd_bl);
	}
	else {
	  ALOGE("Error opening file: %s", strerror(errno));
	}
	if(!strcmp(status,"stop")) {
	   return 0;
	} else {
	   return 1;
	}
}

void VSTRvcService::startBL() {
    if (read_bootAnim_status() == 1) {
        ALOGD("startBL -VSTRvcService, read_bootAnim_status is 1 , Hence setting brightness to 5291 !!!");
        char cmd_BL[200];
        snprintf(cmd_BL, sizeof(cmd_BL), "echo 5291 >  /sys/devices/platform/backlight_lcd0/backlight/backlight_lcd0/brightness ");
        system(cmd_BL);
    } else {
        ALOGD("startBL -VSTRvcService, read_bootAnim_status is 0 , DONT SET ANY BRIGHTNESS HERE !!!");
    }
    FILE *fd_BL;
    char bl_path[256] = "/sys/class/backlight/backlight_lcd0/bl_power";
    int retVal;
    fd_BL = fopen(bl_path, "wb+");
    if(fd_BL!=NULL ){ 
      retVal = fwrite("0",1,1,fd_BL);
      if(retVal > 0) {
        ALOGD("fd_BL ile write is successfull");
      }else{
        ALOGD("fd_BL ile write is successfull");
      }
      fclose(fd_BL);
    } else {
       ALOGE("FD IS NULL ");
    }
}

std::string VSTRvcService::getAnimState() {
    FILE *fd_bl = NULL;
    std::string animState;
    fd_bl = fopen("/mnt/bootanim", "r");
    if (fd_bl != NULL) {
        char status[20];
         if(fscanf(fd_bl, "%s", status) == EOF){     //added statemnts
            ALOGE("Error in reading file\n");
        }
        fclose(fd_bl);
        (animState.append(status));
    }
    return animState;
}


// ---------------------------------------------------------------------------
}; // namespace VSTRvc

void dtclogging()
{
    std::string dtc_hw_fault = "";
    char file_path[256] = "/sys/kernel/tp2860_vloss/vloss_status";
    int fault_value = 0;
    FILE* fd = NULL;
    if ((fd = fopen(file_path, "r")) != NULL) {
        if (fscanf(fd, "%d", &fault_value) != 1) {
            ALOGE("failed to read the fault value from the file");
        }
        fclose(fd);
    } else {
        ALOGE("failed to open and read vloss_status");
    }
    if (fault_value == 1) {
        ALOGE("cameraDTCLOG video loss fault in RVC %d", fault_value);
        dtc_hw_fault = DTC_VIDEOLOSS;
        DTCLOGE("%s ", dtc_hw_fault.c_str());
        property_set("vendor.rvc.videoloss", "1");
    } else {
        dtc_hw_fault = DTC_NOVIDEOLOSS;
        DTCLOGE("%s ", dtc_hw_fault.c_str());
        ALOGE("cameraDTCLOG no videoloss hence clearing DTC %d", fault_value);
    }
}

THREAD_HANDLE(monitor_fastrvc_exit);
void* monitorFastrvcexit(void* data)
{
    ALOGE("Stated waiting for fastrvc exit");
    char fastrvcexit[92];
    int old_rvc_value = -1;
    while (1) {
        property_get("vendor.fastrvc.exit", fastrvcexit, "0");
        int exit_done = atoi(fastrvcexit);
        if (exit_done) {
            ALOGE("UfastRvc exited -DTC logging stopped from vstrvc");
            set_fastrvcexit();
            break;
        }
        // LOG DTC only once when rev gear engaged
        if (gpio_value_rvc != old_rvc_value) {
            old_rvc_value = gpio_value_rvc;
            if (gpio_value_rvc)
                dtclogging();
        }
        usleep(100000);
    }
    return NULL;
}

THREAD_HANDLE(boot_anim_request_handle);
void* monitorBootAnimStatus(void* data)
{
    VSTRvc::VSTRvcService* service = (VSTRvc::VSTRvcService*)data;
    std::string state = service->getAnimState();
    std::strcpy(VSTRvc::bootAnimStatus, state.c_str());
    ALOGD(" monitorBootAnimStatus  Before INOTIFY  status : %s",VSTRvc::bootAnimStatus);
    char uiready[92] = { 0 };
    char animdone[92] = { 0 };
    char appready[92] = { 0 };
    if ((!strcmp(state.c_str(), "start")) || (!strcmp(state.c_str(), "playing")) || (!strcmp(state.c_str(), "stop"))) {

        ALOGD(" NO NEED FOR INOTIFY  status : %s", VSTRvc::bootAnimStatus);
        return NULL;
    } else if (!strcmp(state.c_str(), "init")) {
        ALOGE("Wait for %d sec for HMI_READY and anim played", (timeout_count / 5));
        while (timeout_count != 0) {
            property_get("vendor.animation.done", animdone, "0");
            property_get("vendor.vstsysui.ready", uiready, "0");
            if (atoi(uiready) && atoi(animdone)) {
                ALOGD("Animation done and HMI Ready :::::::::::::::::");
                break;
            } else {
                // Reduce logging
                if ((timeout_count % 10) == 0) {
                    ALOGI("PRIM_CODEC HMI Not up:::::::::::::::::, Continue...");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            timeout_count--;
        }
    } else {
        ALOGD("  NEED FOR WAIT FOR INOTIFY  status : %s", state.c_str());
    }
    property_get("vendor.animation.done", animdone, "0");
    // Send HMI_READY_ACK if animation played and timeout done
    if ((timeout_count == 0) && atoi(animdone)) {
        ALOGE("HMI ready not received (init)- timeout done -----sending VHAL_HMI_READY_ACK ");
        THREAD_CREATE(rvc, monitor_rvc, NULL);
        set_hmi_ready_ack();
        property_set("vendor.vstsysui.ready", "1");
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        property_get("vendor.rvc.home_ready", appready, "0");
        if (!(atoi(appready))) {
            ALOGE("Timeout done VHAL_ANIMATING_ACK not received - remove animation overlay");
            property_set("vendor.rvc.home_ready", "1");
        }
    }
    int sent = 0;
    int full_run_timeout = 35;
    int full_run_received = 0;
    while (1) {
        if (power_mode == 2) {
            ALOGE("Do not wait for animation from fastrvc- next anim from vstrvc");
            break;
        }

        if (((power_mode == 7) || (power_mode == 15)) && (!full_run_received)) {
            full_run_received = 1;
            ALOGE("VSTRVC FULL RUN received");
        }
        if (full_run_received) {
            if (!full_run_timeout) {
                property_set("vendor.animation.done", "1");
                ALOGE("Timeout done - GPIO not set after 3.5 sec of IGN ON - no animation");
            }
            full_run_timeout--;
        }
        property_get("vendor.animation.done", animdone, "0");
        property_get("vendor.vstsysui.ready", uiready, "0");
        if ((!atoi(animdone)) && atoi(uiready)) {
            if (!sent) {
                ALOGE("HMI Ready - Wait to play Animation");
                set_hmi_timeout_ready();
                sent = 1;
            }
        } else if (atoi(animdone) && atoi(uiready)) {
            break;
        } else if (atoi(animdone) && !atoi(uiready)) {
            property_set("vendor.rvc.home_ready", "1");
            break;
        }
        usleep(100000);
    }
    set_hmi_ready_ack();
    // wait two sec to recieve ANIMATING_ACK
    sleep(2);
    property_get("vendor.rvc.home_ready", appready, "0");
    if (!atoi(appready)) {
        ALOGE("HMI_ANIMATING_ACK not received close animation");
        property_set("vendor.rvc.home_ready", "1");
    }
    reset_hmi_ready();
    int fd;
    int wd;
    int length;
    char buffer[20];
    FILE* fd_bl = NULL;
    /*creating the INOTIFY instance*/
    fd = inotify_init();
    /*checking for error*/
    if (fd < 0) {
        ALOGE(" inotify_init failed ");
    }
    char const* const BOOT_ANIMATION_STATUS = "/mnt/bootanim";
    fd_bl = fopen(BOOT_ANIMATION_STATUS, "r");
    if (fd_bl == nullptr) {
        ALOGE("Unable to open file %s: error", BOOT_ANIMATION_STATUS);
        return nullptr;
    }
    if (fscanf(fd_bl, "%s", VSTRvc::bootAnimStatus) == EOF) {
        ALOGE("error in reading file VSTRvc::bootAnimStatus");
    } // added statements
    fclose(fd_bl);
    char xml_config_file_path_tmp[] = "/fastlogo_file/fastanimation_config.xml";
    while (1) {
        wd = inotify_add_watch(fd, BOOT_ANIMATION_STATUS, IN_MODIFY);
        length = read(fd, buffer, 20);
        if (length < 0) {
            ALOGE("read failed");
        }
        fd_bl = fopen(BOOT_ANIMATION_STATUS, "r");
        if (fd_bl != NULL) {
            if (fscanf(fd_bl, "%s", VSTRvc::bootAnimStatus) == EOF) { // added statements
                ALOGE("Error reading from file \n");
            }
            ALOGE("Animation_state_value %s", VSTRvc::bootAnimStatus);
            fclose(fd_bl);
        }
        ALOGD("Got Notify for  status : %s", VSTRvc::bootAnimStatus);
        if ((!strcmp(VSTRvc::bootAnimStatus, "start"))
            || (!strcmp(VSTRvc::bootAnimStatus, "playing"))
            || (!strcmp(VSTRvc::bootAnimStatus, "stop"))) {
            // if (check_logo_gpio(xml_config_file_path_tmp)){
            break;
            //}
        }
    }
    close(fd);
    return nullptr;
}

THREAD_HANDLE(boot_anim_stop_handle);
void* stopAnimThread(void* data) {
	ALOGE("stopAnimThread Enter");
	VSTRvc::VSTRvcService* service = (VSTRvc::VSTRvcService*) data;
	service->stopRvcCamera();
	service->cleanUp();
	service->mBootAnimStopped = true;
	//clean_logo_thread();
	ALOGE("stopAnimThread Exit");
	return NULL;
}
THREAD_HANDLE(boot_anim_handle);
static void* boot_anim_thread(void *data) {
    VSTRvc::VSTRvcService* service =  (VSTRvc::VSTRvcService*)data;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ALOGD("Wait for trigger from Powermodule");
    THREAD_CREATE(boot_anim_request_handle, monitorBootAnimStatus, data);
    pthread_join(boot_anim_request_handle, NULL);


#ifdef BOOT_ANIM_TEST
    if (!service->hasStarted()) {
        //service->setRvcBounds(0, 0, 1552, 720);
        service->startRvcCamera();
    }
#else
     if ((!strcmp(VSTRvc::bootAnimStatus, "start"))
            || (!strcmp(VSTRvc::bootAnimStatus, "playing"))) {
        ALOGW("BOOT ANIMATION START/PLAY REQUEST RECEIVED %s ",
                VSTRvc::bootAnimStatus);
        if (!service->hasStarted()) {
            //service->setRvcBounds(0, 0, 1552, 720);
            //service->startBL();
            service->startRvcCamera();
            service->setAnimStatus("playing");
            //Once boot animation started then we should show for 3 sec at least
            // as per the requirment
            std::this_thread::sleep_for(std::chrono::milliseconds(1800));

        }
    } else if ((!strcmp(VSTRvc::bootAnimStatus, "stop"))) {
        ALOGW("BOOT ANIMATION STOP REQUEST RECEIVED");
        return NULL;
    }
#endif
    ALOGE("S2R :Started, Waiting for HMI and Full run");
    //Wait for 4sec for HMI ready after s2r
    int count = 20;
    char power_state[PROPERTY_VALUE_MAX];
    char HmiS2RReady[92];
    char S2Rappready[92];
    while (count != 0) {
	property_get("vendor.vstsysui.ready",HmiS2RReady,"0");
        if (atoi(HmiS2RReady) && (power_mode==7)) {
            break;
        } else {
            ALOGI("PRIM_CODEC HMI Not up:::::::::::::::::, Continue...");
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        count--;
    }
    if(count==0){
	    ALOGE("S2R HMI ready not received - timeout done ");
	    property_set("vendor.vstsysui.ready","1");
    }
    ALOGE("S2R:send HMI_READY_ACK");
    set_hmi_ready_ack();
    //wait 1 sec to receive ANIMATING_ACK
    int wait_ack = 10;
    while (wait_ack != 0) {
        property_get("vendor.rvc.home_ready",S2Rappready,"0");
	if (atoi(S2Rappready)) {
            break;
        } else {
	    usleep(100000);
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        wait_ack--;
    }
    THREAD_CREATE(boot_anim_stop_handle, stopAnimThread, data);
    if(wait_ack==0){
	    ALOGE("S2R :VHAL_ANIMATING_ACK timeout done"); 
	    property_set("vendor.rvc.home_ready", "1");
    }
    ALOGE("S2R :STOPPED  Animation playback NOW ::::::::::::::::::::::::::::::::");

    int refCount = 2;
    while(!service->mBootAnimStopped && refCount != 0) {
        refCount--;
        ALOGW("Animation not stopped yet Continue...");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    property_get("vhal.power.state.request", power_state, "3");
    ALOGD("backup on bl_power ::::::state ::::%s",power_state);
    switch(atoi(power_state)) {
    case AP_POWER_MODE_CMD_OFF:
    case AP_POWER_MODE_CMD_EMERGENCY_OFF:
    case AP_POWER_MODE_CMD_DISP_OFF:
    case AP_POWER_MODE_CMD_SYSTEM_IDLE:
    case AP_POWER_MODE_CMD_DISP_ON:
        break;
    default:
        ALOGI("*****MAKE_BL_POWER**ON****,");
        service->startBL();
        ALOGI("*****MAKE_BL_POWER*ON*DONE*****,");
        break;
    }
    service->setAnimStatus("stop");
    return NULL;
}

int main(int argc, char **argv) {
    ALOGV(" Enter [%s]",__FUNCTION__);

    (void)argc;
    (void)argv;
#if 1
    VSTRvc::VSTRvcService* service = new VSTRvc::VSTRvcService();
    subscribetovhalprop(service);
    THREAD_CREATE(boot_anim_handle, boot_anim_thread, service);
    THREAD_CREATE(monitor_fastrvc_exit, monitorFastrvcexit,NULL);
    pthread_join(boot_anim_handle, NULL);
    //delete service;
#else
    android::defaultServiceManager()->addService(android::String16("VSTRvcService"),
            new VSTRvc::VSTRvcService());
    android::ProcessState::self()->startThreadPool();
    ALOGD("VSTRvcService service is now ready");

    android::IPCThreadState::self()->joinThreadPool();
    ALOGD("VSTRvcService service thread joined");
#endif
    //reset_hmi_ready();
    ALOGV(" Exit [%s]",__FUNCTION__);
    ALOGD("Exit done after first S2R animation - monitor for S2R animation trigger");
    while(1){
        if (service->getAnimState() == "start") {
            THREAD_CREATE(boot_anim_handle, boot_anim_thread, service);
            usleep(100000);
            pthread_join(boot_anim_handle, NULL);
            reset_hmi_ready();
        }
        usleep(200000);
    }

    return 0;
}



#include <sys/prctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "module.h"
#include "drm_display.h"
#include "util_rvc.h"
#include "xml.h"
#include "VST_rvc_impl.h"
#include <pthread.h>
#include "libxml/parser.h"
#include <cutils/properties.h>

#define USLEEP_UNIT 100000 // 100ms
#define ANIMATED_LOGO_COUNT 20

typedef struct logo_thread_parameter {
    void * handle;
    int mLogoState;
    display_parameters *mDisplayParameters;
}logo_thread_parameter;

static void* rvc_detect_thread(void *data);
static void* logo_thread(void *data);
static bool check_rvc();
int clean_logo_thread();
int check_logo_gpio(void *data);
static logo_thread_parameter *logo_thread_data = NULL;
static bool write_gpio_file(char * file_path, char * buf);

THREAD_HANDLE(rvc_handle);
THREAD_HANDLE(logo_handle);

static link_path *logo_link = NULL;
void *xml_handle = NULL;
char *config_file_path = "/fastlogo_file/fastanimation_config.xml";

MUTEX_HANDLE(mutex_link);

#ifdef SIGNAL_HANDLER
static void sig_handler(int sig);
#endif
int check_logo_gpio(void *data){
        FILE *fd = NULL;
        char file_path[256];
        char * tmp;
        int gpio_number_logo;
        int gpio_level_logo;
        int gpio_value_logo;
        char buf[5];

//TBD once anim gpio enabled
#if 1
	xml_handle = xml_open(data);
        if (!xml_handle) {
          LOG_ERR("open setting file fail");
          return -1;
         }
        tmp = xml_get_setting(xml_handle, "gpio_number_logo");
        LOG_BOOT_PROFILE("Logo tmp gpio number %s\n", tmp);
        if(tmp) {
                gpio_number_logo = atoi(tmp);
                LOG_BOOT_PROFILE("Logo gpio_number is %d", gpio_number_logo);
                xmlFree(tmp);
                sprintf(file_path, "/sys/class/gpio/gpio%d", gpio_number_logo);
                if (access(file_path, F_OK)) {
                        sprintf (file_path, "/sys/class/gpio/export");
                        sprintf(buf, "%d", gpio_number_logo);
                        if(!write_gpio_file(file_path, buf)) {
                                return 0;
                        }
                }
                sprintf (file_path, "/sys/class/gpio/gpio%d/direction", gpio_number_logo);
                sprintf (buf, "%s", "in");
                if(!write_gpio_file(file_path, buf)) {
                        return 0;
                }
                sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_number_logo);
        } else {
                LOG_BOOT_PROFILE("Can't find gpio_number in config file, return");
                return 0;
        }
#else
        // for debug and stress test
        sprintf(file_path, "/bin/rvc_flag");
#endif
        while (!fd) {
                fd = fopen(file_path, "r");
                usleep(50000);
        }
        tmp = xml_get_setting(xml_handle, "gpio_level_logo");
        if(tmp) {
                gpio_level_logo = atoi(tmp);
                LOG_BOOT_PROFILE("Animation gpio_level is %d", gpio_level_logo);
                xmlFree(tmp);
                if (gpio_level_logo){
                       LOG_BOOT_PROFILE("Animation -GPIO level 1 -display anim");
                       fclose(fd);
                       return 1;
               }
               else{
                       LOG_BOOT_PROFILE("Animation -GPIO level 0 -no anim -monitor gpio");
                       //return 0;
                       while(1){
			       rewind(fd);
                               fflush(fd);
                               fscanf(fd, "%d", &gpio_value_logo);
                               if(gpio_value_logo ==1){
				       LOG_BOOT_PROFILE("--Logo gpio val set to : %d",gpio_value_logo);
                                       fclose(fd);
                                       return 1;
                               }
                               usleep(100000);
                       }
               }
        } else {
                LOG_BOOT_PROFILE("can't find gpio_level_logo in config file, return");
                return 0;
        }

}
int stop_camera_rvc() {
    ALOGD(" Enter [%s]", __FUNCTION__);
    if (logo_link) {
        link_stop_modules(logo_link);
    }
    if (logo_thread_data != NULL) {
        logo_thread_data->mLogoState = CAMERA_STATE_PAUSED;
    }
    ALOGD(" Exit [%s]", __FUNCTION__);
    return 0;
}

int clean_camera_rvc() {
    ALOGD(" Enter [%s]", __FUNCTION__);
    if (logo_thread_data != NULL) {
        logo_thread_data->mLogoState = CAMERA_STATE_STOPPED;
    }

#ifdef FASTRVC_CAMERA_SUPPORT
    LOG_INFO("rvc_handle waited");
    THREAD_WAIT(rvc_handle);
#endif
    THREAD_WAIT(logo_handle);
    if (xml_handle != NULL) {
        xml_close(xml_handle);
    }
    ALOGD(" Exit [%s]", __FUNCTION__);
    return 0;
}

int start_camera_rvc(display_parameters *cameraDisplayParameters){

    ALOGD(" Enter [%s]",__FUNCTION__);

    MUTEX_LOCK(mutex_link);
    if (logo_thread_data != NULL) {
        logo_thread_data->mLogoState = CAMERA_STATE_RESUMED;
        ALOGI("RESUMING VSTRVC");
        MUTEX_UNLOCK(mutex_link);
        return 0;
    }

    logo_thread_data = (logo_thread_parameter*) malloc(
            sizeof(logo_thread_parameter));
    if (logo_thread_data == NULL) {
        LOG_ERR("Failed to allocate memory logo_thread_data");
        MUTEX_UNLOCK(mutex_link);
        return -1;
    }

    memset(logo_thread_data, 0, sizeof(logo_thread_parameter));

    logo_thread_data->mLogoState = CAMERA_STATE_IDLE;

    xml_handle = xml_open(config_file_path);
    if (!xml_handle) {
        LOG_ERR("open setting file fail");
        MUTEX_UNLOCK(mutex_link);
        return -1;
    }

    log_level_init(xml_handle);
    device_path_init();
    MUTEX_UNLOCK(mutex_link);
    logo_link = NULL;

    logo_thread_data->handle = xml_handle;

    if (cameraDisplayParameters != NULL) {
        logo_thread_data->mDisplayParameters = (display_parameters*)malloc(sizeof(display_parameters));
        if(logo_thread_data->mDisplayParameters){
            memcpy(logo_thread_data->mDisplayParameters,cameraDisplayParameters,sizeof(display_parameters));
            xml_set_display_parameters(logo_thread_data->mDisplayParameters);
        }
    } else {
        logo_thread_data->mDisplayParameters = NULL;
    }
#ifdef SIGNAL_HANDLER
    /* signal handler */
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigaction(SIGINT, &act, NULL);
    logo_thread_data->mLogoState = CAMERA_STATE_INITIALIZED;
#endif
    /* init */
    //MUTEX_INIT(mutex_link);
#ifdef FASTRVC_CAMERA_SUPPORT
    THREAD_CREATE(rvc_handle, rvc_detect_thread, xml_handle);
#endif

    if (!check_logo_gpio(config_file_path)){
               LOG_ERR("Error in animation GPIO");
               return 0;
       }
    else {
               LOG_ERR("Animation GPIO set - continue");
    }
    THREAD_CREATE(logo_handle, logo_thread, logo_thread_data);

    ALOGD(" Exit [%s]", __FUNCTION__);
    return 0;
}

#ifdef SIGNAL_HANDLER
static void sig_handler(int sig) {
    static int again = 0;
    if (again)
        exit(0);
    again = 1;

    switch (sig) {
    default:
    case SIGINT:
        MUTEX_LOCK(mutex_link);
        if (logo_link)
            link_stop_modules(logo_link);
        MUTEX_UNLOCK(mutex_link);
        exit(0);
        break;
    }
}
#endif
#pragma GCC diagnostic ignored "-Wunused-function"
 static void pause_logo() {
    MUTEX_LOCK(mutex_link);
    LOG_ERR("INSIDE_PAUSE_LOGO");
    if (logo_link != NULL)
        link_pause_modules(logo_link);
    MUTEX_UNLOCK(mutex_link);
}

 static void resume_logo() {
    MUTEX_LOCK(mutex_link);
    LOG_ERR("INSIDE_RESUME_LOGO");
    if (logo_link != NULL) {
        LOG_ERR("INSIDE_RESUME_LOGO_IS_NOT_NULL");
        link_active_modules(logo_link);
        /* waiting for logo display*/
        MUTEX_UNLOCK(mutex_link);
        usleep(50000);
        MUTEX_LOCK(mutex_link);
    }
    MUTEX_UNLOCK(mutex_link);
}
#pragma GCC diagnostic pop
static void* logo_thread(void *data) {
    prctl(PR_SET_NAME, "logo_thread");
    ALOGI("logo_thread start");

    link_path *link;
    struct logo_thread_parameter * thread_data =
            (struct logo_thread_parameter *) data;

    link = xml_create_animated(thread_data->handle);
    if (link == NULL) {
        LOG_ERR("create link fail");
        return NULL;
    }

    int ret;
    ret = link_init_modules(link);
    if (0 != ret) {
        LOG_ERR("link_init_modules fail, ret=%d", ret);
        return NULL;
    }

    /* animated logo */
    MUTEX_LOCK(mutex_link);
    logo_link = link;
    MUTEX_UNLOCK(mutex_link);

    /* camera_thread will resume logo if in rvc */
    if (logo_thread_data->mLogoState != CAMERA_STATE_STOPPED) {
	ret = link_active_modules(logo_link);
        if (0 == ret) {
            LOG_ERR("animated link_active_modules pass");
//            usleep(ANIMATED_LOGO_COUNT * USLEEP_UNIT);
//            // we will show logo until home is ready
//            while (logo_thread_data->mLogoState != CAMERA_STATE_PAUSED) {
//                usleep(USLEEP_UNIT);
//            }
        } else {
            LOG_ERR("link_active_modules fail, ret=%d", ret);
        }
    }
    LOG_INFO("animated logo active");

    if (logo_link) {
        while(logo_thread_data->mLogoState != CAMERA_STATE_STOPPED) {
            usleep(USLEEP_UNIT);
        }
        link_pause_modules(logo_link);
    }

    ALOGI("logo_thread exit");

    return NULL;
}


bool is_home_ready() {
    const char PROP_APP_READY [] = "vendor.app.ready";
    const char PROP_SYSTEM_UI_READY [] = "vendor.systemui.ready";
    const int PROP_ENABLED_VAL = 1;

    char primDispAppPropvalue[512] = {0};
    char systemUIPropvalue[512] = {0};

    //read the HMI props.
    property_get(PROP_APP_READY, primDispAppPropvalue, "-1");
    property_get(PROP_SYSTEM_UI_READY, systemUIPropvalue, "-1");
    int appReady = atoi(primDispAppPropvalue);
    int systemUIReady = atoi(systemUIPropvalue);

    if(appReady == PROP_ENABLED_VAL && systemUIReady == PROP_ENABLED_VAL) {
        ALOGI("PRIM_CODEC HMI properties are set::::::::.");
        return true;
    }
    return false;
}

static volatile bool rvc = false;
static volatile int block_check = 1;
static bool write_gpio_file(char * file_path, char * buf)
{
    int gpio_file;
    size_t count;
    gpio_file = open(file_path, O_WRONLY | O_CREAT, S_IWUSR|S_IWGRP|S_IWOTH);
    if(gpio_file == -1) {
        LOG_ERR("open %s fail, return", file_path);
        // close(gpio_file);
        return false;
    }
    count = write(gpio_file, buf, strlen(buf));
    if(count != strlen(buf)) {
        LOG_ERR("write %s to %s fail, return", buf, file_path);
        close(gpio_file);
        return false;
    }
    close(gpio_file);
    return true;
}
static void* rvc_detect_thread(void *data)
{
    FILE *fd = NULL;
    char file_path[256];
    char * tmp;
    int gpio_number;
    int gpio_level;
    int gpio_value;
    char buf[5];
    bool gpio_last_status;
    bool gpio_current_status;

    prctl(PR_SET_NAME, "rvc_detect_thread");
    LOG_INFO("rvc_detect_thread start");
#if 1
    tmp = xml_get_setting(data, "gpio_number");
    if(tmp) {
        gpio_number = atoi(tmp);
        LOG_DBG("gpio_number is %d", gpio_number);
        xmlFree(tmp);
        sprintf(file_path, "/sys/class/gpio/gpio%d", gpio_number);
        if (access(file_path, F_OK)) {
            sprintf (file_path, "/sys/class/gpio/export");
            sprintf(buf, "%d", gpio_number);
            if(!write_gpio_file(file_path, buf)) {
                block_check = 0;
                return 0;
            }
        }
        sprintf (file_path, "/sys/class/gpio/gpio%d/direction", gpio_number);
        sprintf (buf, "%s", "in");
        if(!write_gpio_file(file_path, buf)) {
            block_check = 0;
            return 0;
        }
        sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_number);
    } else {
        LOG_ERR("can't find gpio_number in config file, return");
        block_check = 0;
        return 0;
    }
#else
    // for debug and stress test
    sprintf(file_path, "/data/local/tmp/rvc_flag");
#endif
    tmp = xml_get_setting(data, "gpio_level");
    if(tmp) {
        gpio_level = atoi(tmp);
        LOG_DBG("gpio_level is %d", gpio_level);
        xmlFree(tmp);
    } else {
        LOG_ERR("can't find gpio_level in config file, return");
        block_check = 0;
        return 0;
    }
    while (!fd) {
        fd = fopen(file_path, "r");
        if (!fd) {
            rvc = false;
            block_check = 0;
            usleep(50000);
        }
    }
    gpio_last_status = gpio_current_status = false;
    while (logo_thread_data->mLogoState != CAMERA_STATE_STOPPED) {
        rewind(fd);
        fflush(fd);
        if(fscanf(fd, "%d", &gpio_value) != 1){
            ALOGE("Failed to read GPIO value");
        }
        rvc = (gpio_value == gpio_level) ? true : false;

        gpio_current_status = rvc;
        block_check = 0;

        if (gpio_current_status != gpio_last_status) {
            if (rvc == true) {
                LOG_DBG("rvc key up--->down");
                isLogoPause = 1;
                //pause_logo();
            } else {
                LOG_DBG("rvc key down--->up");
                isLogoPause = 0;
                //resume_logo();
            }
        }

        gpio_last_status = gpio_current_status;
        usleep(50000);
    }
    fclose(fd);
    LOG_INFO("rvc_detect_thread exit");
    return 0;
}
int clean_logo_thread(){
        LOG_INFO("clean logo_thread data");
        MUTEX_LOCK(mutex_link); //added 
        if (logo_thread_data!= NULL) {
        free(logo_thread_data);
        logo_thread_data = NULL;
	LOG_ERR("logo_thread data cleared");
        }
        MUTEX_UNLOCK(mutex_link);
        return 0;
}
#pragma GCC diagnostic ignored "-Wunused-function"
static bool check_rvc() {
#ifdef FASTRVC_CAMERA_SUPPORT
    while (block_check);
    return rvc;
#else
    return false;
#endif
}
#pragma GCC diagnostic pop

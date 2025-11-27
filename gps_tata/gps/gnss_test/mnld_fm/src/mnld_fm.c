#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <stdlib.h>
#include "hal2mnl_interface.h"
#include "mnld_fm.h"
#include "mtk_lbs_utility.h"
#include "mtk_auto_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "mnldfm"
#endif

int log_dbg_level = L_VERBOSE;

GpsCallbacks_ext* mnld_fm_cbs = NULL;
GpsInterface_ext* mnld_fm_gpsinfs = NULL;

timer_t mnld_fm_test_timer = 0;
MNLD_FM_RESTART_TYPE mnld_fm_restart_type = MNLD_FM_RESTART_TYPE_HOT;
extern mnld_fm_test_result mnld_fm_test_result_body;
int mnld_fm_test_timeout = 0;
int mnld_fm_test_wait_time = 10;//wait time , second

void mnld_fm_show_help(void)
{
    printf("MNLD factory mode test :\r\n");
    printf("------------------------------------------------------------------------------\r\n");
    printf("For GNSS factory mode test or HW check.\r\n");
    printf("------------------------------------------------------------------------------\r\n");
    printf("The command to start MNLD factory mode test:\r\n");
    printf("\tfm_gnss\r\n");
}

void mnld_fm_open_gnss(MNLD_FM_RESTART_TYPE restart_type, GpsInterface_ext* gps_interface, GpsCallbacks_ext* gps_cbs)
{
    GpsCallbacks_ext* cbs = gps_cbs;
    GpsInterface_ext* GpsInterface_ext = gps_interface;

    switch(restart_type) {
        case MNLD_FM_RESTART_TYPE_HOT:
            LOGD("Hot Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_RTI);
            break;
        case MNLD_FM_RESTART_TYPE_WARM:
            LOGD("Warm Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_EPHEMERIS);
            break;
        case MNLD_FM_RESTART_TYPE_COLD:
            LOGD("Cold Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_EPHEMERIS |
                GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO |
                GPS_DELETE_UTC | GPS_DELETE_HEALTH);
            break;
        case MNLD_FM_RESTART_TYPE_FULL:
            LOGD("Full Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_ALL);
            break;
        default:
            LOGE("ERR: read unhandled value=[%d]\n", restart_type);
            return;
    }

    if(GpsInterface_ext != NULL && cbs != NULL)
    {
        GpsInterface_ext->init(cbs);
        //hal2mnl_update_network_state(1,NETWORK_TYPE_WIFI,0,"NULL");
        //hal2mnl_update_network_state(mnld_fm_network_on,mnld_fm_network_type,mnld_fm_network_roaming,"NULL");
        GpsInterface_ext->start();
    }else{
        LOGE("[%s]param error:%d, %d\r\n",GpsInterface_ext, cbs);
    }
}

void mnld_fm_close_gnss(GpsInterface_ext* gps_interface)
{
    GpsInterface_ext* GpsInterface_ext = gps_interface;
//    GpsInterface_ext = gps_device__get_gps_interface("mnld_fm stop");
    if(GpsInterface_ext != NULL)
    {
        GpsInterface_ext->cleanup();
        GpsInterface_ext->stop();
    }else{
        LOGE("param error");
    }

}

void mnld_fm_test_timeout_hdlr(void)
{
    mnld_fm_test_timeout = 1;
}

void main(int argc, char** argv)
{
    int exit_flag = 0;
    int wait_time = 0;
    int idx = 0;
    char *error;
    struct gps_device_t_ext *gpsdev = NULL;
    void *handle;

    //Show the recieved command
    for(idx=0; idx<argc; idx++)
    {
        printf("%s ",argv[idx]);
    }
    printf("\r\n");

    if(argc == MNLD_FM_CMD_CNT_MIN)
    {
        exit_flag = 0;
    }else{
        LOGE("Unknown command!");
        mnld_fm_show_help();
        exit_flag = 1;
    }

    if(!exit_flag)
    {
        handle = dlopen(LIB_GNSS_HAL_DIR"/libgpshal.so.0", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "%s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        gpsdev = (struct gps_device_t_ext *)dlsym(handle, "linux_gps_device");
        if ((error = dlerror()) != NULL) {
            fprintf(stderr, "%s\n", error);
            exit(EXIT_FAILURE);
        }
        mnld_fm_gpsinfs = (GpsInterface_ext*)gpsdev->get_gps_interface(gpsdev);

        mnld_fm_cbs = mnld_fm__get_gps_callbacks();//&mnld_fm_gps_callbacks;
        mnld_fm_test_result_body.test_stage = MNLD_FM_TEST_OPEN;
        mnld_fm_test_result_body.sv_num = 0;
        memset(mnld_fm_test_result_body.sv_list, 0, GNSS_MAX_SVS);
        mnld_fm_test_timeout = 0;

        mnld_fm_open_gnss(mnld_fm_restart_type,mnld_fm_gpsinfs,mnld_fm_cbs);
        if (mnld_fm_test_result_body.test_stage < MNLD_FM_TEST_OPENED) {  //Resolve timing issue, if stage value is bigger than opend no need to set to opened any more
            mnld_fm_test_result_body.test_stage = MNLD_FM_TEST_OPENED;
        }
        //system("clear");
        LOGD("GNSS opened for factory mode testing...");
       // mnld_fm_test_timer = init_timer(mnld_fm_test_timeout_hdlr);
       // start_timer(mnld_fm_test_timer,mnld_fm_restart_interval*1000);
        do{
            usleep(1000);
            wait_time++;
            if(wait_time >= mnld_fm_test_wait_time*1000)
            {
                mnld_fm_test_timeout = 1;
                LOGW("GNSS factory mode test timeout...");
                break;
            }
        }while(mnld_fm_test_result_body.sv_num < 2);
        system("clear");
        LOGD("GNSS factory mode test stage:%d, sv num:%d!",mnld_fm_test_result_body.test_stage, mnld_fm_test_result_body.sv_num);
        if(mnld_fm_test_result_body.sv_num >= 2 /*&& mnld_fm_test_result_body.test_stage >= MNLD_FM_TEST_SV_SEARCHED*/)
        {
            printf("GNSS factory mode test PASS!\r\nSearched %d satellites:",mnld_fm_test_result_body.sv_num);
            for(idx=0; idx<mnld_fm_test_result_body.sv_num; idx++)
            {
                printf(" %d",mnld_fm_test_result_body.sv_list[idx]);
            }
            printf("\r\n");
            exit_flag = 1;
        }else{
            switch(mnld_fm_test_result_body.test_stage)
            {
                case MNLD_FM_TEST_OPEN:
                    LOGD("GNSS factory mode test open FAIL!");
                    break;
                case MNLD_FM_TEST_OPENED:
                    LOGD("GNSS factory mode test engine start FAIL!");
                    break;
                case MNLD_FM_TEST_ENGINE_STARTED:
                    LOGD("GNSS factory mode test sv search FAIL(sv num:%d)!",mnld_fm_test_result_body.sv_num);
                    break;
                case MNLD_FM_TEST_SV_SEARCHED:
                    LOGD("GNSS factory mode test sv search FAIL(sv num:%d)!",mnld_fm_test_result_body.sv_num);
                    break;
                default:
                    LOGD("GNSS factory mode test FAIL(stage:%d, sv num:%d)!",mnld_fm_test_result_body.test_stage, mnld_fm_test_result_body.sv_num);
                    break;
            }
        }
        LOGD("Test Time: %dms",wait_time);
        mnld_fm_close_gnss(mnld_fm_gpsinfs);
        exit_flag = 1;

    }
}

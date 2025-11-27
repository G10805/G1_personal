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
#include "gps_mtk.h"
#include "mnld_test.h"
#include "mtk_lbs_utility.h"
#include "mtk_auto_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "mnldtest"
#endif

int log_dbg_level = L_VERBOSE;

#ifdef CONFIG_GPS_MT3303
#define MNLD_TEST_CHIP_VER "MT3303"
#else
#define MNLD_TEST_CHIP_VER "MT6630"
#endif
#define MNLD_TEST_MNL_VER mnld_test_mnl_ver
#define MNLD_TEST_CLOCK_TYPE 0xFF
#define MNLD_TEST_CLOCK_BUFF 2

/*Socket port nubmer*/
#define PORT 7000
/*The max length of socket receive buffer*/
#define MNL_TEST_REC_BUFF_LEN 2048

#define MNL_TEST_MNL_VER_PMTK "$PMTK705"
#define mnld_test_printf printf

GpsCallbacks_ext* mnld_test_cbs = NULL;
GpsInterface_ext* mnld_test_gpsinfs = NULL;


int valid_ttff_cnt = 0;
int valid_ttff_sum = 0;

int mnld_test_session_end = 0;
int mnld_test_ttff = 0;
int mnld_test_restart_cnt = 0;
int mnld_test_restart_time = 0;
int mnld_test_restart_interval = 0;

int mnld_test_network_type = NETWORK_TYPE_WIFI;
int mnld_test_network_on = 0;
int mnld_test_network_roaming = 0;

timer_t mnld_test_restart_timer = 0;
MNLD_TEST_RESTART_TYPE mnld_test_restart_type = MNLD_TEST_RESTART_TYPE_HOT;
struct timespec mnld_test_gnss_open_tm;
char mnld_test_mnl_ver[MNL_VER_LEN];
extern mnld_test_result mnld_test_result_body;

clock_type mnld_test_clock_type[] = {
    {0xFE,"Co-Clock"},
    {0xFF,"TCXO"}
};

#define MNLD_TEST_NETWORK_TYPE_STR_LEN 8
const char mnld_test_network_type_str[][MNLD_TEST_NETWORK_TYPE_STR_LEN] = {
    {"mobile"},
    {"wifi"}
};

void mnld_test_show_help(void)
{
    mnld_test_printf("MNLD client test :\r\n");
    mnld_test_printf("------------------------------------------------------------------------------\r\n");
    mnld_test_printf("The command to start GNSS test:\r\n");
    mnld_test_printf("\tmnld_test start [start type] [restart times] [restart interval]\r\n");
    mnld_test_printf("\t\t[start type]: \r\n\t\th/H: hot start;\r\n\t\tw/W: Warm start;\r\n\t\tc/C: Cold start;\r\n\t\tf/F: Full start\r\n");
    mnld_test_printf("\t\t[restart times]: integer value, range is 0-1000, default is 0(no restart, always on). \r\n");
    mnld_test_printf("\t\t[restart interval]: integer value, range is 0-3600, the unit is second; the default value is 60\r\n");
    mnld_test_printf("------------------------------------------------------------------------------\r\n");
    mnld_test_printf("The command to stop GNSS test:\r\n");
    mnld_test_printf("\tmnld_test stop\r\n");
    mnld_test_printf("------------------------------------------------------------------------------\r\n");
    mnld_test_printf("The command to update network status:\r\n");
    mnld_test_printf("\tmnld_test network [type] [roaming]\r\n");
    mnld_test_printf("\t[type]: wifi, mobile, disable\r\n");
    mnld_test_printf("\t[roaming]: roaming, the mobile network is in roaming state\r\n");
}

/*
Function:mnld_test_socket_open
Description:open and connect a INET socket by given port number
Param:[IN] port, the port number of socket
Param:[OUT] fd, the socket fd
Return:NULL -- some thing is incorrect; Other value -- open and connect sokcet successfully
*/
int mnld_test_socket_open(int port)
{
    struct sockaddr_in servaddr;
    int socketfd = 0;

    if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOGE("Create socket error:%d,%s\n", errno, strerror(errno));
        return socketfd;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if( connect(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0 )
    {
        LOGE("connect error:%d,%s\n", errno, strerror(errno));
        return -1;
    }

    return(socketfd);
}

#ifdef CONFIG_GPS_MT3303
void mnld_test_get_mnl_ver(void )
{
    LOGI("GNSS chip is 3303, there is no MNL.");
    strcpy(mnld_test_mnl_ver, "GNSS chip is 3303,there is no MNL");
}
#else
#ifdef MTK_ADR_SUPPORT
void mnld_test_get_mnl_ver(void )
{
    LOGI("ADR has used the port 7000, the mnl version is static.");
    strcpy(mnld_test_mnl_ver, "MNL_VER_18070301ALPS05_5.5U_25");
}
#else
void mnld_test_get_mnl_ver(void )
{
    int sock_fd = 0;
    int rec_len = 0;
    int raw_socket_connected = 1;
    char rec_buff[MNL_TEST_REC_BUFF_LEN+1] = {0};
    char *mnl_ver_addr = NULL;
    int i = 0;
    int got_mnl_ver = 0;
    int retry_cnt = 0;

    if((sock_fd = mnld_test_socket_open(PORT)) < 0 )
    {
        LOGE("Socket open error\n");
        strcpy(mnld_test_mnl_ver,"UNKNOWN");
        //raw_socket_connected = 0;
    }else//    if(raw_socket_connected)
    {
        char *outbuff = "$PMTK605*31\r\n";
        int cmd_sent = 0;
        do{
           // if((cmd_sent == 0) && send(sock_fd,outbuff,strlen(outbuff)+1, 0) == -1)
            if((cmd_sent == 0) && hal2mnl_send_pmtk(outbuff,strlen(outbuff)) == -1)
            {
                cmd_sent = 0;
                LOGE("Socket send error(%s)\n",strerror(errno));
            }else{
                cmd_sent = 1;
                if((rec_len = recv(sock_fd, rec_buff, MNL_TEST_REC_BUFF_LEN, 0)) < 0)
                {
                    LOGE("Recieve error(%d):%s\n",errno, strerror(errno));
                    usleep(20000);
                }else{
                    rec_buff[rec_len] = '\0';
                    if(strncmp(rec_buff, MNL_TEST_MNL_VER_PMTK, strlen(MNL_TEST_MNL_VER_PMTK)) == 0)
                    {
                        mnl_ver_addr = strstr(rec_buff,"MNL_VER");
                        strncpy(mnld_test_mnl_ver, mnl_ver_addr, strlen(mnl_ver_addr));
                        for(i=0;i<strlen(mnld_test_mnl_ver);i++)
                        {
                            if(mnld_test_mnl_ver[i] == ',')
                            {
                                mnld_test_mnl_ver[i] = '\0';
                                break;
                            }
                        }
                        got_mnl_ver = 1;
                        LOGD("\nRCV[%d]:%s\n", rec_len, rec_buff);
                    }
                    //Parser
                }
            }
            if(retry_cnt ++ >= 5)
            {
                LOGE("Get mnl version fail\r\n");
                strcpy(mnld_test_mnl_ver,"UNKNOWN");
                break;
            }
        }while(!got_mnl_ver);
        close(sock_fd);
    }
}
#endif
#endif

void mnld_test_open_gnss(MNLD_TEST_RESTART_TYPE restart_type, GpsInterface_ext* gps_interface, GpsCallbacks_ext* gps_cbs)
{
    GpsCallbacks_ext* cbs = gps_cbs;
    GpsInterface_ext* gpsinterface = gps_interface;
    switch(restart_type) {
        case MNLD_TEST_RESTART_TYPE_HOT:
            LOGD("Hot Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_RTI);
            break;
        case MNLD_TEST_RESTART_TYPE_WARM:
            LOGD("Warm Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_EPHEMERIS);
            break;
        case MNLD_TEST_RESTART_TYPE_COLD:
            LOGD("Cold Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_EPHEMERIS |
                GPS_DELETE_POSITION | GPS_DELETE_TIME | GPS_DELETE_IONO |
                GPS_DELETE_UTC | GPS_DELETE_HEALTH);
            break;
        case MNLD_TEST_RESTART_TYPE_FULL:
            LOGD("Full Start\n");
            hal2mnl_gps_delete_aiding_data(GPS_DELETE_ALL);
            break;
        default:
            LOGE("ERR: read unhandled value=[%d]\n", restart_type);
            return;
    }
    if(gpsinterface != NULL && cbs != NULL)
    {
        gpsinterface->init(cbs);
        //hal2mnl_update_network_state(1,NETWORK_TYPE_WIFI,0,"NULL");
        //hal2mnl_update_network_state(mnld_test_network_on,mnld_test_network_type,mnld_test_network_roaming,"NULL");
        gpsinterface->start();
        memset(&(mnld_test_result_body.location),0,sizeof(GpsLocation));
        mnld_test_result_body.fix_type = 0;
        mnld_test_ttff = 0;
        mnld_test_session_end = 0;
       // usleep(500000);
       // mnld_test_connect_mnl();
    #ifndef MNLD_TEST_TTFF_SESSION_BEGIN
        if(clock_gettime(CLOCK_BOOTTIME,&mnld_test_gnss_open_tm) == -1)
        {
            LOGE("Fail to get time(%s).",strerror(errno));
        }
    #endif
    }else{
        LOGE("param error:%d, %d",gpsinterface, cbs);
    }
}

void mnld_test_close_gnss(GpsInterface_ext* gps_interface)
{
    GpsInterface_ext* gpsinterface = gps_interface;
//    gpsinterface = gps_device__get_gps_interface("mnld_test stop");
    if(gpsinterface != NULL)
    {
        gpsinterface->cleanup();
        gpsinterface->stop();
    }else{
        LOGE("[%s]param error\r\n",__func__);
    }

}

MNLD_TEST_RESTART_TYPE mnld_test_get_restart_type(char type_char)
{
    MNLD_TEST_RESTART_TYPE restart_type = MNLD_TEST_RESTART_TYPE_UNKNOWN;
    switch(type_char)
    {
        case 'H':
        case 'h':
        restart_type = MNLD_TEST_RESTART_TYPE_HOT;
        break;

        case 'W':
        case 'w':
        restart_type = MNLD_TEST_RESTART_TYPE_WARM;
        break;

        case 'C':
        case 'c':
        restart_type = MNLD_TEST_RESTART_TYPE_COLD;
        break;

        case 'F':
        case 'f':
        restart_type = MNLD_TEST_RESTART_TYPE_FULL;
        break;

        default :
        LOGE("Restart type error, use default hot start!\r\n");
        restart_type = MNLD_TEST_RESTART_TYPE_UNKNOWN;
        break;

    }
    return restart_type;
}

static void mnld_test_clear_stdin_buff() {
    char c;
    while((c = getchar()) != '\n' && c != EOF);
}


void mnld_test_gnss_restart(void)
{
    int retry_cnt = 0;
    if(mnld_test_restart_cnt < mnld_test_restart_time)
    {
        mnld_test_restart_cnt++;
        mnld_test_close_gnss(mnld_test_gpsinfs);
        while(!mnld_test_session_end)
        {
            if(retry_cnt++>500)
            {
                LOGW("[%s] wait gnss close timeout\r\n",__func__);
                break;
            }
            usleep(10000);
        }
        mnld_test_open_gnss(mnld_test_restart_type,mnld_test_gpsinfs,mnld_test_cbs);
        start_timer(mnld_test_restart_timer,mnld_test_restart_interval*1000);
    }else{
        stop_timer(mnld_test_restart_timer);
    }
}

char* mnld_test_get_clock_type_str(int clock_type_int)
{
    int i = 0;
    int len = sizeof(mnld_test_clock_type)/sizeof(clock_type);

    for(i=0;i<len;i++)
    {
        if(clock_type_int == mnld_test_clock_type[i].type_int)
        {
            break;
        }
    }

    if(i < len)
    {
        return(mnld_test_clock_type[i].type_str);
    }else{
        return("Unknown");
    }
}

void mnld_test_show_test_result(mnld_test_result* result)
{
    if(NULL != result)
    {
        memcpy(result->chip_ver,MNLD_TEST_CHIP_VER,sizeof(MNLD_TEST_CHIP_VER));
        memcpy(result->mnl_ver,MNLD_TEST_MNL_VER,strlen(MNLD_TEST_MNL_VER));
        result->clk_type = MNLD_TEST_CLOCK_TYPE;
        result->clk_buff = MNLD_TEST_CLOCK_BUFF;

        //system("clear");
        LOGD("---------------------------------------------");
        LOGD("Chip:%s",result->chip_ver);
        LOGD("MNL Version:%s",result->mnl_ver);
        LOGD("Clock Type:%s",mnld_test_get_clock_type_str(result->clk_type));
        LOGD("Clock Buffer:%d",result->clk_buff);
        LOGD("---------------------------------------------");
        if(result->ttff[CURR] == 0)
        {
            LOGD("TTFF: - ");
        }else{
            LOGD("TTFF: %d ms",result->ttff[CURR]);
        }

        if(result->ttff[MIN] == 0)
        {
            LOGD("TTFF min: - ");
        }else{
            LOGD("TTFF min: %d ms",result->ttff[MIN]);
        }

        if(result->ttff[MAX] == 0)
        {
            LOGD("TTFF max: - ");
        }else{
            LOGD("TTFF max: %d ms",result->ttff[MAX]);
        }

        if(result->ttff[MEAN] == 0)
        {
            LOGD("TTFF mean: - ");
        }else{
            LOGD("TTFF mean: %d ms",result->ttff[MEAN]);
        }
        LOGD("---------------------------------------------");
        LOGD("Fix Type: %d",result->fix_type);
        LOGD("Flags: 0x%x",result->location.legacyLocation.flags);
        LOGD("Latitude: %.10lf",result->location.legacyLocation.latitude);
        LOGD("Longtitude: %.10lf",result->location.legacyLocation.longitude);
        LOGD("Altitude: %.10lf",result->location.legacyLocation.altitude);
        LOGD("Speed: %fm/s",result->location.legacyLocation.speed);
        LOGD("Bearing: %f",result->location.legacyLocation.bearing);
//        LOGD("Time stamp: %d",result->location.timestamp);
        LOGD("horizontalAccuracyMeters: %f",result->location.horizontalAccuracyMeters);
        LOGD("verticalAccuracyMeters: %f",result->location.verticalAccuracyMeters);
        LOGD("speedAccuracyMetersPerSecond: %f",result->location.speedAccuracyMetersPerSecond);
        LOGD("bearingAccuracyDegrees: %f",result->location.bearingAccuracyDegrees);
        LOGD("Utc time: %s",result->utc_time);
        LOGD("---------------------------------------------");
        LOGD("GNSS testing(%d).",mnld_test_restart_cnt);
    }
}

static int main_running = 1;
static pthread_mutex_t main_mutex;
static pthread_cond_t main_cond;

void daemon_sighlr(int signo)
{
    if ((signo == SIGUSR1) || (signo == SIGINT) || (signo == SIGTERM)) {
        LOGD("catch signal:%d, stop mnld_test\n", signo);
        pthread_mutex_lock(&main_mutex);
        main_running = 0;
        mnld_test_close_gnss(mnld_test_gpsinfs);
        pthread_mutex_unlock(&main_mutex);
        pthread_cond_signal(&main_cond);
    }
    else if (signo == SIGPIPE || signo == SIGTTIN)
        LOGD("catch signal:%d, ignore\n", signo);;
}

void main(int argc, char** argv)
{
    struct sigaction actions;

    actions.sa_handler = daemon_sighlr;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    sigaction(SIGUSR1, &actions, NULL);
    sigaction(SIGINT, &actions, NULL);
    sigaction(SIGTTIN, &actions, NULL);
    sigaction(SIGKILL, &actions, NULL);
    sigaction(SIGTERM, &actions, NULL);

    pthread_mutex_init(&main_mutex, NULL);
    pthread_cond_init(&main_cond, NULL);

    int exit_flag = 0;
    int cnt = 0;
    int index = 0;
    int ret = 0;
    char input_c = 0;
    char *error;
    struct gps_device_t_ext *gpsdev = NULL;
    void *handle;
    MNLD_TEST_ACTION action = MNLD_TEST_ACTION_UNKNOWN;

    //Show the recieved command
    for(index=0; index<argc; index++)
    {
        mnld_test_printf("%s ",argv[index]);
    }
    mnld_test_printf("\r\n");

    if(argc >= 2 && argc <= MNLD_TEST_CMD_CNT_MAX)
    {
        if(!strncmp(argv[1], MNLD_TEST_CMD_OPEN, strlen(MNLD_TEST_CMD_OPEN)))//Open GNSS
        {
            action = MNLD_TEST_ACTION_GNSS_OPEN;
            //re-init restart parameters
            mnld_test_restart_time = 0;
            mnld_test_restart_interval = 60;//Default is 60s
            mnld_test_restart_type = MNLD_TEST_RESTART_TYPE_HOT;

            switch(argc)
            {
                case 5://No break
                mnld_test_restart_interval = atoi(&argv[4][0]);
                case 4://No break
                mnld_test_restart_time = atoi(&argv[3][0]);
                case 3://No break
                mnld_test_restart_type = mnld_test_get_restart_type(argv[2][0]);
                case 2://No break
                case 1://No break
                default:
                break;
            }
            LOGD("mnld_test start.\r\n");
            if(mnld_test_restart_time > 1000)
            {
                LOGE("The max value of restart time is 1000, %d is over this range\r\n",mnld_test_restart_time);
                mnld_test_restart_time = 1000;
            }

            if(mnld_test_restart_interval > 3600)
            {
                LOGE("The max value of restart interval is 3600s(1h), %d is over this range\r\n",mnld_test_restart_interval);
                mnld_test_restart_interval = 3600;
            }

            if(mnld_test_restart_time <= 0)//No restart
            {
                mnld_test_restart_interval = 0;
            }
            LOGD("restart_time:%d, restart_interval:%ds,restart_type:%d\r\n",mnld_test_restart_time,mnld_test_restart_interval,mnld_test_restart_type);
            exit_flag = 0;
        }else if(!strncmp(argv[1],MNLD_TEST_CMD_CLOSE, strlen(MNLD_TEST_CMD_CLOSE))){//Close GNSS
            action = MNLD_TEST_ACTION_GNSS_CLOSE;
            LOGD("mnld_test stop.\r\n");
            exit_flag = 0;
        }else if(!strncmp(argv[1], MNLD_TEST_CMD_NETWORK, strlen(MNLD_TEST_CMD_NETWORK))){
            LOGD("mnld_test set network.\r\n");
            exit_flag = 0;
            if(argc == 3)
            {
                if(!strncmp(argv[2], MNLD_TEST_NETWORK_WIFI, strlen(MNLD_TEST_NETWORK_WIFI)))
                {
                    mnld_test_network_type = NETWORK_TYPE_WIFI;
                    mnld_test_network_on = 1;
                    mnld_test_network_roaming = 0;
                }else if(!strncmp(argv[2],MNLD_TEST_NETWORK_MOBILE, strlen(MNLD_TEST_NETWORK_MOBILE))){
                    mnld_test_network_type = NETWORK_TYPE_MOBILE;
                    mnld_test_network_on = 1;
                    mnld_test_network_roaming = 0;
                }else if(!strncmp(argv[2],MNLD_TEST_NETWORK_DISABLE, strlen(MNLD_TEST_NETWORK_DISABLE)))                {
                    mnld_test_network_type = NETWORK_TYPE_WIFI;
                    mnld_test_network_on = 0;
                    mnld_test_network_roaming = 0;
                }else{
                    LOGW("Network set fail!Unknown command!(%d)\r\n", argc);
                    mnld_test_show_help();
                    exit_flag = 1;
                }
            }else if(argc == 4){
                if(!strncmp(argv[2],MNLD_TEST_NETWORK_MOBILE, strlen(MNLD_TEST_NETWORK_MOBILE)) && !strncmp(argv[3],MNLD_TEST_NETWORK_ROAMING, strlen(MNLD_TEST_NETWORK_ROAMING)))
                {
                    mnld_test_network_type = NETWORK_TYPE_MOBILE;
                    mnld_test_network_on = 1;
                    mnld_test_network_roaming = 1;
                }else{
                    LOGW("Network set fail!Unknown command!(%d)\r\n", argc);
                    mnld_test_show_help();
                    exit_flag = 1;
                }
            }else{
                LOGW("Network set fail! Error cmd count(%d).\r\n",argc);
                mnld_test_show_help();
                exit_flag = 1;
            }

            LOGD("network, type:%s, on:%d, roaming:%d.\r\n",mnld_test_network_type_str[mnld_test_network_type], mnld_test_network_on, mnld_test_network_roaming);
            action = MNLD_TEST_ACTION_SET_NETWORK;
        }else{
            LOGE("Unknown command!\r\n");
            mnld_test_show_help();
            exit_flag = 1;
        }
    }else{
        LOGE("Unknown command!\r\n");
        mnld_test_show_help();
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
        mnld_test_gpsinfs = (GpsInterface_ext*)gpsdev->get_gps_interface(gpsdev);

        mnld_test_cbs = mnld_test__get_gps_callbacks();//&mnld_test_gps_callbacks;
        switch(action)
        {
            case MNLD_TEST_ACTION_GNSS_OPEN:

            valid_ttff_cnt = 0;
            valid_ttff_sum = 0;

            memset(&mnld_test_result_body,0,sizeof(mnld_test_result));
            mnld_test_open_gnss(mnld_test_restart_type,mnld_test_gpsinfs,mnld_test_cbs);
            //system("clear");
            if(mnld_test_restart_interval != 0)
            {
                mnld_test_restart_timer = init_timer(mnld_test_gnss_restart);
                start_timer(mnld_test_restart_timer,mnld_test_restart_interval*1000);
            }
            pthread_mutex_lock(&main_mutex);
            while(main_running) {
                LOGI("main thread going....\n");
                pthread_cond_wait(&main_cond, &main_mutex);
                LOGI("some signal catched, go to exit!!!\n");
            }
            pthread_mutex_unlock(&main_mutex);
            exit_flag = 1;
            break;

            case MNLD_TEST_ACTION_GNSS_CLOSE:
            mnld_test_close_gnss(mnld_test_gpsinfs);

            exit_flag = 1;
            break;

            case MNLD_TEST_ACTION_SET_NETWORK:

            ret = hal2mnl_update_network_state(mnld_test_network_on,mnld_test_network_type,mnld_test_network_roaming,"NULL");
            if(-1 == ret)
            {
                LOGE("Network set fail!\r\n");
            }else{
                LOGD("Network set successfully! type: %s,on:%d,roaming:%d\r\n",mnld_test_network_type_str[mnld_test_network_type],mnld_test_network_on,mnld_test_network_roaming);
            }
            exit_flag = 1;
            break;

            default:
            LOGW("Unknown action(%d)\r\n",action);
            mnld_test_show_help();
            break;
        }

    }
    pthread_cond_destroy(&main_cond);
    pthread_mutex_destroy(&main_mutex);
}

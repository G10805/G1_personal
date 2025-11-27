// SPDX-License-Identifier: MediaTekProprietary
#ifndef __EPO_H__
#define __EPO_H__

#include "curl.h"
#ifdef __cplusplus
extern "C" {
#endif

#define EPO_PATH                    MTK_GPS_DATA_PATH
#define EPO_FILE                    MTK_GPS_DATA_PATH"EPO.DAT"
#define EPO_UPDATE_FILE             MTK_GPS_DATA_PATH"EPOTMP.DAT"
#define EPO_UPDATE_HAL              MTK_GPS_DATA_PATH"EPOHAL.DAT"
#define MTK_EPO_ONE_SV_SIZE  72
#define SECONDS_PER_HOUR (60*60)
#define EPO_DL_MAX_RETRY_TIME 3  //The max retry time of curl_easy_download fail
#define EPO_INVALIDE_DL_MAX_RETRY_TIME 3  //The max retry time of whole download process
#define EPO_INVALIDE_DL_RETRY_SLEEP (100*1000)  //100 ms
#define MAX_EPO_PIECE 10
#define EPO_MERGE_FULL_FILE   -1
#define EPO_MD5_AVAILABLE_BIT   (1<<0)
#define EPO_DAT_AVAILABLE_BIT   (1<<1)
#define EPO_FILE_NAME_MAX_SIZE  60
//#define EPO_MD5_FILE_MAX_SIZE   50
#define EPO_MD5_FILE_MAX_SIZE   500

#define GPS_EPO_FILE_LEN  32
#define GPS_EPO_URL_LEN 256

typedef struct EPO_Status {
    unsigned int EPO_piece_flag[MAX_EPO_PIECE];
    int last_DL_Date;
    int today_retry_time;
} EPO_Status_T;

#ifdef CONFIG_GPS_MT3303
void __getEpoUrl(char* filename, char* url);
#endif

#define mnld_curl_easy_setopt(handle, opt, param) do { \
        if(curl_easy_setopt(handle, opt, param) != CURLE_OK) { \
            LOGE("FUNC[%s], Line[%d], curl_easy_setopt fail, %s(%d)", __func__, __LINE__, strerror(errno), errno); \
        } \
    } while(0)

int epo_downloader_init();
int epo_read_cust_config();
int epo_downloader_is_file_invalid();
int epo_downloader_start();
void epo_update_epo_file();
int epo_is_wifi_trigger_enabled();
int epo_is_epo_download_enabled();
CURLcode curl_easy_download(char* url, char* filename);
int mtk_gps_sys_epo_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond);
int mtk_gps_sys_epo_bd_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond);
void GpsToUtcTime(int i2Wn, double dfTow, time_t* uSecond);
void getEpoUrl(char * filename, char * url);
int mtk_gps_sys_epo_ga_period_start(int fd, unsigned int* u4GpsSecs, time_t* uSecond);
int mtk_gps_sys_read_lock(int fd, off_t offset, int whence, off_t len);

#ifdef __cplusplus
}
#endif

#endif




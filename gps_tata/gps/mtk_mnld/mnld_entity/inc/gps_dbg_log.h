#ifndef __GPS_DBG_LOG_H__
#define __GPS_DBG_LOG_H__

#include "mtk_gps_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX
#define MAX(A,B) ((A)>(B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

#define LOG_FILE            MTK_GPS_DATA_PATH"gpsdebug.log"
#define LOG_FILE_PATH       MTK_GPS_DATA_PATH
#define PATH_SUFFIX         "mtklog/gpsdbglog/"
#define LOG_FILE_EXTEN_NAME ".nma"
#define LOG_FILE_WRITING_EXTEN_NAME ".nmac"

#if defined(__ANDROID_OS_P__)
#define GPS_LOG_PERSIST_STATE "vendor.gpsdbglog.enable"
#define GPS_LOG_PERSIST_PATH "vendor.gpsdbglog.path"
#define GPS_LOG_PERSIST_FLNM "vendor.gpsdbglog.file"
#elif defined(__ANDROID_OS_O__)
#define GPS_LOG_PERSIST_STATE "debug.gpsdbglog.enable"
#define GPS_LOG_PERSIST_PATH "debug.gpsdbglog.path"
#define GPS_LOG_PERSIST_FLNM "debug.gpsdbglog.file"
#endif

#define GPS_LOG_PERSIST_VALUE_ENABLE "1"
#define GPS_LOG_PERSIST_VALUE_DISABLE "0"
#define GPS_LOG_PERSIST_VALUE_NONE "none"

#define GPS_DBG_LOG_FILE_NUM_LIMIT 1000
#define MAX_DBG_LOG_DIR_SIZE       (3UL*1024*1024*1024)
#define MAX_DBG_LOG_FILE_SIZE      (25UL*1024*1024)

enum {
    MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNL = 0x00,
    MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNL = 0x01,
    MTK_GPS_DISABLE_DEBUG_MSG_WR_BY_MNLD = 0x10,
    MTK_GPS_ENABLE_DEBUG_MSG_WR_BY_MNLD = 0x11,
};
// Task synchronization related type
typedef enum{
  MNLD_MUTEX_PINGPANG_WRITE = 0,
  MNLD_MUTEX_MAX
} mnld_mutex_enum;

int gps_dbg_log_thread_init();

int create_mtklogger2mnl_fd();

int mtklogger2mnl_hdlr(int fd);

size_t gps_log_dir_check(char * dirname);

void gps_stop_dbglog_release_condition(void);

void mtklogger_mped_reboot_message_update(void);
int mnl_debug_file_check(char * dbg_file);
//Rename gpsdebug name, .nmac to .nma
void gps_log_file_rename(char *filename_cur);

void gps_dbg_log_property_load(void);

void gps_dbg_log_exit_flush(int force_exit) ;

#ifdef __cplusplus
}
#endif

#endif




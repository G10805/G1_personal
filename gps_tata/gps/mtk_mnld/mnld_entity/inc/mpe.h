#ifndef __MPE_H__
#define __MPE_H__

#include "mtk_gps_type.h"

#ifdef __cplusplus
extern "C" {
#endif
int mpe_function_init(int self_recv);
int mnl_mpe_thread_init();
int mnl2mpe_set_log_path(char* path, int status_flag, int mode_flag);
int mnl2adr_gps_open(void);
int mnl2adr_gps_close(void);
int mnl2adr_mnl_reboot(void);
int mnl2adr_open_gps_done(void);
int mnl2adr_close_gps_done(void);
int mnl2adr_send_nmea_data(const char * nmea_buffer, const UINT32 length);

#ifdef __cplusplus
}
#endif

#endif


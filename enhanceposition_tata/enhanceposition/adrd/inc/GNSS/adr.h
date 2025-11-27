#ifndef __MPE_H__
#define __MPE_H__

#include "mtk_gps_type.h"

#ifdef __cplusplus
extern "C" {
#endif

//Socket name
#define MNLD_MNL2ADR_SOCKET                 "mnl2adr_socket"
#define MNLD_ADR2MNL_SOCKET                 "adr2mnl_socket"

//Define debug buffer size
#define MNL_ADR_MAX_BUFF_SIZE (6*1024)
#define MNL_ADR_NMEA_MAX_SIZE (10*1024)

typedef enum{
    MNL_ADR_TYPE_MNL2ADR_GPS_OPEN = 0x100,
    MNL_ADR_TYPE_MNL2ADR_GPS_OPEN_DONE,
    MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE,
    MNL_ADR_TYPE_MNL2ADR_GPS_CLOSE_DONE,
    MNL_ADR_TYPE_MNL2ADR_MNL_REBOOT,
    MNL_ADR_TYPE_MNL2ADR_NMEA_DATA,

    MNL_ADR_TYPE_ADR2MNL_NMEA_DATA = 0x200,
    MNL_ADR_TYPE_ADR2MNL_PMTK_CMD
}MNL_ADR_TYPE;

int adr_function_init();
int adr_mpe_thread_init();
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


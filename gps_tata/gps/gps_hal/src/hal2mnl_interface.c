#include "hal2mnl_interface.h"
#include "mtk_lbs_utility.h"
#include "data_coder.h"
#include "mtk_auto_log.h"

int log_dbg_level = L_DEBUG;

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "hal2mnl"
#endif

#ifndef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))
#endif

static float count = 0;
static float report_time_interval = 0;

int hal2mnl_hal_reboot() {
    LOGI("hal2mnl_hal_reboot");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);
    put_int(buff, &offset, HAL2MNL_HAL_REBOOT);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_init() {
    LOGI("hal2mnl_gps_init");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_INIT);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_start() {
    LOGI("hal2mnl_gps_start");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_START);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}
int hal2mnl_gps_stop() {
    LOGI("hal2mnl_gps_stop");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_STOP);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_cleanup() {
    LOGI("hal2mnl_gps_cleanup");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_CLEANUP);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_inject_time(int64_t time, int64_t time_reference, int uncertainty) {
    /*LOGD("hal2mnl_gps_inject_time  time=%zd time_reference=%zd uncertainty=%d",
        time, time_reference, uncertainty);*/
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_INJECT_TIME);
    put_long(buff, &offset, time);
    put_long(buff, &offset, time_reference);
    put_int(buff, &offset, uncertainty);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_inject_location(double lat, double lng, float acc) {
    // LOGD("hal2mnl_gps_inject_location  lat,lng %f,%f acc=%f", lat, lng, acc);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_INJECT_LOCATION);
    put_double(buff, &offset, lat);
    put_double(buff, &offset, lng);
    put_float(buff, &offset, acc);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_delete_aiding_data(int flags) {
    LOGD("flag=0x%x", flags);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_DELETE_AIDING_DATA);
    put_int(buff, &offset, flags);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_gps_set_position_mode(gps_pos_mode mode, gps_pos_recurrence recurrence,
    int min_interval, int preferred_acc, int preferred_time, bool lowPowerMode) {
    LOGD("mode=%d recurrence=%d min_interval=%d preferred_acc=%d preferred_time=%d lowpowermode=%d",
        mode, recurrence, min_interval, preferred_acc, preferred_time, lowPowerMode);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_SET_POSITION_MODE);
    put_int(buff, &offset, mode);
    put_int(buff, &offset, recurrence);
    put_int(buff, &offset, min_interval);
    put_int(buff, &offset, preferred_acc);
    put_int(buff, &offset, preferred_time);
    put_byte(buff, &offset, lowPowerMode);

    if (min_interval <= 1000) {
        report_time_interval = 1;
    } else {
        report_time_interval = (float)min_interval/1000;
    }
    LOGD("set report location time interval is %f s\n", report_time_interval);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_data_conn_open(const char* apn) {
    LOGD("apn=[%s]", apn);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_DATA_CONN_OPEN);
    put_string(buff, &offset, apn);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_data_conn_open_with_apn_ip_type(const char* apn, apn_ip_type ip_type) {
    LOGD("apn=[%s] ip_type=%d",
        apn, ip_type);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_DATA_CONN_OPEN_WITH_APN_IP_TYPE);
    put_string(buff, &offset, apn);
    put_int(buff, &offset, ip_type);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_data_conn_closed() {
    LOGD("hal2mnl_data_conn_closed");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_DATA_CONN_CLOSED);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_data_conn_failed() {
    LOGD("hal2mnl_data_conn_failed");
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_DATA_CONN_FAILED);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_set_server(agps_type type, const char* hostname, int port) {
    LOGD("type=%d hostname=[%s] port=%d",
        type, hostname, port);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_SET_SERVER);
    put_int(buff, &offset, type);
    put_string(buff, &offset, hostname);
    put_int(buff, &offset, port);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
    return 0;
}

int hal2mnl_set_ref_location(cell_type type, int mcc, int mnc, int lac, int cid) {
    LOGD("ype=%d mcc=%d mnc=%d lac=%d cid=%d",
        type, mcc, mnc, lac, cid);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_SET_REF_LOCATION);
    put_int(buff, &offset, type);
    put_int(buff, &offset, mcc);
    put_int(buff, &offset, mnc);
    put_int(buff, &offset, lac);
    put_int(buff, &offset, cid);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_set_id(agps_id_type type, const char* setid) {
    LOGD("type=%d setid=[%s]", type, setid);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_SET_ID);
    put_int(buff, &offset, type);
    put_string(buff, &offset, setid);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}


int hal2mnl_ni_message(char* msg, int len) {
    LOGD("len=%d", len);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_NI_MESSAGE);
    put_binary(buff, &offset, msg, len);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

#if defined(__LINUX_OS__)
int hal2mnl_send_pmtk(char* msg, int len) {
    LOGD("len=%d", len);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_SEND_PMTK);
    put_binary(buff, &offset, msg, len);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}
#endif

int hal2mnl_ni_respond(int notif_id, ni_user_response_type user_response) {
    LOGD("notif_id=%d user_response=%d",
        notif_id, user_response);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_NI_RESPOND);
    put_int(buff, &offset, notif_id);
    put_int(buff, &offset, user_response);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_update_network_state(int connected, network_type type, int roaming,
    const char* extra_info) {
    LOGD("connected=%d type=%d roaming=%d extra_info=[%s]",
        connected, type, roaming, extra_info);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_UPDATE_NETWORK_STATE);
    put_int(buff, &offset, connected);
    put_int(buff, &offset, type);
    put_int(buff, &offset, roaming);
    put_string(buff, &offset, extra_info);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_update_network_availability(int available, const char* apn) {
    LOGD("available=%d apn=[%s]",
        available, apn);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_UPDATE_NETWORK_AVAILABILITY);
    put_int(buff, &offset, available);
    put_string(buff, &offset, apn);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_set_gps_measurement(bool enabled, bool enableFullTracking) {
    LOGD("enabled=%d, flag=%d", enabled, enableFullTracking);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_MEASUREMENT);
    put_int(buff, &offset, enabled);
    put_int(buff, &offset, enableFullTracking);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_set_gps_navigation(bool enabled) {
    LOGD("enabled=%d", enabled);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GPS_NAVIGATION);
    put_int(buff, &offset, enabled);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_set_vzw_debug(bool enabled) {
    LOGD("enabled = %d\n", enabled);
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_VZW_DEBUG);
    put_int(buff, &offset, enabled);

    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

int hal2mnl_update_gnss_config(const char* config_data, int length) {
    LOGD("data length:%d\n", length);
    int offset = 0;
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    put_int(buff, &offset, HAL_MNL_INTERFACE_VERSION);

    put_int(buff, &offset, HAL2MNL_GNSS_CONFIG);
    put_int(buff, &offset, length);
    put_string(buff, &offset, config_data);
    return safe_sendto(MTK_HAL2MNL, buff, offset);
}

// -1 means failure
int mnl2hal_hdlr(int fd, mnl2hal_interface* hdlr) {
    char buff[HAL_MNL_BUFF_SIZE] = {0};
    int offset = 0;
    int ver;
    mnl2hal_cmd cmd;
    int read_len;
    int ret = 0;

    read_len = safe_recvfrom(fd, buff, sizeof(buff));
    if (read_len <= 0) {
        LOGE("mnl2hal_hdlr() safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }
    ver = get_int(buff, &offset, sizeof(buff));
    UNUSED(ver);
    cmd = get_int(buff, &offset, sizeof(buff));

    switch (cmd) {
    case MNL2HAL_MNLD_REBOOT: {
        if (hdlr->mnld_reboot) {
            hdlr->mnld_reboot();
        } else {
            LOGE("mnl2hal_hdlr() mnld_reboot is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_LOCATION: {
        if (report_time_interval > ++count) {
            LOGD("count is %f, interval is %f\n", count, report_time_interval);
            break;
        }
        count = 0;

        if (hdlr->location) {
            gps_location location;
            get_binary(buff, &offset, (char*)&location, sizeof(buff), sizeof(gps_location));
            hdlr->location(location);
        } else {
            LOGE("mnl2hal_hdlr() location is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GPS_STATUS: {
        if (hdlr->gps_status) {
            gps_status status;
            get_binary(buff, &offset, (char*)&status, sizeof(buff), sizeof(gps_status));
            hdlr->gps_status(status);
        } else {
            LOGE("mnl2hal_hdlr() gps_status is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GPS_SV: {
        if (hdlr->gps_sv) {
            gnss_sv_info sv;
            get_binary(buff, &offset, (char*)&sv, sizeof(buff), sizeof(gnss_sv_info));
            hdlr->gps_sv(sv);
        } else {
            LOGE("mnl2hal_hdlr() gps_sv is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_NMEA: {
        if (hdlr->nmea) {
            int64_t timestamp = get_long(buff, &offset, sizeof(buff));
            char* nmea = get_string(buff, &offset, sizeof(buff));
            int used_length = nmea - buff;
            int max_length = sizeof(buff) - used_length;
            int length = get_int(buff, &offset, sizeof(buff));
            length = MIN(length,max_length);
            hdlr->nmea(timestamp, nmea, length);
        } else {
            LOGE("mnl2hal_hdlr() nmea is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GPS_CAPABILITIES: {
        if (hdlr->gps_capabilities) {
            gps_capabilites capabilities;
            get_binary(buff, &offset, (char*)&capabilities, sizeof(buff), sizeof(gps_capabilites));
            hdlr->gps_capabilities(capabilities);
        } else {
            LOGE("mnl2hal_hdlr() gps_capabilities is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GPS_MEASUREMENTS: {
        if (hdlr->gps_measurements) {
            gps_data data;
            get_binary(buff, &offset, (char*)&data, sizeof(buff), sizeof(gps_data));
            hdlr->gps_measurements(data);
        } else {
            LOGE("mnl2hal_hdlr() gps_measurements is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GPS_NAVIGATION: {
        if (hdlr->gps_navigation) {
            gps_nav_msg msg;
            get_binary(buff, &offset, (char*)&msg, sizeof(buff), sizeof(gps_nav_msg));
            hdlr->gps_navigation(msg);
        } else {
            LOGE("mnl2hal_hdlr() gps_navigation is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GNSS_MEASUREMENTS: {
        if (hdlr->gnss_measurements) {
            gnss_data data;
            get_binary(buff, &offset, (char*)&data, sizeof(buff), sizeof(gnss_data));
            hdlr->gnss_measurements(data);
        } else {
            LOGE("mnl2hal_hdlr() gnss_measurements is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_GNSS_NAVIGATION: {
        if (hdlr->gnss_navigation) {
            gnss_nav_msg msg;
            get_binary(buff, &offset, (char*)&msg, sizeof(buff), sizeof(gnss_nav_msg));
            hdlr->gnss_navigation(msg);
        } else {
            LOGE("mnl2hal_hdlr() gnss_navigation is NULL");
            ret = -1;
        }
        break;
    }

    case MNL2HAL_REQUEST_WAKELOCK: {
        if (hdlr->request_wakelock) {
            hdlr->request_wakelock();
        } else {
            LOGE("mnl2hal_hdlr() request_wakelock is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_RELEASE_WAKELOCK: {
        if (hdlr->release_wakelock) {
            hdlr->release_wakelock();
        } else {
            LOGE("mnl2hal_hdlr() release_wakelock is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_UTC_TIME: {
        if (hdlr->request_utc_time) {
            hdlr->request_utc_time();
        } else {
            LOGE("mnl2hal_hdlr() request_utc_time is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_DATA_CONN: {
        if (hdlr->request_data_conn) {
            struct sockaddr_storage addr;
            get_binary(buff, &offset, (char*)&addr, sizeof(buff), sizeof(addr));
            hdlr->request_data_conn(&addr);
        } else {
            LOGE("mnl2hal_hdlr() request_data_conn is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_RELEASE_DATA_CONN: {
        if (hdlr->release_data_conn) {
            hdlr->release_data_conn();
        } else {
            LOGE("mnl2hal_hdlr() release_data_conn is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_NI_NOTIFY: {
        if (hdlr->request_ni_notify) {
            int session_id = get_int(buff, &offset, sizeof(buff));
            agps_notify_type type = get_int(buff, &offset, sizeof(buff));
            char* requestor_id = get_string(buff, &offset, sizeof(buff));
            char* client_name = get_string(buff, &offset, sizeof(buff));
            ni_encoding_type requestor_id_encoding = get_int(buff, &offset, sizeof(buff));
            ni_encoding_type client_name_encoding  = get_int(buff, &offset, sizeof(buff));
            hdlr->request_ni_notify(session_id, type, requestor_id, client_name,
                requestor_id_encoding, client_name_encoding);
        } else {
            LOGE("mnl2hal_hdlr() request_ni_notify is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_SET_ID: {
        if (hdlr->request_set_id) {
            request_setid flags = get_int(buff, &offset, sizeof(buff));
            hdlr->request_set_id(flags);
        } else {
            LOGE("mnl2hal_hdlr() request_set_id is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_REF_LOC: {
        if (hdlr->request_ref_loc) {
            request_refloc flags = get_int(buff, &offset, sizeof(buff));
            hdlr->request_ref_loc(flags);
        } else {
            LOGE("mnl2hal_hdlr() request_ref_loc is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_VZW_DEBUG_OUTPUT: {
        if (hdlr->output_vzw_debug) {
            char* str = get_string(buff, &offset, sizeof(buff));
            hdlr->output_vzw_debug(str);
        } else {
            LOGE("mnl2hal_hdlr() output_vzw_debug is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_UPDATE_GNSS_NAME: {
        if (hdlr->update_gnss_name) {
            int length = get_int(buff, &offset, sizeof(buff));
            char* name = get_string(buff, &offset, sizeof(buff));
            hdlr->update_gnss_name(name, length);
        } else {
            LOGE("mnl2hal_hdlr() update_gnss_name is NULL");
            ret = -1;
        }
        break;
    }
    case MNL2HAL_REQUEST_NLP: {
        if (hdlr->request_nlp) {
            bool independentFromGnss = get_byte(buff, &offset, sizeof(buff));
            hdlr->request_nlp(independentFromGnss);
        } else {
            LOGE("mnl2hal_hdlr() update_gnss_name is NULL");
            ret = -1;
        }
        break;
    }
    default: {
        LOGE("mnl2hal_hdlr() unknown cmd=%d", cmd);
        ret = -1;
        break;
    }
    }

    return ret;
}

// -1 means failure
int create_mnl2hal_fd() {
    int fd = socket_bind_udp(MTK_MNL2HAL);
    socket_set_blocking(fd, 0);
    return fd;
}


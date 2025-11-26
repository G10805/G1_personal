/* Copyright (c) 2020, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <log/log.h>
#include <cutils/list.h>
#include <stdbool.h>


#undef ALOGI
#undef ALOGE
#undef ALOGV
#undef ALOGD

#define ALOGI(...)     fprintf(stdout,__VA_ARGS__);\
                       fprintf(stdout, "\n")
#define ALOGE(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")
#define ALOGV(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")
#define ALOGD(...)     fprintf(stderr,__VA_ARGS__);\
                       fprintf(stderr, "\n")

#define ACDB_DEV_TYPE_OUT 1
#define DEFAULT_CAL_MODE  0x1
#define DEFAULT_OFFSET_INDEX 0x1
#define DEFAULT_APP_TYPE_RX_PATH  69936
#define DEFAULT_OUTPUT_SAMPLING_RATE 48000
#define DEFAULT_ACDB_ID 60
#define AFE_PROXY_ACDB_ID 45

enum {
    ACDB_LOADER_INIT_V1 = 1,
    ACDB_LOADER_INIT_V2,
    ACDB_LOADER_INIT_V3,
    ACDB_LOADER_INIT_V4,
};

enum{
    MSG_INIT_V2 = 50,
    MSG_INIT_V3,
    MSG_INIT_V4,
    MSG_INIT_ACDB,
    MSG_GET_DEF_APP_TYPE,
    MSG_SEND_COM_CUSTOM_TOP,
    MSG_DEALLOCATE_ACDB,
    MSG_DEALLOCATE_CAL,
    MSG_SEND_VOICE_CAL_V2,
    MSG_SEND_VOICE_CAL,
    MSG_RELOAD_VOICE_VOL_TABLE,
    MSG_SEND_AUDIO_CAL,
    MSG_SEND_AUDIO_CAL_V2,
    MSG_SEND_AUDIO_CAL_V3,
    MSG_SEND_AUDIO_CAL_V4,
    MSG_SEND_AUDIO_CAL_V6,
    MSG_SEND_LISTEN_DEV_CAL,
    MSG_SEND_LISTEN_LSM_CAL,
    MSG_SEND_LISTEN_LSM_CAL_V1,
    MSG_SEND_ANC_CAL,
    MSG_SEND_ANC_DATA,
    MSG_GET_AUD_VOL_STEPS,
    MSG_SEND_GAIN_DEP_CAL,
    MSG_GET_REMOTE_ACDB_ID,
    MSG_GET_ECRX_DEVICE,
    MSG_GET_CALIBRATION,
    MSG_SET_AUDIO_CAL_V2,
    MSG_GET_AUDIO_CAL_V2,
    MSG_SEND_META_INFO,
    MSG_SEND_META_INFO_LIST,
    MSG_SET_CODEC_DATA,
    MSG_IS_INITIALIZED,
    MSG_RELOAD_ACDB_FILES,
    MSG_RELOAD_ACDB_FILES_V2
};

struct acdb_init_data_v4 {
    char                   *cvd_version;
    char                   *snd_card_name;
    struct listnode        *meta_key_list;
    bool                   *is_instance_id_supported;
};

typedef struct acdb_audio_cal_cfg {
    unsigned int    persist;
    unsigned int    snd_dev_id;
    unsigned int    dev_id;
    int     acdb_dev_id;
    unsigned int    app_type;
    unsigned int    topo_id;
    unsigned int    sampling_rate;
    unsigned int    cal_type;
    unsigned int    module_id;
    unsigned short    instance_id;
    unsigned short    reserved;
    unsigned int    param_id;
} acdb_audio_cal_cfg_t;

typedef struct acdb_data_buf {
    int         msgID;
    int         hdrSize;
    int         bufPos;
    int         readPos;
    int         bufSize;
    char        data[2048];
}acdb_data_buf_t;

int readInt(int32_t *pArg, acdb_data_buf_t * data_buf)
{
    if (data_buf == NULL || pArg == NULL)
        return -1;
    if ((data_buf->readPos+sizeof(int32_t)) <= data_buf->bufPos) {
        const void* data = data_buf->data + data_buf->readPos;
        data_buf->readPos += sizeof(int32_t);
        *pArg =  *(int32_t *)data;
        //printf("recvDataPos %d *pArg %d\n", data_buf->readPos, *pArg);
        return 0;
    } else
        return -1;
}


typedef int  (*acdb_init_v2_t)(const char *, char *, int);
typedef int  (*acdb_init_v3_t)(const char *, char *, struct listnode *);
typedef int  (*acdb_init_v4_t)(void *, int);
typedef int  (*acdb_init_t)(void);
typedef int  (*acdb_get_default_app_type_t)(void);
typedef int  (*acdb_send_common_custom_topology_t)(void);
typedef void (*acdb_deallocate_t)(void);
typedef void (*acdb_deallocate_cal_t)(int, int);
typedef void (*acdb_send_voice_cal_v2_t)(int, int, int);
typedef void (*acdb_send_voice_cal_t)(int, int);
typedef int  (*acdb_reload_vocvoltable_t)(int);
typedef void (*acdb_send_audio_cal_t)(int, int);
typedef void (*acdb_send_audio_cal_v2_t)(int, int, int, int);
typedef void (*acdb_send_audio_cal_v3_t)(int, int, int, int, int);
typedef void (*acdb_send_audio_cal_v4_t)(int, int, int, int, int, int);
typedef void (*acdb_send_audio_cal_v6_t)(int, int, int, int, int, int, int, int);
typedef void (*acdb_send_listen_device_cal_t)(int, int, int, int);
typedef int  (*acdb_send_listen_lsm_cal_t)(int, int, int, int);
typedef int  (*acdb_send_listen_lsm_cal_v1_t)(int, int, int, int, int);
typedef int  (*acdb_send_anc_cal_t)(int);
typedef void (*acdb_send_tabla_anc_data_t)(void);
typedef int (*acdb_get_aud_volume_steps_t)(void);
typedef int (*acdb_send_gain_dep_cal_t)(int, int, int, int, int);
typedef int (*acdb_get_remote_acdb_id_t)(unsigned int);
typedef int (*acdb_get_ecrx_device_t)(int);
typedef int (*acdb_get_calibration_t)(char *, int, void *);
typedef int (*acdb_set_audio_cal_v2_t)(void *, void*, unsigned int);
typedef int (*acdb_get_audio_cal_v2_t)(void *, void*, unsigned int *);
typedef int (*acdb_send_meta_info_t)(int, int);
typedef int (*acdb_send_meta_info_list_t)(struct listnode *);
typedef int (*acdb_set_codec_data_t)(void *, char *);
typedef bool (*acdb_is_initialized_t)(void);
typedef int (*acdb_reload_acdb_files_t)(char *, char *, char *, int);
typedef int (*acdb_reload_acdb_files_v2_t)(char *, char *, char *, struct listnode *);

struct platform_data {
    void                               *acdb_handle;
    acdb_init_v2_t                     acdb_init_v2;
    acdb_init_v3_t                     acdb_init_v3;
    acdb_init_v4_t                     acdb_init_v4;
    acdb_init_t                        acdb_init;
    acdb_get_default_app_type_t        acdb_get_default_app_type;
    acdb_send_common_custom_topology_t acdb_send_common_custom_topology;
    acdb_deallocate_t                  acdb_deallocate;
    acdb_deallocate_cal_t              acdb_deallocate_cal;
    acdb_send_voice_cal_v2_t           acdb_send_voice_cal_v2;
    acdb_send_voice_cal_t              acdb_send_voice_cal;
    acdb_reload_vocvoltable_t          acdb_reload_vocvoltable;
    acdb_send_audio_cal_t              acdb_send_audio_cal;
    acdb_send_audio_cal_v2_t           acdb_send_audio_cal_v2;
    acdb_send_audio_cal_v3_t           acdb_send_audio_cal_v3;
    acdb_send_audio_cal_v4_t           acdb_send_audio_cal_v4;
    acdb_send_audio_cal_v6_t           acdb_send_audio_cal_v6;
    acdb_send_listen_device_cal_t      acdb_send_listen_device_cal;
    acdb_send_listen_lsm_cal_t         acdb_send_listen_lsm_cal;
    acdb_send_listen_lsm_cal_v1_t      acdb_send_listen_lsm_cal_v1;
    acdb_send_anc_cal_t                acdb_send_anc_cal;
    acdb_send_tabla_anc_data_t         acdb_send_tabla_anc_data;
    acdb_get_aud_volume_steps_t        acdb_get_aud_volume_steps;
    acdb_send_gain_dep_cal_t           acdb_send_gain_dep_cal;
    acdb_get_remote_acdb_id_t          acdb_get_remote_acdb_id;
    acdb_get_ecrx_device_t             acdb_get_ecrx_device;
    acdb_get_calibration_t             acdb_get_calibration;
    acdb_set_audio_cal_v2_t            acdb_set_audio_cal_v2;
    acdb_get_audio_cal_v2_t            acdb_get_audio_cal_v2;
    acdb_send_meta_info_t              acdb_send_meta_info;
    acdb_send_meta_info_list_t         acdb_send_meta_info_list;
    acdb_set_codec_data_t              acdb_set_codec_data;
    acdb_is_initialized_t              acdb_is_initialized;
    acdb_reload_acdb_files_t           acdb_reload_acdb_files;
    acdb_reload_acdb_files_v2_t        acdb_reload_acdb_files_v2;
    struct acdb_init_data_v4           acdb_init_data;
    struct listnode                    acdb_meta_key_list;
};

struct audio_cal_info_metainfo {
    unsigned int nKey;
};

struct meta_key_list {
    struct listnode list;
    struct audio_cal_info_metainfo cal_info;
};

/****************** acdb test ***********************/

char acdb_lib[100] = {0};

int main(int argc, char **argv)
{
    char *snd_card_name = "sa8155-adp-star-snd-card";// snd card name!
    struct platform_data *my_data = NULL;
    const char* error;
    int32_t rc = 0;
    unsigned int acdb_id = DEFAULT_ACDB_ID;
    int testcase = 50;
    int lib_id = 0;
    static bool acdb_instance_id_support = false;
    struct meta_key_list key_info;
    acdb_data_buf_t data_buf;
    /* parse command line arguments */
    printf("parse args start: -a : acdb_id , -t test_case, -l : lib_case\n");
    argv ++;
    while (*argv) {
       if (strcmp(*argv, "-a") == 0) {
            argv++;
            if (*argv)
                acdb_id = atoi(*argv);
        }
        if (strcmp(*argv, "-t") == 0) {
            argv++;
            if (*argv)
                testcase = atoi(*argv);
        }
        if (strcmp(*argv, "-l") == 0) {
            argv++;
            if (*argv)
                lib_id = atoi(*argv);
            if (lib_id == 0)
                strlcpy(acdb_lib, "/usr/lib/libacdbloader.so", sizeof(acdb_lib));
            else if (lib_id == 1)
                strlcpy(acdb_lib, "/usr/lib/libacdbloaderclient.so", sizeof(acdb_lib));
            else if (lib_id == 2)
                strlcpy(acdb_lib, "/vendor/lib64/libacdbloadersocketclient.so", sizeof(acdb_lib));
            else
                printf("0:libacdbloader.so\n \
                        1:libacdbloaderclient.so \n \
                        2:libacdbloadersocketclient.so \n  \
                        others:invalid\n");
        }

        if (*argv)
            argv++;
    }
    printf("parse args end\n");

    printf("acdb_loader start testcase %d acdb_lib: %s \n", testcase, acdb_lib);
    my_data = calloc(1, sizeof(struct platform_data));
    if (!my_data) {
        ALOGE("my_data calloc failed\n");
        rc = -ENOMEM;
        return 0;
    }

    data_buf.readPos = 0;
    data_buf.bufSize = sizeof(acdb_data_buf_t);
    readInt(&key_info.cal_info.nKey, &data_buf);
    list_init(&my_data->acdb_meta_key_list);
    list_add_tail(&my_data->acdb_meta_key_list, &key_info.list);

    error = dlerror();
    my_data->acdb_handle = dlopen(acdb_lib, RTLD_LAZY);
    error = dlerror();
    if (error != NULL || my_data->acdb_handle == NULL) {
        printf("%s: DLOPEN failed for %s with error %s\n", __func__, acdb_lib, error);
        rc = 1;
        goto free_mydata;
    }
    printf("DLOPEN success for %s\n", acdb_lib);

    switch(testcase) {
        case MSG_INIT_V2: {
            // Test acdb_init_v2
            my_data->acdb_init_v2 = (acdb_init_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_init_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_init_v2 == NULL) {
                printf("%s: DLSYM failed for acdb_loader_init_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_init_v2) {
                printf("acdb_init_v2\n");
                char cvd_version[] = "2.4";
                char snd_card_name[] = "sa6155-adp-star-snd-card";
                int metainfoid = 0;
                rc = my_data->acdb_init_v2(snd_card_name, cvd_version, metainfoid);
                if (rc) {
                    printf("acdb_init_v2 failed\n");
                    goto close_acdbhandle;
                }
            }
            break;
        }
        case MSG_INIT_V3: {
        // Test acdb_init_v3
            my_data->acdb_init_v3 = (acdb_init_v3_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_init_v3");
            error = dlerror();
            if (error != NULL || my_data->acdb_init_v3 == NULL) {
                printf("%s: DLSYM failed for acdb_loader_init_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_init_v3) {
                printf("acdb_init_v3\n");
                char cvd_version[] = "2.4";
                char snd_card_name[] = "sa6155-adp-star-snd-card";
                rc = my_data->acdb_init_v3(snd_card_name, cvd_version, &my_data->acdb_meta_key_list);
                if (rc) {
                    printf("acdb_init_v3 failed\n");
                    goto close_acdbhandle;
                }
            }
            break;
        }
        case MSG_INIT_V4: {
        // Test acdb_loader_init_v4
            my_data->acdb_init_v4 = (acdb_init_v4_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_init_v4");
            error = dlerror();
            if (error != NULL || my_data->acdb_init_v4 == NULL) {
                printf("%s: DLSYM failed for acdb_loader_init_v4 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_init_v4) {
                printf("acdb_init_v4\n");
                my_data->acdb_init_data.cvd_version = "2.4";
                my_data->acdb_init_data.snd_card_name = "sa6155-adp-star-snd-card";
                my_data->acdb_init_data.meta_key_list = &my_data->acdb_meta_key_list;
                my_data->acdb_init_data.is_instance_id_supported = &acdb_instance_id_support;
                rc = my_data->acdb_init_v4(&my_data->acdb_init_data, ACDB_LOADER_INIT_V4);
                if (rc) {
                    printf("acdb_init_v4 failed\n");
                    goto close_acdbhandle;
                }
            }
            break;
        }
        case MSG_INIT_ACDB: {
        // Test acdb_init
            my_data->acdb_init = (acdb_init_t)dlsym(my_data->acdb_handle,"acdb_loader_init_ACDB");
            if (my_data->acdb_init == NULL) {
                ALOGE("%s: dlsym error for acdb_loader_init_ACDB", __func__);
                goto close_acdbhandle;
            }

            if (my_data->acdb_init) {
                printf("acdb_init\n");
                char snd_card_name[] = "sa6155-adp-star-snd-card";
                rc = my_data->acdb_init();
                if (rc) {
                    printf("acdb_loader_init_ACDB failed\n");
                    goto close_acdbhandle;
                }
            }
            break;
        }
        case MSG_GET_DEF_APP_TYPE: {
        // Test acdb_get_default_app_type
            my_data->acdb_get_default_app_type = (acdb_get_default_app_type_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_default_app_type");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_default_app_type == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_default_app_type with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_default_app_type) {
                ALOGD("acdb_loader_get_default_app_type \n");
                rc = my_data->acdb_get_default_app_type();
                if (rc != -1) {
                    printf("acdb_get_default_app_type rc %d\n", rc);
                    goto close_acdbhandle;
                }
            }
            break;
        }
        case MSG_SEND_COM_CUSTOM_TOP: {
        // Test acdb_send_common_custom_topology
            my_data->acdb_send_common_custom_topology = (acdb_send_common_custom_topology_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_common_custom_topology");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_common_custom_topology == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_common_custom_topology with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_common_custom_topology) {
                ALOGD("acdb_loader_send_common_custom_topology \n");
                my_data->acdb_send_common_custom_topology();
            }
            break;
        }
        case MSG_DEALLOCATE_ACDB: {
        // Test acdb_loader_deallocate_ACDB
            my_data->acdb_deallocate = (acdb_deallocate_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_deallocate_ACDB");
            error = dlerror();
            if (error != NULL || my_data->acdb_deallocate == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_deallocate_ACDB with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_deallocate) {
                ALOGD("acdb_loader_deallocate_ACDB \n");
                my_data->acdb_deallocate();
            }
            break;
        }
        case MSG_DEALLOCATE_CAL: {
            my_data->acdb_deallocate_cal = (acdb_deallocate_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_deallocate_cal");

            int capability = 0;
            int use_case = 14;
            error = dlerror();
            if (error != NULL || my_data->acdb_deallocate_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_deallocate_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_deallocate_cal) {
                ALOGD("acdb_loader_deallocate_cal \n");
                my_data->acdb_deallocate_cal(capability, use_case);
            }
            break;
        }
        case MSG_SEND_VOICE_CAL_V2: {
            // Test acdb_send_voice_cal_v2
            my_data->acdb_send_voice_cal_v2 = (acdb_send_voice_cal_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_voice_cal_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_voice_cal_v2 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_voice_cal_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_voice_cal_v2) {
                ALOGD("acdb_send_voice_cal_v2 \n");
                int acdb_rx_id = 60, acdb_tx_id = 11;
                int featureset = 0;
                my_data->acdb_send_voice_cal_v2(acdb_rx_id, acdb_tx_id, featureset);
            }
            break;
        }
        case MSG_SEND_VOICE_CAL: {
            // Test acdb_loader_send_voice_cal
            my_data->acdb_send_voice_cal = (acdb_send_voice_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_voice_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_voice_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_voice_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_voice_cal) {
                ALOGD("acdb_loader_send_voice_cal \n");
                int acdb_rx_id = 60, acdb_tx_id = 11;
                my_data->acdb_send_voice_cal(acdb_rx_id, acdb_tx_id);
            }
            break;
        }
        case MSG_RELOAD_VOICE_VOL_TABLE: {
            // Test acdb_loader_reload_vocvoltable
            my_data->acdb_reload_vocvoltable = (acdb_reload_vocvoltable_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_reload_vocvoltable");
            error = dlerror();
            if (error != NULL || my_data->acdb_reload_vocvoltable == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_reload_vocvoltable with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_reload_vocvoltable) {
                ALOGD("acdb_loader_reload_vocvoltable \n");
                int featureset = 0;
                my_data->acdb_reload_vocvoltable(featureset);
            }
            break;
        }
        case MSG_SEND_AUDIO_CAL: {
            // Test acdb_send_audio_cal
            my_data->acdb_send_audio_cal = (acdb_send_audio_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_audio_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_audio_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_audio_cal) {
                ALOGD("acdb_loader_send_audio_cal \n");
                my_data->acdb_send_audio_cal(AFE_PROXY_ACDB_ID, ACDB_DEV_TYPE_OUT);
            }
            break;
        }
        case MSG_SEND_AUDIO_CAL_V2: {
            // Test acdb_send_audio_cal_v2
            my_data->acdb_send_audio_cal_v2 = (acdb_send_audio_cal_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_audio_cal_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_audio_cal_v2 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_audio_cal_v2) {
                ALOGD("acdb_loader_send_audio_cal_v2 \n");
                my_data->acdb_send_audio_cal_v2(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE);
            }
            break;
        }
        case MSG_SEND_AUDIO_CAL_V3: {
            // Test acdb_send_audio_cal_v3
            my_data->acdb_send_audio_cal_v3 = (acdb_send_audio_cal_v3_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_audio_cal_v3");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_audio_cal_v3 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v3 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_audio_cal_v3) {
                ALOGD("acdb_loader_send_audio_cal_v3 \n");
                my_data->acdb_send_audio_cal_v3(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0);
            }
            break;
        }
        case MSG_SEND_AUDIO_CAL_V4: {
            // Test acdb_send_audio_cal_v4
            my_data->acdb_send_audio_cal_v4 = (acdb_send_audio_cal_v4_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_audio_cal_v4");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_audio_cal_v4 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v4 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_audio_cal_v4) {
                ALOGD("acdb_loader_send_audio_cal_v4 \n");
                my_data->acdb_send_audio_cal_v4(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0, DEFAULT_OUTPUT_SAMPLING_RATE);
            }
            break;
        }
        case MSG_SEND_AUDIO_CAL_V6: {
            // Test acdb_send_audio_cal_v6
            my_data->acdb_send_audio_cal_v6 = (acdb_send_audio_cal_v6_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_audio_cal_v6");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_audio_cal_v6 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_audio_cal_v6 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_audio_cal_v6) {
                ALOGD("acdb_loader_send_audio_cal_v6 \n");
                my_data->acdb_send_audio_cal_v6(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0, DEFAULT_OUTPUT_SAMPLING_RATE, DEFAULT_CAL_MODE, DEFAULT_OFFSET_INDEX);
            }
            break;
        }
        case MSG_SEND_LISTEN_DEV_CAL: {
        // Test acdb_send_listen_device_cal
            my_data->acdb_send_listen_device_cal = (acdb_send_listen_device_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_listen_device_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_listen_device_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_listen_device_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_listen_device_cal) {
                ALOGD("acdb_loader_send_listen_device_cal \n");
                my_data->acdb_send_listen_device_cal(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE);
            }
            break;
        }
        case MSG_SEND_LISTEN_LSM_CAL: {
        // Test acdb_send_listen_lsm_cal
            my_data->acdb_send_listen_lsm_cal = (acdb_send_listen_lsm_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_listen_lsm_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_listen_lsm_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_listen_lsm_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_listen_lsm_cal) {
                ALOGD("acdb_loader_send_listen_lsm_cal \n");
                int hw_type = 1;
                my_data->acdb_send_listen_lsm_cal(acdb_id, DEFAULT_APP_TYPE_RX_PATH, 1, hw_type);
            }
            break;
        }
        case MSG_SEND_LISTEN_LSM_CAL_V1: {
            // Test acdb_loader_send_listen_lsm_cal_v1
            my_data->acdb_send_listen_lsm_cal_v1 = (acdb_send_listen_lsm_cal_v1_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_listen_lsm_cal_v1");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_listen_lsm_cal_v1 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_listen_lsm_cal_v1 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_listen_lsm_cal_v1) {
                ALOGD("acdb_loader_send_listen_lsm_cal_v1 \n");
                int hw_type = 1;
                my_data->acdb_send_listen_lsm_cal_v1(acdb_id, DEFAULT_APP_TYPE_RX_PATH, 1, hw_type, 0);
            }
            break;
        }
        case MSG_SEND_ANC_CAL: {
            // Test acdb_loader_send_anc_cal
            my_data->acdb_send_anc_cal = (acdb_send_anc_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_anc_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_anc_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_anc_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_anc_cal) {
                ALOGD("acdb_loader_send_anc_cal \n");
                my_data->acdb_send_anc_cal(acdb_id);
            }
            break;
        }
        case MSG_SEND_ANC_DATA: {
            // Test acdb_send_tabla_anc_data
            my_data->acdb_send_tabla_anc_data = (acdb_send_tabla_anc_data_t)dlsym(my_data->acdb_handle,
                                                         "send_tabla_anc_data");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_tabla_anc_data == NULL) {
                ALOGE("%s: DLSYM failed for send_tabla_anc_data with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_tabla_anc_data) {
                ALOGD("send_tabla_anc_data \n");
                my_data->acdb_send_tabla_anc_data();
            }
            break;
        }
        case MSG_GET_AUD_VOL_STEPS: {
            // Test acdb_get_aud_volume_steps
            my_data->acdb_get_aud_volume_steps = (acdb_get_aud_volume_steps_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_aud_volume_steps");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_aud_volume_steps == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_aud_volume_steps with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_aud_volume_steps) {
                ALOGD("acdb_loader_get_aud_volume_steps \n");
                my_data->acdb_get_aud_volume_steps();
            }
            break;
        }
        case MSG_SEND_GAIN_DEP_CAL: {
            // Test acdb_loader_send_gain_dep_cal
            my_data->acdb_send_gain_dep_cal = (acdb_send_gain_dep_cal_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_send_gain_dep_cal");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_gain_dep_cal == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_send_gain_dep_cal with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_gain_dep_cal) {
                ALOGD("acdb_loader_send_gain_dep_cal \n");
                my_data->acdb_send_gain_dep_cal(acdb_id, DEFAULT_APP_TYPE_RX_PATH, ACDB_DEV_TYPE_OUT, 1, -1);
            }
            break;
        }
        case MSG_GET_REMOTE_ACDB_ID: {
            // Test acdb_get_remote_acdb_id
            my_data->acdb_get_remote_acdb_id = (acdb_get_remote_acdb_id_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_remote_acdb_id");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_remote_acdb_id == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_remote_acdb_id with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_remote_acdb_id) {
                ALOGD("acdb_loader_get_remote_acdb_id \n");
                my_data->acdb_get_remote_acdb_id(acdb_id);
            }
            break;
        }
        case MSG_GET_ECRX_DEVICE: {
            // Test acdb_loader_get_ecrx_device
            my_data->acdb_get_ecrx_device = (acdb_get_ecrx_device_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_ecrx_device");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_ecrx_device == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_ecrx_device with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_ecrx_device) {
                ALOGD("acdb_loader_get_ecrx_device \n");
                my_data->acdb_get_ecrx_device(acdb_id);
            }
            break;
        }
        case MSG_GET_CALIBRATION: {
#if 0
            // Test acdb_loader_get_calibration
            my_data->acdb_get_calibration = (acdb_get_calibration_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_calibration");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_calibration == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_calibration with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_calibration) {
                ALOGD("acdb_loader_get_calibration \n");
                my_data->acdb_get_calibration(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0, DEFAULT_OUTPUT_SAMPLING_RATE);
            }
#endif
            break;
        }
        case MSG_SET_AUDIO_CAL_V2: {
#if 0
            // Test acdb_loader_set_audio_cal_v2
            my_data->acdb_set_audio_cal_v2 = (acdb_set_audio_cal_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_set_audio_cal_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_set_audio_cal_v2 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_set_audio_cal_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_set_audio_cal_v2) {
                ALOGD("acdb_loader_set_audio_cal_v2 \n");
                my_data->acdb_set_audio_cal_v2(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0, DEFAULT_OUTPUT_SAMPLING_RATE);
            }
#endif
            break;
        }
        case MSG_GET_AUDIO_CAL_V2: {
#if 0
            // Test acdb_loader_get_audio_cal_v2
            my_data->acdb_get_audio_cal_v2 = (acdb_get_audio_cal_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_get_audio_cal_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_get_audio_cal_v2 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_get_audio_cal_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_get_audio_cal_v2) {
                ALOGD("acdb_loader_get_audio_cal_v2 \n");
                my_data->acdb_get_audio_cal_v2(acdb_id, ACDB_DEV_TYPE_OUT, DEFAULT_APP_TYPE_RX_PATH,
                                                DEFAULT_OUTPUT_SAMPLING_RATE, 0, DEFAULT_OUTPUT_SAMPLING_RATE);
            }
#endif
            break;
        }
        case MSG_SEND_META_INFO: {
            // Test send_meta_info
            my_data->acdb_send_meta_info = (acdb_send_meta_info_t)dlsym(my_data->acdb_handle,
                                                         "send_meta_info");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_meta_info == NULL) {
                ALOGE("%s: DLSYM failed for send_meta_info with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_meta_info) {
                ALOGD("send_meta_info \n");
                int metaInfoKey = 0;
                int buf_idx = 0;
                my_data->acdb_send_meta_info(metaInfoKey, buf_idx);
            }
            break;
        }
        case MSG_SEND_META_INFO_LIST: {
            // Test acdb_send_meta_info_list
            my_data->acdb_send_meta_info_list = (acdb_send_meta_info_list_t)dlsym(my_data->acdb_handle,
                                                         "send_meta_info_list");
            error = dlerror();
            if (error != NULL || my_data->acdb_send_meta_info_list == NULL) {
                ALOGE("%s: DLSYM failed for send_meta_info_list with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_send_meta_info_list) {
                ALOGD("send_meta_info_list \n");
                my_data->acdb_send_meta_info_list(&my_data->acdb_meta_key_list);
            }
            break;
        }
        case MSG_SET_CODEC_DATA: {
            // Test acdb_set_codec_data_t
            my_data->acdb_set_codec_data = (acdb_set_codec_data_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_set_codec_data");
            error = dlerror();
            if (error != NULL || my_data->acdb_set_codec_data == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_set_codec_data with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_set_codec_data) {
                uint16_t vbat_adc_data[2];
                vbat_adc_data[0] = 1;
                vbat_adc_data[1] = 1;
                ALOGD("acdb_loader_set_codec_data \n");
                my_data->acdb_set_codec_data(&vbat_adc_data[0], "anc_cal");
            }
            break;
        }
        case MSG_IS_INITIALIZED: {
            // Test acdb_is_initialized
            my_data->acdb_is_initialized = (acdb_is_initialized_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_is_initialized");
            error = dlerror();
            if (error != NULL || my_data->acdb_is_initialized == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_is_initialized with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_is_initialized) {
                ALOGD("acdb_loader_is_initialized \n");
                my_data->acdb_is_initialized();
            }
            break;
        }
        case MSG_RELOAD_ACDB_FILES: {
            // Test acdb_loader_reload_acdb_files
            my_data->acdb_reload_acdb_files = (acdb_reload_acdb_files_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_reload_acdb_files");
            error = dlerror();
            if (error != NULL || my_data->acdb_reload_acdb_files == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_reload_acdb_files with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_reload_acdb_files) {
                ALOGD("acdb_loader_reload_acdb_files \n");
                char new_path_name[] = "/vendor/etc/acdbdata";
                char cvd_version[] = "2.4";
                char snd_card_name[] = "sa6155-adp-star-snd-card";
                int metainfoid = 0;
                my_data->acdb_reload_acdb_files(new_path_name, snd_card_name, cvd_version,metainfoid);
            }
            break;
        }
        case MSG_RELOAD_ACDB_FILES_V2: {
            // Test acdb_loader_reload_acdb_files_v2
            my_data->acdb_reload_acdb_files_v2 = (acdb_reload_acdb_files_v2_t)dlsym(my_data->acdb_handle,
                                                         "acdb_loader_reload_acdb_files_v2");
            error = dlerror();
            if (error != NULL || my_data->acdb_reload_acdb_files_v2 == NULL) {
                ALOGE("%s: DLSYM failed for acdb_loader_reload_acdb_files_v2 with error %s\n", __func__, error);
                rc = 1;
                goto close_acdbhandle;
            }

            if (my_data->acdb_reload_acdb_files_v2) {
                ALOGD("acdb_loader_reload_acdb_files_v2 \n");
                char new_path_name[] = "/vendor/etc/acdbdata";
                char cvd_version[] = "2.4";
                char snd_card_name[] = "sa6155-adp-star-snd-card";
                my_data->acdb_reload_acdb_files_v2(new_path_name, snd_card_name, cvd_version,&my_data->acdb_meta_key_list);
            }
            break;
        }
        default:
            break;
    }

close_acdbhandle:
    dlclose(my_data->acdb_handle);
free_mydata:
    free(my_data);

    return rc;
}

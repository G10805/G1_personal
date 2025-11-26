/**
*==============================================================================
*  \file adie_rtc_api.h
*  \brief
*                      M C S  A P I  H E A D E R  F I L E
*
*      This header defines the APIs nessesary to control basic audio media
*      services such as playback and record
*
*  \copyright
*      Copyright (c) 2020 Qualcomm Technologies, Inc.
*      All Rights Reserved.
*      Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*==============================================================================
*/

#ifndef MCS_API_H
#define MCS_API_H

#include "ar_osal_types.h"
#include "ar_osal_error.h"

#define MCS_MAX_FILENAME_LENGTH 256

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
** Structure Definitions and Enums
** ----------------------------------------------------------------------- */

typedef enum mcs_api_major_version_t
{
    MCS_API_MAJOR_VERSION_1 = 0x1,
}mcs_api_major_version_t;

typedef enum mcs_api_minor_version_t
{
    /**< Initial version */
    MCS_API_MINOR_VERSION_0 = 0x0,
}mcs_api_minor_version_t;

/**< Media control commands */
typedef enum mcs_command_t
{
    MCS_CMD_UNKNOWN = 0x00,
    MCS_CMD_PLAY = 0x01,
    MCS_CMD_REC = 0x02,
    MCS_CMD_PLAY_REC = 0x03,
    MCS_CMD_STOP = 0x04,
} mcs_command_t;

/**< A <key,value> pair */
typedef struct mcs_key_value_pair_t
{
    uint32_t key;
    uint32_t value;

}mcs_key_value_pair_t;

/**< Maintains a list of key value pairs */
typedef struct mcs_key_vector_t
{
    /**< number of key-value pairs */
    uint32_t num_kvps;
    /**< key-value pairs */
    mcs_key_value_pair_t *kvp;
}mcs_key_vector_t;

/**< Specifies the set of playback modes used in mcs_play_t */
typedef enum mcs_playback_mode_t
{
    MCS_PLAYBACK_REGULAR,
    MCS_PLAYBACK_ANC_S_PATH,
    MCS_PLAYBACK_ANC_E_PATH,
    MCS_PLAYBACK_RAS,
} mcs_playback_mode_t;

/**< Specifies the set of recording modes used in the ATS_CMD_MCS_RECORD request*/
typedef enum mcs_record_mode_t
{
    MCS_RECORD_REGULAR,
    MCS_RECORD_ANC_P_PATH,
    MCS_RECORD_ANC_S_PATH,
    MCS_RECORD_ANC_E_PATH
} mcs_record_mode_t;

/**< Media properties for a device or stream*/
typedef struct mcs_properties_t
{
    /**< The device/stream sample rate*/
    uint32_t sample_rate;
    /**< The bit width of each sample*/
    uint16_t bit_width;
    /**< Indicates the alignment of bits_per_sample in sample_word_size.Relevant only when bits_per_sample is 24 and word_size is 32*/
    uint16_t alignment;
    /**< Bits needed to store one sample*/
    uint16_t bits_per_sample;
    /**< Q factor of the PCM data*/
    uint16_t q_factor;
    /**< Indicates whether PCM samples are stored in little endian or big endian format.*/
    uint16_t endianness;
    /**< Number of channels*/
    uint16_t num_channels;
    /**< Determines what channels are used. Up to 32 channels are supported. If not supported set to 0*/
    uint8_t *channel_mapping;
}mcs_properties_t;

/**< Contains properties for controlling audio playback */
typedef struct mcs_play_t
{
    /**< playback device properties*/
    mcs_properties_t device_properties;
    /**< playback stream properties*/
    mcs_properties_t stream_properties;
    /**< Graph Key Vector representing the playback usecase*/
    mcs_key_vector_t graph_key_vector;
    /**< Type of playback: 0(regular), 1(anc_spath), 2(anc_epath), 3(ras)*/
    mcs_playback_mode_t playback_mode;
    /**< Playback duration in seconds*/
    int32_t playback_duration_sec;
    /**< Length of the file name/path*/
    int32_t filename_len;
    /**< Name of/path to file to save recording to*/
    char filename[MCS_MAX_FILENAME_LENGTH];
}mcs_play_t;


/**< Contains properties for controlling audio record */
typedef struct mcs_record_t
{
    /**< record device properties*/
    mcs_properties_t device_properties;
    /**< record stream properties*/
    mcs_properties_t stream_properties;
    /**< Graph Key Vector representing the record usecase*/
    mcs_key_vector_t graph_key_vector;
    /**< Type of record: 0(regular), 1(aanc_path), 2(aanc_spath), 3(aanc_epath)*/
    mcs_record_mode_t record_mode;
    /**< Save recording to file: 0 (ignore write), 1 (write data)*/
    uint32_t write_to_file;
    /**< Record duration in seconds*/
    int32_t record_duration_sec;
    /**< Length of the file name/path*/
    int32_t filename_len;
    /**< Name of/path to file to save recording to*/
    char filename[MCS_MAX_FILENAME_LENGTH];
}mcs_record_t;

/**< Contains properties for controlling audio playback and record simultaneously */
typedef struct mcs_play_rec_t
{
    mcs_play_t playback_session;
    mcs_record_t record_session;
}mcs_play_rec_t;

typedef struct mcs_version
{
    /**< Major version for changes that break backward compatibility */
    uint32_t major;
    /**< Major version for changes that do not break backward compatibility */
    uint32_t minor;
}mcs_version_t;

/****************************************************************************
** Functions
*****************************************************************************/

int32_t mcs_init(void);

int32_t mcs_deinit(void);

int32_t mcs_get_api_version(mcs_version_t* api_version);

int32_t mcs_stream_cmd(
    uint32_t    cmd,
    uint8_t     *cmd_buf,
    uint32_t    cmd_buf_size,
    uint8_t     *rsp_buf,
    uint32_t    rsp_buf_size,
    uint32_t    *rsp_buf_bytes_filled);

#ifdef __cplusplus
}
#endif

#endif /* MCS_API_H */


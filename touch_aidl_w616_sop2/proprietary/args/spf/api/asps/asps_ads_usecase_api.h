#ifndef _ASPS_ADS_USECASE_API_H_
#define _ASPS_ADS_USECASE_API_H_
/**
 * \file asps_ads_useacase_api.h
 * \brief
 *    This file contains PCM DATA Usecase ID specific APIs.
 *
 *    Copyright (c) 2021 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ar_defs.h"

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


/*------------------------------------------------------------------------------
 *  Usecase ID definition
 *----------------------------------------------------------------------------*/

/*ADS usecase ID */
#define ASPS_USECASE_ID_PCM_DATA 0x0B00100A

#ifdef ASPS_USECASE_ID_PCM_DATA /* definitions specific to PCM DATA usecase */

/**
   PCM DATA usecase(ASPS_USECASE_ID_PCM_DATA) sepcific payload. This payload contains the
   type of the stream required by the sensor.
   This structure is preceeded by param_id_asps_sensor_usecase_register_t structure. */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"
struct asps_pcm_data_usecase_register_payload_t
{
   //stream type for pcm data takes values {0,1,2}
   // 0 : INVALID, 1 : STREAM_TYPE_PCM_RAW, 2 : STREAM_TYPE_PCM_NS
   uint32_t stream_type;
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct asps_pcm_data_usecase_register_payload_t asps_pcm_data_usecase_register_payload_t;

#endif /* ASPS_USECASE_ID_PCM_DATA */

#ifdef ASPS_USECASE_ID_PCM_DATA /* definitions specific to PCM DATA usecase */

/** PCM DATA usecase (ASPS_USECASE_ID_PCM_DATA) specific Payload structure. This structure is preceeded
    by event_id_asps_sensor_usecase_info_t. */

#include "spf_begin_pack.h"
#include "spf_begin_pragma.h"

struct asps_audio_data_usecase_register_ack_payload_t
{
   uint32_t sns_rd_ep_miid;
   /**< Module instance ID of SNS read end point module. */

   uint32_t mfc_miid;
   /**< Module instance ID of MFC module. */

   uint32_t pcm_cnv_miid;
   /**< Module instance ID of PCM_CNV module. */
}
#include "spf_end_pragma.h"
#include "spf_end_pack.h"
;
typedef struct asps_audio_data_usecase_register_ack_payload_t asps_audio_data_usecase_register_ack_payload_t;

#endif /* ASPS_USECASE_ID_PCM_DATA */

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* _ASPS_ADS_USECASE_API_H_ */

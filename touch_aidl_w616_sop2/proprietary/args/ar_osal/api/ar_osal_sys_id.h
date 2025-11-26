#ifndef AR_OSAL_SYS_ID_H
#define AR_OSAL_SYS_ID_H

/**
 *
 * \file ar_osal_sys_id.h
 *
 * \brief
 *      Defines Supported Sub-System IDs.
 * \cond
 *  Copyright (c) 2020 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 * \endcond
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


/**
* Invalid sub system
*/
#define AR_SUB_SYS_ID_INVALID         0
/**
* Used for MODEM DSP sub system
 */
#define AR_MODEM_DSP                 1
/**
* Used for ADSP sub system
*/
#define AR_AUDIO_DSP                 2
/**
* Used for AP sub system
*/
#define AR_APSS                      3
/**
* Used for SENSOR DSP sub system
*/
#define AR_SENSOR_DSP                4
/**
* Used for COMPUTE DSP sub system
*/
#define AR_COMPUTE_DSP               5
#define AR_CC_DSP                    6

/**
* IDs 7/8/9 used in QACT for WDSP/WHS/KYMERA
*/

/**
 * must be the same as APM_PROC_DOMAIN_ID_GDSP_0 in apm_graph_properties.h
 */
#define AR_GP_DSP0                  0xA
#define AR_GP_DSP1                  0xB

/**
* First sub system ID
*/
#define AR_SUB_SYS_ID_FIRST    AR_MODEM_DSP
/**
* Last sub system ID
*/
#define AR_SUB_SYS_ID_LAST     AR_GP_DSP1

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif //AR_OSAL_SYS_ID_H

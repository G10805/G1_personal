#ifndef _APM_GRAPH_PROPERTIES_H_
#define _APM_GRAPH_PROPERTIES_H_
/**
 * \file apm_graph_properties.h
 * \brief
 *          This file contains APM container commands and events structures definition
 *
 *
 * \copyright
 *  Copyright (c) 2019, 2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/apm/apm_graph_properties.h#11 $
*/
// clang-format on

#include "ar_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

/** Legacy Container Capability IDs. To be removed. */
/**< Enumeration for container capability ID Pre/Post
  Processing (PP) */
#define APM_CONTAINER_CAP_ID_PP                   0x1

/**< Enumeration for container capability ID
  Compression/Decompression (CD) */
#define APM_CONTAINER_CAP_ID_CD                   0x2

/**< Enumeration for container capability ID End Point(EP) */
#define APM_CONTAINER_CAP_ID_EP                   0x3

/**< Enumeration for container capability ID Offload (OLC) */
#define APM_CONTAINER_CAP_ID_OLC                  0x4


/**< Enumeration for container type for Specialized Container */
#define APM_CONTAINER_TYPE_ID_SC                   0x0B001000

/**< Enumeration for container type for Generic Container */
#define APM_CONTAINER_TYPE_ID_GC                   0x0B001001

/**< Enumeration for container type OffLoad Container (OLC) */
#define APM_CONTAINER_TYPE_ID_OLC                  0x0B001002


/////////////////DOMAIN IDs//////////////////////////

/**  THESE HAVE TO BE SAME AS GPR IDs
  refer gpr_ids_domains.h */

/**< Enumeration for container processor domain for
  Invalid Domain */
#define APM_PROC_DOMAIN_ID_INVALID                0x0

/**< Enumeration for container procesor domain
  Modem DSP */
#define APM_PROC_DOMAIN_ID_MDSP                   0x1

/**< Enumeration for container processor domain for
  Audio DSP */
#define APM_PROC_DOMAIN_ID_ADSP                   0x2

/**< Enumeration for container procesor doman
  Sensors DSP */
#define APM_PROC_DOMAIN_ID_SDSP                   0x4

/**< Enumeration for container procesor doman
  Compute DSP */
#define APM_PROC_DOMAIN_ID_CDSP                   0x5

////////////////////////////////////////////////////////
/**< Enumeration for invalid PROP ID */
#define APM_PROP_ID_INVALID                       0x0

/**< Enumeration for don't care ID */
#define APM_PROP_ID_DONT_CARE AR_NON_GUID(0xFFFFFFFF)

#ifdef __H2XML__
// Enums required for module H2XML generation.
enum processors {
   PROC_DOMAIN_ADSP = APM_PROC_DOMAIN_ID_ADSP,  /**< @h2xmle_name {ADSP} */
   PROC_DOMAIN_MDSP = APM_PROC_DOMAIN_ID_MDSP,  /**< @h2xmle_name {MDSP} */
   PROC_DOMAIN_SDSP = APM_PROC_DOMAIN_ID_SDSP,  /**< @h2xmle_name {SDSP} */
   PROC_DOMAIN_CDSP = APM_PROC_DOMAIN_ID_CDSP   /**< @h2xmle_name {CDSP} */
};

enum containerCapOld {
   APM_CONTAINER_CAP_INVALID_OLD = APM_PROP_ID_INVALID,  /**< @h2xmle_name {INVALID} */
   APM_CONTAINER_CAP_PP = APM_CONTAINER_CAP_ID_PP,                 /**< @h2xmle_name {PP} */
   APM_CONTAINER_CAP_CD = APM_CONTAINER_CAP_ID_CD,                 /**< @h2xmle_name {CD} */
   APM_CONTAINER_CAP_EP = APM_CONTAINER_CAP_ID_EP,                 /**< @h2xmle_name {EP} */
   APM_CONTAINER_CAP_OLC = APM_CONTAINER_CAP_ID_OLC,           /**< @h2xmle_name {OLC} */
};

enum containerCap {
   APM_CONTAINER_TYPE_INVALID = APM_PROP_ID_INVALID,    /**< @h2xmle_name {Invalid} */
   APM_CONTAINER_TYPE_SC  = APM_CONTAINER_TYPE_ID_SC,   /**< @h2xmle_name {Specialized} @h2xmle_replace{APM_CONTAINER_CAP_ID_PP} */
   APM_CONTAINER_TYPE_GC  = APM_CONTAINER_TYPE_ID_GC,   /**< @h2xmle_name {Generic}     @h2xmle_replace{APM_CONTAINER_CAP_ID_CD, APM_CONTAINER_CAP_ID_EP}  */
   APM_CONTAINER_TYPE_OLC = APM_CONTAINER_TYPE_ID_OLC,  /**< @h2xmle_name {Offload}     @h2xmle_replace{APM_CONTAINER_CAP_ID_OLC}*/
};
#endif

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _APM_GRAPH_PROPERTIES_H_ */

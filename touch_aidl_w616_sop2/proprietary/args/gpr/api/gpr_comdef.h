#ifndef __GPR_COMDEF_H__
#define __GPR_COMDEF_H__

/**
 * @file  gpr_comdef.h
 * @brief This file contains GPR common defs
 *
 *  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/gpr.common/1.0/api/gpr_comdef.h#8 $
*/
// clang-format on


/******************************************************************************
 * Includes                                                                    *
 *****************************************************************************/
#include "ar_osal_types.h"


/****************************************************************************/
//GPR Core Export Macros

/****************************************************************************/

/** @addtogroup gpr_macros
@{ */
/** @name GPR Core Export Macros
@{ */

#ifdef __cplusplus
  #define GPR_INTERNAL extern "C"
    /**< Export macro that indicates a function is internal to the GPR. */
#else
  #define GPR_INTERNAL extern
    /**< Export macro that indicates a function is internal to the GPR. */
#endif /* __cplusplus */


#ifdef __cplusplus
  #define GPR_EXTERNAL extern "C"
    /**< Export macro that indicates an external function, which is intended
         for client use. */
#else
  #define GPR_EXTERNAL extern
    /**< Export macro that indicates an external function, which is intended
         for client use. */
#endif /* __cplusplus */

/** @} */ /* end_name APR Core Export Macros */
/** @} */ /* end_addtogroup gpr_macros */


/****************************************************************************/
/* Compiler-specific Macros*/
/****************************************************************************/

#ifndef inline
  #if defined( __ARMCC_VERSION )
    #define inline __inline
  #elif defined( _MSC_VER )
    //#define inline __inline
  #endif
#endif /* inline */

#endif /* __GPR_COMDEF_H__ */


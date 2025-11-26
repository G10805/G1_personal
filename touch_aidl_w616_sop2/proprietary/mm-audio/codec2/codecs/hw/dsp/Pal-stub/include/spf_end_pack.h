/**
 * \file spf_end_pack.h
 * \brief
 *           This file defines pack attributes for different compilers to be used to pack spf API data structures
 *
 * \copyright
 *    Copyright (c) 2019-2021 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/ar_utils/spf_end_pack.h#5 $
*/
// clang-format on


#if defined( __qdsp6__ )
/* No packing atrributes for Q6 compiler; all structs manually packed */
#elif defined( __GNUC__ )
  __attribute__((packed));
#elif defined( __arm__ )
#elif defined( _MSC_VER )
  #pragma pack( pop )
#elif defined (__H2XML__)
  __attribute__((packed))
#else
  #error "Unsupported compiler."
#endif /* __GNUC__ */

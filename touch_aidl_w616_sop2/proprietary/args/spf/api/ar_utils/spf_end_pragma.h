/**
 * \file spf_end_pragma.h
 * \brief 
 *  	 This file defines pragma to ignore zero length array warnings for different compilers
 * 
 * \copyright
 *    Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
// clang-format off
/*
$Header: //components/rel/avs.fwk/1.0/api/ar_utils/spf_end_pragma.h#3 $
*/
// clang-format on
 
#if defined( __qdsp6__ )
  #pragma GCC diagnostic pop
#elif defined( __GNUC__ )
#elif defined( __arm__ )
#elif defined( _MSC_VER )
  #pragma warning( pop )
#elif defined (__H2XML__)
#else
  #error "Unsupported compiler."
#endif /* __GNUC__ */


/**
@file gk_begin_pragma.h

@brief This file defines pragma to ignore zero length array warnings
for different compilers
*/
/*===========================================================================
NOTE: The @brief description and any detailed descriptions above do not appear
      in the PDF.

      The HexagonMM_mainpage.dox file contains all file/group descriptions that
      are in the output PDF generated using Doxygen and Latex. To edit or
      update any of the file/group text in the PDF, edit the
      HexagonMM_mainpage.dox file or contact Tech Pubs.

      The above description for this file is part of the adsp_api_pack group
      description in the HexagonMM_mainpage.dox file.
===========================================================================*/

/*===========================================================================
  Copyright (c) 2018-2019, 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*========================================================================
Edit History

when       who     what, where, why
--------   ---     -------------------------------------------------------
09/22/10    sd      (Tech Pubs) Edited Doxygen markup and comments.
10/15/18    np      Created file.

========================================================================== */

#if defined( __qdsp6__ )
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wzero-length-array"
#elif defined( __GNUC__ )
#elif defined( __arm__ )
#elif defined( _MSC_VER )
  #pragma warning( push )
  #pragma warning( disable:4200 )
#elif defined( __H2XML__)
#else
  #error "Unsupported compiler."
#endif

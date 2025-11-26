/**
@file gk_end_pack.h

@brief This file defines pack attributes for different compilers to be used to
pack gecko API data structures.

*//*===========================================================================
NOTE: The @brief description and any detailed descriptions above do not appear
      in the PDF.

      The HexagonMM_mainpage.dox file contains all file/group descriptions that
      are in the output PDF generated using Doxygen and Latex. To edit or
      update any of the file/group text in the PDF, edit the
      HexagonMM_mainpage.dox file or contact Tech Pubs.

      The above description for this file is part of the "adsp_api_pack" group
      description in the HexagonMM_mainpage.dox file.
===========================================================================*/

/*===========================================================================
  Copyright (c) 2018-2019, 2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

/*========================================================================
Edit History

$Header: //components/rel/avs.fwk/1.0/api/mmutils/gk_end_pack.h#3 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
09/22/10    sd      (Tech Pubs) Edited Doxygen markup and comments.
06/07/10   rkc      Created file.

========================================================================== */

#if defined( __qdsp6__ )
/* No packing atrributes for Q6 compiler; all structs manually packed */
#elif defined( __GNUC__ )
  __attribute__((packed))
#elif defined( __arm__ )
#elif defined( _MSC_VER )
  #pragma pack( pop )
#elif defined (__H2XML__)
  __attribute__((packed))
#else
  #error "Unsupported compiler."
#endif /* __GNUC__ */

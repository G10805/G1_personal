/*========================================================================

*//** @file hyp_video_test.c

@par DESCRIPTION:
Video hypervisor unit test

@par FILE SERVICES:

@par EXTERNALIZED FUNCTIONS:
See below.

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*//*========================================================================*/
/*========================================================================
                             Edit History

$Header: $

when       who     what, where, why
--------   ---     -------------------------------------------------------
06/28/17   aw      Unify and update all logs in hyp-video
05/08/17   sm      Update for new hyp-video architecture
04/19/17   sm      Add unit test for video hypervisor
========================================================================== */

#include <stdio.h>
#include "hyp_debug.h"
#include "hyp_video_test.h"

int main(void)
{
    HYP_VIDEO_TEST_TITLE ("Video Input hypervisor test");
    vinput_main();

    return 0;
}

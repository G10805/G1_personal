/*
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __PPD_H__
#define __PPD_H__

#include <stdio.h>

#ifdef POSTPROC_DEBUG
#define log_debug(x) printf("postproc_debug: %s\n", x)
#else
#define log_debug(x)
#endif

#define log_error(x) printf("postproc_error: %s\n", x)
#define log_info(x) printf("postproc_info: %s\n", x)

class postprocTester {
 public:
  bool sendDppsCommand(char* cmd);

  postprocTester() {}
  ~postprocTester() {}
};

#endif  // __PPD_H__

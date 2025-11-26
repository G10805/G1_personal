/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All rights reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <utils/RefBase.h>
#include <utils/Log.h>
#include <assert.h>

#include "acdb-loader-server.h"

#define LOG_TAG "acdb_loader_service"
#define BOOT_MARKER_MAX_LEN 50
#define BOOT_KPI_PATH      "/sys/kernel/boot_kpi/kpi_values"

using namespace audiocal;

static void acdbloader_place_marker(const char *name)
{
  int fd = 0;

  if ((NULL == name) || (strlen(name) > BOOT_MARKER_MAX_LEN)) {
    ALOGE("Invalid marker name");
    return;
  }
  fd = open(BOOT_KPI_PATH, O_WRONLY);
  if (fd > 0) {
    write(fd, name, strlen(name));
    close(fd);
  }
}

int main(int argc __attribute__((__unused__)), char **argv __attribute__((__unused__))) {
    int32_t err = 0;
    android::sp<android::IServiceManager> sm = android::defaultServiceManager();
    assert(sm != 0);
    err = sm->addService(android::String16(ACDB_LOADER_SERVICE), new acdb_loader_server(), false);
    ALOGD("ACDB Loader Server is alive Err = %d", err);
    if (!err)
      acdbloader_place_marker("M - ACDB Loader Server Ready");
    android::ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
    return err;
}


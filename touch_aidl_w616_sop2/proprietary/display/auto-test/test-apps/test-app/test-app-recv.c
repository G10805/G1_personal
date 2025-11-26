/*
 * Copyright (c) 2020, 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <drm/msm_drm.h>
#ifdef __MIN_ANDROID_VER_T__
#include <display/drm/sde_drm.h>
#else
#include <drm/sde_drm.h>
#endif
#include <fcntl.h>
#include "test-app-recv.h"

#define ADEBUG

#ifdef ADEBUG
#define DEBUG_PRINT		ALOGV
#define DEBUG_PRINT_INFO 	ALOGI
#define DEBUG_PRINT_ERROR	ALOGE
#else
#define DEBUG_PRINT		printf
#define DEBUG_PRINT_INFO	printf
#define DEBUG_PRINT_ERROR	printf

#endif

static const int kMaxStringLength = 1024;

int main(int argc ,char *argv[])
{
	int i = 0;
	int ret = 0;
	int fd = -1;
	/* default card */
	int card = 1;
	char card_path[256];
	struct pollfd poll_fd[1];
	int32_t size;
	uint32_t event;
	uint32_t event_type = 0;
	uint32_t crtc_id = 0;
	bool exit_threads = false;
	char event_data[kMaxStringLength] = {0};
	struct drm_msm_event_req req = {};
	drmModeRes *res = NULL;
	struct drm_msm_event_resp *event_resp = NULL;

	if (argc < 2) {
		DEBUG_PRINT_ERROR("card number is missing!\n");
		DEBUG_PRINT_ERROR("Usage:\n");
		DEBUG_PRINT_ERROR("\t%s <card_num>  <event_type> [crtc_id]\n", argv[0]);
		DEBUG_PRINT_ERROR("\t   card_num     DRI card to be used to listen the DRM events\n");
		DEBUG_PRINT_ERROR("\t   event_type   type for DRM event, or 0 for only the recovery success/failure events\n");
		DEBUG_PRINT_ERROR("\t   crtc_id      optinal CRTC id, or applicable for all CRTCs\n");
		return -EINVAL;
	}

	if (argc < 3) {
		DEBUG_PRINT_ERROR("Event type missing!\n");
		DEBUG_PRINT_ERROR("Usage:\n");
		DEBUG_PRINT_ERROR("\t%s <card_num>  <event_type> [crtc_id]\n", argv[0]);
		DEBUG_PRINT_ERROR("\t   card_num     DRI card to be used to listen the DRM events\n");
		DEBUG_PRINT_ERROR("\t   event_type   type for DRM event, or 0 for only the recovery success/failure events\n");
		DEBUG_PRINT_ERROR("\t   crtc_id      optinal CRTC id, or applicable for all CRTCs\n");
		return -EINVAL;
	}

	card = atoi(argv[1]);
	event_type = atoi(argv[2]);

	if (argc > 3)
		crtc_id = atoi(argv[3]);

	snprintf(card_path, sizeof(card_path), "/dev/dri/card%d", card);

	DEBUG_PRINT_INFO("Opening card %d\n", card);

	fd = open(card_path, O_RDWR, 0);

	if (fd < 0) {
		DEBUG_PRINT_ERROR("drmOpen failed with error %d", fd);
		return -EINVAL;
	}

	poll_fd[0].fd = fd;
	poll_fd[0].events =  POLLIN | POLLPRI | POLLERR;

	res = drmModeGetResources(fd);

	if (event_type) {
		if (crtc_id) {
			req.object_id = crtc_id;
			req.object_type = DRM_MODE_OBJECT_CRTC;
			req.event = event_type; //parsing the drm event type

			DEBUG_PRINT("Register Event: 0x%x for crtc %d\n",req.event, req.object_id);
			ret = drmIoctl(poll_fd[0].fd, DRM_IOCTL_MSM_REGISTER_EVENT, &req);
		} else {
			for (i = 0; i < res->count_crtcs; i++) {
				drmModeCrtcPtr crtc = drmModeGetCrtc(fd, res->crtcs[i]);
				req.object_id = crtc->crtc_id;
				req.object_type = DRM_MODE_OBJECT_CRTC;
				req.event = event_type; //parsing the drm event type

				DEBUG_PRINT("Register Event 0x%x for crtc %d\n",req.event, req.object_id);
				ret = drmIoctl(poll_fd[0].fd, DRM_IOCTL_MSM_REGISTER_EVENT, &req);
			}
		}
		if (ret) {
			DEBUG_PRINT_ERROR("Register event 0x%x failed\n", event_type);
			return -EINVAL;
		}
	}

	// Register recovery success/failure event
	for (i = 0; i < res->count_crtcs; i++) {
		drmModeCrtcPtr crtc = drmModeGetCrtc(fd, res->crtcs[i]);
		req.object_id = crtc->crtc_id;
		req.object_type = DRM_MODE_OBJECT_CRTC;
		req.event = DRM_EVENT_RECOVERY_SUCCESS;

		DEBUG_PRINT("Register DRM_EVENT_RECOVERY_SUCCESS Event for crtc %d\n", req.object_id);
		ret = drmIoctl(poll_fd[0].fd, DRM_IOCTL_MSM_REGISTER_EVENT, &req);

		req.event = DRM_EVENT_RECOVERY_FAILURE;
		DEBUG_PRINT("Register DRM_EVENT_RECOVERY_FAILURE Event for crtc %d\n", req.object_id);
		ret = drmIoctl(poll_fd[0].fd, DRM_IOCTL_MSM_REGISTER_EVENT, &req);
	}

	while (!exit_threads) {
		int error = poll(poll_fd, 1, -1);
		int32_t j = 0;

		if (error <= 0) {
			DEBUG_PRINT_ERROR("poll failed. error = %s", strerror(errno));
			continue;
		}

		size = (int32_t)pread(poll_fd[0].fd, event_data, kMaxStringLength, 0);
		if (size < 0) {
			DEBUG_PRINT_ERROR("Inside size less than 0 \n");
			return -EINVAL;
		}
		if (size > kMaxStringLength) {
			DEBUG_PRINT_ERROR("event size %d is greater than event buffer size %d\n", size, kMaxStringLength);
			return -EINVAL;
		}
		if (size < (int32_t)sizeof(*event_resp)) {
			DEBUG_PRINT_ERROR("size %d exp %zd\n", size, sizeof(*event_resp));
			return -EINVAL;
		}

		while (j < size) {
			event_resp = (struct drm_msm_event_resp *)&event_data[j];
			if (crtc_id && crtc_id != event_resp->info.object_id)
				continue;
			switch (event_resp->base.type) {
				case DRM_EVENT_SDE_UNDERRUN:
					DEBUG_PRINT_INFO("DRM_EVENT_SDE_UNDERRUN at CRTC %d\n", event_resp->info.object_id);
					break;
				case DRM_EVENT_SDE_SMMUFAULT:
					DEBUG_PRINT_INFO("DRM_EVENT_SDE_SMMUFAULT at CRTC %d\n", event_resp->info.object_id);
					break;
				case DRM_EVENT_SDE_VSYNC_MISS:
					DEBUG_PRINT_INFO("DRM_EVENT_SDE_VSYNC_MISS at CRTC %d\n", event_resp->info.object_id);
					break;
				case DRM_EVENT_BRIDGE_ERROR:
					DEBUG_PRINT_INFO("DRM_EVENT_BRIDGE_ERROR at CRTC %d\n", event_resp->info.object_id);
					break;
				case DRM_EVENT_RECOVERY_SUCCESS: {
					event = *(uint32_t *)event_resp->data;
					switch (event) {
					case DRM_EVENT_SDE_UNDERRUN:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_SUCCESS for UNDERRUN at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_SDE_SMMUFAULT:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_SUCCESS for SMMU FAULT at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_SDE_VSYNC_MISS:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_SUCCESS for VSYNC miss at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_BRIDGE_ERROR:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_SUCCESS for Bridge error at CRTC %d\n", event_resp->info.object_id);
						break;
					default:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_SUCCESS for 0x%x at CRTC %d\n", event, event_resp->info.object_id);
						break;
					}
					break;
				}
				case DRM_EVENT_RECOVERY_FAILURE: {
					event = *(uint32_t *)event_resp->data;
					switch (event) {
					case DRM_EVENT_SDE_UNDERRUN:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_FAILURE for UNDERRUN at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_SDE_SMMUFAULT:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_FAILURE for SMMU FAULT at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_SDE_VSYNC_MISS:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_FAILURE for VSYNC miss at CRTC %d\n", event_resp->info.object_id);
						break;
					case DRM_EVENT_BRIDGE_ERROR:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_FAILURE for Bridge error at CRTC %d\n", event_resp->info.object_id);
						break;
					default:
						DEBUG_PRINT_INFO("DRM_EVENT_RECOVERY_FAILURE for 0x%x at CRTC %d\n", event, event_resp->info.object_id);
						break;
					}
					break;
				}
				default: {
					DEBUG_PRINT_ERROR("invalid event 0x%x received\n", event_resp->base.type);
					break;
				}
			}
			j += event_resp->base.length;
		}
		//exit_threads = true;
	}

	close(poll_fd[0].fd);
	poll_fd[0].fd = -1;
	DEBUG_PRINT_INFO("exiting test-app \n");
	return 0;
}

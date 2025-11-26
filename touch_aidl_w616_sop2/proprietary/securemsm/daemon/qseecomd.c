/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                        HLOS QSEECom Daemon

GENERAL DESCRIPTION
	QSEECom Daemon starts the listeners that are used for interacting
with QSEECom.

EXTERNALIZED FUNCTIONS
	None

INITIALIZATION AND SEQUENCING REQUIREMENTS

Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries. 
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/capability.h>
#ifndef TARGET_ENABLE_SMCI_SYSLISTENER
#include <QSEEComAPI.h>
#endif
#ifdef ANDROID
#include <utils/Log.h>
#include <common_log.h>
#include <android/log.h>
#include <cutils/properties.h>
#endif
#include <sys/prctl.h>
#ifdef TARGET_ENABLE_SMCI_SYSLISTENER
#include "listenerFunc.h"
#endif

#ifdef ANDROID_CHANGES

#include <private/android_filesystem_config.h>
#else
#define AID_SYSTEM 1000
#endif

/** adb log */
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "QSEECOMD: "
#endif

#define LOG_NDDEBUG 0 //Define to enable LOGD

#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#define LOG_NDEBUG  0 //Define to enable LOGV
#endif

#ifndef TARGET_ENABLE_SMCI_SYSLISTENER
/* List of services id's */
#define FILE_SERVICE 10
#define TIME_SERVICE 11
#define RPMB_SERVICE 0x2000
#define GPT_SERVICE 0x2001
#define SSD_SERVICE  0x3000
#define SECURE_UI_SERVICE  0x4000
#define ML_SERVICE 0x5000 //Reserving the number here to avoid future conflicts. The listener will be started from the ML daemon.
#define MDTP_SERVICE 0x6000
#define GPFILE_SERVICE 0x7000
#define TLOC_SERVICE 0x8000 //TLOC Daemon listener service ID
#define QIS_SERVICE  0x9000 // QSEE Interrupt Service ID
#define SECCAM_SERVICE 0xA000   // Secure camera listener is implemented as a stand alone daemon
#define SP_SERVICE   0xB000   // Secure Processor (SP) listener service ID
#define OPS_SERVICE_ID       0xC000 // OPS listener
#define GPREQCANCEL_SERVICE  0x1100 // GP Request Cancellation Service ID

/* End of list */
#endif //!TARGET_ENABLE_SMCI_SYSLISTENER

#ifdef OE
#include <syslog.h>
#define LOGI(...) syslog(LOG_NOTICE, "INFO:" __VA_ARGS__)
#define LOGV(...) syslog(LOG_NOTICE,"VERB:" __VA_ARGS__)
#define LOGD(...) syslog(LOG_DEBUG,"DBG:" __VA_ARGS__)
#define LOGE(...) syslog(LOG_ERR,"ERR:" __VA_ARGS__)
#define LOGW(...) syslog(LOG_WARNING,"WRN:" __VA_ARGS__)
#endif

#ifndef TARGET_ENABLE_SMCI_SYSLISTENER

struct qseecom_listener_services {
	char *service_name;
	int  id;
	char *file_name;
	char *file_start;
	char *file_stop;
	void *qhandle;
	char *file_init;
};

static struct qseecom_listener_services listeners[] = {
#ifndef LISTENERS_VIRTUALIZED

#ifdef COMPILE_RPMB_LISTENER
	/* Change the value of RPMB_LISTENER_INDEX incase you need to change this order */
	{
		.service_name = "RPMB system services",
		.id = RPMB_SERVICE,
#ifdef OE
		.file_name = "librpmb.so.1",
#else
		.file_name = "librpmb.so",
#endif
		.file_start = "rpmb_start",
		.file_stop = "rpmb_close",
		.qhandle = NULL,
		.file_init = "rpmb_init_service",
	},
#endif //COMPILE_RPMB_LISTENER
#ifdef COMPILE_GPT_LISTENER
	/* Change the value of GPT_LISTENER_INDEX incase you need to change this order */
	{
		.service_name = "GPT system services",
		.id = GPT_SERVICE,
#ifdef OE
		.file_name = "libgpt.so.1",
		.file_init = NULL,
#else
		.file_name = "libgpt.so",
		.file_init = "gpt_init_service",
#endif
		.file_start = "gpt_start",
		.file_stop = "gpt_close",
		.qhandle = NULL,
	},
#endif //COMPILE_GPT_LISTENER
	/* Change the value of SSD_LISTENER_INDEX incase you need to change this order */
	{
		.service_name = "SSD system services",
		.id = SSD_SERVICE,

#ifdef OE
		.file_name = "libssd.so.1",
		.file_init = NULL,
#else
		.file_name = "libssd.so",
		.file_init = "ssd_init_service",
#endif
		.file_start = "ssd_start",
		.file_stop = "ssd_close",
		.qhandle = NULL,
	},
#ifdef COMPILE_OPS_LISTENER
	{
		.service_name = "output protection service listener",
		.id = OPS_SERVICE_ID,
		.file_name = "libops.so",
		.file_start = "ops_start",
		.file_stop = "ops_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
#endif //COMPILE_OPS_LISTENER

#endif //!LISTENERS_VIRTUALIZED
	{
		.service_name = "time services",
		.id = TIME_SERVICE,

#ifdef OE
		.file_name = "libdrmtime.so.1",
#else
		.file_name = "libdrmtime.so",
#endif
		.file_start = "atime_start",
		.file_stop = "atime_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
	{
		.service_name = "file system services",
		.id = FILE_SERVICE,
#ifdef OE
		.file_name = "libdrmfs.so.1",
#else
		.file_name = "libdrmfs.so",
#endif
		.file_start = "fs_start",
		.file_stop = "fs_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
	{
		.service_name = "gpfile system services",
		.id = GPFILE_SERVICE,
#ifdef OE
		.file_name = "libdrmfs.so.1",
#else
		.file_name = "libdrmfs.so",
#endif
		.file_start = "gpfs_start",
		.file_stop = "gpfs_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
#ifdef ENABLE_LV_SECURITY
	{
		.service_name = "gp request cancellation services",
		.id = GPREQCANCEL_SERVICE,
		.file_name = "libGPreqcancel.so.1",
		.file_start = "gp_reqcancel_start",
		.file_stop = "gp_reqcancel_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
#endif // ENABLE_LV_SECURITY
#ifndef OE
	{
		.service_name = "interrupt services",
		.id = QIS_SERVICE,
		.file_name = "libqisl.so",
		.file_start = "qisl_start",
		.file_stop = "", // 1. unknown prototype. 2. never called anyway.
		.qhandle = NULL,
		.file_init = NULL,
	},
	{
		.service_name = "gp request cancellation services",
		.id = GPREQCANCEL_SERVICE,
		.file_name = "libGPreqcancel.so",
		.file_start = "gp_reqcancel_start",
		.file_stop = "gp_reqcancel_close",
		.qhandle = NULL,
		.file_init = NULL,
	},
	{
		.service_name = "secure processor services", // SP-to-TZ interrupt
		.id         = SP_SERVICE,
		.file_name  = "libspl.so",
		.file_start = "spl_start",
		.file_stop  = "spl_stop",
		.qhandle    = NULL,
		.file_init  = NULL,
	},
#endif  //!OE
};
#endif //!TARGET_ENABLE_SMCI_SYSLISTENER

static int qseecomd_update_cap(void)
{
	int retvalue = 0;
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap[2];

	memset(&header, 0, sizeof(header));
	memset(&cap, 0, sizeof(cap));
	prctl(PR_SET_KEEPCAPS, 1, 0, 0, 0);

#ifndef ENABLE_LV_SECURITY
	retvalue =  setgid(AID_SYSTEM);
	if(retvalue != 0){
		LOGE("setgid error");
		return retvalue;
	}
	retvalue = setuid(AID_SYSTEM);
	if(retvalue != 0){
		LOGE("setuid error");
		return retvalue;
	}
#endif

	header.version = _LINUX_CAPABILITY_VERSION_3;
	header.pid = 0; /* self */

	cap[CAP_TO_INDEX(CAP_SYS_RAWIO)].permitted |= CAP_TO_MASK(CAP_SYS_RAWIO);
	cap[CAP_TO_INDEX(CAP_SYS_RAWIO)].effective |= CAP_TO_MASK(CAP_SYS_RAWIO);
	cap[CAP_TO_INDEX(CAP_SYS_RAWIO)].inheritable = 0;

	/* Enable block suspend for wakelock usage */
	cap[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].permitted |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
	cap[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].effective |= CAP_TO_MASK(CAP_BLOCK_SUSPEND);
	cap[CAP_TO_INDEX(CAP_BLOCK_SUSPEND)].inheritable = 0;

	retvalue = capset(&header, cap);
	if (retvalue < 0)
		LOGE("qseecomd setcap failed. retvalue=%d, errno=%d\n",
				retvalue, errno);

	return 0;
}

#ifndef TARGET_ENABLE_SMCI_SYSLISTENER
/* @brief This function calls all the starting functions of the listeners.
*/

/* TODO: If initialising a listener service fails,
 * there's no cleanup mechanism for already initialised service.
 * Need to add same.
 */
int init_listener_services(void)
{
	int ret = 0;
	int num_listeners = 0;
	int index = 0;

	/* Init the listener services */
	num_listeners = sizeof(listeners)/sizeof(struct qseecom_listener_services);
	for (index = 0; index < num_listeners; index++) {
		int (*service_initialize)(void) = NULL;

		if (!listeners[index].file_init)
			continue;

		listeners[index].qhandle  = dlopen(listeners[index].file_name, RTLD_NOW);
		if (!listeners[index].qhandle) {
			LOGE("Init dlopen(%s, RLTD_NOW) is failed.... %s\n",
				listeners[index].file_name, dlerror());
			return -1;
		}
		LOGD("Init dlopen(%s, RTLD_NOW) succeeds",
					listeners[index].file_name);

		service_initialize = (int (*)(void))dlsym(listeners[index].qhandle,
						listeners[index].file_init);
		if (!service_initialize) {
			LOGE("file_init handle (%s) not found for %s, error = %s",
				listeners[index].file_init,
				listeners[index].file_name, dlerror());
			return -1;
		}
		LOGD("Init::Init dlsym(g_FSHandle %s) succeeds",
					listeners[index].file_init);

		ret = (*service_initialize)();
		LOGD("Init %s ret = %d", listeners[index].file_init, ret);
		if (ret){
			LOGE("Init for dlsym(g_FSHandle, %s) fail, ret = %d",
				listeners[index].file_init, ret);
			return -1;
		}
		LOGD("%s: init (Listener ID = %d)",
			listeners[index].service_name, listeners[index].id );
	}
	return ret;
}

int start_listener_services(void)
{
	int ret = 0;
	int num_listeners = 0;
	int index = 0;

	/* Start the listener services */
	num_listeners = sizeof(listeners)/sizeof(struct qseecom_listener_services);
	LOGD("Total listener services to start = %d", num_listeners);
	for (index = 0; index < num_listeners; index++) {
		int (*service_start)(int) = NULL;

		listeners[index].qhandle  = dlopen(listeners[index].file_name, RTLD_NOW);
		if (!listeners[index].qhandle) {
			LOGE("Init dlopen(%s, RLTD_NOW) is failed.... %s\n",
				listeners[index].file_name, dlerror());
			continue;
		}

		/* Normal file_start for all services */
		service_start = (int (*)(int))dlsym(listeners[index].qhandle,
						listeners[index].file_start);
		if (!service_start) {
			LOGE("file_start handle (%s) not found for %s, error = %s",
				listeners[index].file_start,
				listeners[index].file_name, dlerror());
			return -1;
		}
		LOGD("Init::Init dlsym(g_FSHandle %s) succeeds",
				listeners[index].file_start);

		ret = (*service_start)(listeners[index].id);
		LOGD("Init %s ret = %d", listeners[index].file_start, ret);
		if (ret){
			LOGE("Init dlsym(g_FSHandle, %s) fail",
				listeners[index].file_start);
			return -1;
		}
		LOGD("%s: Started (Listener ID = %d)",
			listeners[index].service_name, listeners[index].id );
	}
	return ret;
}
#endif //!TARGET_ENABLE_SMCI_SYSLISTENER

static void qseecomd_load_cmnlib(void)
{
#if (defined ENABLE_CMNLIB_LOAD) && (!defined TARGET_ENABLE_SMCI_SYSLISTENER)
	struct QSEECom_handle *cmnlib_handle = NULL;
	int ret = 0;
	int rc = 0;

	ret = QSEECom_start_app(&cmnlib_handle, "/firmware/image/", "smplap32", 1024);
	rc = QSEECom_shutdown_app(&cmnlib_handle);

	if (ret || rc)
		LOGE("Failed to load cmnlib\n");
#endif
}

#ifdef HLOS_LOAD_KEYMASTER
static int qseecomd_load_keymaster(void)
{
	struct QSEECom_handle *keymaster_handle = NULL;
	return QSEECom_start_app(&keymaster_handle, "/vendor/firmware_mnt/image", "keymaste", 1024);
}
#else
static int qseecomd_load_keymaster(void)
{
	return 0;
}
#endif

#ifdef OE
static int property_set(const char *key, const char *value)
{
	return 0;
}
#endif

#ifndef TARGET_ENABLE_SMCI_SYSLISTENER
int main() {
	int ret = 0;
	int status = 0;
	uint32_t retry_count = 1200;
	int m_driver_fd = -1;
	char *driver_node = "/dev/qseecom";

	LOGD("qseecomd: Checking if the node %s can be accessed", driver_node);
	while (retry_count) {
	    if (access(driver_node, R_OK) == 0) {
	        LOGD("SUCCESS: While opening the device %s", driver_node);
	        break;
	    } else {
	        usleep(50 * 1000); //sleep for 50 ms
	    }
	    retry_count--;
	}
	if (retry_count == 0) {
	    LOGE("ERROR: While opening the device %s ", driver_node);
	    return -1;
	}

	LOGD("qseecom listener services process entry PPID = %d", getppid());

	signal(SIGPIPE, SIG_IGN);

	if(0 != init_listener_services()) {
		LOGE("ERROR: Failed to init listener services.");
		return -1;
	}

	if(0 != qseecomd_update_cap()){
		LOGE("qseecomd permission reset failed, not starting services");
		return -1;
	}

	/* listener services (qceecom daemon) continues */
	LOGD("qseecom listener service threads starting!!! ");
	if (0 != start_listener_services()) {
		LOGE("ERROR: Failed to start listener services.");
		return -1;
	}

	qseecomd_load_cmnlib();

	ret = qseecomd_load_keymaster();
	if (ret)
		LOGE("Loading keymaster app failied with ret = %d", ret);
	else
		property_set("vendor.sys.listeners.registered", "true");

	LOGD("QSEECOM DAEMON RUNNING ");
	pause();
	LOGD("qseecom listener services process exiting!!! ");
	return -1;
}
#else
int main() {
	uint32_t retry_count = 1200;
	const char *driver_node = "/dev/smcinvoke";
	int ret = 0;
	LOGD("qseecomd: Checking if the node %s can be accessed", driver_node);
	while (retry_count) {
		if (access(driver_node, R_OK) == 0) {
			LOGD("SUCCESS: While opening the device %s", driver_node);
			break;
		} else {
			usleep(50 * 1000); //sleep for 50 ms
		}
		retry_count--;
	}
	if (retry_count == 0) {
		LOGE("ERROR: While opening the device %s ", driver_node);
		return -1;
	}

	LOGD("smcinvoke listener services process entry PPID = %d", getppid());

	signal(SIGPIPE, SIG_IGN);

	if (0 != init_listeners()) {
		LOGE("ERROR: Failed to start listener services.");
		return -1;
	}

	if(0 != qseecomd_update_cap()){
		LOGE("qseecomd permission reset failed, not starting services");
		return -1;
	}

	ret = start_listeners_smci();
	if(ret) {
		LOGE("ERROR: listeners registration failed");
		return -1;
	}

	qseecomd_load_cmnlib();

	ret = qseecomd_load_keymaster();
	if (ret)
		LOGE("Loading keymaster app failied with ret = %d", ret);
	else
		property_set("vendor.sys.listeners.registered", "true");

	LOGE("QSEECOM DAEMON RUNNING ");
	pause();
	LOGD("smcinvoke listener services process exiting!!! ");
	return -1;
}
#endif //!TARGET_ENABLE_SMCI_SYSLISTENER

#ifdef __cplusplus
}
#endif

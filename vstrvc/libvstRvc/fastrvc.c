/*
* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/
#include <sys/prctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "module.h"
#include "drm_display.h"
#include "util_rvc.h"
#include "xml.h"
#include <cutils/properties.h>
#include "libxml/parser.h"

#ifdef __ANDROID__
#include <sys/system_properties.h>
#define HOME_PROP "vendor.rvc.home_ready"
#define LOGO_PROP "vendor.rvc.logo_done"
#define LOGO_START "0"
#define LOGO_DONE "1"

extern int __system_properties_init();

static void set_logo_status_prop(char *prop, char *value)
{
	if (__system_property_set(prop,value) != 0) {
		LOG_ERR("set logo statu property to %s error: %s\n", value, strerror(errno));
	} else {
		LOG_DBG("set logo statu property to %s success\n", value);
	}
}
#endif

static bool is_home_ready(void *xml_handle);
static void* camera_thread(void *data);
static void* logo_thread(void *data);
static void* rvc_detect_thread(void *data);
static void sig_handler(int sig);
static bool check_rvc();

static void help_cmd();

#define USLEEP_UNIT 100000 // 100ms

#ifdef FASTRVC_STATIC_LOGO_SUPPORT
#define STATIC_LOGO_COUNT 10
#else
#define STATIC_LOGO_COUNT 0
#endif

#ifdef FASTRVC_ANIMATED_LOGO_SUPPORT
#define ANIMATED_LOGO_COUNT 20
#else
#define ANIMATED_LOGO_COUNT 0
#endif

#define TOTAL_COUNT (STATIC_LOGO_COUNT + ANIMATED_LOGO_COUNT);

#define SET_CAMERA_THREAD_PRIORITY	0
#define SET_LOGO_THREAD_PRIORITY	0

struct cmdline_options {
	bool first_stage;
	bool logo;
};
static int parse_cmdline(int argc, char *argv[], struct cmdline_options *opt);
// global variable
static link_path *logo_link;
static link_path *camera_link;
static bool in_kernel_domain;

THREAD_HANDLE(camera_handle);
MUTEX_HANDLE(mutex_link);

int main(int argc, char *argv[])
{
	THREAD_HANDLE(rvc_handle);
	void *xml_handle;
	struct cmdline_options cmd_opt;

	memset(&cmd_opt, 0, sizeof(cmd_opt));

	LOG_INFO("fastrvc main start");
	if (parse_cmdline(argc, argv, &cmd_opt) != 0) {
		help_cmd();
		return -1;
	}

	xml_handle = xml_open(argv[1]);
	if (!xml_handle) {
		LOG_ERR("open setting file fail");
		return -1;
	}

	log_level_init(xml_handle);
	if(cmd_opt.first_stage)
		device_path_init();
	else
		device_path_init_snd_boot();

	logo_link = NULL;
	camera_link = NULL;

	/* signal handler */
	struct sigaction act;
	act.sa_handler = sig_handler;
	sigaction(SIGINT, &act, NULL);

	in_kernel_domain = cmd_opt.first_stage;
	/* init */
	MUTEX_INIT(mutex_link);
#ifdef FASTRVC_CAMERA_SUPPORT
	THREAD_CREATE(rvc_handle, rvc_detect_thread, xml_handle);
#endif

	// we run logo_thread in main thread
	if (cmd_opt.logo)
		logo_thread(xml_handle);

#ifdef FASTRVC_CAMERA_SUPPORT
	LOG_INFO("rvc_handle waited");
	THREAD_WAIT(rvc_handle);
#endif

	xml_close(xml_handle);
	LOG_INFO("fastrvc main exit");
#ifdef FASTRVC_CAMERA_SUPPORT
	if (cmd_opt.first_stage) {
		for(int i=2; i<argc; i++)
			argv[i] = NULL;
		LOG_INFO("re-execute fastrvc");
		execve(argv[0], argv, NULL);
		LOG_INFO("re-execute fastrvc error, errno is %s", strerror(errno));
	}
#endif
	return 0;
}

static void help_cmd()
{
	printf("initfastrvc <config.xml> [--logo | --first_stage]\n");
}

static int parse_cmdline(int argc, char *argv[], struct cmdline_options *opt)
{
	if (argc < 2)
		return -1;
	if (!strstr(argv[1], "xml"))
		return -1;

	for (int i=2; i<argc; i++) {
		if(strcmp(argv[i], "--first_stage") == 0) {
			opt->first_stage = true;
		} else if(strcmp(argv[i], "--logo") == 0) {
			opt->logo = true;
		}
	}
	return 0;
}

static void sig_handler(int sig)
{
	static int again = 0;
	if (again)
		exit(0);
	again = 1;

	switch (sig) {
		default:
		case SIGINT:
			MUTEX_LOCK(mutex_link);
			if (logo_link)
				link_stop_modules(logo_link);
			if (camera_link)
				link_stop_modules(camera_link);
			MUTEX_UNLOCK(mutex_link);
			exit(0);
			break;
	}
}

static void* logo_thread(void *data)
{
	link_path *static_logo;
	link_path *animated_logo;

	prctl(PR_SET_NAME, "logo_thread");

#if SET_LOGO_THREAD_PRIORITY
	pid_t pid = getpid();
	struct sched_param param;

	param.sched_priority = sched_get_priority_max(SCHED_RR);
	LOG_DBG("param.sched_priority is %d", param.sched_priority);

	sched_setscheduler(pid, SCHED_RR, &param);
	if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0)
		LOG_INFO("pthread_setschedparam fail");
#endif

	LOG_INFO("logo start");

	static_logo = NULL;
	animated_logo = NULL;

#ifdef FASTRVC_STATIC_LOGO_SUPPORT
	static_logo = xml_create_static(data);

	if (static_logo == NULL) {
		LOG_ERR("static logo create fail");
		goto end_logo;
	}
	if (0 != link_init_modules(static_logo)) {
		LOG_ERR("static logo init fail");
		goto end_logo;
	}
#endif

#ifdef FASTRVC_ANIMATED_LOGO_SUPPORT
	animated_logo = xml_create_animated(data);

	if (animated_logo == NULL) {
		LOG_ERR("animated logo create fail");
		goto end_logo;
	}
	if (0 != link_init_modules(animated_logo)) {
		LOG_ERR("animated logo init fail");
		goto end_logo;
	}
#endif

#ifdef FASTRVC_STATIC_LOGO_SUPPORT
	/* static logo */
	MUTEX_LOCK(mutex_link);
	logo_link = static_logo;
	MUTEX_UNLOCK(mutex_link);

	/* camera_thread will resume logo if in rvc */
	if (!check_rvc()) {
		if (0 != link_active_modules(logo_link))
			goto end_logo;
	}
	LOG_INFO("static logo active");
#ifdef FASTRVC_ANIMATED_LOGO_SUPPORT
	usleep(STATIC_LOGO_COUNT * USLEEP_UNIT);
	link_pause_modules(static_logo);
#endif
#endif

#ifdef FASTRVC_ANIMATED_LOGO_SUPPORT
	/* animated logo */
	MUTEX_LOCK(mutex_link);
	logo_link = animated_logo;
	MUTEX_UNLOCK(mutex_link);

	/* camera_thread will resume logo if in rvc */
	if (!check_rvc()) {
		if (0 != link_active_modules(logo_link))
			goto end_logo;
	}
	LOG_INFO("animated logo active");
#endif

	if (logo_link) {
		while(!is_home_ready(data))
			usleep(USLEEP_UNIT);
		link_pause_modules(logo_link);
	}

end_logo:
	MUTEX_LOCK(mutex_link);
	logo_link = NULL;
	MUTEX_UNLOCK(mutex_link);

	if (static_logo)
		link_stop_modules(static_logo);
	if (animated_logo)
		link_stop_modules(animated_logo);
#ifdef __ANDROID__
	set_logo_status_prop(LOGO_PROP, LOGO_DONE);
#endif
	LOG_INFO("logo done");
	return NULL;
}

#ifdef __ANDROID__
static bool is_home_ready(void *xml_handle)
{
	(void) xml_handle;
	char value[10] = {0};
	static int inited = 0;
	static bool home_ready = false;

	if (home_ready == true)
		return true;

	if (!inited) {
		if (__system_properties_init() != 0)
			return false;
		inited = 1;
	}

	if (__system_property_get(HOME_PROP, value) == 0)
		return false;

	LOG_DBG("read %s: %s", HOME_PROP, value);

	if (value[0] == '1') {
		home_ready = true;
		return true;
	}

	return false;
}
#else
static bool is_home_ready(void *xml_handle)
{
	DIR *dir;
	struct dirent *next;
	char name[100];
	char path[100];
	FILE *fd;
	int len;
	int pid;
	static bool ready = false;
	static char *home_daemon = NULL;

	if (ready == true)
		return true;

	if (!home_daemon) {
		/*
		*	<setting>
		*		<home_daemon value="name"/>
		*	</setting>
		*/
		home_daemon = xml_get_setting(xml_handle, "home_daemon");
		if (!home_daemon) {
			LOG_WARN("cannot get home_daemon from setting, we will not wait");
			ready = true;
			return ready;
		}
	}

	dir = opendir("/proc/");
	if (!dir) {
		LOG_ERR("opendir fail");
		return false;
	}

	while (1) {
		next = readdir(dir);
		if (!next)
			break;
		if (isdigit(next->d_name[0])) {
			pid = atoi(next->d_name);

			sprintf(path, "/proc/%d/comm", pid);
			fd = fopen(path, "r");
			if (!fd) {
				continue;
			}
			len = fread(name, 1, sizeof(name), fd);
			fclose(fd);
			name[len] = '\0';

			if (strstr(name, home_daemon)) {
				LOG_INFO("%s is ready", home_daemon);
				ready = true;
				home_ready = true;
				break;
			}
		}
	}
	closedir(dir);

	return ready;
}
#endif

static volatile bool rvc = false;
static volatile int block_check = 1;
static bool write_gpio_file(char * file_path, char * buf)
{
	int gpio_file;
	int count;
	gpio_file = open(file_path, O_WRONLY, S_IWUSR|S_IWGRP|S_IWOTH);
	if(gpio_file == -1) {
		LOG_ERR("open %s fail, return", file_path);
		close(gpio_file);
		return false;
	}
	count = write(gpio_file, buf, strlen(buf));
	if(count != strlen(buf)) {
		LOG_ERR("write %s to %s fail, return", buf, file_path);
		close(gpio_file);
		return false;
	}
	close(gpio_file);
	return true;
}
static void* rvc_detect_thread(void *data)
{
	FILE *fd = NULL;
	char file_path[256];
	char * tmp;
	int gpio_number;
	int gpio_level;
	int gpio_value;
	char buf[5];
	bool gpio_last_status;
	bool gpio_current_status;

	prctl(PR_SET_NAME, "rvc_detect_thread");
	LOG_INFO("rvc_detect_thread start");
#if 1
	tmp = xml_get_setting(data, "gpio_number");
	if(tmp) {
		gpio_number = atoi(tmp);
		LOG_DBG("gpio_number is %d", gpio_number);
		xmlFree(tmp);
		sprintf(file_path, "/sys/class/gpio/gpio%d", gpio_number);
		if (access(file_path, F_OK)) {
			sprintf (file_path, "/sys/class/gpio/export");
			sprintf(buf, "%d", gpio_number);
			if(!write_gpio_file(file_path, buf)) {
				block_check = 0;
				return 0;
			}
		}
		sprintf (file_path, "/sys/class/gpio/gpio%d/direction", gpio_number);
		sprintf (buf, "%s", "in");
		if(!write_gpio_file(file_path, buf)) {
			block_check = 0;
			return 0;
		}
		sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_number);
	} else {
		LOG_ERR("can't find gpio_number in config file, return");
		block_check = 0;
		return 0;
	}
#else
	// for debug and stress test
	sprintf(file_path, "/bin/rvc_flag");
#endif
	tmp = xml_get_setting(data, "gpio_level");
	if(tmp) {
		gpio_level = atoi(tmp);
		LOG_DBG("gpio_level is %d", gpio_level);
		xmlFree(tmp);
	} else {
		LOG_ERR("can't find gpio_level in config file, return");
		block_check = 0;
		return 0;
	}
	while (!fd) {
		fd = fopen(file_path, "r");
		if (!fd) {
			rvc = false;
			block_check = 0;
			usleep(50000);
		}
	}
	gpio_last_status = gpio_current_status = false;
	while (1) {
		rewind(fd);
		fflush(fd);
		fscanf(fd, "%d", &gpio_value);
		rvc = (gpio_value == gpio_level) ? true : false;
		gpio_current_status = rvc;
		block_check = 0;

		if (gpio_current_status != gpio_last_status) {
			if (rvc == true) {
				//create camera thread
				LOG_DBG("rvc key up--->down");
				THREAD_CREATE(camera_handle, camera_thread, data);
			} else {
				LOG_DBG("rvc key down--->up");
				THREAD_WAIT(camera_handle);
			}
		}

		/*
		* if running in kernel domain, it's time to switch to fastrvc domain
		*/
		if(in_kernel_domain && !rvc && is_home_ready(data))
			break;

		gpio_last_status = gpio_current_status;
		usleep(50000);
	}
	fclose(fd);
	LOG_INFO("rvc_detect_thread exit");
	return 0;
}

static bool check_rvc()
{
#ifdef FASTRVC_CAMERA_SUPPORT
	while (block_check);
	return rvc;
#else
	return false;
#endif
}


static void pause_logo()
{
	MUTEX_LOCK(mutex_link);
	if (logo_link != NULL)
		link_pause_modules(logo_link);
	MUTEX_UNLOCK(mutex_link);
}

static void resume_logo()
{
	MUTEX_LOCK(mutex_link);
	if (logo_link != NULL) {
		link_active_modules(logo_link);
		/* waiting for logo display*/
		usleep(50000);
	}
	MUTEX_UNLOCK(mutex_link);
}

static void* camera_thread(void *data)
{
	prctl(PR_SET_NAME, "camera_thread");
	LOG_INFO("camera_thread start");
#if SET_CAMERA_THREAD_PRIORITY
	pid_t pid = getpid();
	struct sched_param param;

	param.sched_priority = sched_get_priority_max(SCHED_RR);
	LOG_DBG("param.sched_priority is %d", param.sched_priority);

	sched_setscheduler(pid, SCHED_RR, &param);
	if (pthread_setschedparam(pthread_self(), SCHED_RR, &param) != 0)
		LOG_INFO("pthread_setschedparam fail");
#endif
	int null_pause = 0;
	link_path *link;

	if (camera_link == NULL) {
		link = xml_create_camera(data);
		if (link == NULL) {
			LOG_ERR("create link fail");
			return NULL;
		}

		MUTEX_LOCK(mutex_link);
		camera_link = link;
		MUTEX_UNLOCK(mutex_link);
	} else {
		link_restart_modules(camera_link);
	}

	if (0 != link_init_modules(camera_link)) {
		LOG_ERR("link_init_modules fail");
		return NULL;
	}

	if (check_rvc() == true) {
		pause_logo();

		if (0 != link_active_modules(camera_link)) {
			resume_logo();
			goto end_camera;
		}
		while (check_rvc())
			usleep(1000);

		/*
		* if home is not ready, need freeze camera frame
		* so that can switch to logo smoothly
		*/
		if (!is_home_ready(data) && null_pause == 0) {
			link_config_modules(camera_link, "null_pause", 1);
			null_pause = 1;
		}

		link_pause_modules(camera_link);
		resume_logo();
	}

end_camera:
	link_stop_modules(camera_link);
	//link_destroy(camera_link);

	//MUTEX_LOCK(mutex_link);
	//camera_link = NULL;
	//MUTEX_UNLOCK(mutex_link);

	return NULL;
}

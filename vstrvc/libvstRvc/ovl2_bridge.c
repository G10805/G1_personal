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
#include "util.h"
#include "xml.h"
#include <cutils/properties.h>
#include "libxml/parser.h"


#define SET_OVL2_THREAD_PRIORITY	1
extern int __system_properties_init();

static link_path *ovl_link;
static void* ovl_thread(void *data);

MUTEX_HANDLE(mutex_link);

static void sig_handler(int sig);
#define OVL2_BRIDGE_CTS_REFINE

#ifdef OVL2_BRIDGE_CTS_REFINE
#define HOME_PROP "vendor.rvc.home_ready"
static bool in_kernel_domain;
static volatile bool rvc = false;
static bool is_home_ready()
{
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
#endif

int main(int argc, char *argv[])
{
	THREAD_HANDLE(ovl_handle);
	void *xml_handle;

	LOG_INFO("ovl-bridge main start");

	device_path_init();

	xml_handle = xml_open(argv[1]);
	if (!xml_handle) {
		LOG_INFO("open setting file fail");
		return -1;
	}
    LOG_INFO("ovl-bridge main LINE:%d",__LINE__);
#ifdef OVL2_BRIDGE_CTS_REFINE
    if(argc >2 && strcmp(argv[2], "--first_stage") == 0) {
	    in_kernel_domain = true;
        LOG_INFO("ovl-bridge main start in kernel_domain");
    }else{
        in_kernel_domain = false;
        LOG_INFO("ovl-bridge main start in non kernel_domain");
    }
#endif
	log_level_init(xml_handle);

	/* signal handler */
	struct sigaction act;
	act.sa_handler = sig_handler;
	sigaction(SIGINT, &act, NULL);

	MUTEX_INIT(mutex_link);
	THREAD_CREATE(ovl_handle, ovl_thread, xml_handle);
	THREAD_WAIT(ovl_handle);

	xml_close(xml_handle);
#ifdef OVL2_BRIDGE_CTS_REFINE
	if (in_kernel_domain) {
		for(int i=2; i<argc; i++)
			argv[i] = NULL;
		LOG_INFO("re-execute ovl2bridge");
		execve(argv[0], argv, NULL);
		LOG_INFO("re-execute ovl2bridge error, errno is %s", strerror(errno));
	}
#endif
	LOG_INFO("ovl-bridge main exit");

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
			if (ovl_link)
				link_stop_modules(ovl_link);
			MUTEX_UNLOCK(mutex_link);
			exit(0);
			break;
	}
}


#if SET_OVL2_THREAD_PRIORITY
void set_highest_normal_thread_priority()
{
	// use maximum CFS priority
	pid_t pid = getpid();
	const struct sched_param sc_p = {
		.sched_priority = sched_get_priority_max(SCHED_OTHER)
	};

	if(0 == sched_setscheduler(pid, SCHED_OTHER, &sc_p)){
		LOG_INFO("set priority to SCHED_OTHER %d", sc_p.sched_priority);
	} else {
		LOG_ERR("sched_setscheduler failed with %d", errno);
	}

	// CFS also requires nice() value
	const int nice_val = -20;
	if(nice_val == nice(nice_val)) {
		LOG_INFO("set nice to %d", nice_val);
	} else {
		LOG_ERR("failed to set nice value");
	}
}
#endif
static void* ovl_thread(void *data)
{
	prctl(PR_SET_NAME, "ovl_thread");
	LOG_INFO("ovl_thread start");
	link_path *link = NULL;
	char * link_name = "ovl2_bridge";
#ifdef OVL2_BRIDGE_CTS_REFINE
	FILE *fd = NULL;
	char file_path[256]={0};
	char * tmp = NULL;
	int gpio_number = 0;
	int gpio_level = 0;
	int gpio_value = 0;
#endif
#if SET_OVL2_THREAD_PRIORITY
	set_highest_normal_thread_priority();
#endif
    int retry_count = 0;
#ifdef OVL2_BRIDGE_CTS_REFINE
    if (in_kernel_domain){
        tmp = xml_get_setting(data, "gpio_number");
        if(tmp) {
            gpio_number = atoi(tmp);
            LOG_DBG("ovl2_bridge gpio_number is %d", gpio_number);
            xmlFree(tmp);
            sprintf(file_path, "/sys/class/gpio/gpio%d", gpio_number);
            if (!access(file_path, F_OK)) {
                LOG_INFO("ovl2_bridge access gpio_number :%d failed",gpio_number);
            }else{
                sprintf(file_path, "/sys/class/gpio/gpio%d/value", gpio_number);
            }
        }
        tmp = xml_get_setting(data, "gpio_level");
        if(tmp) {
            gpio_level = atoi(tmp);
            LOG_DBG("ovl2_bridge gpio_level is %d", gpio_level);
            xmlFree(tmp);
        } else {
            LOG_ERR("ovl2_bridge can't find gpio_level in config file, return");
        }
	    while (!fd) {
            fd = fopen(file_path, "r");
            if (!fd) {
                LOG_ERR("ovl_thread open %s failed error:%s",file_path,strerror(errno));
                rvc = false;
                usleep(50000);
            }
	    }
    }
#endif

	link = xml_create_link(data, link_name);
	if (link == NULL) {
		LOG_ERR("create link fail");
		goto Err;
	}

	MUTEX_LOCK(mutex_link);
	ovl_link = link;
	MUTEX_UNLOCK(mutex_link);
	LOG_INFO("ovl_thread link init modules");
	if (0 != link_init_modules(link)) {
		LOG_ERR("link_init_modules fail");
		goto Err;
	}

	LOG_INFO("ovl_thread link active modules");
	if (0 != link_active_modules(link)) {
		LOG_ERR("link active modules fail.");
		goto Err;
	}
	while (1){
#ifdef OVL2_BRIDGE_CTS_REFINE
        if (in_kernel_domain && fd!= NULL){
            rewind(fd);
            fflush(fd);
            fscanf(fd, "%d", &gpio_value);
            rvc = (gpio_value == gpio_level) ? true : false;
        }
        if(in_kernel_domain && !rvc && is_home_ready()){
            if(fd != NULL){
                fclose(fd);
                fd = NULL;
            }
			break;
        }
        if (in_kernel_domain){
            usleep(50000);
        }else{
            usleep(1000000);
        }
#else
        usleep(1000000);
#endif
	}
Err:
    if(link != NULL){
		LOG_INFO("ovl_thread link pause modules");
		link_pause_modules(link);

		LOG_INFO("ovl_thread link stop modules");
		link_stop_modules(ovl_link);
    }
	if(fd != NULL){
		fclose(fd);
	}
    LOG_INFO("ovl_thread exit");
	return NULL;
}

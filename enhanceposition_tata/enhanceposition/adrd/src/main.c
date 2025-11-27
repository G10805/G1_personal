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
* MediaTek Inc. (C) 2017. All rights reserved.
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
//daemon for adr
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "module_daemon.h"
#include "config-parser.h"
#include "log.h"

extern int varconfig_main();
struct adr_config *config = NULL;
struct daemon_device *g_daemon_ctx = NULL;
static char *name;
int gnss_varient =0;
int main(int argc, char *argv[])
{
    while (!gnss_varient) {
	    gnss_varient = varconfig_main();
	    ALOGE("gnss_varient :: %d", gnss_varient);
    	    if (gnss_varient == 3)
    	    {
       		name = "/vendor/etc/adr/adr_alt.conf";
		break;
    	    }else if (gnss_varient == 4)
    	    {
      		 name = "/vendor/etc/adr/adr_punch.conf";
		 break;
    	    }else if (gnss_varient == 5)
	    {
	         name = "/vendor/etc/adr/adr_Tiago.conf";
	         break;
	    }else if (gnss_varient == 6)
	    {
	         name = "/vendor/etc/adr/adr_Tigor.conf";
	         break;
	    }
	    else
            {
                name = "/vendor/etc/adr/adr_alt.conf";
		break;
            }
    	  ALOGE("Varient Config file selected is %s : ", name);
	}
    struct daemon_device *dev;
    struct sigaction actions;
    //const char *name  = getenv("ADR_CONFIG_FILE");
    actions.sa_handler = daemon_sighlr;
    sigemptyset(&actions.sa_mask);
    actions.sa_flags = 0;
    sigaction(SIGUSR1, &actions, NULL);
    sigaction(SIGINT, &actions, NULL);
    sigaction(SIGPIPE, &actions, NULL);

    dev = (daemon_device*)zalloc(sizeof(daemon_device));
    if (!dev) {
        LOG_ERROR("Fail to create daemon_device");
        return -1;
    }
    g_daemon_ctx = dev;

    /*parse commandline options*/
    daemon_parse_commandline(dev, argc, argv);
    adr_modules_init(&dev->modInf);
    /*parse adr config file, if the name is NULL, use the default filename "/system/vendor/etc/adr/adr.conf"*/
    config = adr_config_parse(name);
    if(!config){
        LOG_ERROR("load config file %s failed, adrd exit",
                    name? name : ADR_DEFAULT_CONFIG_FILE);
        adr_modules_deinit(dev->modInf->private);
        free(dev);
        return 0;
    }

    daemon_driver_init(dev);
    daemon_driver_start(dev);

    pthread_mutex_lock(&dev->mutex);
    while(dev->running) {
        LOG_INFO("adrd main thread going....");
        pthread_cond_wait(&dev->cond, &dev->mutex);
        LOG_ERROR("some signal catched, go to exit!!!");
    }
    pthread_mutex_unlock(&dev->mutex);

    daemon_driver_stop(dev);
    daemon_driver_deinit(dev);
    if (config)
        adr_config_destroy(config);

    adr_modules_deinit(dev->modInf->private);
    free(dev);

    return 0;
}

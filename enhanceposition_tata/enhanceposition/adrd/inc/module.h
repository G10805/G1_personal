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

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

#include "adr_util.h"
#include "zalloc.h"
#include "log.h"

#define GNSS_POS_DATA_LEN 2      //GNSS  cache one frame for ADR
#define GNSS_TIM_DATA_LEN 5
#define ODOM_CAR_DATA_LEN 16     //ODOM car cache one frame for ADR

#define  BIT(n) 1 << n

/*name: ADR_DATA_TYPE
  *role: mark the data type, data, cmd, ack or other. the type also stand
  *       for a caps for the module, only the capability match the related the
  *       bit can receive the data of the type.
  *member:
  *    @DATA_ODOM: from odom to gnss & sensor

  *    @DATA_SENSOR_MEMS: from sensor to adr algo, sanity
  *    @DATA_SENSOR_BARO: from sensor to gnss

  *    @DATA_GNSS_TIME: from gnss to sensor
  *    @DATA_GNSS_NMEA: from gnss to daemon, debug
  *    @DATA_GNSS_PARSERD: from gnss to algo
  *    @DATA_GNSS_STORE: from gnss to data_store

  *    @DATA_ADR_ALGO: from algo to debug
  *    @DATA_ADR_ALGO_PVT: from algo to sanity & data_store
  *    @DATA_ADR_ALGO_MNLD: from algo to daemon 

  *    @DATA_DAEMON_NMEA: from daemon to gnss

  *    @CMD_ADR_PADR: cmd to adr(Header is $PADR)
  *    @CMD_ADR_PMTK: cmd to adr(Header is $PMTK) from powerGPS or algo
  *    @CMD_GNSS_ACK: ack from gnss
  *    @CMD_ALGO_ACK: ack from algo
  */
enum ADR_DATA_TYPE {
    DATA_ODOM = BIT(0),

    DATA_SENSOR_MEMS = BIT(1),
    DATA_SENSOR_BARO = BIT(2),
    DATA_SENSOR_STORE = BIT(3),

    DATA_GNSS_TIME = BIT(4),
    DATA_GNSS_NMEA = BIT(5),
    DATA_GNSS_PARSERD = BIT(6),
    DATA_GNSS_STORE = BIT(7),

    DATA_ADR_ALGO = BIT(8),
    DATA_ADR_ALGO_PVT = BIT(9),
    DATA_ADR_ALGO_MNLD = BIT(10),

    DATA_DAEMON_NMEA = BIT(11),

    CMD_ADR_PADR = BIT(12),
    CMD_ADR_PMTK = BIT(13),
    CMD_GNSS_ACK = BIT(14),
    CMD_ALGO_ACK = BIT(15),
};

/*name: module_box
  *role: box is container to transfer message between modules
  *member:
  *    @from: which module send the message(such as mtk_gnss)
  *    @index: the index of box, max is MODULE_MESSAGE_MAX_LEN
  *    @type: data type which defined in ADR_DATA_TYPE
  *    @data: the pointer going to transfer
  *    @priv: the second pointer going to transfer(not always used)
  *    @count: the log of the size of @data
  */
typedef struct module_box {
    char *from;
    int32_t index;
    int32_t type;
    void *data;
    void *priv;
    int32_t count;
} module_box;

/*name: module
  *role: module is a abstract of a device(like module_gnss)
  *member:
  *    @link: be used insert a list which managed by module fw
  *    @name: driver name
  *    @driver: function provided by module
  *    @driver_data: is module native struct device pointer
  *    @modules_fw: the second pointer going to transfer(not always used)
  */
typedef struct module {
    struct module_list link;
    char *name;
    struct module_driver *driver;
    void *driver_data;
    void *modules_fw;
} module;

typedef int (*module_init)(module *mod);
typedef void (*module_deinit)(module *mod);

/*name: modules_Inf
  *role: expose the interface to application to access the fw
  *member:
  *    @private: cache fw adr_module data struct
  *    @create: create a module for assign device
  *    @destroy: destroy a module for assign device
  *    @ops_forall: execute a ops for all modules
  */
typedef struct modules_Inf {
    void *private;
    module* (*create)(module_init init, void *data);
    void (*destroy)(module_deinit deinit, module *mod);
    void* (*ops_forall)(char *p, void *data);
}modules_Inf;

typedef void (*process_data)(module *mod, module_box *box);

/*name: module_driver
  *role: provide interface for module to registry to fw
  *member:
  *    @name: the module of name(such as mtk_gnss)
  *    @capability: the caps to process data type(such as DATA_ODOM), 
  *                        which defined in ADR_DATA_TYPE.
  *    @init: init function
  *    @start: start the module
  *    @download: function to process data from FW
  *    @stop: stop run of the module
  *    @deinit: recycle the resource of the module
  *    @registry: interface to FW to registry callback(upload)
  */
typedef struct module_driver {
    char *name;
    int32_t capability;
    int32_t (*init)(struct module *mod);
    void (*start)(struct module *mod);
    void (*start_done)(struct module *mod);
    void (*download)(struct module *mod, module_box *box);
    void (*stop)(struct module *mod);
    void (*deinit)(struct module *mod);
    void (*registry)(struct module *mod, process_data callback);
}module_driver;

static inline void module_set_name(module *mod, char *name)
{
    mod->name = strdup(name);
}

static inline char* module_get_name(module *mod)
{
    return mod->name;
}

static inline void module_register_driver(module *mod, module_driver *driver, void *driver_data)
{
    mod->driver = driver;
    mod->driver_data = driver_data;
}

static inline void* module_get_driver_data(module *mod)
{
    return mod->driver_data;
}

module_box *get_available_module_box(module *mod);
void release_module_box(module_box *box);

int module_gnss_init(module *mod);
int module_sensor_init(module *mod);
int module_odom_init(module *mod);
int module_adr_algo_init(module *mod);
int module_debug_init(module *mod);
int module_sanity_init(module *mod);
int module_store_init(module *mod);

void module_gnss_deinit(module *mod);
void module_sensor_deinit(module *mod);
void module_odom_deinit(module *mod);
void module_adr_algo_deinit(module *mod);
void module_debug_deinit(module *mod);
void module_sanity_deinit(module *mod);
void module_store_deinit(module *mod);

int32_t adr_modules_deinit(void *data);
int32_t adr_modules_init(modules_Inf **Inf);

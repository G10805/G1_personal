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

#include "module.h"

#define MODULE_MESSAGE_MAX_LEN 32

typedef struct adr_modules {
    struct module_list list;
    module_box *box[MODULE_MESSAGE_MAX_LEN];
    pthread_mutex_t box_mutex;
} adr_modules;

module_box *get_available_module_box(module *mod)
{
    int32_t cnt = 0;
    adr_modules *fw = (adr_modules *)(mod->modules_fw);
    module_box *target = NULL;

    pthread_mutex_lock(&fw->box_mutex);
    while(cnt < MODULE_MESSAGE_MAX_LEN) {
        if (!fw->box[cnt]) {
            fw->box[cnt] = (module_box *)zalloc(sizeof(module_box));
            fw->box[cnt]->from = mod->name;
            fw->box[cnt]->index = cnt;
            target = fw->box[cnt];
            break;
        } else {
            if (!fw->box[cnt]->from) {
                fw->box[cnt]->from = mod->name;
                target = fw->box[cnt];
                break;
            }
        }
        cnt += 1;
    }
    pthread_mutex_unlock(&fw->box_mutex);

    return target;
}

void release_module_box(module_box *box)
{
    if (!box)
        return;

    box->data = NULL;
    box->from = NULL;
}

void free_module_boxes(adr_modules *fw)
{
    int32_t cnt = 0;

    while(cnt < MODULE_MESSAGE_MAX_LEN) {
       if (fw->box[cnt]) {
           free(fw->box[cnt]);
           fw->box[cnt] = NULL;
       }
       cnt += 1;
   }
}

void modules_dispatch_data(module *mod, module_box *box)
{
    struct module *m;

    adr_modules *fw = (adr_modules *)(mod->modules_fw);
    module_list_for_each(m, &fw->list, link)
        if((m->driver->capability) & (box->type))
            if(m->driver->download) {
                LOG_DEBUG("dispatch data(from:%s to:%s)", box->from, m->driver->name);
                m->driver->download(m, box);
            }
}

void* modules_driver_deinit(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m;
    module_list_for_each(m, &fw->list, link)
        if(m->driver->deinit) {
            LOG_INFO("module:%s go to deinit", m->driver->name);
            m->driver->deinit(m);
        }
    return NULL;
}

void* modules_driver_init(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m, *next_m;
    module_list_for_each_safe(m, next_m, &fw->list, link) {
        if(m->driver->init) {
            LOG_INFO("module:%s go to init", m->driver->name);
            if(m->driver->init(m) !=0) {
                LOG_ERROR("module:%s init failed, removing it from the list", 
                    m->driver->name);
                module_list_remove(&m->link);
            }
        }
    }
    return NULL;
}

void* modules_driver_registry(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m, *next_m;
    module_list_for_each_safe(m, next_m, &fw->list, link) {
        if(m->driver->registry) {
            LOG_INFO("module:%s go to registry", m->driver->name);
            m->driver->registry(m, modules_dispatch_data);
        }
    }
    return NULL;
}

void* modules_driver_stop(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m;
    module_list_for_each(m, &fw->list, link)
        if(m->driver->stop) {
            LOG_INFO("module:%s go to stop", m->driver->name);
            m->driver->stop(m);
        }

    return NULL;
}

void* modules_driver_start(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m;
    module_list_for_each(m, &fw->list, link)
        if(m->driver->start) {
            LOG_INFO("module:%s go to start", m->driver->name);
            m->driver->start(m);
        }
    return NULL;
}

void* modules_driver_start_done(void *data)
{
    adr_modules *fw = (adr_modules *)data;
    struct module *m;
    module_list_for_each(m, &fw->list, link)
        if(m->driver->start_done) {
            LOG_INFO("module:%s go to start_done", m->driver->name);
            m->driver->start_done(m);
        }
    return NULL;
}

void* modules_ops_forall(char *p, void *data)
{
    if (!strcmp(p, "init"))
        modules_driver_init(data);
    else if (!strcmp(p, "registry"))
        modules_driver_registry(data);
    else if (!strcmp(p, "start"))
        modules_driver_start(data);
    else if (!strcmp(p, "start_done"))
        modules_driver_start_done(data);
    else if (!strcmp(p, "stop"))
        modules_driver_stop(data);
    else if (!strcmp(p, "deinit"))
        modules_driver_deinit(data);
    else {
        LOG_ERROR("module cmd:%s undefine?", p);
        abort();
    }
    return NULL;
}

void modules_destroy(module_deinit deinit, module *mod)
{
    if (mod){
        deinit(mod);
        /*because if driver init failed, the module has been removed from the framework list.
                     so we should confirm wether the module is still in the the framework list.*/
        if(mod->link.prev && mod->link.next)
            module_list_remove(&mod->link);
        free(mod);
    }
}

module* modules_create(module_init init, void *data)
{
    module *mod;
    adr_modules *fw = (adr_modules *)data;
    mod = (module*)zalloc(sizeof(*mod));
    if (!mod)
        return NULL;

    module_list_init(&mod->link);
    if ((init(mod) != 0) || (mod->driver == NULL)) {
        free(mod);
        return NULL;
    }
    mod->modules_fw = data;
    module_list_insert(&fw->list, &mod->link);

    return mod;
}

struct modules_Inf modInf = {
    .private = NULL,
    .create = modules_create,
    .destroy = modules_destroy,
    .ops_forall = modules_ops_forall,
};

int32_t adr_modules_deinit(void *data)
{
    adr_modules *fw = (adr_modules *)data;

    free_module_boxes(fw);
    pthread_mutex_destroy(&fw->box_mutex);
    free(fw);

    return 0;
}

int32_t adr_modules_init(modules_Inf **Inf)
{
    adr_modules *fw;
    fw = (adr_modules*)zalloc(sizeof(*fw));
    if (!fw)
        return -1;

    //open_rtc_dev();
    module_list_init(&fw->list);
    modInf.private = (void *)fw;
    *Inf = &modInf;
    pthread_mutex_init(&fw->box_mutex, NULL);

    return 0;
}


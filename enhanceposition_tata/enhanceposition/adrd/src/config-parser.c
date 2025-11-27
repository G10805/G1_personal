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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "config-parser.h"
#include "log.h"

static int
open_config_file(struct adr_config *c, const char *name)
{
    if (name) {
        /* Current working directory. */
        snprintf(c->path, sizeof c->path, "%s", name);
    } else {
        snprintf(c->path, sizeof c->path, "%s", ADR_DEFAULT_CONFIG_FILE);
    }
    return open(c->path, O_RDONLY | O_CLOEXEC);
}

static struct adr_config_entry *
config_section_get_entry(struct adr_config_section *section,
             const char *key)
{
    struct adr_config_entry *e;

    if (section == NULL)
        return NULL;
    module_list_for_each(e, &section->entry_list, link)
        if (strcmp(e->key, key) == 0)
            return e;

    return NULL;
}

struct adr_config_section *
adr_config_get_section(struct adr_config *config, const char *section,
              const char *key, const char *value)
{
    struct adr_config_section *s;
    struct adr_config_entry *e;

    if (config == NULL)
        return NULL;
    module_list_for_each(s, &config->section_list, link) {
        if (strcmp(s->name, section) != 0)
            continue;
        if (key == NULL)
            return s;
        e = config_section_get_entry(s, key);
        if (e && strcmp(e->value, value) == 0)
            return s;
    }

    return NULL;
}

int adr_config_section_get_int32_t(struct adr_config_section *section,
                  const char *key,
                  int32_t *value, int32_t default_value)
{
    struct adr_config_entry *entry;
    char *end;
    long int outl;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    outl = strtol(entry->value, &end, 0);

    *value = strtol(entry->value, &end, 0);

    if(outl > *value) {
        LOG_ERROR("SECTION INT key:%s , value overflow outl/value:%ld %d", key, outl, *value);
        *value = default_value;
        return -1;
    } else if(outl < *value) {
        *value = default_value;
        LOG_ERROR("SECTION INT key:%s , value underflow outl/value:%ld %d", key, outl, *value);
        return -1;
    }
    if (*end != '\0') {
        *value = default_value;
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int adr_config_section_get_int64_t(struct adr_config_section *section,
                  const char *key,
                  int64_t *value, int64_t default_value)
{
    struct adr_config_entry *entry;
    char *end;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    *value = strtoll(entry->value, &end, 0);
    if (*end != '\0') {
        *value = default_value;
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int adr_config_section_get_uint(struct adr_config_section *section,
                   const char *key,
                   uint32_t *value, uint32_t default_value)
{
    struct adr_config_entry *entry;
    char *end;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    *value = strtoul(entry->value, &end, 0);
    if (*end != '\0') {
        *value = default_value;
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int adr_config_section_get_double(struct adr_config_section *section,
                 const char *key,
                 double *value, double default_value)
{
    struct adr_config_entry *entry;
    char *end;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    *value = strtod(entry->value, &end);
    if (*end != '\0') {
        *value = default_value;
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int adr_config_section_get_char(struct adr_config_section *section,
                 const char *key,
                 char *value, const char default_value)
{
    struct adr_config_entry *entry;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    *value = *(entry->value);

    return 0;
}

int adr_config_section_get_string(struct adr_config_section *section,
                 const char *key,
                 char **value, const char *default_value)
{
    struct adr_config_entry *entry;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        if (default_value)
            *value = strdup(default_value);
        else
            *value = NULL;
        errno = ENOENT;
        return -1;
    }

    *value = strdup(entry->value);

    return 0;
}

int adr_config_section_get_bool(struct adr_config_section *section,
                   const char *key,
                   int *value, int default_value)
{
    struct adr_config_entry *entry;

    entry = config_section_get_entry(section, key);
    if (entry == NULL) {
        *value = default_value;
        errno = ENOENT;
        return -1;
    }

    if (strcmp(entry->value, "false") == 0)
        *value = 0;
    else if (strcmp(entry->value, "true") == 0)
        *value = 1;
    else {
        *value = default_value;
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static struct adr_config_section *
config_add_section(struct adr_config *config, const char *name)
{
    struct adr_config_section *section;

    section = malloc(sizeof *section);
    if (section == NULL)
        return NULL;

    section->name = strdup(name);
    if (section->name == NULL) {
        free(section);
        return NULL;
    }

    module_list_init(&section->entry_list);
    module_list_insert(config->section_list.prev, &section->link);

    return section;
}

static struct adr_config_entry *
section_add_entry(struct adr_config_section *section,
          const char *key, const char *value)
{
    struct adr_config_entry *entry;

    entry = malloc(sizeof *entry);
    if (entry == NULL)
        return NULL;

    entry->key = strdup(key);
    if (entry->key == NULL) {
        free(entry);
        return NULL;
    }

    entry->value = strdup(value);
    if (entry->value == NULL) {
        free(entry->key);
        free(entry);
        LOG_ERROR("the key: %s has not value", key);
        return NULL;
    }
    LOG_DEBUG("name:%s", entry->key);

    module_list_insert(section->entry_list.prev, &entry->link);

    return entry;
}

struct adr_config *adr_config_parse(const char *name)
{
    FILE *fp;
    char line[512], *p;
    struct stat filestat;
    struct adr_config *config;
    struct adr_config_section *section = NULL;
    int i, fd;

    config = malloc(sizeof *config);
    if (config == NULL)
        return NULL;

    module_list_init(&config->section_list);

    fd = open_config_file(config, name);
    if (fd == -1) {
        LOG_ERROR("open config file failed");
        free(config);
        return NULL;
    }

    if (fstat(fd, &filestat) < 0 ||
        !S_ISREG(filestat.st_mode)) {
        LOG_ERROR("it is not a regular file");
        close(fd);
        free(config);
        return NULL;
    }

    fp = fdopen(fd, "r");
    if (fp == NULL) {
        free(config);
        return NULL;
    }

    while (fgets(line, sizeof line, fp)) {
        switch (line[0]) {
        case ';':
        case '#':
        case '\n':
            continue;
        case '[':
            p = strchr(&line[1], ']');
            if (!p || p[1] != '\n') {
                fprintf(stderr, "malformed "
                    "section header: %s\n", line);
                fclose(fp);
                adr_config_destroy(config);
                return NULL;
            }
            p[0] = '\0';
            section = config_add_section(config, &line[1]);
            LOG_DEBUG("section:%s", &line[1]);
            continue;
        default:
            p = strchr(line, '=');
            if (!p || p == line || !section) {
                fprintf(stderr, "malformed "
                    "config line: %s\n", line);
                fclose(fp);
                adr_config_destroy(config);
                return NULL;
            }

            p[0] = '\0';
            p++;
            while (isspace(*p))
                p++;
            i = strlen(p);
            while (i > 0 && isspace(p[i - 1])) {
                p[i - 1] = '\0';
                i--;
            }
            section_add_entry(section, line, p);
            continue;
        }
    }

    fclose(fp);

    return config;
}

const char *
adr_config_get_full_path(struct adr_config *config)
{
    return config == NULL ? NULL : config->path;
}

int
adr_config_next_section(struct adr_config *config,
               struct adr_config_section **section,
               const char **name)
{
    if (config == NULL)
        return 0;

    if (*section == NULL)
        *section = container_of(config->section_list.next,
                    struct adr_config_section, link);
    else
        *section = container_of((*section)->link.next,
                    struct adr_config_section, link);

    if (&(*section)->link == &config->section_list)
        return 0;

    *name = (*section)->name;

    return 1;
}

void
adr_config_destroy(struct adr_config *config)
{
    struct adr_config_section *s, *next_s;
    struct adr_config_entry *e, *next_e;

    if (config == NULL)
        return;

    module_list_for_each_safe(s, next_s, &config->section_list, link) {
        module_list_for_each_safe(e, next_e, &s->entry_list, link) {
            free(e->key);
            free(e->value);
            free(e);
        }
        free(s->name);
        free(s);
    }

    free(config);
}

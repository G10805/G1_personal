/**
 * @file ccidbgr.c
 *
 * This file implements the cci application that can R/W cci devices.
 *
 * Copyright (c) 2013, 2016-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <media/cam_req_mgr.h>
#include <media/ais_sensor.h>

#define CCI_DBGR_VERSION "1.1"

#define MAXBUFSIZE 10

#define FILE_NAME_LEN   256

#define PRINT printf
#define PRINT_DBG(fmt...) \
    do { \
        if (verbose) { PRINT(fmt); } \
    }while(0)

typedef struct
{
    unsigned int    dev;
    unsigned int    master;
    char            in_file_name[FILE_NAME_LEN];
    char            out_file_name[FILE_NAME_LEN];
    FILE*           in_fp;
    FILE*           out_fp;
    int             fd_cci;
    uint32_t        slave_addr;
    uint32_t        i2c_freq;
    uint32_t        addr_type;
    uint32_t        data_type;
} cci_dbgr_ctx;

struct menu_controls {
    const char *name;
    void (*fcn)(cci_dbgr_ctx *dbgr_ctx);
};

#define IOCTL(n, r, p) ioctl_name(n, r, p, #r)

#define CCI_DEFAULT_FREQ_MODE 2 //SENSOR_I2C_MODE_CUSTOM

static int verbose = 0;

static int ioctl_name(int fd, unsigned long int request, void *parm, const char *name)
{
    int retval = ioctl(fd, request, parm);

    if (retval < 0)
        PRINT("%s: failed: %s\n", name, strerror(errno));
    else
        PRINT_DBG("%s: ok\n", name);

    return retval;
}

static void sensor_query_cap(int fd, struct cam_sensor_query_cap* sensor_cap)
{
    int rc = 0;
    struct cam_control cam_cmd;

    memset(&cam_cmd, 0x0, sizeof(cam_cmd));

    cam_cmd.op_code     = CAM_QUERY_CAP;
    cam_cmd.size        = sizeof(*sensor_cap);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved    = 0;
    cam_cmd.handle      = (uint64_t)sensor_cap;

    rc = IOCTL(fd, VIDIOC_CAM_CONTROL, &cam_cmd);

    return;
}

static void cci_power_up(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_sensor_cmd_i2c_pwrup power_up;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&power_up, 0x0, sizeof(power_up));

    cci_cmd.op_code     = AIS_SENSOR_I2C_POWER_UP;
    cci_cmd.size        = sizeof(power_up);
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.handle      = (uint64_t)&power_up;

    power_up.master = dbgr_ctx->master;
    power_up.retries = 1;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    printf("cci power up [%s]\n", (rc ? "FAIL" : "ok"));

    return;
}

static void cci_power_down(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_sensor_cmd_i2c_pwrdown power_down;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&power_down, 0x0, sizeof(power_down));

    cci_cmd.op_code     = AIS_SENSOR_I2C_POWER_DOWN;
    cci_cmd.size        = 0;
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.handle      = (uint64_t)&power_down;


    power_down.master = dbgr_ctx->master;
    power_down.retries = 1;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    printf("cci power down [%s]\n", (rc ? "FAIL" : "ok"));

    return;
}

static int cci_read(cci_dbgr_ctx   *dbgr_ctx,
                       uint32_t        reg_addr,
                       uint32_t       *reg_data)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_cci_cmd_t cci_cfg;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&cci_cfg, 0x0, sizeof(cci_cfg));

    cci_cmd.op_code     = AIS_SENSOR_I2C_READ;
    cci_cmd.size        = sizeof(cci_cfg);
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.reserved    = 0;
    cci_cmd.handle      = (uint64_t)&(cci_cfg);

    cci_cfg.cci_client.cci_i2c_master = dbgr_ctx->master;
    cci_cfg.cci_client.i2c_freq_mode = CCI_DEFAULT_FREQ_MODE;
    cci_cfg.cci_client.retries = 3;

    cci_cfg.cmd.i2c_read.reg_addr = reg_addr;
    cci_cfg.cmd.i2c_read.addr_type = dbgr_ctx->addr_type;
    cci_cfg.cmd.i2c_read.data_type = dbgr_ctx->data_type;
    cci_cfg.cmd.i2c_read.i2c_config.slave_addr = dbgr_ctx->slave_addr;
    cci_cfg.cmd.i2c_read.i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    *reg_data = cci_cfg.cmd.i2c_read.reg_data;

    return rc;
}

static void cci_read_menu(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    uint32_t reg_addr = 0, reg_data = 0;
    char buf[MAXBUFSIZE];

    printf("<addr>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        reg_addr = strtol(buf, NULL, 16);
    }

    rc = cci_read(dbgr_ctx, reg_addr, &reg_data);

    printf("\t 0x%x [%d %d] <- 0x%x [%s]\n", reg_addr,
            dbgr_ctx->addr_type, dbgr_ctx->data_type,
            reg_data, (rc ? "FAIL" : "ok"));
}

static int cci_read_burst(cci_dbgr_ctx   *dbgr_ctx,
                       uint32_t  reg_addr,
                       uint32_t  count,
                       uint32_t *data)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_cci_cmd_t cci_cfg;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&cci_cfg, 0x0, sizeof(cci_cfg));

    cci_cmd.op_code     = AIS_SENSOR_I2C_READ_BURST;
    cci_cmd.size        = sizeof(cci_cfg);
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.reserved    = 0;
    cci_cmd.handle      = (uint64_t)&(cci_cfg);

    cci_cfg.cci_client.cci_i2c_master = dbgr_ctx->master;
    cci_cfg.cci_client.i2c_freq_mode = CCI_DEFAULT_FREQ_MODE;
    cci_cfg.cci_client.retries = 0;

    cci_cfg.cmd.i2c_burst.reg_addr = reg_addr;
    cci_cfg.cmd.i2c_burst.count = count;
    cci_cfg.cmd.i2c_burst.data = data;
    cci_cfg.cmd.i2c_burst.addr_type = dbgr_ctx->addr_type;
    cci_cfg.cmd.i2c_burst.data_type = dbgr_ctx->data_type;
    cci_cfg.cmd.i2c_burst.i2c_config.slave_addr = dbgr_ctx->slave_addr;
    cci_cfg.cmd.i2c_burst.i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    return rc;
}

static void cci_read_burst_menu(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    uint32_t reg_addr = 0;
    char buf[MAXBUFSIZE];
    uint32_t *data, count = 0;

    printf("<starting reg_addr>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        reg_addr = strtol(buf, NULL, 16);
    }
    printf("<reg count to read>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        count = strtol(buf, NULL, 10);
        if ((count <=0) || (count > 65535))
            printf("Invalid read size specified \n");
    }

    data = (uint32_t *)calloc(count, sizeof(uint32_t));
    if (data == NULL)
    {
        printf("Mem alloc failed\n");
        return;
    }

    rc = cci_read_burst(dbgr_ctx, reg_addr, count, data);
    printf("\t  read burst [%d %d] [%s]\n",
            dbgr_ctx->addr_type, dbgr_ctx->data_type,
            (rc ? "FAIL" : "ok"));
    if (rc == 0)
    {
        for (int i = 0; i < count; i++)
            printf("\t  0x%x <- 0x%x \n", (reg_addr + i), data[i]);
    }
    free(data);
}

static int cci_write_burst(cci_dbgr_ctx  *dbgr_ctx,
                         uint32_t reg_addr,
                         uint32_t count,
                         unsigned int *write_reg)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_cci_cmd_t cci_cfg;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&cci_cfg, 0x0, sizeof(cci_cfg));

    cci_cmd.op_code     = AIS_SENSOR_I2C_WRITE_BURST;
    cci_cmd.size        = sizeof(cci_cfg);
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.reserved    = 0;
    cci_cmd.handle      = (uint64_t)&cci_cfg;

    cci_cfg.cci_client.cci_i2c_master = dbgr_ctx->master;
    cci_cfg.cci_client.i2c_freq_mode = CCI_DEFAULT_FREQ_MODE;
    cci_cfg.cci_client.retries = 0;

    cci_cfg.cmd.i2c_burst.data = write_reg;
    cci_cfg.cmd.i2c_burst.count = count;
    cci_cfg.cmd.i2c_burst.reg_addr = reg_addr;
    cci_cfg.cmd.i2c_burst.addr_type = dbgr_ctx->addr_type;
    cci_cfg.cmd.i2c_burst.data_type = dbgr_ctx->data_type;
    cci_cfg.cmd.i2c_burst.i2c_config.slave_addr = dbgr_ctx->slave_addr;
    cci_cfg.cmd.i2c_burst.i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    return rc;
}

static void cci_write_burst_menu(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0, idx = 0;
    char buf[MAXBUFSIZE];
    char file_name[FILE_NAME_LEN];
    char *p = NULL;
    uint32_t reg_addr = 0, count = 0;
    FILE* fp = NULL;
    unsigned int *write_data;

    printf("<starting reg_addr>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        reg_addr = strtol(buf, NULL, 16);
    }
    printf("<count to write>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        count = strtol(buf, NULL, 10);
        if ((count <=0) || (count > 65535))
            printf("Invalid write size specified\n");
    }
    printf("<Enter filename containing %d count of data to write>\n", count);

    if (fgets(file_name, sizeof(file_name), stdin) != NULL)
    {
        if (file_name[strlen(file_name) - 1] == '\n')
        {
            file_name[strlen(file_name) - 1] = '\0';
        }
        fp = fopen(file_name, "rb");
    }
    if (NULL == fp)
    {
        printf("open file %s failed\n", file_name);
        return;
    }

    write_data = (unsigned int *)calloc(count, sizeof(unsigned int));
    if (NULL == write_data)
    {
        printf("Failed to allocate mem of size %d\n", count);
        fclose(fp);
        return;
    }
    idx = 0;
    //read input file and fill reg_data
    while (!feof(fp))
    {
        if (fgets(buf, sizeof(buf), fp) == NULL)
            break;

        write_data[idx] = strtol(buf, &p, 0);
        idx++;

        if (idx == count)
            break;
    }
    fclose(fp);

    if (idx != count)
    {
        printf("Only %d count of data found in file\n", idx);
        free(write_data);
        return;
    }

    rc = cci_write_burst(dbgr_ctx, reg_addr, count, write_data);
    free(write_data);

    PRINT("\t burst write 0x%x [%d %d] [%s]\n", reg_addr,
            dbgr_ctx->addr_type, dbgr_ctx->data_type,
            (rc ? "FAIL" : "ok"));
}

static int cci_write(cci_dbgr_ctx  *dbgr_ctx,
                         uint32_t       reg_addr,
                         uint32_t       reg_data)
{
    int rc = 0;
    struct cam_control cci_cmd;
    struct ais_cci_cmd_t cci_cfg;

    memset(&cci_cmd, 0x0, sizeof(cci_cmd));
    memset(&cci_cfg, 0x0, sizeof(cci_cfg));

    cci_cmd.op_code     = AIS_SENSOR_I2C_WRITE;
    cci_cmd.size        = sizeof(cci_cfg);
    cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cci_cmd.reserved    = 0;
    cci_cmd.handle      = (uint64_t)&cci_cfg;

    cci_cfg.cci_client.cci_i2c_master = dbgr_ctx->master;
    cci_cfg.cci_client.i2c_freq_mode = CCI_DEFAULT_FREQ_MODE;
    cci_cfg.cci_client.retries = 3;

    cci_cfg.cmd.i2c_write.wr_payload.reg_addr = reg_addr;
    cci_cfg.cmd.i2c_write.wr_payload.reg_data = reg_data;
    cci_cfg.cmd.i2c_write.addr_type = dbgr_ctx->addr_type;
    cci_cfg.cmd.i2c_write.data_type = dbgr_ctx->data_type;
    cci_cfg.cmd.i2c_write.i2c_config.slave_addr = dbgr_ctx->slave_addr;
    cci_cfg.cmd.i2c_write.i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;

    rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

    return rc;
}

static void cci_write_menu(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    char buf[MAXBUFSIZE];
    uint32_t reg_addr = 0, reg_data = 0;

    printf("<addr>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        reg_addr = strtol(buf, NULL, 16);
    }
    printf("<data>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        reg_data = strtol(buf, NULL, 16);
    }

    rc = cci_write(dbgr_ctx, reg_addr, reg_data);

    PRINT("\t 0x%x [%d %d] <- 0x%x [%s]\n", reg_addr,
            dbgr_ctx->addr_type, dbgr_ctx->data_type,
            reg_data, (rc ? "FAIL" : "ok"));
}

 static void cci_write_sync_array_menu(cci_dbgr_ctx *dbgr_ctx)
   {
       int rc = 0;
       struct ais_cci_cmd_t cci_cfg;
       struct cam_control cci_cmd;
       uint32_t reg_addr = 0, reg_data = 0, tmp = 0, slave_addr = 0;
       uint16_t cid = 0;
       uint16_t csid = 0;
       uint32_t i;
       uint32_t wr_array_idx = 0;
       char buf[MAXBUFSIZE];

       memset(&cci_cmd, 0x0, sizeof(cci_cmd));
       memset(&cci_cfg, 0x0, sizeof(cci_cfg));

       cci_cmd.op_code     = AIS_SENSOR_I2C_WRITE_ARRAY_SYNC;
       cci_cmd.size        = sizeof(cci_cfg);
       cci_cmd.handle_type = CAM_HANDLE_USER_POINTER;
       cci_cmd.reserved    = 0;
       cci_cmd.handle      = (uint64_t)&cci_cfg;


       cci_cfg.cci_client.cci_i2c_master = dbgr_ctx->master;
       cci_cfg.cci_client.i2c_freq_mode = CCI_DEFAULT_FREQ_MODE;
       cci_cfg.cci_client.retries = 3;

       printf("<csid>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           csid = strtol(buf, NULL, 10);
       }
       printf("<cid>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           cid = strtol(buf, NULL, 10);
       }

       cci_cfg.cmd.wr_sync.sync_cfg.cid = cid;
       cci_cfg.cmd.wr_sync.sync_cfg.csid = csid;
       cci_cfg.cmd.wr_sync.sync_cfg.delay = 0;
       cci_cfg.cmd.wr_sync.sync_cfg.line = 0;


       printf("<#0 slave address>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           slave_addr = strtol(buf, NULL, 10);
       }

       printf("<#0 pair number>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           tmp = strtol(buf, NULL, 10);
       }

       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array =
        (struct ais_sensor_i2c_wr_payload *)malloc(tmp * sizeof(struct ais_sensor_i2c_wr_payload));
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].i2c_config.slave_addr = slave_addr;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].addr_type = 2;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].data_type = 1;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].count = tmp;
       for(i = 0; i < tmp; i++)
       {
           printf("<addr>\n");
           if (fgets(buf, sizeof(buf), stdin) != NULL)
           {
               reg_addr = strtol(buf, NULL, 16);
           }
           printf("<data>\n");
           if (fgets(buf, sizeof(buf), stdin) != NULL)
           {
               reg_data = strtol(buf, NULL, 16);
           }
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].reg_addr = reg_addr;
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].reg_data = reg_data;
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].delay = 0;
       }
       wr_array_idx++;

       printf("<#1 slave address>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           slave_addr = strtol(buf, NULL, 10);
       }

       printf("<#1 pair number>\n");
       if (fgets(buf, sizeof(buf), stdin) != NULL)
       {
           tmp = strtol(buf, NULL, 10);
       }

       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array =
       (struct ais_sensor_i2c_wr_payload *)malloc(tmp * sizeof(struct ais_sensor_i2c_wr_payload));
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].i2c_config.slave_addr = slave_addr;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].i2c_config.i2c_freq_mode = dbgr_ctx->i2c_freq;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].addr_type = 2;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].data_type = 1;
       cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].count = tmp;
       for(i = 0; i < tmp; i++)
       {
           printf("<addr>\n");
           if (fgets(buf, sizeof(buf), stdin) != NULL)
           {
               reg_addr = strtol(buf, NULL, 16);
           }
           printf("<data>\n");
           if (fgets(buf, sizeof(buf), stdin) != NULL)
           {
               reg_data = strtol(buf, NULL, 16);
           }
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].reg_addr = reg_addr;
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].reg_data = reg_data;
           cci_cfg.cmd.wr_sync.wr_cfg[wr_array_idx].wr_array[i].delay = 0;
       }
       wr_array_idx++;

       cci_cfg.cmd.wr_sync.num_wr_cfg = wr_array_idx;

       rc = IOCTL(dbgr_ctx->fd_cci, VIDIOC_CAM_CONTROL, &cci_cmd);

       PRINT("\t SYNC_ARRAY [%s]\n", (rc ? "FAIL" : "ok"));

       free(cci_cfg.cmd.wr_sync.wr_cfg[0].wr_array);
       free(cci_cfg.cmd.wr_sync.wr_cfg[1].wr_array);

    return;
}

static void cci_update_menu(cci_dbgr_ctx *dbgr_ctx)
{
    char buf[MAXBUFSIZE];
    PRINT("<slave addr>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        dbgr_ctx->slave_addr = strtol(buf, NULL, 16);
    }
    printf("<addr_type>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        dbgr_ctx->addr_type = strtol(buf, NULL, 10);
    }
    printf("<data type>\n");
    if (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        dbgr_ctx->data_type = strtol(buf, NULL, 10);
    }
}

static struct menu_controls cci_menu_ctrls[] = {
    {"cci_read", cci_read_menu},
    {"cci_write", cci_write_menu},
    {"cci_write_sync_array", cci_write_sync_array_menu},
    {"cci_read_burst", cci_read_burst_menu},
    {"cci_write_burst", cci_write_burst_menu},
    {"cci_update", cci_update_menu},
};

static void cci_menu(cci_dbgr_ctx *dbgr_ctx)
{
    int i = 0;
    char buf[MAXBUFSIZE];
    int max = sizeof(cci_menu_ctrls) / sizeof(cci_menu_ctrls[0]);

    while (1)
    {
        while (1) {
            PRINT("MAIN MENU\n");
            for (i = 0; i < max; i++) {
                PRINT("\t[ %d ] - %s\n", i, cci_menu_ctrls[i].name);
            }
            PRINT("\t[ %d ] - exit\n", i);

            do {
                printf("<Enter value>\n");
                if (fgets(buf, sizeof(buf), stdin) != NULL)
                {
                    i = strtol(buf, NULL, 10);
                }
            } while (i > max);

            if (i == max) {
                PRINT("exiting...\n");
                return;
            }
            else
                cci_menu_ctrls[i].fcn(dbgr_ctx);
        }
    }
}

static int find_cci(cci_dbgr_ctx *dbgr_ctx)
{
    struct media_device_info mdev_info;
    int found_cci = 0;
    int num_media_devices = 0;
    char dev_name[64];
    int rc = 0, dev_fd = 0;
    int num_entities = 1;

    while (!found_cci)
    {
        struct media_entity_desc entity;
        memset(&entity, 0, sizeof(entity));

        snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
        dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
        if (dev_fd < 0) {
            PRINT_DBG("Done discovering media devices\n");
            break;
        }

        num_media_devices++;
        rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
        if (rc < 0) {
            PRINT("Error: ioctl media_dev failed: %s\n", strerror(errno));
            close(dev_fd);
            return rc;
        }

        if (strncmp(mdev_info.model, CAM_REQ_MGR_VNODE_NAME, sizeof(mdev_info.model)) != 0) {
            close(dev_fd);
            continue;
        }

        while (!found_cci)
        {
            entity.id = num_entities;
            int rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
            if (rc < 0) {
                PRINT_DBG("Done enumerating media entities\n");
                close(dev_fd);
                rc = 0;
                break;
            }

            num_entities = entity.id | MEDIA_ENT_ID_FLAG_NEXT;

            PRINT_DBG("entity name %s type 0x%x group id %d\n",
                    entity.name, entity.type, entity.group_id);

            switch (entity.type)
            {
            case CAM_CCI_DEVICE_TYPE:
            {
                struct cam_sensor_query_cap sensor_cap;

                snprintf(dev_name, sizeof(dev_name), "/dev/%s", entity.name);
                dbgr_ctx->fd_cci = open(dev_name, O_RDWR | O_NONBLOCK);
                if (dbgr_ctx->fd_cci < 0) {
                    PRINT("FAILED TO OPEN SENSOR\n");
                    exit(0);
                }

                memset(&sensor_cap, 0x0, sizeof(sensor_cap));
                sensor_query_cap(dbgr_ctx->fd_cci, &sensor_cap);

                if (sensor_cap.slot_info == dbgr_ctx->dev)
                {
                    found_cci = 1;
                }
            }
                break;
            default:
                break;
            }
        }

        close(dev_fd);
    }

    return found_cci;
}

static void cci_proc_in_file(cci_dbgr_ctx *dbgr_ctx)
{
    int rc = 0;
    size_t len = 0 ;
    char* line = NULL;
    ssize_t nread;
    uint32_t addr, data, delay = 0;

    while ((nread = getline(&line, &len, dbgr_ctx->in_fp)) != -1)
    {
        if ('u' == line[0])
        {
            if (sscanf(line, "up: slaveaddr %x addrtype %d datatype %d",
                &dbgr_ctx->slave_addr, &dbgr_ctx->addr_type, &dbgr_ctx->data_type) != 3)
            {
                dbgr_ctx->slave_addr = dbgr_ctx->addr_type = dbgr_ctx->data_type = 0;
                PRINT("incorrect number of arguments to update command, %s\n",
                    line);
                return;
            }
        }
        if ('r' == line[0])
        {
            if (1 == sscanf(line, "rd: addr %x", &addr))
            {
                rc = cci_read(dbgr_ctx, addr, &data);
                if (NULL == dbgr_ctx->out_fp)
                {
                    printf("rd: addr 0x%x data 0x%x [%s]\n",
                        addr, data, (rc ? "fail" : "ok"));
                }
                else
                {
                    fprintf(dbgr_ctx->out_fp, "rd: addr 0x%x data 0x%x [%s]\n",
                        addr, data, (rc ? "fail" : "ok"));
                }
            }
            else
            {
                PRINT("incorrect number of arguments to read command, %s\n",
                    line);
                return;
            }
        }
        if ('w' == line[0])
        {
            if (3 == sscanf(line, "wr: addr %x data %x delay %d", &addr, &data, &delay))
            {
                rc = cci_write(dbgr_ctx, addr, data);
                if (NULL == dbgr_ctx->out_fp)
                {
                    printf("wr: addr 0x%x data 0x%x [%s]\n",
                        addr, data, (rc ? "fail" : "ok"));
                }
                else
                {
                    fprintf(dbgr_ctx->out_fp, "wr: addr 0x%x data 0x%x [%s]\n",
                        addr, data, (rc ? "fail" : "ok"));
                }
                if (delay)
                    usleep(delay);
            }
            else
            {
                PRINT("incorrect number of arguments to write command, %s\n",
                    line);
                return;
            }
        }
    }
}

static void cci_file(cci_dbgr_ctx *dbgr_ctx)
{
    if (dbgr_ctx->in_file_name[0] != 0)
    {
        dbgr_ctx->in_fp = fopen(dbgr_ctx->in_file_name, "r");
        if (NULL == dbgr_ctx->in_fp)
        {
            PRINT("failed to open in_file_name=%s, err=%d\n",
                dbgr_ctx->in_file_name, errno);
            goto exit;
        }
    }
    else
    {
        PRINT("no in_file_name\n");
        goto exit;
    }

    if (dbgr_ctx->out_file_name[0] != 0)
    {
        dbgr_ctx->out_fp = fopen(dbgr_ctx->out_file_name, "w");
        if (NULL == dbgr_ctx->out_fp)
        {
            PRINT("failed to open out_file_name=%s, err=%d\n",
                dbgr_ctx->out_file_name, errno);
            goto exit;
        }
    }

    cci_proc_in_file(dbgr_ctx);

exit:
    if (dbgr_ctx->in_fp)
        fclose(dbgr_ctx->in_fp);
    if (dbgr_ctx->out_fp)
        fclose(dbgr_ctx->out_fp);
}
void print_version(void)
{
    PRINT("CCI DBGR v" CCI_DBGR_VERSION "\n");
}

void print_help(void)
{
    print_version();
    PRINT("-verbose   verbose mode\n");
    PRINT("-infile=filename [-outfile=filename]\n");
    PRINT("  -infile  - file containing register sequences\n");
    PRINT("  -outfile - optional file where results are written (default is console)\n");
    PRINT("-dev=#     to specify CCI core\n");
    PRINT("-master=#  to specify CCI bus on the CCI core\n");
}

int main(int argc, char **argv)
{
    cci_dbgr_ctx dbgr_ctx;
    int found_cci = 0, i = 0, use_file= 0;

    memset(&dbgr_ctx, 0, sizeof(dbgr_ctx));
    dbgr_ctx.dev        = 0;
    dbgr_ctx.master     = 0;
    dbgr_ctx.fd_cci     = -1;
    dbgr_ctx.slave_addr = 0x0;
    dbgr_ctx.i2c_freq   = CCI_DEFAULT_FREQ_MODE;
    dbgr_ctx.addr_type  = 0x0;
    dbgr_ctx.data_type  = 0x0;

    print_version();

    for (i = 1; i < argc; i++)
    {
        const char *tok;

        if (!strncmp(argv[i], "-dev=", strlen("-dev=")))
        {
            tok = argv[i] + strlen("-dev=");
            dbgr_ctx.dev = atoi(tok);
        }
        else if (!strncmp(argv[i], "-master=", strlen("-master=")))
        {
            tok = argv[i] + strlen("-master=");
            dbgr_ctx.master = atoi(tok);
        }
        else if (!strncmp(argv[i], "-h", strlen("-h")))
        {
            print_help();
            return 0;
        }
        else if (!strncmp(argv[i], "-verbose", strlen("-verbose")))
        {
            verbose = 1;
        }
        else if (!strncmp(argv[i], "-infile=", strlen("-infile=")))
        {
            if (sscanf(argv[i], "-infile=%256s", dbgr_ctx.in_file_name) != 1)
            {
                print_help();
                return 0;
            }
            use_file = 1;
        }
        else if (!strncmp(argv[i], "-outfile=", strlen("-outfile=")))
        {
            if (sscanf(argv[i], "-outfile=%256s", dbgr_ctx.out_file_name) != 1)
            {
                print_help();
                return 0;
            }
        }
    }

    found_cci = find_cci(&dbgr_ctx);

    if (found_cci)
    {
        cci_power_up(&dbgr_ctx);

        if (use_file)
        {
            cci_file(&dbgr_ctx);
        }
        else
        {
            cci_menu(&dbgr_ctx);
        }

        cci_power_down(&dbgr_ctx);

        close(dbgr_ctx.fd_cci);
    }

    exit(0);
}

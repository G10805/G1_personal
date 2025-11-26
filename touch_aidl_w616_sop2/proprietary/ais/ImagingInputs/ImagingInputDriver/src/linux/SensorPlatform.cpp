/**
 * @file SensorPlatform.c
 *
 * @brief SensorPlatform Implementation
 *
 * Copyright (c) 2010-2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
                        INCLUDE FILES FOR MODULE
=========================================================================== */
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <linux/media.h>
#include <media/ais_sensor.h>

#include "CameraOSServices.h"
#include "CameraPlatformLinux.h"
#include "SensorPlatform.h"
#include "SensorDebug.h"

/* ---------------------------------------------------------------------------
 ** Macro Definitions
 ** ------------------------------------------------------------------------ */

/* ===========================================================================
                DEFINITIONS AND DECLARATIONS FOR MODULE
=========================================================================== */

/* --------------------------------------------------------------------------
 ** Constant / Define Declarations
 ** ----------------------------------------------------------------------- */
#define MULTII2C_QUEUE_SIZE 64
#define MAX_INTERRUPT_GPIO_PER_INPUT 3

/* CCI HW Mapping

IFE Full 0/2  mapped SET_CID_SYNC_TIMER_0
IFE Full 1/3  mapped SET_CID_SYNC_TIMER_1
IFE_Lite 0/2  mapped SET_CID_SYNC_TIMER_3
IFE_Lite 1/3  mapped SET_CID_SYNC_TIMER_2

|  IFE   | IFE0 | IFE1 | LITE0 | LITE1 | IFE2 | IFE3 | LITE2 | LITE3 |
----------------------------------------------------------------------
| MAP_ID |  0   |  1   |   3   |   2   |  4   |  5   |   7   |   6   |

*/
static uint16_t id_map_table[8] ={0,1,3,2,4,5,7,6};


/* --------------------------------------------------------------------------
 ** Type Declarations
 ** ----------------------------------------------------------------------- */
class SensorPlatformLinux;

typedef struct
{
    enum camera_gpio_type gpio_id;
    uint32 nIRQ;
    uint32 gpio_num;
    byte isUsed;
    char  tname[64];
    sensor_intr_callback_t pCbFcn;
    void *pData;
    SensorPlatformLinux *pCtxt;
} SensorPlatformInterruptType;

class SensorPlatformLinux : public SensorPlatform
{
public:
    SensorPlatformLinux(sensor_lib_t* p_sensor_lib)
    : m_hThread(NULL)
    , m_isAbortIntrPollThread(FALSE)
    {
        m_pSensorLib = p_sensor_lib;
    }

    CameraResult Init();
    void Deinit();

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorPowerUp -
     *
     * DESCRIPTION: Executes power up sequence defined in sensor library
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorPowerUp();

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorPowerDown -
     *
     * DESCRIPTION: Executes power down sequence defined in sensor library
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorPowerDown();

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorWriteI2cSettingArray -
     *
     * DESCRIPTION: Write I2C setting array
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorWriteI2cSettingArray(struct camera_i2c_reg_setting_array *settings);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveWriteI2cSetting_sync -
     *
     * DESCRIPTION: Write I2C setting array with sync mode
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveWriteI2cSetting_sync(struct camera_i2c_sync_cfg sync_cfg,
            struct camera_i2c_reg_setting_array_sync* reg_setting_array_sync, unsigned int size);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveWriteI2cSetting -
     *
     * DESCRIPTION: Slave Write I2C setting array
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveWriteI2cSetting(unsigned short slave_addr,
            struct camera_i2c_reg_setting *setting);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveI2cBulkWrite -
     *
     * DESCRIPTION: Slave I2C Bulk write
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveI2cBulkWrite(unsigned short slave_addr,
            struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds);
    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveI2cRead -
     *
     * DESCRIPTION: Slave read I2C reg
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveI2cRead(unsigned short slave_addr,
            struct camera_reg_settings_t *setting);

	/* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveI2cBulkRead -
     *
     * DESCRIPTION: Slave bulk read using I2C read burst mode
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveI2cBulkRead(unsigned short slave_addr,
            struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSlaveI2cBulkWriteRead -
     *
     * DESCRIPTION: Slave Custom Bulk Write followed by bulk read without I2C stop
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSlaveI2cBulkWriteRead (
            unsigned short slave_addr, struct camera_i2c_bulk_reg_setting *write_reg_setting,
            struct camera_i2c_bulk_reg_setting *read_reg_setting, boolean exec_pending_i2ccmds);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorWriteI2cSetting -
     *
     * DESCRIPTION: Write I2C setting
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorWriteI2cSetting(struct camera_i2c_reg_setting *setting);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorExecutePowerSetting -
     *
     * DESCRIPTION: Execute Sensor power config sequence
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorExecutePowerSetting(struct camera_power_setting *power_settings,
            unsigned short nSize, CameraPowerEventType mode);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSetupGpioInterrupt -
     *
     * DESCRIPTION: Setup gpio id as interrupt
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSetupGpioInterrupt(enum camera_gpio_type gpio_id,
            sensor_intr_callback_t cb, void *data);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorSetupCciFrameSync -
     *
     * DESCRIPTION: Setup CCI frame sync
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorSetupCciFrameSync(unsigned int fsync_freq);

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorTriggerCciFrameSync -
     *
     * DESCRIPTION: Trigger CCI frame sync
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorTriggerCciFrameSync();

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorPowerResume -
     *
     * DESCRIPTION: Executes power resume sequence defined in sensor library
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorPowerResume();

    /* ---------------------------------------------------------------------------
     * FUNCTION    - SensorPowerSuspend -
     *
     * DESCRIPTION: Executes power suspend sequence defined in sensor library
     * ------------------------------------------------------------------------ */
    virtual CameraResult SensorPowerSuspend();

private:
    void SensorPlatformIntrProcessEvent();
    static int SensorPlatformIntrPollThread(void* arg);

    sensor_lib_t* m_pSensorLib;

    int m_sensorFd;
    int m_cciFd;
    CameraSensorIndex m_eCameraInterface;

    // i2c/cci
    CameraSensorCciDeviceType m_cciDeviceIndex;
    CameraSensorI2CMasterType m_I2CMasterIndex;
    enum camera_i2c_freq_mode m_I2CBusFrequency;
    byte m_I2CSlaveAddr;
    byte m_I2CRegisterSize;
    byte m_I2CDataSize;

    // interrupt
    SensorPlatformInterruptType m_interrupts[MAX_INTERRUPT_GPIO_PER_INPUT];

    /*power*/
    struct ais_sensor_probe_cmd *m_probeCmd;

    CameraThread m_hThread;
    int m_pipeFd[2];
    int m_isAbortIntrPollThread;
};

/*===========================================================================
 * FUNCTION - LOG_IOCTL -
 *
 * DESCRIPTION: Wrapper for logging and to trace ioctl calls.
 * ------------------------------------------------------------------------ */
#define ATRACE_BEGIN_SNPRINTF(...)
#define ATRACE_END(...)
static int LOG_IOCTL(int d, int request, void* par1, const char* trace_func)
{
    int ret;
    ATRACE_BEGIN_SNPRINTF(35,"Camera:sensorIoctl %s", trace_func);
    ret = ioctl(d, request, par1);
    ATRACE_END();
    return ret;
}


static uint8_t sensor_sdk_util_get_kernel_i2c_type(
    enum camera_i2c_reg_addr_type addr_type)
{
    switch (addr_type) {
    case CAMERA_I2C_BYTE_ADDR:
        return 1;
    case CAMERA_I2C_WORD_ADDR:
        return 2;
    case CAMERA_I2C_3B_ADDR:
        return 3;
    default:
        AIS_LOG(SENSOR_PLATFORM, HIGH, "Invalid addr_type = %d", addr_type);
        return 0;
    }
}

static uint8_t sensor_sdk_util_get_kernel_i2c_type(
    enum camera_i2c_data_type data_type)
{
    switch (data_type) {
    case CAMERA_I2C_BYTE_DATA:
        return 1;
    case CAMERA_I2C_WORD_DATA:
        return 2;
    case CAMERA_I2C_DWORD_DATA:
        return 4;
    default:
        AIS_LOG(SENSOR_PLATFORM, HIGH, "Invalid data_type = %d", data_type);
        return 0;
    }
}

static uint16_t sensor_sdk_util_get_kernel_power_seq_type(
    enum camera_power_seq_type seq_type, long config_val)
{
    switch (seq_type) {
    case CAMERA_POW_SEQ_CLK:
        return 0; //MCLK
        break;
    case CAMERA_POW_SEQ_GPIO:
    case CAMERA_POW_SEQ_VREG:
        return config_val;
        break;
    case CAMERA_POW_SEQ_I2C:
    default:
        AIS_LOG(SENSOR_PLATFORM, ERROR, "Invalid seq_type = %d, config_val = %d", seq_type, config_val);
        return -1;
    }
}

static void translate_sensor_reg_setting(
    struct ais_sensor_cmd_i2c_wr_array *setting_k,
    struct camera_i2c_reg_setting *setting_u)
{
    setting_k->count = setting_u->size;
    setting_k->wr_array =
        (struct ais_sensor_i2c_wr_payload *)setting_u->reg_array;
    setting_k->addr_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->addr_type);
    setting_k->data_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->data_type);
    //setting_k->delay = setting_u->delay;
}

static void translate_sensor_reg_setting_array(
    struct ais_sensor_cmd_i2c_wr_array *setting_k,
    struct camera_i2c_reg_setting_array *setting_u)
{
    setting_k->count = setting_u->size;
    setting_k->wr_array =
        (struct ais_sensor_i2c_wr_payload *)setting_u->reg_array;
    setting_k->addr_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->addr_type);
    setting_k->data_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->data_type);
    //setting_k->delay = setting_u->delay;
}

static void translate_sensor_read_reg(
    struct ais_sensor_cmd_i2c_read *setting_k,
    struct camera_reg_settings_t *setting_u)
{
    setting_k->reg_addr = setting_u->reg_addr;
    setting_k->addr_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->addr_type);
    setting_k->data_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->data_type);
}

static void translate_sensor_burst_setting(
    struct ais_sensor_cmd_i2c_burst *setting_k,
    struct camera_i2c_bulk_reg_setting *setting_u)
{
    setting_k->count = setting_u->size;
    setting_k->addr_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->addr_type);
    setting_k->data_type =
        sensor_sdk_util_get_kernel_i2c_type(setting_u->data_type);
    setting_k->reg_addr = setting_u->reg_addr;
    setting_k->data = (uint32_t *)setting_u->reg_data;
}

static uint8_t sensor_sdk_util_get_i2c_freq_mode(
    enum camera_i2c_freq_mode i2c_freq_mode)
{
    switch (i2c_freq_mode){
    case SENSOR_I2C_MODE_STANDARD:
        return 0;
    case SENSOR_I2C_MODE_FAST:
        return 1;
    case SENSOR_I2C_MODE_CUSTOM:
        return 2;
    case SENSOR_I2C_MODE_FAST_PLUS:
        return 3;
    default:
        AIS_LOG(SENSOR_PLATFORM, ERROR, "Invalid i2c_freq_mode = %d", i2c_freq_mode);
        return 4;
    }
}

static void translate_sensor_power_up_settings(
    struct ais_sensor_power_config *power_cmd,
    struct camera_power_setting *power_setting_array_info,
    unsigned short size)
{
    struct ais_power_settings *power_up_setting_k = power_cmd->power_up_setting;
    struct camera_power_setting* power_up_array = power_setting_array_info;

    power_cmd->size_up = size;

    if (power_cmd->size_up > AIS_MAX_POWER_SEQ)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "power up seq too long (%d). Will truncate to %d",
            power_cmd->size_up, AIS_MAX_POWER_SEQ);
        power_cmd->size_up = AIS_MAX_POWER_SEQ;
    }

    for (uint16_t i = 0; i < power_cmd->size_up; i++) {
        power_up_setting_k[i].power_seq_type =
            sensor_sdk_util_get_kernel_power_seq_type(
                power_up_array[i].seq_type,
                power_up_array[i].seq_val);
        power_up_setting_k[i].config_val_low =
            power_up_array[i].config_val;
        power_up_setting_k[i].delay =
            power_up_array[i].delay;
    }
}

static void translate_sensor_power_down_settings(
    struct ais_sensor_power_config *power_cmd,
    struct camera_power_setting *power_setting_array_info,
    unsigned short size)
{
    struct ais_power_settings *power_down_setting_k = power_cmd->power_down_setting;
    struct camera_power_setting* power_down_array = power_setting_array_info;

    power_cmd->size_down = size;

    if (power_cmd->size_down > AIS_MAX_POWER_SEQ)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "power down seq too long (%d). Will truncate to %d",
            power_cmd->size_down, AIS_MAX_POWER_SEQ);
        power_cmd->size_down = AIS_MAX_POWER_SEQ;
    }

    for (uint16_t i = 0; i < power_cmd->size_down; i++) {
        power_down_setting_k[i].power_seq_type =
            sensor_sdk_util_get_kernel_power_seq_type(
                power_down_array[i].seq_type,
                power_down_array[i].seq_val);
        power_down_setting_k[i].config_val_low =
            power_down_array[i].config_val;
        power_down_setting_k[i].delay =
            power_down_array[i].delay;
    }
}

static void translate_sensor_slave_info(
    struct ais_sensor_probe_cmd *probe_cmd,
    struct camera_sensor_slave_info *slave_info_u)
{
    ais_sensor_power_config* power_cmd = &probe_cmd->power_config;
    probe_cmd->i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        slave_info_u->i2c_freq_mode);
    probe_cmd->i2c_config.slave_addr = slave_info_u->slave_addr;

    translate_sensor_power_up_settings(power_cmd, slave_info_u->power_setting_array.power_up_setting_a,
        slave_info_u->power_setting_array.size_up);
    translate_sensor_power_down_settings(power_cmd, slave_info_u->power_setting_array.power_down_setting_a,
        slave_info_u->power_setting_array.size_down);
}

void SensorPlatformLinux::SensorPlatformIntrProcessEvent()
{
    int rc;
    int i = 0;
    struct v4l2_event v4l2_event = {};
    struct ais_sensor_event_data *event_data = NULL;
    SensorPlatformInterruptType *pPlatformIntr;
    int idx;

    rc = ioctl(m_sensorFd, VIDIOC_DQEVENT, &v4l2_event);
    if (rc >= 0)
    {
        event_data = (struct ais_sensor_event_data *)v4l2_event.u.data;
        switch(v4l2_event.type) {
        case AIS_SENSOR_EVENT_TYPE:
        {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "Sensor platform event id=0x%x", v4l2_event.id);
            for (idx =0; idx < MAX_INTERRUPT_GPIO_PER_INPUT; ++idx)
            {
                pPlatformIntr = &m_interrupts[idx];
                //Read the register inside this callback
                if (pPlatformIntr->isUsed && pPlatformIntr->gpio_num == v4l2_event.id)
                {
                    pPlatformIntr->pCbFcn(pPlatformIntr->pData);
                    break;
                }
            }

            if (idx == MAX_INTERRUPT_GPIO_PER_INPUT)
            {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "event id %d can't be handled", v4l2_event.id);
            }
            break;
        }

        default:
            AIS_LOG(SENSOR_PLATFORM, ERROR, "Sensor platform %d - Unknown event %d", m_eCameraInterface, v4l2_event.type);
            break;
        }

    }
    else
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "Sensor platform VIDIOC_DQEVENT failed");
    }
}

/* ----------------------------------------------------------------------------
 *    FUNCTION        SensorPlatformIntrPollThread
 *    DESCRIPTION     Interrupt polling thread
 *    DEPENDENCIES
 *    PARAMETERS
 *
 *    RETURN VALUE
 *    SIDE EFFECTS
 * ----------------------------------------------------------------------------
 */
int SensorPlatformLinux::SensorPlatformIntrPollThread(void* arg)
{
    int rc;
    struct pollfd pollfds[2];
    int pollNumFds;
    SensorPlatformLinux* pCtxt = (SensorPlatformLinux*)arg;

    if (pCtxt == NULL)
    {
        //TODO: propagate error state
        AIS_LOG(SENSOR_PLATFORM, ERROR,  "sensor event thread has NULL context");
        return -1;
    }

    rc = pipe(pCtxt->m_pipeFd);
    if (rc < 0)
    {
       AIS_LOG(SENSOR_PLATFORM, ERROR, "Failed to create sensor pipe %d");
       return -1;
    }

    AIS_LOG(SENSOR_PLATFORM, ERROR,"%s: I'm running for instance ...", __FUNCTION__);

    pollfds[0].fd     = pCtxt->m_pipeFd[0];
    pollfds[0].events = POLLIN|POLLRDNORM;
    pollfds[1].fd = pCtxt->m_sensorFd;
    pollfds[1].events = POLLIN|POLLRDNORM|POLLPRI;
    pollNumFds        = 2;

    while (!pCtxt->m_isAbortIntrPollThread)
    {
        /* block infinitely to wait for interrupt or being cancelled */
        rc = poll(pollfds, pollNumFds, -1);
        if (rc <= 0)
        {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "%s: poll failed %d", __func__, rc);
            continue;
        }
        if ((POLLIN == (pollfds[0].revents & POLLIN)) &&
           (POLLRDNORM == (pollfds[0].revents & POLLRDNORM)))
        {
            AIS_LOG(SENSOR_PLATFORM, MED, "got event from pipe to exit");
        }
        else
        {
            pCtxt->SensorPlatformIntrProcessEvent();
        }
    }

    AIS_LOG(SENSOR_PLATFORM, MED, "Sensor Platform %d Event thread is exiting...", pCtxt->m_eCameraInterface);
    return 0;
}

/* ---------------------------------------------------------------------------
 *    FUNCTION        SensorPlatformInit
 *    DESCRIPTION     Initialize sensor platform with dev id
 *    DEPENDENCIES    None
 *    PARAMETERS
 *    RETURN VALUE    sensor platform ctxt ptr
 *    SIDE EFFECTS    None
 * ------------------------------------------------------------------------ */
SensorPlatform* SensorPlatform::SensorPlatformInit(sensor_lib_t* p_sensor_lib)
{
    CameraResult result = CAMERA_SUCCESS;

    SensorPlatformLinux* pCtxt = new SensorPlatformLinux(p_sensor_lib);
    if (pCtxt)
    {
        (void)pCtxt->Init();

        if (CAMERA_SUCCESS != result)
        {
            delete pCtxt;
            pCtxt = NULL;
        }
    }

    return pCtxt;
}

CameraResult SensorPlatformLinux::Init()
{
    CameraResult result = CAMERA_SUCCESS;
    int devIdx;

    m_eCameraInterface = m_pSensorLib->sensor_slave_info.camera_id;
    CameraPlatform_GetI2cMasterInfo(m_eCameraInterface, &m_cciDeviceIndex, &m_I2CMasterIndex);
    devIdx = CameraPlatform_GetCsiCore(m_eCameraInterface);

    AIS_LOG(SENSOR_PLATFORM, HIGH, "SensorPlatform init camera_id %d devIdx %u", m_eCameraInterface, devIdx);

    if (devIdx < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "camera_id %d invalid devIdx %u", m_eCameraInterface, devIdx);
        result = CAMERA_EFAILED;
    }
    else
    {
        const char* path = CameraPlatformGetDevPath(AIS_SUBDEV_SENSOR, devIdx);
        if (path)
        {
            m_sensorFd = open(path, O_RDWR | O_NONBLOCK);
            if (m_sensorFd <= 0) {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "cannot open '%s'", path);
                result = CAMERA_EFAILED;
            }
        }
        else
        {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "sensor %d subdev not available", m_eCameraInterface);
            result = CAMERA_EFAILED;
        }
    }

    if (CAMERA_SUCCESS == result)
    {
        const char* cci_path = CameraPlatformGetDevPath(AIS_SUBDEV_CCI, m_cciDeviceIndex);

        if (cci_path)
        {
            m_cciFd = open(cci_path, O_RDWR | O_NONBLOCK);
            if (m_cciFd <= 0) {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "cannot open '%s'", cci_path);
            }
        }
        else
        {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "cci subdev not available");
        }
    }

    if (CAMERA_SUCCESS == result)
    {
        boolean ret = TRUE;
        int32_t rc = 0, i;

        struct camera_power_setting_array *power_setting_array;
        power_setting_array = &m_pSensorLib->sensor_slave_info.
            power_setting_array;

        m_probeCmd = (struct ais_sensor_probe_cmd *)CameraAllocate(
            CAMERA_ALLOCATE_ID_SENSOR_PLATFORM_CTXT,
            sizeof(*m_probeCmd));

        if(!m_probeCmd) {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to malloc probe cmd for %s",
                m_pSensorLib->sensor_slave_info.sensor_name);
            result = CAMERA_ENOMEMORY;
        }
        else
        {
            int rc;

            translate_sensor_slave_info(m_probeCmd,
                &m_pSensorLib->sensor_slave_info);


            /* Pass slave information to kernel and probe */
            struct cam_control cam_cmd = {};

            cam_cmd.op_code     = AIS_SENSOR_PROBE_CMD;
            cam_cmd.size        = sizeof(*m_probeCmd);
            cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
            cam_cmd.reserved    = 0;
            cam_cmd.handle      = (uint64_t)m_probeCmd;

            if ((rc = LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "probe")) < 0)
            {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "[%s] AIS_SENSOR_PROBE_CMD [0x%x] failed %d",
                    m_pSensorLib->sensor_slave_info.sensor_name,
                    AIS_SENSOR_PROBE_CMD, rc);
                result = CAMERA_EFAILED;
            }
        }

        if(CAMERA_SUCCESS == result)
        {
            int i = 0;

            for (i = 0;  i < MAX_INTERRUPT_GPIO_PER_INPUT; i++)
            {
                m_interrupts[i].isUsed = FALSE;
            }
        }

        if (CAMERA_SUCCESS != result)
        {
            if (m_probeCmd)
            {
                CameraFree(CAMERA_ALLOCATE_ID_SENSOR_PLATFORM_CTXT, m_probeCmd);
            }
        }
    }

    return result;
}

/* ---------------------------------------------------------------------------
 *    FUNCTION        SensorPlatformDeinit
 *    DESCRIPTION     Deinitialize sensor platform with dev id
 *    DEPENDENCIES    None
 *    PARAMETERS
 *    RETURN VALUE    sensor platform ctxt ptr
 *    SIDE EFFECTS    None
 * ------------------------------------------------------------------------ */
void SensorPlatform::SensorPlatformDeinit(SensorPlatform* pPlatform)
{
    SensorPlatformLinux* pCtxt = static_cast<SensorPlatformLinux*>(pPlatform);

    if (pCtxt)
    {
        pCtxt->Deinit();

        delete pCtxt;
    }
}

void SensorPlatformLinux::Deinit()
{
    struct cam_control cam_cmd = {};
    int ret = 0;
    struct v4l2_event_subscription sub = {};
    int i;

    for (i = 0; i < MAX_INTERRUPT_GPIO_PER_INPUT; i++)
    {
        m_interrupts[i].isUsed = FALSE;
    }

    if (m_hThread)
    {
        m_isAbortIntrPollThread = TRUE;
        write(m_pipeFd[1], &m_interrupts[0].isUsed, sizeof(m_interrupts[0].isUsed));
        CameraReleaseThread(m_hThread);
        m_hThread = NULL;
    }

    cam_cmd.op_code = AIS_SENSOR_INTR_DEINIT;
    cam_cmd.size = 0;
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;

    if (ioctl(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd) < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "AIS_SENSOR_INTR_DEINIT failed");
    }


    //Unsubscribe events
    sub.type = AIS_SENSOR_EVENT_TYPE;
    if (ioctl(m_sensorFd, VIDIOC_UNSUBSCRIBE_EVENT, &sub) < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "SensorPlatformDeinit VIDIOC_UNSUBSCRIBE_EVENT failed %d",ret);
    }

    close(m_sensorFd);

    if (m_probeCmd)
    {
        CameraFree(CAMERA_ALLOCATE_ID_SENSOR_PLATFORM_CTXT, m_probeCmd);
    }
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSetupGpioInterrupt -
 *
 * DESCRIPTION: Setup gpio id as interrupt
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSetupGpioInterrupt(enum camera_gpio_type gpio_id,
    sensor_intr_callback_t cb, void *data)
{
    CameraSensorGPIO_IntrPinType pinInfo;
    SensorPlatformInterruptType *pPlatformIntr = NULL;
    struct cam_control cam_cmd = {};
    CameraResult result = CAMERA_SUCCESS;
    int freeIdx = MAX_INTERRUPT_GPIO_PER_INPUT;
    int rc;
    int i = 0;

    for (i = 0; i < MAX_INTERRUPT_GPIO_PER_INPUT; i++)
    {
        pPlatformIntr = &m_interrupts[i];

        if (pPlatformIntr->isUsed && pPlatformIntr->gpio_id == gpio_id)
        {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "GPIO%d Interrupt is already setup", gpio_id);
            freeIdx = MAX_INTERRUPT_GPIO_PER_INPUT;
            break;
        }

        if (!pPlatformIntr->isUsed && MAX_INTERRUPT_GPIO_PER_INPUT == freeIdx)
        {
            freeIdx = i;
            break;
        }
    }


    if (freeIdx < MAX_INTERRUPT_GPIO_PER_INPUT)
    {
        m_probeCmd->gpio_intr_config[freeIdx].gpio_num = -1;
        if (CameraSensorGPIO_GetIntrPinInfo(m_eCameraInterface, gpio_id, &pinInfo))
        {
            result = CAMERA_SUCCESS;
            m_probeCmd->gpio_intr_config[freeIdx].gpio_num =
                    gpio_id - CAMERA_GPIO_INTR;
            switch (pinInfo.trigger)
            {
            case 0: /* trigger NONE */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000000;
                break;
            case 1: /* trigger RISING */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000001;
                break;
            case 2: /* trigger FALLING */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000002;
                break;
            case 3: /* trigger EDGE */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000003;
                break;
            case 4: /* trigger LEVEL_LOW */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000008;
                break;
            case 5: /* trigger LEVEL_HIGH */
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000004;
                break;
            default:
                m_probeCmd->gpio_intr_config[freeIdx].gpio_cfg0 =
                        0x00000000;
                break;
            }
        }
        else
        {
            AIS_LOG(SENSOR_PLATFORM, WARN, "fail to GetIntrPinInfo %d", gpio_id);
        }

        if (CAMERA_SUCCESS == result)
        {
            struct v4l2_event_subscription sub = {};
            sub.type = AIS_SENSOR_EVENT_TYPE;
            sub.id = m_probeCmd->gpio_intr_config[freeIdx].gpio_num;
            rc = ioctl(m_sensorFd, VIDIOC_SUBSCRIBE_EVENT, &sub);
            if(rc < 0)
            {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "[%s] VIDIOC_SUBSCRIBE_EVENT [0x%x] failed %d",
                        m_pSensorLib->sensor_slave_info.sensor_name,
                        AIS_SENSOR_EVENT_TYPE, rc);
                result = CAMERA_EFAILED;
            }
        }

        if (CAMERA_SUCCESS == result)
        {
            cam_cmd.op_code = AIS_SENSOR_INTR_INIT;
            cam_cmd.size = sizeof(struct ais_sensor_gpio_intr_config);
            cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
            cam_cmd.reserved    = 0;
            cam_cmd.handle = (uint64_t)&m_probeCmd->gpio_intr_config[freeIdx];

            rc = ioctl(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "intr_init");
            if (rc < 0)
            {
                result = CAMERA_EFAILED;
            }
        }

        if (CAMERA_SUCCESS == result)
        {
            pPlatformIntr->pCtxt = this;
            pPlatformIntr->isUsed = TRUE;
            pPlatformIntr->pCbFcn = cb;
            pPlatformIntr->pData = data;
            pPlatformIntr->gpio_id = gpio_id;
            pPlatformIntr->gpio_num = gpio_id - CAMERA_GPIO_INTR;

            if (m_hThread == NULL)
            {
                snprintf(pPlatformIntr->tname, sizeof(pPlatformIntr->tname),
                    "sensor_platform_intr_poll_thread_%d", m_eCameraInterface);
                result = CameraCreateThread(CAMERA_THREAD_PRIO_DEFAULT, 0,
                        SensorPlatformIntrPollThread,
                        (void*)this,
                        0,
                        pPlatformIntr->tname,
                        &m_hThread);
                (void)CameraSetThreadPriority(m_hThread,
                        CAMERA_THREAD_PRIO_HIGH_REALTIME);

                if (result != CAMERA_SUCCESS)
                {
                    pPlatformIntr->isUsed = FALSE;
                    m_hThread = NULL;
                    //@todo: any cleanup...
                }
            }
        }
    }
    else
    {
        result = CAMERA_EFAILED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSetupCciFrameSync -
 *
 * DESCRIPTION: Setup CCI frame sync
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSetupCciFrameSync(unsigned int fsync_freq)
{
    CAM_UNUSED(fsync_freq);

    AIS_LOG(SENSOR_PLATFORM, ERROR, "Unsupported feature");

    return CAMERA_EUNSUPPORTED;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorTriggerCciFrameSync -
 *
 * DESCRIPTION: Trigger CCI frame sync
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorTriggerCciFrameSync()
{
    AIS_LOG(SENSOR_PLATFORM, ERROR, "Unsupported feature");

    return CAMERA_EUNSUPPORTED;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorPowerUp -
 *
 * DESCRIPTION: Executes power up sequence defined in sensor library
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorPowerUp()
{
    struct cam_control cam_cmd = {};

    SENSOR_FUNCTIONENTRY("");

    cam_cmd.op_code     = AIS_SENSOR_POWER_UP;
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.size        = 0;

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "power_up") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "AIS_SENSOR_POWER_UP failed");
        return CAMERA_EFAILED;
    }
    AIS_LOG(SENSOR_PLATFORM, HIGH, "AIS_SENSOR_POWER_UP success on camera Id: %d", m_eCameraInterface);

    SENSOR_FUNCTIONEXIT("");

    return CAMERA_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorPowerDown -
 *
 * DESCRIPTION: Executes power down sequence defined in sensor library
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorPowerDown()
{
    struct cam_control cam_cmd = {};

    SENSOR_FUNCTIONENTRY("");

    cam_cmd.op_code     = AIS_SENSOR_POWER_DOWN;
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.size        = 0;

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "power_down") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "AIS_SENSOR_POWER_DOWN failed");
        return CAMERA_EFAILED;
    }
    AIS_LOG(SENSOR_PLATFORM, HIGH, "AIS_SENSOR_POWER_DOWN success on camera Id: %d", m_eCameraInterface);

    SENSOR_FUNCTIONEXIT("");

    return CAMERA_SUCCESS;
}

static void translate_sensor_power_sequence(
    struct ais_sensor_power_settings_seq *power_cmd,
    struct camera_power_setting *power_setting_array_info,
    unsigned short size)
{
    struct ais_power_settings *power_setting_k = power_cmd->power_seq_settings;
    struct camera_power_setting* power_array = power_setting_array_info;

    power_cmd->power_seq_size = size;

    if (power_cmd->power_seq_size > AIS_MAX_POWER_SEQ)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "power up seq too long (%d). Will truncate to %d",
            power_cmd->power_seq_size, AIS_MAX_POWER_SEQ);
        power_cmd->power_seq_size = AIS_MAX_POWER_SEQ;
    }

    for (uint16_t i = 0; i < power_cmd->power_seq_size; i++) {
        power_setting_k[i].power_seq_type =
            sensor_sdk_util_get_kernel_power_seq_type(
                power_array[i].seq_type,
                power_array[i].seq_val);
        power_setting_k[i].config_val_low =
            power_array[i].config_val;
        power_setting_k[i].delay =
            power_array[i].delay;
    }
}
/* ---------------------------------------------------------------------------
 * FUNCTION SensorExecutePowerSetting
 * DESCRIPTION Execute Sensor power config sequence
 * DEPENDENCIES None
 * PARAMETERS PowerConfig sequence and its size
 * RETURN VALUE None
 * SIDE EFFECTS None
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorExecutePowerSetting(
    struct camera_power_setting *power_settings, unsigned short nSize, CameraPowerEventType mode)
{
    CameraResult rc = CAMERA_SUCCESS;

    SENSOR_FUNCTIONENTRY("");

    switch (mode)
    {
    case CAMERA_POWER_DOWN:
    case CAMERA_POWER_UP: {

        struct cam_control cam_cmd = {};

        ais_sensor_power_settings_seq* sensor_power_seq_cmd;

        sensor_power_seq_cmd = (struct ais_sensor_power_settings_seq *)CameraAllocate(
                CAMERA_ALLOCATE_ID_SENSOR_PLATFORM_CTXT,
                sizeof(*sensor_power_seq_cmd));

        if(!sensor_power_seq_cmd) {
            AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to malloc power sequence cmd");
            rc = CAMERA_ENOMEMORY;
        }
        else {
            translate_sensor_power_sequence(sensor_power_seq_cmd, power_settings, nSize);
        }

        if (rc == 0)
        {
            cam_cmd.op_code     = AIS_SENSOR_EXEC_POWER_SEQ;
            cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
            cam_cmd.size        = sizeof(*sensor_power_seq_cmd);
            cam_cmd.handle      = (uint64_t)sensor_power_seq_cmd;
            cam_cmd.reserved    = 0;

            if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "power_op") < 0)
            {
                AIS_LOG(SENSOR_PLATFORM, ERROR, "Execute Power sequence with mode = %d failed on camera Id: %d",
                    mode, m_eCameraInterface);
                rc = CAMERA_EFAILED;
            }
            else
            {
                AIS_LOG(SENSOR_PLATFORM, HIGH, "Execute Power sequence with mode = %d success on camera Id: %d",
                    mode, m_eCameraInterface);
            }
        }
    }
        break;
    case CAMERA_POWER_SUSPEND:
    case CAMERA_POWER_RESUME:
        break;
    default:
        break;
    }

    SENSOR_FUNCTIONEXIT("");

    return rc;
} /* SensorDriver_ExecutePowerConfig */

/*===========================================================================
 * FUNCTION    - SensorSlaveI2cBulkWrite -
 *
 * DESCRIPTION:  Slave bulk write using I2C write burst mode
 *==========================================================================*/
CameraResult SensorPlatformLinux::SensorSlaveI2cBulkWrite(unsigned short slave_addr,
                        struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds)
{
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_sensor_cmd_i2c_burst i2c_write_burst = {};
    CameraResult rc = CAMERA_SUCCESS;
    CAM_UNUSED(exec_pending_i2ccmds);

    cam_cmd.op_code = AIS_SENSOR_I2C_WRITE_BURST;
    cam_cmd.size = sizeof(i2c_write_burst);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&i2c_write_burst;

    translate_sensor_burst_setting(&i2c_write_burst, reg_setting);
    i2c_write_burst.i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        m_pSensorLib->sensor_slave_info.i2c_freq_mode);
    i2c_write_burst.i2c_config.slave_addr = slave_addr;

    AIS_LOG(SENSOR_PLATFORM, LOW, "write (%x) addr %x count %d", slave_addr,
        i2c_write_burst.reg_addr, i2c_write_burst.count);

    for (i = 0; i < i2c_write_burst.count; i++)
    {
        AIS_LOG(SENSOR_PLATFORM, LOW, "To write 0x%x <- 0x%x ",
            (i2c_write_burst.reg_addr + i), i2c_write_burst.data[i]);
    }

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "slave_write_i2c_burst") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to write in burst mode %d count of data to 0x%x:0x%x",
                i2c_write_burst.count, slave_addr, i2c_write_burst.reg_addr);
        rc = CAMERA_EFAILED;
    }

    return rc;
}


/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSlaveI2cBulkWriteRead -
 *
 * DESCRIPTION: Slave Custom Bulk Write followed by bulk read without I2C stop
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSlaveI2cBulkWriteRead (
        unsigned short slave_addr, struct camera_i2c_bulk_reg_setting *write_reg_setting,
        struct camera_i2c_bulk_reg_setting *read_reg_setting, boolean exec_pending_i2ccmds)
{
    AIS_LOG(SENSOR_PLATFORM, ERROR, "Function not supported");
    return CAMERA_EUNSUPPORTED;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorWriteI2cSettingArray -
 *
 * DESCRIPTION: Write I2C setting array
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorWriteI2cSettingArray(
    struct camera_i2c_reg_setting_array *settings)
{
    CameraResult result = CAMERA_SUCCESS;
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_sensor_cmd_i2c_wr_array i2c_write = {};
    unsigned short slave_addr = m_pSensorLib->sensor_slave_info.slave_addr;

    cam_cmd.op_code = AIS_SENSOR_I2C_WRITE_ARRAY;
    cam_cmd.size = sizeof(i2c_write);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&i2c_write;

    translate_sensor_reg_setting_array(&i2c_write, settings);
    i2c_write.i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        m_pSensorLib->sensor_slave_info.i2c_freq_mode);
    i2c_write.i2c_config.slave_addr = slave_addr;

    AIS_LOG(SENSOR_PLATFORM, HIGH, "slave %x reg array size %d", slave_addr,
        i2c_write.count);
    for (i = 0; i < i2c_write.count; i++)
    {
        AIS_LOG(SENSOR_PLATFORM, LOW, "addr %x data %x delay = %d",
            i2c_write.wr_array[i].reg_addr,
            i2c_write.wr_array[i].reg_data,
            i2c_write.wr_array[i].delay);
    }

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "write_i2c_array") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to write array to 0x%x %d", slave_addr, i2c_write.count);
        result = CAMERA_EFAILED;
    }

    return result;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSlaveWriteI2cSetting_sync -
 *
 * DESCRIPTION: Write I2C setting array with sync
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSlaveWriteI2cSetting_sync(
    struct camera_i2c_sync_cfg sync_cfg,
    struct camera_i2c_reg_setting_array_sync* reg_setting_array_sync, unsigned int size)
{
    CameraResult result = CAMERA_SUCCESS;
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_cci_cmd_t cci_cfg = {};

    cam_cmd.op_code = AIS_SENSOR_I2C_WRITE_ARRAY_SYNC;
    cam_cmd.size = sizeof(cci_cfg);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&cci_cfg;

    cci_cfg.cmd.wr_sync.sync_cfg.cid = sync_cfg.cid;
    cci_cfg.cmd.wr_sync.sync_cfg.csid = id_map_table[sync_cfg.csid];
    cci_cfg.cmd.wr_sync.sync_cfg.delay = sync_cfg.delay;
    cci_cfg.cmd.wr_sync.sync_cfg.line = sync_cfg.line;
    cci_cfg.cmd.wr_sync.num_wr_cfg = size;

    cci_cfg.cci_client.cci_i2c_master = m_I2CMasterIndex;

    m_I2CBusFrequency = m_pSensorLib->sensor_slave_info.i2c_freq_mode;
    cci_cfg.cci_client.i2c_freq_mode = m_I2CBusFrequency;

    cci_cfg.cci_client.retries = 3;

    for(i = 0; i < size; i++)
    {
       cci_cfg.cmd.wr_sync.wr_cfg[i].i2c_config.slave_addr = reg_setting_array_sync->sid;
       cci_cfg.cmd.wr_sync.wr_cfg[i].i2c_config.i2c_freq_mode = reg_setting_array_sync->i2c_freq_mode;
       cci_cfg.cmd.wr_sync.wr_cfg[i].addr_type = reg_setting_array_sync->addr_type;
       cci_cfg.cmd.wr_sync.wr_cfg[i].data_type = reg_setting_array_sync->data_type;
       cci_cfg.cmd.wr_sync.wr_cfg[i].count = reg_setting_array_sync->size;
       cci_cfg.cmd.wr_sync.wr_cfg[i].wr_array = (struct ais_sensor_i2c_wr_payload *)reg_setting_array_sync->reg_array;
       reg_setting_array_sync++;
    }

    if (LOG_IOCTL(m_cciFd, VIDIOC_CAM_CONTROL, &cam_cmd, "write_i2c_array_sync") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to write array with sync");
        result = CAMERA_EFAILED;
    }

    return result;
}


/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSlaveWriteI2cSetting -
 *
 * DESCRIPTION: Slave Write I2C setting array
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSlaveWriteI2cSetting(
    unsigned short slave_addr, struct camera_i2c_reg_setting *setting)
{
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_sensor_cmd_i2c_wr_array i2c_write = {};

    cam_cmd.op_code = AIS_SENSOR_I2C_WRITE_ARRAY;
    cam_cmd.size = sizeof(i2c_write);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&i2c_write;

    translate_sensor_reg_setting(&i2c_write, setting);
    i2c_write.i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        m_pSensorLib->sensor_slave_info.i2c_freq_mode);
    i2c_write.i2c_config.slave_addr = slave_addr;

    AIS_LOG(SENSOR_PLATFORM, MED, "slave %x reg array size %d", slave_addr,
        i2c_write.count);
    for (i = 0; i < i2c_write.count; i++)
    {
        AIS_LOG(SENSOR_PLATFORM, LOW, "addr %x data %x delay = %d",
            i2c_write.wr_array[i].reg_addr,
            i2c_write.wr_array[i].reg_data,
            i2c_write.wr_array[i].delay);
    }

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "write_i2c_setting") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to write array to 0x%x %d", slave_addr, i2c_write.count);
        return CAMERA_EFAILED;
    }

    if (setting->delay)
    {
        CameraSleep(setting->delay);
    }

    return CAMERA_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorSlaveI2cRead -
 *
 * DESCRIPTION: Slave read I2C reg
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorSlaveI2cRead(unsigned short slave_addr,
    struct camera_reg_settings_t *setting)
{
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_sensor_cmd_i2c_read i2c_read = {};

    cam_cmd.op_code = AIS_SENSOR_I2C_READ;
    cam_cmd.size = sizeof(i2c_read);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&i2c_read;

    translate_sensor_read_reg(&i2c_read, setting);
    i2c_read.i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        m_pSensorLib->sensor_slave_info.i2c_freq_mode);
    i2c_read.i2c_config.slave_addr = slave_addr;

    AIS_LOG(SENSOR_PLATFORM, LOW, "read (%x) addr %x", slave_addr,
        i2c_read.reg_addr);

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "slave_read_i2c") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to read 0x%x:0x%x", slave_addr, i2c_read.reg_addr);
        return CAMERA_EFAILED;
    }

    setting->reg_data = i2c_read.reg_data;
    AIS_LOG(SENSOR_PLATFORM, LOW, "read (%x) addr %x data %x", slave_addr,
        i2c_read.reg_addr, i2c_read.reg_data);

    return CAMERA_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - SensorSlaveI2cBulkRead -
 *
 * DESCRIPTION:  Slave bulk read using I2C read burst mode
 *==========================================================================*/
CameraResult SensorPlatformLinux::SensorSlaveI2cBulkRead(unsigned short slave_addr,
                        struct camera_i2c_bulk_reg_setting *reg_setting, boolean exec_pending_i2ccmds)
{
    uint32_t i = 0;
    struct cam_control cam_cmd = {};
    struct ais_sensor_cmd_i2c_burst i2c_read_burst = {};
    CameraResult rc = CAMERA_SUCCESS;
    CAM_UNUSED(exec_pending_i2ccmds);

    cam_cmd.op_code = AIS_SENSOR_I2C_READ_BURST;
    cam_cmd.size = sizeof(i2c_read_burst);
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.reserved = 0;
    cam_cmd.handle = (uint64_t)&i2c_read_burst;

    translate_sensor_burst_setting(&i2c_read_burst, reg_setting);
    i2c_read_burst.i2c_config.i2c_freq_mode = sensor_sdk_util_get_i2c_freq_mode(
        m_pSensorLib->sensor_slave_info.i2c_freq_mode);
    i2c_read_burst.i2c_config.slave_addr = slave_addr;

    AIS_LOG(SENSOR_PLATFORM, LOW, "read (%x) addr %x", slave_addr,
        i2c_read_burst.reg_addr);

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "slave_read_i2c_burst") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "failed to read in burst mode 0x%x:0x%x", slave_addr, i2c_read_burst.reg_addr);
        rc = CAMERA_EFAILED;
    }

    for (i = 0; i<i2c_read_burst.count; i++)
    {
        reg_setting->reg_data[i] = i2c_read_burst.data[i];
        AIS_LOG(SENSOR_PLATFORM, LOW, "read (0x%x) addr 0x%x data 0x%x", slave_addr,
            i2c_read_burst.reg_addr + i, i2c_read_burst.data[i]);
    }

    return rc;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorWriteI2cSetting -
 *
 * DESCRIPTION: Write I2C setting
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorWriteI2cSetting(
    struct camera_i2c_reg_setting *setting)
{
    return SensorSlaveWriteI2cSetting(m_pSensorLib->sensor_slave_info.slave_addr, setting);
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorPowerResume -
 *
 * DESCRIPTION: Executes power resume sequence defined in sensor library
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorPowerResume()
{
    struct cam_control cam_cmd = {};

    SENSOR_FUNCTIONENTRY("");

    cam_cmd.op_code     = AIS_SENSOR_I2C_POWER_UP;
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.size        = 0;

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "i2c_power_up") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "AIS_SENSOR_I2C_POWER_UP failed");
        return CAMERA_EFAILED;
    }
    AIS_LOG(SENSOR_PLATFORM, HIGH, "AIS_SENSOR_POWER_RESUME success on camera Id: %d", m_eCameraInterface);
    SENSOR_FUNCTIONEXIT("");

    return CAMERA_SUCCESS;
}

/* ---------------------------------------------------------------------------
 * FUNCTION    - SensorPowerSuspend -
 *
 * DESCRIPTION: Executes power suspend sequence defined in sensor library
 * ------------------------------------------------------------------------ */
CameraResult SensorPlatformLinux::SensorPowerSuspend()
{
    struct cam_control cam_cmd = {};

    SENSOR_FUNCTIONENTRY("");

    cam_cmd.op_code     = AIS_SENSOR_I2C_POWER_DOWN;
    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
    cam_cmd.size        = 0;

    if (LOG_IOCTL(m_sensorFd, VIDIOC_CAM_CONTROL, &cam_cmd, "i2c_power_down") < 0)
    {
        AIS_LOG(SENSOR_PLATFORM, ERROR, "AIS_SENSOR_I2C_POWER_DOWN failed");
        return CAMERA_EFAILED;
    }
    AIS_LOG(SENSOR_PLATFORM, HIGH, "AIS_SENSOR_POWER_SUSPEND success on camera Id: %d", m_eCameraInterface);
    SENSOR_FUNCTIONEXIT("");

    return CAMERA_SUCCESS;
}

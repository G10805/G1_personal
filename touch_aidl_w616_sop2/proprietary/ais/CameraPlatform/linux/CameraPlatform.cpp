/**
 * @file CameraPlatform.cpp
 *
 * @brief Implementation of the CameraPlatform methods.
 *
 * Copyright (c) 2011-2021, 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/*============================================================================
                        INCLUDE FILES
============================================================================*/
#include "AEEstd.h"

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <linux/media.h>
#include <media/ais_isp.h>
#include <media/cam_req_mgr.h>
#include <media/cam_sync.h>
#include <media/ais_sensor.h>

#include "CameraOSServices.h"
#include "CameraPlatform.h"
#include "CameraPlatformLinux.h"
#include "CameraConfig.h"
#include "CameraReqMgr.h"

/*============================================================================
                        INTERNAL DEFINITIONS
============================================================================*/
/* Linux platform types */
enum {
    PLATFORM_SUBTYPE_SA8155_ADP_STAR    = 0x0,
    PLATFORM_SUBTYPE_SA8155_ADP_AIR     = 0x1,
    PLATFORM_SUBTYPE_SA8155_ADP_ALCOR   = 0x2,
    PLATFORM_SUBTYPE_SA8155_ADP_INVALID,
};

#define MAX_NUM_SUBDEVS 32
#define MAX_PATH_SIZE 64
#define DEVNODE_ACCESS_CHECK_CNT 1000
#if defined(__ANDROID__)
#if !defined(AIS_EARLYSERVICE) || defined(ANDROID_T_ABOVE)
#define AIS_DEV_PATH "/dev/"
#else
#define AIS_DEV_PATH "/early_services/dev/"
#endif
#else
#define AIS_DEV_PATH "/dev/"
#endif



#define COPY_DEV_PATH(_to_, _from_) \
    snprintf(_to_, sizeof(_to_), AIS_DEV_PATH "%s", _from_);

typedef struct
{
    AisSubdevIdType id;
    uint32 subdevIdx;

    char path[MAX_PATH_SIZE];
    int fd;

    void*  pKmdQueryCap;
    uint32 hKmdDevHandle;
    uint32 hKmdSessionHandle;

    int32 hSecureIOMMU;     ///< This CSLHw KMD device Secure IOMMU handle
    int32 hNonSecureIOMMU;  ///< This CSLHw KMD device Non Secure IOMMU handle
}AisKmdSubdev;

typedef struct
{
    boolean bInitialized;

    AisKmdSubdev sAisSubdevs[MAX_NUM_SUBDEVS];
    uint32 numSubdevs;

    CamReqManager*     m_pCamReqMgr;
    CameraMutex        m_allocLock;
    int mReqMgrFd;

    //For camera config
    void*  hCameraConfig;
    const  CameraBoardType *pBoardInfo;
    const  ICameraConfig *pCameraConfigIf;
    CameraChannelInfoType const *pChannelInfo;
    int nChannels;

    CameraPlatformChipIdType chipId;
    uint32                   chipVersion;
    CameraPlatformTitanVersion titanVersion;
    CameraPlatformTitanHWVersion titanHwVersion;
    CameraPlatformIdType platformId;
    CameraSocId          multiSocId;

}CameraPlatformContext_t;


static CameraPlatformContext_t gPlatformCtxt =
{
    .bInitialized = FALSE,
    .multiSocId = SOC_ID_0
};
/*============================================================================
                        INTERNAL API DECLARATIONS
============================================================================*/
static boolean CameraPlatformLoadConfigLib(void);
static void    CameraPlatformUnloadConfigLib(void);

/*============================================================================
                        EXTERNAL API DEFINITIONS
============================================================================*/
int CameraPlatform_GetCsiCore(CameraSensorIndex idx)
{
    int csiCore = -1;
    if (NULL != gPlatformCtxt.pBoardInfo && idx < CAMSENSOR_ID_MAX)
    {
        csiCore = gPlatformCtxt.pBoardInfo->camera[idx].csiInfo[0].csiId;
    }
    else
    {
        CAM_MSG(ERROR, "Board not initialized, returning -1");
    }

    return csiCore;
}


boolean CameraPlatformReadHWVersion()
{
    boolean result = FALSE;
    int32  socFd;
    char   buf[32]  = { 0 };
    int32  chipsetVersion = -1;
    char   fileName[] = "/sys/devices/soc0/soc_id";
    int    ret = 0;

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        ret = read(socFd, buf, sizeof(buf) - 1);
        if (-1 == ret)
        {
            CAM_MSG(ERROR, "Unable to read soc_id");
        }
        else
        {
            gPlatformCtxt.chipId = (CameraPlatformChipIdType)strtol(buf, NULL, 0);
            result = TRUE;
        }

        close(socFd);
    }

    return result;
}

boolean CameraPlatformReadPlatformId()
{
    boolean result = FALSE;
    int32  socFd;
    char   buf[32]  = { 0 };
    int32  platformId = -1;
    char   fileName[] = "/sys/devices/soc0/platform_subtype_id";
    int    ret = 0;

    socFd = open(fileName, O_RDONLY);

    if (socFd > 0)
    {
        ret = read(socFd, buf, sizeof(buf) - 1);
        if (-1 == ret)
        {
            CAM_MSG(ERROR, "Unable to read platform_subtype_id");
        }
        else
        {
            platformId = atoi(buf);
            result = TRUE;
        }

        close(socFd);
    }
    /* Map the Linux platform type to CameraPlatformIdType */
    switch (platformId)
    {
    case PLATFORM_SUBTYPE_SA8155_ADP_STAR:
    {
        gPlatformCtxt.platformId = HARDWARE_PLATFORM_ADP_STAR;
        break;
    }
    case PLATFORM_SUBTYPE_SA8155_ADP_AIR:
    {
        gPlatformCtxt.platformId = HARDWARE_PLATFORM_ADP_AIR;
        break;
    }
    case PLATFORM_SUBTYPE_SA8155_ADP_ALCOR:
    {
        gPlatformCtxt.platformId = HARDWARE_PLATFORM_ADP_ALCOR;
        break;
    }
    default:
        CAM_MSG(ERROR, "%s: unsupported platform type %d", __FUNCTION__, platformId);
        gPlatformCtxt.platformId = HARDWARE_PLATFORM_INVALID;
        break;
    }

    return result;
}

void CameraPlatformSetKMDHandle(AisKmdHandletype type, AisSubdevIdType id, uint32 subdev_idx, uint32 handle)
{
    int i = 0;

    CAM_MSG(MEDIUM, "Entry SetKMD type %d id %d subdevidx %d devHandler 0x%x", type, id, subdev_idx, handle);

    for (i = 0; i < MAX_NUM_SUBDEVS; i++)
    {
        if (id == gPlatformCtxt.sAisSubdevs[i].id &&
                subdev_idx == gPlatformCtxt.sAisSubdevs[i].subdevIdx)
        {
            if( AIS_HANDLE_TYPE_SESSION == type)
            {
                gPlatformCtxt.sAisSubdevs[i].hKmdSessionHandle = handle;
                CAM_MSG(LOW, "Success SetKMD id %d subdevidx %d sessionHandler 0x%x", id, subdev_idx, handle);
            }
            else if(AIS_HANDLE_TYPE_DEVICE == type)
            {

                gPlatformCtxt.sAisSubdevs[i].hKmdDevHandle = handle;
                CAM_MSG(LOW, "Success SetKMD id %d subdevidx %d devHandler 0x%x", id, subdev_idx, handle);
            }
            else
            {
                CAM_MSG(HIGH, "Failed SetKMD type %d id %d subdevidx %d type 0x%x", type, id, subdev_idx);
            }
            break;
        }
    }
}

uint32 CameraPlatformGetKMDHandle(AisKmdHandletype type, AisSubdevIdType id, uint32 subdev_idx)
{
    int i = 0;
    uint32 handle = 0;

    CAM_MSG(MEDIUM, "Entry GetKMD type %d id %d subdevidx %d", type, id, subdev_idx);

    for (i = 0; i < MAX_NUM_SUBDEVS; i++)
    {
        if (id == gPlatformCtxt.sAisSubdevs[i].id &&
                subdev_idx == gPlatformCtxt.sAisSubdevs[i].subdevIdx)
        {
            if( AIS_HANDLE_TYPE_SESSION == type)
            {
                handle = gPlatformCtxt.sAisSubdevs[i].hKmdSessionHandle;
                CAM_MSG(LOW, "Success GetKMD id %d subdevidx %d sessionHandler 0x%x", id, subdev_idx, handle);
            }
            else if(AIS_HANDLE_TYPE_DEVICE == type)
            {
                handle = gPlatformCtxt.sAisSubdevs[i].hKmdDevHandle;

                CAM_MSG(LOW, "Success GetKMD id %d subdevidx %d devHandler 0x%x", id, subdev_idx, handle);
            }
            else
            {
                CAM_MSG(HIGH, "Failed GetKMD id %d subdevidx %d type 0x%x", id, subdev_idx, type );
            }
            break;
        }
    }

    return handle;
}


const char* CameraPlatformGetDevPath(AisSubdevIdType id, uint32 subdev_idx)
{
    int i;
    for (i = 0; i < MAX_NUM_SUBDEVS; i++)
    {
        if (id == gPlatformCtxt.sAisSubdevs[i].id &&
                subdev_idx == gPlatformCtxt.sAisSubdevs[i].subdevIdx)
            return gPlatformCtxt.sAisSubdevs[i].path;
    }

    return NULL;
}

int CameraPlatformGetFd(AisSubdevIdType id, uint32 subdev_idx)
{
    int i = 0, fd = 0;

    for (i = 0; i < MAX_NUM_SUBDEVS; i++)
    {
        if (id == gPlatformCtxt.sAisSubdevs[i].id &&
                subdev_idx == gPlatformCtxt.sAisSubdevs[i].subdevIdx)
        {
            fd = gPlatformCtxt.sAisSubdevs[i].fd;
            break;
        }
    }

    return fd;
}

void CameraPlatformClearFd(AisSubdevIdType id, uint32 subdev_idx)
{
    int i = 0;

    for (i = 0; i < MAX_NUM_SUBDEVS; i++)
    {
        if (id == gPlatformCtxt.sAisSubdevs[i].id &&
                subdev_idx == gPlatformCtxt.sAisSubdevs[i].subdevIdx)
        {
            gPlatformCtxt.sAisSubdevs[i].fd = 0;
            break;
        }
    }
}

static boolean CameraPlatformLoadDevPaths(CameraPlatformContext_t* pCtxt)
{
    struct media_device_info mdev_info;
    int num_media_devices = 0;
    char dev_name[32];
    unsigned int retry_cnt = 0;
    int rc = 0, dev_fd = 0, sd_fd = 0;
    int num_entities;
    int ret = -1;

    CAM_MSG(HIGH, "E");

    while (1)
    {
        AisKmdSubdev* pAisSubdev;
        struct media_entity_desc entity;
        memset(&entity, 0, sizeof(entity));
        snprintf(dev_name, sizeof(dev_name), AIS_DEV_PATH "media%d", num_media_devices);
#ifdef ANDROID_T_ABOVE
        ret = access("/dev/media0", F_OK);
        while ((ret == -1) && (retry_cnt < DEVNODE_ACCESS_CHECK_CNT)) {
                ret = access("/dev/media0", F_OK);
                if(ret == -1)
                  usleep(10000);
                retry_cnt++;
        }
        if (ret == 0)
        {
            CAM_MSG(HIGH, "media device is available");
            usleep(10000);
        } else
        {
            CAM_MSG(ERROR, "Error: media dev access failed: %s", strerror(errno));
        }
#endif
        dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
        if (dev_fd < 0)
        {
            CAM_MSG(HIGH, "Done discovering media devices");
            break;
        }

        num_media_devices++;
        rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
        if (rc < 0)
        {
            CAM_MSG(ERROR, "Error: ioctl media_dev failed: %s", strerror(errno));
            close(dev_fd);
            return rc;
        }

        if (strncmp(mdev_info.model, CAM_REQ_MGR_VNODE_NAME, sizeof(mdev_info.model)) == 0)
        {
            if (pCtxt->numSubdevs == MAX_NUM_SUBDEVS)
            {
                CAM_MSG(ERROR, "Reached max number of subdevs (%d)!", MAX_NUM_SUBDEVS);
                return FALSE;
            }

            pCtxt->sAisSubdevs[pCtxt->numSubdevs].id = AIS_SUBDEV_REQMGR_NODE;
            pCtxt->sAisSubdevs[pCtxt->numSubdevs].fd = dev_fd;
            pCtxt->numSubdevs++;

            num_entities = 1;
            while (1)
            {
                entity.id = num_entities;
                rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
                if (rc < 0) {
                    CAM_MSG(HIGH, "Done enumerating media entities");
                    close(dev_fd);
                    rc = 0;
                    break;
                }

                num_entities = entity.id | MEDIA_ENT_ID_FLAG_NEXT;

                CAM_MSG(MEDIUM, "entity name %s type %d group id %d",
                    entity.name, entity.type, entity.group_id);

                if (pCtxt->numSubdevs == MAX_NUM_SUBDEVS)
                {
                    CAM_MSG(ERROR, "Reached max number of subdevs (%d)!", MAX_NUM_SUBDEVS);
                    return FALSE;
                }

                pAisSubdev = &pCtxt->sAisSubdevs[pCtxt->numSubdevs];

                switch (entity.type)
                {
                case AIS_IFE_DEVICE_TYPE:
                {
                    CAM_MSG(ERROR, "Detected AIS IFE DEVICE!");

                    struct cam_control       ioctlCmd;
                    struct cam_isp_query_cap_cmd* pIfeCap;

                    pAisSubdev->id = AIS_SUBDEV_IFE;
                    pAisSubdev->pKmdQueryCap = CameraAllocate(CAMERA_ALLOCATE_ID_PLATFORM,
                        sizeof(struct cam_isp_query_cap_cmd));
                    if (!pAisSubdev->pKmdQueryCap)
                    {
                        CAM_MSG(ERROR, "Failed to allocate Query Cap", pAisSubdev->path);
                        return FALSE;
                    }

                    pIfeCap = (struct cam_isp_query_cap_cmd*)pAisSubdev->pKmdQueryCap;

                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd <= 0) {
                        CAM_MSG(ERROR, "cannot open '%s'", pAisSubdev->path);
                        continue;
                    }

                    ioctlCmd.op_code     = AIS_IFE_QUERY_CAPS;
                    ioctlCmd.size        = sizeof(struct cam_isp_query_cap_cmd);
                    ioctlCmd.handle_type = AIS_IFE_CMD_TYPE;
                    ioctlCmd.reserved    = 0;
                    ioctlCmd.handle      = (uint64)(pIfeCap);

                    rc = ioctl(sd_fd, VIDIOC_CAM_CONTROL, &ioctlCmd);
                    if (rc < 0) {
                        CAM_MSG(ERROR, "%s query rc %d", pAisSubdev->path, rc);
                        close(sd_fd);
                        continue;
                    }

                    pAisSubdev->subdevIdx = pIfeCap->reserved;
                    pAisSubdev->hNonSecureIOMMU = pIfeCap->device_iommu.non_secure;
                    pAisSubdev->hSecureIOMMU = pIfeCap->device_iommu.secure;
                    pAisSubdev->fd = sd_fd;
                    pCtxt->numSubdevs++;

                    CAM_MSG(ERROR, "Detected AIS IFE%d %u", pIfeCap->reserved, pIfeCap->device_iommu.non_secure);

                    break;
                }
#if 0
                case CAM_IFE_DEVICE_TYPE:
                {
                    struct cam_control       ioctlCmd;
                    struct cam_query_cap_cmd queryCapsCmd;
                    struct cam_isp_query_cap_cmd* pIfeCap;

                    pAisSubdev->id = AIS_SUBDEV_IFE;
                    pAisSubdev->subdevIdx = ife_idx++;

                    pAisSubdev->pKmdQueryCap = CameraAllocate(CAMERA_ALLOCATE_ID_PLATFORM,
                        sizeof(struct cam_isp_query_cap_cmd));
                    if (!pAisSubdev->pKmdQueryCap)
                    {
                        CAM_MSG(ERROR, "Failed to allocate Query Cap", pAisSubdev->path);
                        return FALSE;
                    }

                    pIfeCap = (struct cam_isp_query_cap_cmd*)pAisSubdev->pKmdQueryCap;

                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd <= 0) {
                        CAM_MSG(ERROR, "cannot open '%s'", pAisSubdev->path);
                        continue;
                    }

                    queryCapsCmd.size        = sizeof(struct cam_isp_query_cap_cmd);
                    queryCapsCmd.handle_type = CAM_HANDLE_USER_POINTER;
                    queryCapsCmd.caps_handle = (uint64)(pAisSubdev->pKmdQueryCap);

                    ioctlCmd.op_code     = CAM_QUERY_CAP;
                    ioctlCmd.size        = sizeof(struct cam_query_cap_cmd);
                    ioctlCmd.handle_type = CAM_HANDLE_USER_POINTER;
                    ioctlCmd.reserved    = 0;
                    ioctlCmd.handle      = (uint64)(&queryCapsCmd);

                    rc = ioctl(sd_fd, VIDIOC_CAM_CONTROL, &ioctlCmd);
                    if (rc < 0) {
                        CAM_MSG(ERROR, "%s query rc %d", pAisSubdev->path, rc);
                        close(sd_fd);
                        continue;
                    }

                    pAisSubdev->hNonSecureIOMMU = pIfeCap->device_iommu.non_secure;
                    pAisSubdev->hSecureIOMMU = pIfeCap->device_iommu.secure;

                    pAisSubdev->fd = sd_fd;
                    pCtxt->numSubdevs++;
                    break;
                }
#endif
                case CAM_CSIPHY_DEVICE_TYPE:
                {
                    struct cam_control cam_cmd;
                    struct cam_csiphy_query_cap* pCsiphyCap = NULL;

                    pAisSubdev->id = AIS_SUBDEV_CSIPHY;

                    pAisSubdev->pKmdQueryCap = CameraAllocate(CAMERA_ALLOCATE_ID_PLATFORM,
                        sizeof(struct cam_csiphy_query_cap));
                    if (!pAisSubdev->pKmdQueryCap)
                    {
                        CAM_MSG(ERROR, "Failed to allocate Query Cap", pAisSubdev->path);
                        return FALSE;
                    }

                    pCsiphyCap = (struct cam_csiphy_query_cap*)pAisSubdev->pKmdQueryCap;
                    memset(pCsiphyCap, 0x0, sizeof(*pCsiphyCap));

                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd < 0) {
                        CAM_MSG(ERROR, "Open subdev failed: %s", pAisSubdev->path);
                        continue;
                    }

                    memset(&cam_cmd, 0x0, sizeof(cam_cmd));
                    cam_cmd.op_code     = CAM_QUERY_CAP;
                    cam_cmd.size        = sizeof(*pCsiphyCap);
                    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
                    cam_cmd.reserved    = 0;
                    cam_cmd.handle      = (uint64)pCsiphyCap;
                    rc = ioctl(sd_fd, VIDIOC_CAM_CONTROL, &cam_cmd);
                    if (rc < 0) {
                        CAM_MSG(ERROR, "%s query rc %d", pAisSubdev->path, rc);
                        close(sd_fd);
                        continue;
                    }

                    pAisSubdev->subdevIdx = pCsiphyCap->slot_info;
                    pAisSubdev->fd = sd_fd;
                    pCtxt->numSubdevs++;
                    break;
                }
                case CAM_SENSOR_DEVICE_TYPE:
                {
                    struct cam_control cam_cmd;
                    struct cam_sensor_query_cap sensor_cap;

                    pAisSubdev->id = AIS_SUBDEV_SENSOR;

                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd < 0) {
                        CAM_MSG(ERROR, "Open subdev failed: %s", pAisSubdev->path);
                        continue;
                    }

                    memset(&cam_cmd, 0x0, sizeof(cam_cmd));
                    memset(&sensor_cap, 0x0, sizeof(sensor_cap));

                    cam_cmd.op_code     = CAM_QUERY_CAP;
                    cam_cmd.size        = sizeof(sensor_cap);
                    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
                    cam_cmd.reserved    = 0;
                    cam_cmd.handle      = (uint64)&sensor_cap;
                    rc = ioctl(sd_fd, VIDIOC_CAM_CONTROL, &cam_cmd);
                    if (rc < 0) {
                        CAM_MSG(ERROR, "%s query rc %d", pAisSubdev->path, rc);
                        close(sd_fd);
                        continue;
                    }
                    close(sd_fd);

                    pAisSubdev->subdevIdx = sensor_cap.slot_info;

                    pCtxt->numSubdevs++;
                    break;
                }
                case CAM_VNODE_DEVICE_TYPE:
                {
                    pAisSubdev->id = AIS_SUBDEV_REQMGR;
                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd <= 0) {
                        CAM_MSG(ERROR, "cannot open '%s'", pAisSubdev->path);
                        continue;
                    }
                    else
                    {
                        CAM_MSG(MEDIUM, "Opened the Camera request manager");
                    }
                    pAisSubdev->fd = sd_fd;
                    pCtxt->mReqMgrFd = sd_fd;
                    pCtxt->numSubdevs++;
                    break;
                }
                case CAM_CCI_DEVICE_TYPE:
                {
                    struct cam_control cam_cmd;
                    struct cam_sensor_query_cap sensor_cap;

                    pAisSubdev->id = AIS_SUBDEV_CCI;

                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd < 0) {
                        CAM_MSG(ERROR, "Open subdev failed: %s", pAisSubdev->path);
                        continue;
                    }

                    memset(&cam_cmd, 0x0, sizeof(cam_cmd));
                    memset(&sensor_cap, 0x0, sizeof(sensor_cap));

                    cam_cmd.op_code     = CAM_QUERY_CAP;
                    cam_cmd.size        = sizeof(sensor_cap);
                    cam_cmd.handle_type = CAM_HANDLE_USER_POINTER;
                    cam_cmd.reserved    = 0;
                    cam_cmd.handle      = (uint64)&sensor_cap;
                    rc = ioctl(sd_fd, VIDIOC_CAM_CONTROL, &cam_cmd);
                    if (rc < 0) {
                        CAM_MSG(ERROR, "%s query rc %d", pAisSubdev->path, rc);
                        close(sd_fd);
                        continue;
                    }
                    close(sd_fd);

                    pAisSubdev->subdevIdx = sensor_cap.slot_info;

                    pCtxt->numSubdevs++;
                    break;
                }
                case CAM_CPAS_DEVICE_TYPE:
                    pAisSubdev->id = AIS_SUBDEV_CPAS;
                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    pCtxt->numSubdevs++;
                    break;
                case CAM_ICP_DEVICE_TYPE:
                case CAM_LRME_DEVICE_TYPE:
                case CAM_JPEG_DEVICE_TYPE:
                case CAM_FD_DEVICE_TYPE:
                    /*Unsupported for now*/
                    break;
                default:
                    CAM_MSG(ERROR, "Unknown: %s - %d", entity.name, entity.group_id);
                    break;
                }
            }
        }
        else if (strncmp(mdev_info.model, CAM_SYNC_DEVICE_NAME, sizeof(mdev_info.model)) == 0)
        {
            if (pCtxt->numSubdevs == MAX_NUM_SUBDEVS)
            {
                CAM_MSG(ERROR, "Reached max number of subdevs (%d)!", MAX_NUM_SUBDEVS);
                return FALSE;
            }

            pCtxt->sAisSubdevs[pCtxt->numSubdevs].id = AIS_SUBDEV_SYNCMGR_NODE;
            pCtxt->sAisSubdevs[pCtxt->numSubdevs].fd = dev_fd;
            pCtxt->numSubdevs++;

            num_entities = 1;
            while (1)
            {
                entity.id = num_entities;
                rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
                if (rc < 0) {
                    CAM_MSG(MEDIUM, "Done enumerating media entities");
                    close(dev_fd);
                    rc = 0;
                    break;
                }

                num_entities = entity.id | MEDIA_ENT_ID_FLAG_NEXT;

                CAM_MSG(MEDIUM, "entity name %s type %d group id %d",
                    entity.name, entity.type, entity.group_id);

                if (pCtxt->numSubdevs == MAX_NUM_SUBDEVS)
                {
                    CAM_MSG(ERROR, "Reached max number of subdevs (%d)!", MAX_NUM_SUBDEVS);
                    return FALSE;
                }

                pAisSubdev = &pCtxt->sAisSubdevs[pCtxt->numSubdevs];

                switch (entity.type)
                {
                case CAM_SYNC_DEVICE_TYPE:
                {
                    pAisSubdev->id = AIS_SUBDEV_SYNCMGR;
                    COPY_DEV_PATH(pAisSubdev->path, entity.name);
                    sd_fd = open(pAisSubdev->path, O_RDWR | O_NONBLOCK);
                    if (sd_fd <= 0) {
                        CAM_MSG(ERROR, "cannot open '%s'", pAisSubdev->path);
                        continue;
                    }
                    else
                    {
                        CAM_MSG(MEDIUM, "Opened the Camera SYNC manager");
                    }
                    pAisSubdev->fd = sd_fd;
                    pCtxt->numSubdevs++;
                    break;
                }
                }
            }
        }
        else
        {
            close(dev_fd);
            continue;
        }
    }

    return TRUE;
}

#if 0
static int CameraPlatformSetClkStatus(CameraPlatformContext_t* pCtxt, unsigned int value)
{
    int rc = 0;
    unsigned int i = 0;

    CAM_MSG(HIGH, "E");
    for (i = 0; i < pCtxt->numSubdevs; i++)
    {
        if (pCtxt->sAisSubdevs[i].id == AIS_SUBDEV_IFE)
        {
            if (pCtxt->sAisSubdevs[i].fd)
            {
                rc = ioctl(pCtxt->sAisSubdevs[i].fd, VIDIOC_MSM_ISP_SET_CLK_STATUS, &value);
                if (rc < 0) {
                    CAM_MSG(ERROR, "%s set clk status %d failed: rc %d",
                               pCtxt->sAisSubdevs[i].path, value, rc);
                    return rc;
                }

                CAM_MSG(HIGH, "[%d] Set clk status subdev %s, value %d, subdev_idx %d",
                            i, pCtxt->sAisSubdevs[i].path, value,
                            pCtxt->sAisSubdevs[i].subdevIdx);
            }
            else
            {
                CAM_MSG(ALWAYS, "[%d] VFE fd 0, subdev %s, subdev_idx %d",
                            i, pCtxt->sAisSubdevs[i].path,
                            pCtxt->sAisSubdevs[i].subdevIdx);
            }
        }
    }

    CAM_MSG(HIGH, "X rc=%d", rc);

    return rc;
}
#endif

/**
 * This function registers the camera with the sleep module.
 */
void CameraRegisterWithSleep(void)
{
}

/**
 * This function de-registers the camera with the sleep module.
 */
void CameraDeregisterWithSleep(void)
{
}

/**
 * This function indicates to the sleep module that it is ok to sleep.
 */
void CameraPermitSleep(void)
{
}

/**
 * This function indicates to the sleep module that it is not ok to sleep.
 */
void CameraProhibitSleep(void)
{
}

/**
 * This function is used to configure GPIO pins for camera.
 *
 * @param[in] idx The camera index.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIOConfigCamera (CameraSensorIndex idx)
{
    (void)idx;
    return TRUE;
}

/**
 * This function is used to unconfigure GPIO pins for camera.
 *
 * @param[in] idx The camera index.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIOUnConfigCamera(CameraSensorIndex idx)
{
    (void)idx;
    return TRUE;
}

/**
 * This function is used to read the logic level of a GPIO pin.
 *
 * @param[in] idx The camera index.
 * @param[in] GPIOSignal GPIO pin to read
 * @param[out] GPIOValue logic level read
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_In(
        CameraSensorIndex idx,
        CameraSensorGPIO_SignalType GPIOSignal,
        CameraSensorGPIO_ValueType *pGPIOValue)
{
    (void)idx;
    (void)GPIOSignal;
    (void)pGPIOValue;
    return TRUE;
}

/**
 * This function is used to set the logic level of a GPIO pin.
 *
 * @param[in] idx The camera index.
 * @param[in] GPIOSignal GPIO pin to set
 * @param[in] GPIOValue logic level to set
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_Out(CameraSensorIndex idx,
    CameraSensorGPIO_SignalType GPIOSignal,
    CameraSensorGPIO_ValueType GPIOValue)
{
    (void)idx;
    (void)GPIOSignal;
    (void)GPIOValue;
    return TRUE;
}

/**
 * @brief This function initializes the camera platform and should be called before all
 * other functions.
 *
 * @return TRUE on success
 */
boolean CameraPlatformInit(void)
{
    int i;
    boolean result = TRUE;

    if (gPlatformCtxt.bInitialized)
    {
        CAM_MSG(ERROR, "CameraPlatform already initialized");
        return FALSE;
    }

    std_memset(&gPlatformCtxt, 0x0, sizeof(gPlatformCtxt));

    if (CAMERA_SUCCESS != CameraOSServicesInitialize(NULL))
    {
        result = FALSE;
    }

    if (TRUE == result)
    {
        result = CameraPlatformReadHWVersion();
    }

    if (TRUE == result)
    {
        result = CameraPlatformReadPlatformId();
    }

    if (TRUE == result)
    {
        result = CameraPlatformLoadDevPaths(&gPlatformCtxt);
    }

    if (TRUE == result)
    {
        result = CameraPlatformLoadConfigLib();
    }

    if (TRUE == result)
    {
        result = CameraSensorI2C_Init();
    }

    if (TRUE == result)
    {
        result = CameraSensorGPIO_Init();
    }

    if (TRUE == result)
    {
        gPlatformCtxt.m_pCamReqMgr = CamReqManager::CreateInstance();
        if (!gPlatformCtxt.m_pCamReqMgr)
        {
            result = FALSE;
        }
    }

    if (TRUE == result)
    {
        result = (CameraCreateMutex(&gPlatformCtxt.m_allocLock) == CAMERA_SUCCESS) ? TRUE : FALSE;
    }

    if (TRUE == result)
    {
        gPlatformCtxt.bInitialized = TRUE;
    }
    else
    {
        CameraPlatformDeinit();
    }

    return result;
}

/**
 * @brief This function deinits the camera platform. No other functions than
 * CameraPlatformInit should be called after this call.
 */
void CameraPlatformDeinit(void)
{
    int i;
    boolean result = TRUE;

    if (gPlatformCtxt.m_pCamReqMgr)
    {
        CamReqManager::DestroyInstance();
        gPlatformCtxt.m_pCamReqMgr = nullptr;
    }

    if (gPlatformCtxt.m_allocLock)
    {
        CameraDestroyMutex(gPlatformCtxt.m_allocLock);
        gPlatformCtxt.m_allocLock = NULL;
    }

    result = CameraSensorI2C_DeInit();
    CAM_MSG_ON_ERR((FALSE == result), "CameraSensorI2C_DeInit failed with %d", result);

    result = CameraSensorGPIO_DeInit();
    CAM_MSG_ON_ERR((FALSE == result), "CameraSensorGPIO_DeInit failed with %d", result);

    CameraPlatformUnloadConfigLib();

    (void)CameraOSServicesUninitialize();

    gPlatformCtxt.bInitialized = FALSE;
}

/*
 * @brief This function assigns a callback funtion to be called from CameraReqManager
 * during camera power events.
 */
CameraResult CameraPlatformRegisterPowerCallback(CameraPowerEventCallable pfnCallback, void* pUsrData)
{
    CameraResult result = CAMERA_SUCCESS;

    CAM_MSG(MEDIUM, "Register callback for PowerEvents");
    result = gPlatformCtxt.m_pCamReqMgr->RegisterPowerCallback(pfnCallback, pUsrData);

    return result;
}

/**
 * This returns the number of CCI device cores on the HW platform.
 *
 * @return The number of CCI device cores
 *
 */
uint8 CameraPlatform_GetNumCciDeviceCores(void)
{
    CAM_MSG(ERROR, "Unsupported API");

    return 0;
}

/**
 * This returns the number of hw blocks with the given type.
 *
 * @param[in] hwBlockType
 *
 * @return The number of hw blocks with the given hwBlockType
 *
 */
uint8 CameraPlatformGetNumHWBlocks(CameraHwBlockType hwBlockType)
{
    (void)hwBlockType;

    CAM_MSG(ERROR, "Unsupported API");

    return 0;
}

/**
 * This returns the virtual base address of requested hw block registers.
 *
 * @return The virtual base address of hw block Registers.
 *         This function will return NULL if hw block is not supported.
 *         The caller should check for NULL return value.
 */
void* CameraPlatformGetVirtualAddress(CameraHwBlockType hwBlockType, uint8 id)
{
    (void)hwBlockType;
    (void)id;

    CAM_MSG(ERROR, "Unsupported API");

    return NULL;
}

/**
 * This method retrieve a input device physical interface information
 * such as csi core, number of lanes, etc...
 *
 * @param[in] CameraDeviceId
 * @param[out] pInterf
 *
 * @return CAMERA_SUCCESS if successful
 */
CameraResult CameraPlatformGetInputDeviceInterface(uint32 CameraDeviceId,
        CameraCsiInfo* pInterf)
{
    CameraResult rc = CAMERA_EFAILED;
    if (gPlatformCtxt.pBoardInfo && pInterf)
    {
        for (int idx = 0; idx < CAMSENSOR_ID_MAX; idx++)
        {
            if (gPlatformCtxt.pBoardInfo->camera[idx].devId == CameraDeviceId)
            {
                for(int i = 0; i < CSIPHY_CORE_MAX; i++)
                {
                    pInterf[i] = gPlatformCtxt.pBoardInfo->camera[idx].csiInfo[i];
                    rc = CAMERA_SUCCESS;
                }
                break;
            }
        }
    }
    return rc;
}

/**
 * This method retrieves information related to IFE
 *
 * @param[in] csiphy
 * @param[out] pInterf
 *
 * @return CAMERA_SUCCESS if successful
 */
CameraResult CameraPlatformGetCsiIfeMap(uint32 csiphy,
        CameraCsiInfo* pInterf)
{
    CameraResult rc = CAMERA_EFAILED;

    if (gPlatformCtxt.pBoardInfo && pInterf)
    {
        for (uint32 idx = 0; idx < CAMSENSOR_ID_MAX; idx++)
        {
            if (gPlatformCtxt.pBoardInfo->camera[idx].devId)
            {
                for(uint32 csiIdx = 0; csiIdx < CSIPHY_CORE_MAX; csiIdx++)
                {
                    if (0 == gPlatformCtxt.pBoardInfo->camera[idx].csiInfo[csiIdx].numLanes)
                    {
                        break;
                    }

                    if (gPlatformCtxt.pBoardInfo->camera[idx].csiInfo[csiIdx].csiId == csiphy)
                    {
                        *pInterf = gPlatformCtxt.pBoardInfo->camera[idx].csiInfo[csiIdx];
                        rc = CAMERA_SUCCESS;
                        break;
                    }
                }
            }

            if (CAMERA_SUCCESS == rc)
            {
                break;
            }
        }
    }

    return rc;
}

/**
 * This method retrieve CameraHwBoardType
 *
 * @return CameraHwBoardType
 */
CameraHwBoardType CameraPlatformGetCameraHwBoardType(void)
{
    CameraHwBoardType boardType = CAMERA_HW_BOARD_NONE;

    if (gPlatformCtxt.pBoardInfo)
    {
        boardType = gPlatformCtxt.pBoardInfo->boardType;
    }

    return boardType;
}

/**
 * This method retrieve CameraEnginSettings
 *
 * @return CameraEnginSettings
 */
const CameraEngineSettings* CameraPlatformGetEngineSettings(void)
{
    return &gPlatformCtxt.pBoardInfo->engineSetting;
}

/**
 * This method retrieve CameraSensorIndex from CameraDeviceId
 *
 * @return CAMERA_SUCCESS if successful
 */
const CameraDeviceConfigType* CameraPlatformGetInputDeviceConfig(uint32 CameraDeviceId, CameraSensorIndex* pIndex)
{
    const CameraDeviceConfigType* devConfig = NULL;
    if (gPlatformCtxt.pBoardInfo)
    {
        int idx = 0;
        for (idx = 0; idx < CAMSENSOR_ID_MAX; idx++)
        {
            if (gPlatformCtxt.pBoardInfo->camera[idx].devId == CameraDeviceId)
            {
                devConfig = &gPlatformCtxt.pBoardInfo->camera[idx].devConfig;
                if (pIndex)
                {
                    *pIndex = (CameraSensorIndex)idx;
                }
                break;
            }
        }
    }
    return devConfig;
}

/**
 * This method gets the input device config from device idx
 *
 * @return CameraSensorBoardType
 */
const CameraSensorBoardType* CameraPlatformGetSensorBoardType(CameraSensorIndex idx)
{
    const CameraSensorBoardType* sensorInfo = NULL;
    if (NULL != gPlatformCtxt.pBoardInfo && idx < CAMSENSOR_ID_MAX)
    {
        sensorInfo = &gPlatformCtxt.pBoardInfo->camera[idx];
    }
    return sensorInfo;
}

/**
 * This method gets the input device config from device ID
 *
 * @return CameraSensorBoardType
 */
const CameraSensorBoardType* CameraPlatformGetDeviceSensorBoardType(CameraDeviceIDType devId)
{
    const CameraSensorBoardType* sensorInfo = NULL;

    for (int idx = 0; idx < CAMSENSOR_ID_MAX; idx++)
    {
        if (gPlatformCtxt.pBoardInfo->camera[idx].devId == devId)
        {
            sensorInfo = &gPlatformCtxt.pBoardInfo->camera[idx];
            break;
        }
    }

    return sensorInfo;
}

/**
 * This function obtains pins info from camera config.
 *
 * @return TRUE if operation was successful; FALSE otherwise.
 */
boolean CameraSensorGPIO_GetIntrPinInfo(const CameraSensorIndex idx,
        CameraSensorGPIO_SignalType pin, CameraSensorGPIO_IntrPinType *pPinInfo)
{
    boolean bReturnVal = FALSE;

    if (pPinInfo)
    {
        const CameraSensorBoardType *sensorInfo = CameraPlatformGetSensorBoardType(idx);
        if (sensorInfo)
        {
            int i = 0;
            for (i = 0; i < MAX_CAMERA_DEV_INTR_PINS; i++)
            {
                if (pin == sensorInfo->intr[i].gpio_id)
                {
                    *pPinInfo = sensorInfo->intr[i];
                    bReturnVal = TRUE;
                    break;
                }
            }


        }
    }

    return bReturnVal;
}


#ifdef AIS_BUILD_STATIC_CONFIG
extern ICameraConfig const * GetCameraConfigInterface(void);
#endif
static boolean CameraPlatformLoadConfigLib(void)
{
    boolean result = FALSE;
    GetCameraConfigInterfaceType pfnGetCameraConfigInterface = 0;

    /* Load camera platform board libraries, and acquire main
     * camera config interface to load board specific data:
     * - hCameraConfig, pBoardInfo, pChannelInfo, nChannels
     */
#ifdef AIS_BUILD_STATIC_CONFIG
    pfnGetCameraConfigInterface = GetCameraConfigInterface;
    gPlatformCtxt.hCameraConfig = (void*)-1;
#else
    if (0 == (gPlatformCtxt.hCameraConfig = dlopen(CAMERA_BOARD_LIBRARY, RTLD_NOW|RTLD_GLOBAL)))
    {
        CAM_MSG(ERROR, "'%s' loading failed '%s'", CAMERA_BOARD_LIBRARY, dlerror());
    }
    else if (0 == (pfnGetCameraConfigInterface = (GetCameraConfigInterfaceType)
                   dlsym(gPlatformCtxt.hCameraConfig, GET_CAMERA_CONFIG_INTERFACE)))
    {
        CAM_MSG(ERROR, "'%s' interface not found '%s'", CAMERA_BOARD_LIBRARY, dlerror());
    }
    else
#endif
    if (0 == (gPlatformCtxt.pCameraConfigIf = pfnGetCameraConfigInterface()))
    {
        CAM_MSG(ERROR, "'%s' bad init interface", CAMERA_BOARD_LIBRARY);
    }
    else if (0 == gPlatformCtxt.pCameraConfigIf->CameraConfigInit)
    {
        CAM_MSG(ERROR, "'%s' bad interface", CAMERA_BOARD_LIBRARY);
    }
    else if (0 != gPlatformCtxt.pCameraConfigIf->CameraConfigInit())
    {
        CAM_MSG(ERROR, "'%s' CameraConfigInit failed", CAMERA_BOARD_LIBRARY);
    }
    else if (0 == gPlatformCtxt.pCameraConfigIf->GetCameraConfigVersion)
    {
        CAM_MSG(ERROR, "'%s' bad interface", CAMERA_BOARD_LIBRARY);
    }
    else if (CAMERA_BOARD_LIBRARY_VERSION != gPlatformCtxt.pCameraConfigIf->GetCameraConfigVersion())
    {
        CAM_MSG(ERROR, "'%s' runtime version mismatch", CAMERA_BOARD_LIBRARY);
    }
    else if (0 == gPlatformCtxt.pCameraConfigIf->GetCameraBoardInfo)
    {
        CAM_MSG(ERROR, "'%s' bad interface", CAMERA_BOARD_LIBRARY);
    }
    else if (0 == (gPlatformCtxt.pBoardInfo = gPlatformCtxt.pCameraConfigIf->GetCameraBoardInfo()))
    {
        CAM_MSG(ERROR, "'%s' GetCameraBoardInfo failed", CAMERA_BOARD_LIBRARY);
    }
    else if (0 == gPlatformCtxt.pCameraConfigIf->GetCameraChannelInfo)
    {
        CAM_MSG(ERROR, "'%s' bad interface", CAMERA_BOARD_LIBRARY);
    }
    else if (0 != gPlatformCtxt.pCameraConfigIf->GetCameraChannelInfo(&gPlatformCtxt.pChannelInfo, &gPlatformCtxt.nChannels))
    {
        CAM_MSG(ERROR, "'%s' GetCameraChannelInfo failed", CAMERA_BOARD_LIBRARY);
    }
    else if (!gPlatformCtxt.pChannelInfo || (0 >= gPlatformCtxt.nChannels))
    {
        CAM_MSG(ERROR, "'%s' GetCameraChannelInfo(0x%p, %d) bad data",
            CAMERA_BOARD_LIBRARY, gPlatformCtxt.pChannelInfo, gPlatformCtxt.nChannels);
    }
    else
    {
        // up to here, all required interfaces have been loaded;
        // required data has been loaded.
        result = TRUE;

        CAM_MSG(HIGH, "'%s' loaded successfully", CAMERA_BOARD_LIBRARY);
    }

    return result;
}

static void CameraPlatformUnloadConfigLib(void)
{
    gPlatformCtxt.pBoardInfo   = 0;
    gPlatformCtxt.pChannelInfo = 0;
    gPlatformCtxt.nChannels    = 0;
    if (0 != gPlatformCtxt.hCameraConfig)
    {
        // camera_config is assumed to be loaded
        if (0 != gPlatformCtxt.pCameraConfigIf &&
            0 != gPlatformCtxt.pCameraConfigIf->CameraConfigDeInit)
        {
            gPlatformCtxt.pCameraConfigIf->CameraConfigDeInit();
        }
#ifndef AIS_BUILD_STATIC_CONFIG
        (void)dlclose(gPlatformCtxt.hCameraConfig);
#endif
        gPlatformCtxt.hCameraConfig = 0;
    }
    gPlatformCtxt.pCameraConfigIf = 0;
}

/**
 * This method retrieves a pointer to the channel info from CameraConfig.
 *
 * @return CAMERA_SUCCESS if successful
 */
CameraResult CameraPlatformGetChannelData(const CameraChannelInfoType ** ppChannelData,
                                 int* const pNumChannels)
{
    CameraResult result = CAMERA_SUCCESS;

    if (!ppChannelData || !pNumChannels)
    {
        CAM_MSG(ERROR, "Null parameter(s)");
        result = CAMERA_EBADPARM;
    }
    else if (!gPlatformCtxt.hCameraConfig || !gPlatformCtxt.pChannelInfo || 0 >= gPlatformCtxt.nChannels)
    {
        CAM_MSG(ERROR, "Bad State");
        result = CAMERA_EBADSTATE;
    }
    else
    {
        *ppChannelData = gPlatformCtxt.pChannelInfo;
        *pNumChannels = gPlatformCtxt.nChannels;

        result        = CAMERA_SUCCESS;
    }

    return result;
}

/**
 * This function is used to read Chip ID.
 *
 * @return CameraPlatformChipIdType
 */
CameraPlatformChipIdType CameraPlatform_GetChipId(void)
{
    return gPlatformCtxt.chipId;
}

/**
 * This function is used to read Chip ID Version.
 *
 * @return uint32
 */
uint32 CameraPlatform_GetChipVersion(void)
{
    return 0;
}

/**
 * This function is used to read Platform ID.
 *
 * @return CameraPlatformIdType
 */
CameraPlatformIdType CameraPlatform_GetPlatformId(void)
{
    return gPlatformCtxt.platformId;
}

static void CameraBufferPopulateMmuHandles(
    int32*       pMMUHandles,
    uint32*      pNumHandles,
    uint32 flags,
    const CameraHwBlockType* pHwBlocks,
    uint32 nHwBlocks)
{
    uint32 i, j;
    uint32 nMmuHandles = *pNumHandles;

    for (i = 0; i < nHwBlocks; i++)
    {
        AisSubdevIdType subdevType = AIS_SUBDEV_IFE;
        if (pHwBlocks[i] != CAMERA_HWBLOCK_IFE)
        {
            //@todo: skip mapping for now as we only map for IFE
            CAM_MSG(ALWAYS, "We are not mapping for block %d for now", pHwBlocks[i]);
            continue;
        }


        for (j = 0; j < gPlatformCtxt.numSubdevs; j++)
        {
            if (gPlatformCtxt.sAisSubdevs[j].id != subdevType)
            {
                continue;
            }

            if (flags & CAMERA_BUFFER_FLAG_SECURE)
            {
                pMMUHandles[nMmuHandles++] = gPlatformCtxt.sAisSubdevs[j].hSecureIOMMU;
            }
            else
            {
                pMMUHandles[nMmuHandles++] = gPlatformCtxt.sAisSubdevs[j].hNonSecureIOMMU;
            }

            break;
        }

    }

    *pNumHandles = nMmuHandles;
}

static uint32 TranslateAlignmentToKMD(uint32 alignment)
{
    //take log2 of alignment to get exponent
    uint32 exp = 0;

    while (alignment > 1)
    {
        alignment = alignment >> 1;
        exp++;
    }

    return exp;
}

static void* OSMemMap(
    int     bufferFD,
    size_t  bufferLength,
    size_t  offset)
{
    void* pMem = NULL;

    if ((bufferLength > 0) && (bufferFD >= 0))
    {
        pMem = mmap(NULL, bufferLength, (PROT_READ | PROT_WRITE), MAP_SHARED, bufferFD, offset);
        if (MAP_FAILED == pMem)
        {
            CAM_MSG(ERROR, "mmap() failed! errno=%d, bufferFD=%zd, bufferLength=%d offset=%zd",
                           errno, bufferFD, bufferLength, offset);
            pMem = NULL;
        }
    }

    return pMem;
}

static int OSMemUnmap(
    void*   pAddr,
    size_t  size)
{
    int result = -1;

    if (NULL != pAddr)
    {
        result = munmap(pAddr, size);
        if (0 != result)
        {
            CAM_MSG(ERROR, "munmap() failed! errno %d", errno);
        }
    }

    return result;
}

/**
 * This function allocates physical buffer memory with size rounded up to the nearest
 * alignment specified. The allocated physical memory can be contiguous or non-contiguous
 * depending on the flags passed. If pHwBlocks and nHwBlocks are set, the buffer is also mapped.
 *
 * @param[in/out] pBuffer   Buffer info to be allocated (size field is used)
 * @param[in] flags         flags on the type of allocation
 * @param[in] alignment     Buffer alignment
 * @param[in] pHwBlocks     List of hw blocks to map to
 * @param[in] nHwBlocks     Number of hw blocks in pHwBlocks
 *
 * @return CameraResult
 */
CameraResult CameraBufferAlloc(AisBuffer* pBuffer,
    uint32 flags,
    uint32 alignment,
    const CameraHwBlockType* pHwBlocks,
    uint32 nHwBlocks)
{
    CameraResult rc = CAMERA_EFAILED;
    size_t bufferSize = pBuffer->size;
    struct cam_mem_mgr_alloc_cmd allocCmd = {};
    allocCmd.len   = bufferSize;
    allocCmd.align = TranslateAlignmentToKMD(alignment);

    CameraBufferPopulateMmuHandles(allocCmd.mmu_hdls, &allocCmd.num_hdl, flags, pHwBlocks, nHwBlocks);

    allocCmd.flags = CAM_MEM_FLAG_HW_READ_WRITE;

    if (flags & CAMERA_BUFFER_FLAG_SECURE)
    {
        allocCmd.flags |= CAM_MEM_FLAG_CP_PIXEL;
    }
    else
    {
        allocCmd.flags |= CAM_MEM_FLAG_UMD_ACCESS | CAM_MEM_FLAG_KMD_ACCESS;
    }

    if (flags & CAMERA_BUFFER_FLAG_CACHED)
    {
        allocCmd.flags |= CAM_MEM_FLAG_CACHE;
    }

    CameraLockMutex(gPlatformCtxt.m_allocLock);
    CAM_MSG(MEDIUM, "CAM_REQ_MGR_ALLOC_BUF %d, %d, 0x%x", allocCmd.len, allocCmd.align, allocCmd.flags);

    rc = gPlatformCtxt.m_pCamReqMgr->Ioctl2(CAM_REQ_MGR_ALLOC_BUF, &allocCmd, 0, sizeof(allocCmd));
    CameraUnlockMutex(gPlatformCtxt.m_allocLock);
    if (CAMERA_SUCCESS == rc)
    {
        pBuffer->flags = flags;
        pBuffer->pMemHndl = (void*)(uintptr_t)allocCmd.out.fd;
        pBuffer->pDa = allocCmd.out.buf_handle;
        pBuffer->isInternal = TRUE;
        if (!(flags & CAMERA_BUFFER_FLAG_SECURE))
        {
            pBuffer->pVa = OSMemMap(allocCmd.out.fd, allocCmd.len, 0);
        }
    }


    return rc;
}

/**
 * This function frees and unmaps buffer
 *
 * @param[in] pBuffer   Buffer to be freed
 *
 * @return CameraResult
 */
void CameraBufferFree(AisBuffer* pBuffer)
{
    if (!pBuffer)
    {
        CAM_MSG(ERROR, "Null ptr");
        return;
    }

    if (!pBuffer->isInternal)
    {
        CAM_MSG(ERROR, "Cannot free non-internal buffer");
        return;
    }

    if (pBuffer->pVa)
    {
        OSMemUnmap(pBuffer->pVa, pBuffer->size);
        pBuffer->pVa = NULL;
    }

    if (pBuffer->pMemHndl)
    {
        close((int)(uintptr_t)pBuffer->pMemHndl);
        pBuffer->pMemHndl = NULL;
    }

    if (pBuffer->pDa)
    {
        CameraResult rc;
        struct cam_mem_mgr_release_cmd releaseCmd = {};
        releaseCmd.buf_handle = pBuffer->pDa;

        CameraLockMutex(gPlatformCtxt.m_allocLock);
        rc = gPlatformCtxt.m_pCamReqMgr->Ioctl2(CAM_REQ_MGR_RELEASE_BUF, &releaseCmd, 0, sizeof(releaseCmd));
        CameraUnlockMutex(gPlatformCtxt.m_allocLock);
        CAM_MSG_ON_ERR(rc, "Failed to release buffer");

        pBuffer->pDa = 0;
    }
}

/**
* This function maps a buffer into the camera SMMU context.
*
* @param[in] pBuffer    The buffer to be mapped
* @param[in] flags      Bit mask of flags for allocation (cont, cached, secure, read/write, etc...)
* @param[in] devAddr    Dev address if fixed mapping flag is set
* @param[in] pHwBlocks  List of hw blocks to map to
* @param[in] nHwBlocks  Number of hw blocks in pHwBlocks

* @return CameraResult
*/
CameraResult CameraBufferMap(AisBuffer* pBuffer,
        uint32 flags,
        uint64 devAddr,
        const CameraHwBlockType* pHwBlocks,
        uint32 nHwBlocks)
{
    CameraResult result = CAMERA_SUCCESS;

    CAM_UNUSED(devAddr);

    if (!pBuffer || !pBuffer->pMemHndl)
    {
        CAM_MSG(ERROR, "null pMemHandle");
        return CAMERA_EBADPARM;
    }
    if (!pHwBlocks)
    {
        CAM_MSG(ERROR, "null pHwBlocks");
        return CAMERA_EFAILED;
    }

    struct cam_mem_mgr_map_cmd  mapCmd = {};

    CameraBufferPopulateMmuHandles(mapCmd.mmu_hdls, &mapCmd.num_hdl, flags, pHwBlocks, nHwBlocks);

    mapCmd.flags = CAM_MEM_FLAG_HW_READ_WRITE;

    if (flags & CAMERA_BUFFER_FLAG_SECURE)
    {
        mapCmd.flags |= CAM_MEM_FLAG_CP_PIXEL;
    }

    mapCmd.fd = (int)(uintptr_t)pBuffer->pMemHndl;

    CameraLockMutex(gPlatformCtxt.m_allocLock);
    result = gPlatformCtxt.m_pCamReqMgr->Ioctl2(CAM_REQ_MGR_MAP_BUF, &mapCmd, 0, sizeof(mapCmd));
    CameraUnlockMutex(gPlatformCtxt.m_allocLock);

    CAM_MSG_ON_ERR(result, "Failed to Map buffer %d 0x%x", mapCmd.fd, mapCmd.flags);

    if (CAMERA_SUCCESS == result)
    {
        pBuffer->pDa = mapCmd.out.buf_handle;
        //map VA if non-secure
        if (!(flags & CAMERA_BUFFER_FLAG_SECURE))
        {
            pBuffer->pVa = OSMemMap(mapCmd.fd, pBuffer->size, 0);
        }
    }

    return result;
}

/**
* This function unmaps a buffer from the camera SMMU context.
*
* @param[in] pBuffer The buffer to be unmapped
*/
void CameraBufferUnmap(AisBuffer* pBuffer)
{
    struct cam_mem_mgr_release_cmd releaseCmd = {};
    CameraResult                   result;

    if (NULL == pBuffer)
    {
        CAM_MSG(ERROR, "IFE: pBufferInfo is NULL");
        return;
    }
    else if (0 == pBuffer->pDa)
    {
        CAM_MSG(ERROR, "IFE: pBufferInfo is not mapped");
        return;
    }

    releaseCmd.buf_handle = pBuffer->pDa;

    CameraLockMutex(gPlatformCtxt.m_allocLock);
    result = gPlatformCtxt.m_pCamReqMgr->Ioctl2(CAM_REQ_MGR_RELEASE_BUF, &releaseCmd, 0, sizeof(releaseCmd));
    CameraUnlockMutex(gPlatformCtxt.m_allocLock);
    CAM_MSG_ON_ERR(result, "Failed to unmap buffer");

    pBuffer->pDa = 0;

    if (pBuffer->pVa) {
        OSMemUnmap(pBuffer->pVa, pBuffer->size);
        pBuffer->pVa = NULL;
    }

    return;
}

/**
* This function looks up the device address that is mapped to a specific HW block
*
* @param[in] pBuffer The buffer that was mapped
* @param[in] hwBlock The HW block for which the DA is needed
*
* @return The device address corresponding to the buffer
*/
uint64 CameraBufferGetDeviceAddress(AisBuffer* pBuffer, CameraHwBlockType hwBlock)
{
    //@todo: for now single DA for all blocks
    CAM_UNUSED(hwBlock);

    return pBuffer->pDa;
}

/**
* This function invalidates the cached buffer
*
* @param[in] pBuffer  The buffer to be cached invalidated
* @return CameraResult
*/
CameraResult CameraBufferCacheInvalidate(AisBuffer* pBuffer)
{
    //@todo: implement
    CAM_UNUSED(pBuffer);

    return CAMERA_SUCCESS;
}

/**
 * This function is used to get Multi SOC Environment.
 *
 * @return boolean
 */
boolean CameraPlatform_GetMultiSocEnv(void)
{
    if (NULL != gPlatformCtxt.pBoardInfo)
    {
        return (gPlatformCtxt.pBoardInfo->multiSocEnable > 0) ? TRUE : FALSE;
    }
    else
    {
        CAM_MSG(HIGH, "Multi SOC Enviroment  is not assigned, Default is single soc environment");
        return FALSE;
    }
}

/**
 * This function is used to get Soc ID.
 *
 * @return uint32
 */
uint32 CameraPlatform_GetMultiSocId(void)
{
    return gPlatformCtxt.multiSocId;
}

/**
 * This function is used to read actual Soc ID.
 *
 * @return uint32
 */
static uint32 CameraPlatform_ReadMultiSocId(void)
{
    return SOC_ID_0;
}

/**
 * This function is used to identify which CCI Device Core and I2C bus Master Index the sensors are configured on.
 *
 * param[in] idx  Camera index
 * param[out] cciDeviceCore  CCI Device Core ID
 * param[out] i2cMasterIndex  I2C bus Master Index
 *
 */
void CameraPlatform_GetI2cMasterInfo(CameraSensorIndex idx,
                                     CameraSensorCciDeviceType* cciDeviceCore,
                                     CameraSensorI2CMasterType* i2cMasterIndex)
{
    if (NULL != gPlatformCtxt.pBoardInfo && idx < CAMSENSOR_ID_MAX)
    {
        *cciDeviceCore = (CameraSensorCciDeviceType)gPlatformCtxt.pBoardInfo->camera[idx].i2cPort[SOC_ID_0].device_id;
        *i2cMasterIndex = (CameraSensorI2CMasterType)gPlatformCtxt.pBoardInfo->camera[idx].i2cPort[SOC_ID_0].port_id;
    }
    else
    {
        *cciDeviceCore = CAMSENSOR_CCI_DEVICE_MAX;
        *i2cMasterIndex = CAMSENSOR_I2C_MASTER_MAX;
        CAM_MSG(ERROR, "Board not initialized, returning Max");
    }
}

/**
 * This function is used to initialize GPIO Block.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_Init(void)
{
    return TRUE;
}

/**
 * This function is used to deinitialize GPIO Block.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_DeInit(void)
{
    return TRUE;
}

/**
 * This function is used to configure GPIO pins for camera communication
 *      control interface.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_ConfigCameraCCI(void)
{
    return TRUE;
}

/**
 * This function is used to unconfigure GPIO pins for camera communication
 *      control interface.
 *
 * @return TRUE - if successful
 *         FALSE - if failed
 */
boolean CameraSensorGPIO_UnConfigCameraCCI(void)
{
    return TRUE;
}

/**
 * This function is used to enable camera clocks
 *
 * @return CAMERA_SUCCESS - if successful
 */
CameraResult CameraPlatformClockEnable(CameraPlatformClockID clk_id)
{
    CameraResult result = CAMERA_SUCCESS;
#if 0
    struct clk_mgr_cfg_data cfg_cmd;
    int rc = 0;

    if (gPlatformCtxt.aisMgrFd)
    {
        memset(&cfg_cmd, 0, sizeof(struct clk_mgr_cfg_data));
        cfg_cmd.cfg_type = AIS_CLK_ENABLE;
        rc = ioctl(gPlatformCtxt.aisMgrFd, VIDIOC_MSM_AIS_CLK_CFG, &cfg_cmd);
        if (rc < 0) {
            result = CAMERA_EFAILED;
            CAM_MSG(ERROR, "VIDIOC_MSM_AIS_CLK_CFG failed: errorno = %d", rc);
            return result;
        }

        rc = CameraPlatformSetClkStatus(&gPlatformCtxt, 1);
        if (rc < 0) {
            result = CAMERA_EFAILED;
            CAM_MSG(ERROR, "VIDIOC_MSM_AIS_SET_CLK_STATUS 1 failed: %d", rc);
        }
    }
    else
    {
        CAM_MSG(ERROR, "AIS_MGR subdev not available");
        result = CAMERA_EFAILED;
    }
#endif
    return result;
}

/**
 * This function is used to disable camera clocks
 *
 * @return CAMERA_SUCCESS - if successful
 */
CameraResult CameraPlatformClockDisable(CameraPlatformClockID clk_id)
{
    CameraResult result = CAMERA_SUCCESS;
#if 0
    struct clk_mgr_cfg_data cfg_cmd;
    int rc = 0;

    if (gPlatformCtxt.aisMgrFd)
    {
        rc = CameraPlatformSetClkStatus(&gPlatformCtxt, 0);
        if (rc < 0) {
            result = CAMERA_EFAILED;
            CAM_MSG(ERROR, "VIDIOC_MSM_AIS_SET_CLK_STATUS 0 failed: %d", rc);
            return result;
        }

        memset(&cfg_cmd, 0, sizeof(struct clk_mgr_cfg_data));
        cfg_cmd.cfg_type = AIS_CLK_DISABLE;
        rc = ioctl(gPlatformCtxt.aisMgrFd, VIDIOC_MSM_AIS_CLK_CFG, &cfg_cmd);
        if (rc < 0) {
            result = CAMERA_EFAILED;
            CAM_MSG(ERROR, "VIDIOC_MSM_AIS_CLK_CFG failed: errorno = %d", rc);
        }
    }
    else
    {
        CAM_MSG(ERROR, "AIS_MGR subdev not available");
        result = CAMERA_EFAILED;
    }
#endif
    return result;
}

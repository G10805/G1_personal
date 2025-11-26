/**
 * Copyright (c) 2022-2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "ams_prm"
#include <log/log.h>
#include <errno.h>
#include <stdlib.h>
#include "audio_hw_clk_api.h"
#include "audio_hw_dma_api.h"
#include "audio_hw_lpm_api.h"
#include "ams_prm.h"
#include "prm_api.h"
#include "ams_gpr_common.h"
#include "gpr_ids_domains.h"
#include "apm_api.h"
#include "ams_osal_mutex.h"
#include "ams_rsc_mgr.h"
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
#include "agm_api.h"
#endif
#ifdef AMS_AGM_HW_RSC_CONFIG_V2_EN
#include <tinyalsa/asoundlib.h>
#endif
extern struct ams_rsc_mgr_gpr_ctx_s ams_rsc_mgr_gpr_ctx;
#define AMS_PRM_VIRT_SOUND_CARD_NUM 100  // virtual sound card
static struct ams_prm_ctx_s
{
    ams_osal_mutex_t lock;
    struct mixer *mixer;
} ams_prm_ctx;

static uint32_t ams_prm_get_mclk_id(uint8_t intf_idx)
{
    uint32_t mclk_id = AMS_MLCK_INVALID;
    switch (intf_idx)
    {
    case INTF_IDX_PRIMARY:
        mclk_id = CLOCK_ID_MCLK_1;
        break;
    case INTF_IDX_SECONDARY:
        mclk_id = CLOCK_ID_MCLK_2;
        break;
    case INTF_IDX_TERTIARY:
        mclk_id = CLOCK_ID_MCLK_3;
        break;
    case INTF_IDX_QUATERNARY:
        mclk_id = CLOCK_ID_MCLK_4;
        break;
    case INTF_IDX_QUINARY:
        mclk_id = CLOCK_ID_QUI_MI2S_OSR; // 0x10A
        break;
    default:
        break;
    }
    return mclk_id;
}

static uint32_t ams_prm_alloc_packet(uint32_t payload_size, void **payload)
{
    struct ams_prm_header *prm_header = NULL;
    void *packet = NULL;

    packet = calloc(1, payload_size);
    if (!packet)
        return ENOMEM;
    prm_header = (struct ams_prm_header *)packet;
    prm_header->miid = PRM_MODULE_INSTANCE_ID;
    prm_header->param_id = 0;
    prm_header->param_size = payload_size - sizeof(struct ams_prm_header);
    prm_header->error_code = 0;

    *payload = packet;
    return 0;
}

static int32_t ams_prm_hw_rsc_custom_config(uint32_t cmd, void *payload, int32_t payload_size, void *buff, uint32_t *buff_size)
{
    int32_t rc = 0;
    struct apm_cmd_header_t *cmd_header;
    uint8_t *cmd_payload, *module_param;
    struct gpr_packet_t *send_pkt;

    ALOGD("%s:allocate pkt for pld sz %d", __func__, payload_size);
    ALOGD("%s:lock mutex", __func__);
    ams_osal_mutex_lock(ams_gpr_common_ctx.mutex);
    rc = ams_gpr_common_allocate_gpr_packet(cmd, GPR_IDS_DOMAIN_ID_APPS_V, GPR_IDS_DOMAIN_ID_ADSP_V,
                                            AMS_GPR_SRC_PORT, PRM_MODULE_INSTANCE_ID,
                                            sizeof(*cmd_header) + AMS_ALIGN_8BYTE(payload_size), 0, &send_pkt);

    if (rc)
    {
        ALOGE("Failed with err %d to allocate GPR packet of sz %d", rc, sizeof(*cmd_header) + AMS_ALIGN_8BYTE(payload_size));
        goto exit;
    }
    cmd_payload = GPR_PKT_GET_PAYLOAD(uint8_t, send_pkt);
    memset(cmd_payload, 0, sizeof(*cmd_header) + AMS_ALIGN_8BYTE(payload_size));

    cmd_header = (struct apm_cmd_header_t *)cmd_payload;
    cmd_header->payload_size = AMS_ALIGN_8BYTE(payload_size);
    module_param = cmd_payload + sizeof(*cmd_header);
    memcpy(module_param, (void *)payload,
           payload_size);
    *(uint32_t *)module_param = PRM_MODULE_INSTANCE_ID;

    if (buff)
    {
        ams_gpr_common_ctx.rsp_buff = buff;          // move this to ams_gpr_common_send_spf_cmd_wait_for_rsp
        ams_gpr_common_ctx.rsp_buff_sz = *buff_size; // move this to ams_gpr_common_send_spf_cmd_wait_for_rsp
    }

    rc = ams_gpr_common_send_spf_cmd_wait_for_rsp(send_pkt, ams_gpr_common_ctx.psig_obj, NULL);
    if (rc)
        ALOGE("hw rsc cmd 0x%x failed:%d", PRM_CMD_REQUEST_HW_RSC, rc);

    if (buff)
        *buff_size = ams_gpr_common_ctx.rsp_buff_sz; // move this to ams_gpr_common_send_spf_cmd_wait_for_rsp

exit:
    ams_osal_mutex_unlock(ams_gpr_common_ctx.mutex);
    ALOGD("%s:unlock mutex", __func__);

    return rc;
}

static int32_t ams_prm_request_hw_rsc_custom_config(void *payload, int32_t payload_size, void *buff, uint32_t *buff_size)
{
    return ams_prm_hw_rsc_custom_config(PRM_CMD_REQUEST_HW_RSC, payload, payload_size, buff, buff_size);
}

static int32_t ams_prm_release_hw_rsc_custom_config(void *payload, int32_t payload_size, void *buff, uint32_t *buff_size)
{
    return ams_prm_hw_rsc_custom_config(PRM_CMD_RELEASE_HW_RSC, payload, payload_size, buff, buff_size);
}

static int32_t ams_prm_virt_mixer_open(void)
{
    int32_t rc = 0;
    ams_prm_ctx.mixer = mixer_open(AMS_PRM_VIRT_SOUND_CARD_NUM);
    if (!ams_prm_ctx.mixer){
        ALOGE("Error when open mixer!");
        rc = EINVAL;
    }
    return rc;
}

static int32_t ams_prm_virt_mixer_close(void)
{
    int32_t rc = 0;
    if (ams_prm_ctx.mixer)
        mixer_close(ams_prm_ctx.mixer);
    return rc;
}

static struct mixer_ctl * ams_prm_virt_mixer_get_ctl(char *control)
{
   struct mixer *mixer = NULL;

    char *stream = "PCM";
    char *mixer_str = NULL;
    struct mixer_ctl *ctl = NULL;
    int ctl_len = 0,ret = 0;
    mixer = ams_prm_ctx.mixer;
    if (!mixer){
        ALOGE("Error when open mixer!");
        goto exit;
    }
    ctl_len = strlen(stream) + 4 + strlen(control) + 1;
    mixer_str = calloc(1, ctl_len);
    if (!mixer_str) {
        goto exit;
    }
    snprintf(mixer_str, ctl_len, "%s%d %s", stream, 117, control);
    ctl = mixer_get_ctl_by_name(mixer, mixer_str);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", mixer_str);
    }
exit:
    if (mixer_str)
        free(mixer_str);
    return ctl;
}
/**-------------------------------------------------------------------------*
 *! \brief    entry functions
 **-------------------------------------------------------------------------*/
// Called only once!
int32_t ams_prm_init(void)
{
    int32_t rc = 0;

    rc = ams_osal_mutex_create(&ams_prm_ctx.lock);
    if (rc)
    {
        ALOGE("failed create mutex %d", rc);
        rc = EAGAIN;
        goto prm_deinit;
    }
    //TODO: open mixer here
    if (ams_prm_virt_mixer_open()){
        ALOGE("Error opening virtual mixer!\n");
    }
    ALOGD("prm module initialized\n");
    return rc;
destroy_mutex:
    ams_osal_mutex_destroy(ams_prm_ctx.lock);
prm_deinit:

    return rc;
}
// Called only once!
int32_t ams_prm_deinit(void)
{
    ALOGD("prm module deinitialized\n");
    //TODO: close mixer here
    if (ams_prm_virt_mixer_close()){
        ALOGE("Error closing virtual mixer!\n");
    }
    ams_osal_mutex_destroy(ams_prm_ctx.lock);
    return 0;
}

int32_t ams_prm_dev_enable(struct aud_dev_obj *dev_obj)
{
    int32_t rc = 0;
    uint32_t mclk_id = 0;
    struct ams_prm_clk_enable *mclk = NULL;

    ALOGD("enable index %d mode %d\n", dev_obj->intf_index, dev_obj->intf_mode);

    if (!g_ams_mclk_enable[dev_obj->intf_index])
    {
        rc = 0;
        goto exit;
    }

    if (dev_obj->intf_mode != INTF_MODE_MI2S)
    {
        ALOGD("skip enable mclk for non-i2s interface!");
        rc = 0;
        goto exit;
    }
    if (dev_obj->intf_mode == INTF_MODE_MI2S)
    {
        if (dev_obj->intf_index == INTF_IDX_QUINARY)
        {
            ALOGD("skip enable mclk i2s quin interface!");
            return 0;
        }
    }
    mclk_id = ams_prm_get_mclk_id(dev_obj->intf_index);
    if (mclk_id == AMS_MLCK_INVALID)
    {
        rc = EINVAL;
        ALOGE("skip enable mclk: id invalid!");
        goto exit;
    }
    ALOGD("Start to enable MCLK");
    mclk = calloc(1, sizeof(struct ams_prm_clk_enable) + 1 * sizeof(struct ams_prm_clk_obj));
    if (!mclk)
    {
        ALOGE("cannot allocate mem for mclk!");
        rc = ENOMEM;
        goto exit;
    }
    mclk->num_clks = 1;
    mclk->clk_objs[0].id = mclk_id;

    if (dev_obj->intf_mode != INTF_MODE_MI2S)
    {
        ALOGD("clk 1.536MHz\n");
        mclk->clk_objs[0].freq = AMS_CLK_1_P536_MHZ;
    }
    else
    {
        ALOGD("clk 12.288MHz\n");
        mclk->clk_objs[0].freq = AMS_CLK_12_P288_MHZ;
    }
    mclk->clk_objs[0].attri = CLOCK_ATTRIBUTE_COUPLE_NO;
    mclk->clk_objs[0].root = CLOCK_ROOT_DEFAULT;
    rc = ams_prm_clk_enable(mclk);
    ALOGD("ams_prm_dev_enable: mclk %d enable ret [%d]!", mclk_id, rc);
    if (rc)
    {
        ALOGE("ams_prm_dev_enable: mclk %d enable failed [%d]!", mclk_id, rc);
    }
exit:
    if (mclk)
        free(mclk);
    return rc;
}

int32_t ams_prm_dev_disable(struct aud_dev_obj *dev_obj)
{
    int32_t rc = 0;
    uint32_t mclk_id = 0;
    struct ams_prm_clk_disable *mclk = NULL;
    ALOGD("disable index %d mode %d\n", dev_obj->intf_index, dev_obj->intf_mode);

    if (!g_ams_mclk_enable[dev_obj->intf_index])
    {
        ALOGD("Skip disable for % index", dev_obj->intf_index);
        rc = 0;
        goto exit;
    }

    if (dev_obj->intf_mode != INTF_MODE_MI2S)
    {
        ALOGD("skip disable mclk for non-i2s interface!");
        rc = 0;
        goto exit;
    }
    if (dev_obj->intf_mode == INTF_MODE_MI2S)
    {
        if (dev_obj->intf_index == INTF_IDX_QUINARY)
        {
            ALOGD("skip disable mclk i2s quin interface!");
            return 0;
        }
    }
    mclk_id = ams_prm_get_mclk_id(dev_obj->intf_index);
    if (mclk_id == AMS_MLCK_INVALID)
    {
        ALOGE("Skip disable mclk: id invalid");
        rc = EINVAL;
        goto exit;
    }
    ALOGD("Start to disable MCLK");

    mclk = calloc(1, sizeof(struct ams_prm_clk_disable) + 1 * sizeof(uint32_t));
    if (!mclk)
    {
        ALOGE("cannot allocate mem for mclk!");
        rc = ENOMEM;
        goto exit;
    }
    mclk->num_clks = 1;
    mclk->clk_ids[0] = mclk_id;
    rc = ams_prm_clk_disable(mclk);
    ALOGD("ams_prm_dev_disable: mclk %d disable ret [%d]!", mclk_id, rc);
    if (rc)
    {
        ALOGE("ams_prm_dev_disable: mclk %d disable failed [%d]!", mclk_id, rc);
    }
exit:
    if (mclk)
        free(mclk);

    return rc;
}

int32_t ams_prm_clk_enable(struct ams_prm_clk_enable *clks)
{
    int32_t rc = 0;
    audio_hw_clk_cfg_t *clk_enable = NULL;
    audio_hw_clk_cfg_req_param_t *clk_req;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    int32_t i;
    struct gpr_packet_t *send_pkt = NULL, *rcv_pkt = NULL;
    uint8_t *cmd_payload, *module_param;
    struct apm_cmd_header_t *cmd_header;

    ALOGD("Start to enable CLK:num clks %d", clks->num_clks);
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(audio_hw_clk_cfg_req_param_t) + clks->num_clks * sizeof(audio_hw_clk_cfg_t);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d for CLK!", payload_size);
        goto exit;
    }
    ((struct ams_prm_header *)payload)->param_id = PARAM_ID_RSC_AUDIO_HW_CLK;
    clk_req = (audio_hw_clk_cfg_req_param_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    clk_req->num_clock_id = clks->num_clks;
    ALOGE("clk req:num %d", clk_req->num_clock_id);
    clk_enable = ++clk_req;
    for (i = 0; i < clks->num_clks; i++)
    {
        clk_enable[i].clock_id = clks->clk_objs[i].id;
        clk_enable[i].clock_freq = clks->clk_objs[i].freq;
        clk_enable[i].clock_attri = clks->clk_objs[i].attri;
        clk_enable[i].clock_root = clks->clk_objs[i].root;
    }
    ALOGD("clk req:id %d", clk_enable[0].clock_id);
    ALOGD("clk req:freq %d", clk_enable[0].clock_freq);
    ALOGD("clk req:attri %d", clk_enable[0].clock_attri);
    ALOGD("clk req:root %d", clk_enable[0].clock_root);
#ifdef AMS_AGM_HW_RSC_CONFIG_EN

    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REQ, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
        rc = EINVAL;
        goto exit;
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscReq";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin get ret %d",rc);

#else
    rc = ams_prm_request_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (rc)
        ALOGE("Failed to request clk!");
exit:
    if (payload)
        free(payload);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

int32_t ams_prm_clk_disable(struct ams_prm_clk_disable *clks)
{
    int32_t rc = 0;
    audio_hw_clk_rel_cfg_t *clk_disable = NULL;
    audio_hw_clk_cfg_req_param_t *clk_req;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    int32_t i;

    ALOGD("Start to disable CLK");
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(audio_hw_clk_cfg_req_param_t) + clks->num_clks * sizeof(audio_hw_clk_rel_cfg_t);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d!", payload_size);
        goto exit;
    }
    ((struct ams_prm_header *)payload)->param_id = PARAM_ID_RSC_AUDIO_HW_CLK;
    clk_req = (audio_hw_clk_cfg_req_param_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    clk_req->num_clock_id = clks->num_clks;
    clk_disable = ++clk_req;
    for (i = 0; i < clks->num_clks; i++)
    {
        clk_disable[i].clock_id = clks->clk_ids[0];
    }
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REL, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscRel";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin get ret %d",rc);
#else
    rc = ams_prm_release_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (rc)
        ALOGE("Failed to release clk!");
exit:
    if (payload)
        free(payload);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

int32_t ams_prm_dma_request(enum dma_dir_type type, struct ams_prm_dma_rsc *dma_rsc)
{
    int32_t rc = 0;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    dma_rsc_t *dma_req = NULL, *dma_rsp = NULL;
    int32_t i;

    ALOGD("Start DMA request");
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(dma_rsc_t) + dma_rsc->num_dma_channels * sizeof(uint32_t);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d!", payload_size);
        goto exit;
    }
    buff_size = sizeof(struct prm_cmd_rsp_hw_rsc_cfg_t) + sizeof(dma_rsc_t) + dma_rsc->num_dma_channels * sizeof(uint32_t); // TODO:check if enough
    buff = calloc(1, buff_size);
    ((struct ams_prm_header *)payload)->param_id = (type == RD_DMA) ? PARAM_ID_RSC_DMA_READ_CHANNEL : PARAM_ID_RSC_DMA_WRITE_CHANNEL;
    dma_req = (dma_rsc_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    dma_req->num_dma_channels = dma_rsc->num_dma_channels;
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REQ, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscReq";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, buff, buff_size);
    ALOGD("AGM mixer plugin get ret %d",rc);
#else
    rc = ams_prm_request_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (rc)
    {
        ALOGE("Failed to request DMA res!");
    }
    else
    {
        dma_rsp = ((uint8_t *)buff + sizeof(struct prm_cmd_rsp_hw_rsc_cfg_t));
        uint32_t *dma_idxs = ++dma_rsp;
        for (i = 0; i < dma_rsc->num_dma_channels; i++)
        {
            dma_rsc->dma_idx[i] = dma_idxs[i];
        }
    }
exit:
    if (payload)
        free(payload);
    free(buff);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

int32_t ams_prm_dma_release(enum dma_dir_type type, struct ams_prm_dma_rsc *dma_rsc)
{
    int32_t rc = 0;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    dma_rsc_t *dma_req = NULL;
    int32_t i;

    ALOGD("Start DMA release");
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(dma_rsc_t) + dma_rsc->num_dma_channels * sizeof(uint32_t);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d!", payload_size);
        goto exit;
    }

    ((struct ams_prm_header *)payload)->param_id = (type == RD_DMA) ? PARAM_ID_RSC_DMA_READ_CHANNEL : PARAM_ID_RSC_DMA_WRITE_CHANNEL;
    dma_req = (dma_rsc_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    dma_req->num_dma_channels = dma_rsc->num_dma_channels;
    uint32_t *dma_idxs = ++dma_req;
    for (i = 0; i < dma_rsc->num_dma_channels; i++)
    {
        dma_idxs[i] = dma_rsc->dma_idx[i];
    }
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REL, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscRel";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin get ret %d",rc);
#else
    rc = ams_prm_release_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (rc)
        ALOGE("Failed to release DMA res!");
exit:
    if (payload)
        free(payload);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

int32_t ams_prm_lpm_request(struct ams_prm_lpm_rsc *lpm_rsc)
{
    int32_t rc = 0;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    lpm_rsc_request_t *lpm_req = NULL;
    lpm_rsp_rsc_request_t *lpm_req_rsp;

    ALOGD("Start LPM request");
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(lpm_rsc_request_t);
    buff_size = sizeof(struct prm_cmd_rsp_hw_rsc_cfg_t) + sizeof(lpm_rsp_rsc_request_t); // TODO: check which size is needed in response
    buff = calloc(1, buff_size);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d!", payload_size);
        goto exit;
    }

    ((struct ams_prm_header *)payload)->param_id = PARAM_ID_RSC_LOW_PWR_MEM;
    lpm_req = (lpm_rsc_request_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    lpm_req->lpm_type = lpm_rsc->lpm_type;
    lpm_req->size_in_bytes = lpm_rsc->size_in_bytes;
    ALOGE("LPM req buf size %d!", buff_size);
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REQ, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscReq";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, buff, buff_size);
    ALOGD("AGM mixer plugin get ret %d",rc);
#else
    rc = ams_prm_request_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (!rc)
    {
        lpm_req_rsp = ((uint8_t *)buff + sizeof(struct prm_cmd_rsp_hw_rsc_cfg_t));
        lpm_rsc->lpm_type = lpm_req_rsp->lpm_type;
        lpm_rsc->size_in_bytes = lpm_req_rsp->size_in_bytes;
        lpm_rsc->phy_addr_lsw = lpm_req_rsp->phy_addr_lsw;
        lpm_rsc->phy_addr_msw = lpm_req_rsp->phy_addr_msw;
    }
    else
        ALOGE("Failed to request LPM rsc!");
exit:
    if (payload)
        free(payload);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

int32_t ams_prm_lpm_release(struct ams_prm_lpm_rsc *lpm_rsc)
{
    int32_t rc = 0;
    void *payload = NULL;
    size_t payload_size = 0;
    void *buff = NULL;
    uint32_t buff_size = 0;
    lpm_rsc_release_t *lpm_rel = NULL;

    ALOGD("Start LPM release");
    ams_osal_mutex_lock(ams_prm_ctx.lock);
    payload_size = sizeof(struct ams_prm_header) + sizeof(lpm_rsc_release_t);
    rc = ams_prm_alloc_packet(payload_size, &payload);
    if (rc != 0)
    {
        ALOGE("Error when allocating packet of size %d!", payload_size);
        goto exit;
    }

    ((struct ams_prm_header *)payload)->param_id = PARAM_ID_RSC_LOW_PWR_MEM;
    lpm_rel = (lpm_rsc_request_t *)((uint8_t *)payload + sizeof(struct ams_prm_header));
    lpm_rel->lpm_type = lpm_rsc->lpm_type;
    lpm_rel->size_in_bytes = lpm_rsc->size_in_bytes;
    lpm_rel->phy_addr_lsw = lpm_rsc->phy_addr_lsw;
    lpm_rel->phy_addr_msw = lpm_rsc->phy_addr_msw;
#ifdef AMS_AGM_HW_RSC_CONFIG_EN
    rc = agm_hw_rsc_config(AGM_HW_CONFIQ_REL, payload, payload_size, buff, &buff_size);
    if (rc){
        ALOGE("Failed to use AGM API!");
    }
#elif AMS_AGM_HW_RSC_CONFIG_V2_EN

    char *control = "hwRscRel";
    struct mixer_ctl *ctl = NULL;

    ctl = ams_prm_virt_mixer_get_ctl(control);
    if (!ctl) {
        ALOGE("Invalid mixer control: %s\n", control);
        rc = EINVAL;
        goto exit;
    }
    rc = mixer_ctl_set_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin set ret %d",rc);
    rc = mixer_ctl_get_array(ctl, payload, payload_size);
    ALOGD("AGM mixer plugin get ret %d",rc);
#else
    rc = ams_prm_release_hw_rsc_custom_config(payload, payload_size, buff, &buff_size);
#endif
    if (rc)
        ALOGE("Failed to release LPM rsc!");
exit:
    if (payload)
        free(payload);
    ams_osal_mutex_unlock(ams_prm_ctx.lock);
    return rc;
}

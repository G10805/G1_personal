/**
 * Copyright (c) 2023 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "drm_sample_app.h"
#include <BufferAllocator/BufferAllocatorWrapper.h>
#include <linux/dma-buf.h>
#include <linux/qcedev.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <utils/Log.h>
#include <vmmem_wrapper.h>
#include <cstdint>
#include <iostream>
#include "CAppLoader.h"
#include "IAppLoader.h"
#include "IClientEnv.h"
#include "IDRMSample.h"
#include "TZCom.h"

using namespace std;

typedef class BufferAllocator BufferAllocator;

#define SIZE_4K 4096

static Object g_app_obj = Object_NULL;
static Object g_app_controller = Object_NULL;

typedef struct {
    int32_t ifd_data_fd;
    unsigned char *ion_sbuffer;
    uint32_t sbuf_len;
    int32_t qcedev_fd;
    uint64_t ce_vaddr;
    char *heap;
    bool is_secure;
    bool is_tzva;
    Object mem_obj;
} drm_buf_t;

/******************************************************************************
 *                         Local Functions
 *****************************************************************************/

/*
 * Description: FSP Client help
 *
 * In:          void
 * Out:         void
 * Return:      void
 */
static void drm_sample_help_usage(void)
{
    printf(
        "**********************************************************************"
        "**********\n");
    printf(
        "****                      Sample DRM CLIENT USAGE                     "
        "      ****\n");
    printf(
        "**********************************************************************"
        "**********\n");
    printf(
        " Usage - drm_sample_app -c <cmd_id> -ta <ta location> -h\n"
        " cmd_id:\n"
        " Run all tests       - 1 | Crypto SMMU VA test   - 2 | Crypto TZ VA "
        "test   - 3\n"
        " SFS test            - 4 | RPMB test             - 5 | HDCP test      "
        "     - 6\n"
        " License test with CPZ license    - 7  *license path + license name* "
        "\n"
        " License test without CPZ license - 8  *serial num of license to be "
        "removed* *license path + license name* \n"
        " Example to run all tests: drm_sample_app -c 1 -ta "
        "/vendor/firmware_mnt/image/drmsampleapp.mbn"
        "----------------------------------------------------------------------"
        "---------\n\n");
}

/*
 * Description: API to get a file size
 *
 * In:          file_name  :  File name
 * Out:         size       :  Size of the file
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
static int get_file_size(const char *file_name, size_t &size)
{
    FILE *file = NULL;
    int ret = 0;

    file = fopen(file_name, "r");
    CHECK_COND_ERR((file != NULL), DRM_FAILURE);

    GUARD(fseek(file, 0L, SEEK_END));

    size = ftell(file);
    CHECK_COND_ERR((size != -1), DRM_FAILURE);

exit:
    if (file) {
        fclose(file);
    }
    return ret;
}

/*
 * Description: Read and fill a buffer with file contents
 *
 * In:          file_name   : Name of the file
 * In:          size        : Size of the file
 * Out:         buffer      : buffer containg the file
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
static int read_file(const char *file_name, size_t size, uint8_t *buffer)
{
    FILE *file = NULL;
    size_t read_bytes = 0;
    int ret = DRM_SUCCESS;

    file = fopen(file_name, "r");
    CHECK_COND_ERR((file != NULL), DRM_FAILURE);
    read_bytes = fread(buffer, 1, size, file);
    CHECK_COND_ERR((read_bytes == size), DRM_FAILURE);

exit:
    if (file) {
        fclose(file);
    }
    return ret;
}

/*
 * Description: Load app from buffer
 *
 * In:          app_path   : Path to the app on device
 * Out:         void
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
static int32_t load_app_from_buffer(Object app_loader, const char *app_path)
{
    int32_t ret = DRM_SUCCESS;
    size_t size = 0;
    uint8_t *buffer = NULL;

    GUARD(get_file_size(app_path, size));
    CHECK_COND_ERR((size > 0), DRM_FAILURE);

    buffer = (uint8_t *)malloc(sizeof(uint8_t[size]));
    CHECK_COND_ERR((buffer != NULL), DRM_FAILURE);

    GUARD(read_file(app_path, size, buffer));
    ALOGI("load %s, size %zu, buffer %p\n", app_path, size, buffer);

    GUARD(
        IAppLoader_loadFromBuffer(app_loader, buffer, size, &g_app_controller));

exit:
    if (buffer) free(buffer);

    if (!Object_isERROR(ret) && !Object_isNull(g_app_controller)) {
        ALOGI("Load app %s succeeded\n", app_path);
    } else {
        ALOGE("Load app %s failed: %d\n", app_path, ret);
        if (Object_isNull(g_app_controller)) {
            ALOGE("g_app_controller is NULL!\n");
            ret = DRM_FAILURE;
        }
    }
    return ret;
}

/*
 * Description: Load TA using SMCInvoke
 *
 * In:          app_path   : Path to the app on device
 * Out:         void
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
static int32_t load_ta_with_smc_invoke(const char *app_path)
{
    int32_t ret = DRM_SUCCESS;
    Object client_env = Object_NULL;
    Object app_loader = Object_NULL;

    GUARD(TZCom_getClientEnvObject(&client_env));
    GUARD(IClientEnv_open(client_env, CAppLoader_UID, &app_loader));
    GUARD(load_app_from_buffer(app_loader, app_path));
    GUARD(IAppController_getAppObject(g_app_controller, &g_app_obj));
    CHECK_COND_ERR(!Object_isNull(g_app_obj), Object_ERROR);

exit:
    Object_ASSIGN_NULL(app_loader);
    Object_ASSIGN_NULL(client_env);
    return ret;
}

/*
 * Description: Unload TA that uses SMC Invoke.
 *
 * In:          void
 * Out:         void
 * Return:      on success :- Object_OK
 *              on failure :- Object_ERROR
 */

static int32_t unload_ta_with_smc_invoke()
{
    int32_t ret = Object_OK;

    if (Object_isNull(g_app_obj)) goto exit;

    GUARD(IAppController_unload(g_app_controller));

    ALOGI("Unloaded drmsampleapp successfully");

exit:
    Object_ASSIGN_NULL(g_app_obj);
    Object_ASSIGN_NULL(g_app_controller);
    return ret;
}

/*
 * Description: Modify VMs that have access to a dma buf.
 *              In this implementation we will make our buffer secure
 *              by lending to the cp_bitstream heap.
 *
 * In:          fd         :  Buffer fd
 * Out:         void
 * Return:      on success :- Object_OK
 *              on failure :- Any other error code
 */
static int32_t cp_lend_dma_buf(int32_t fd)
{
    int32_t ret = DRM_SUCCESS;
    VmMem *vmmem = NULL;
    VmHandle vm_handle = 0;
    char *heap = (char *)"qcom,cp_bitstream";
    bool is_exclusive_owner = false;
    VmHandle vm_handle_arr[1] = {0};
    uint32_t perm_arr[1] = {VMMEM_READ | VMMEM_WRITE};

    GUARD(IsExclusiveOwnerDmabuf(fd, &is_exclusive_owner));

    if (is_exclusive_owner == false) {
        /* Lend the buffer only if it is non-secure,
         * else do nothing, since the buffer is already lent,
         * and we exit from the function with a success
         * return code
         */
        goto exit;
    }

    vmmem = CreateVmMem();
    if (vmmem == NULL) {
        ALOGE("Error: CreateVmMem failed, errno = %x", errno);
        goto exit;
    }

    vm_handle = FindVmByName(vmmem, heap);
    if (vm_handle < 0) {
        ALOGE("Error: FindVmByName failed, errno = %x", errno);
        goto exit;
    }
    vm_handle_arr[0] = vm_handle;

    /* This is a workaround for the TZ busy issues which
     * cause lend operation to fail and subsequently the
     * secure usecase to fail as well
     */
    for (int i = 1; i <= 5; i++) {
        ret = LendDmabuf(vmmem, fd, vm_handle_arr, perm_arr,
                         C_LENGTHOF(vm_handle_arr));
        if (ret < 0) {
            ALOGE("Error: LendDmabuf failed %d time", i);
            if (i == 5) {
                ALOGE("LendDmabuf failed 5 times, goto exit");
                ERROR(-1);
            }
            continue;
        } else if (ret == 0) {
            ALOGI("LendDmabuf succeeded %d time", i);
            break;
        }
    }

exit:
    if (vmmem != NULL) {
        FreeVmMem(vmmem);
    }
    return ret;
}

/*
 * Description: API to allocate and map a buffer based on the given heap.
 *              Secure buffers are not mapped.
 *
 * In/Out:      handle     :  buffer handle
 * In:          size       :  buffer size
 * In:          heap       :  buffer heap
 * In:          is_secure  :  secure flag
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
static int32_t buffer_alloc(drm_buf_t *handle, uint32_t size, char *heap,
                            bool is_secure)
{
    int32_t ret = DRM_SUCCESS;
    int32_t map_fd = -1;

    unsigned char *v_addr = NULL;
    uint32_t alligned_size = (size + 4095) & (~4095);
    uint32_t align = 0;
    uint32_t flags = 0;

    struct dma_buf_sync buf_sync = {0};
    BufferAllocator *buffer_allocator = NULL;

    do {
        buffer_allocator = CreateDmabufHeapBufferAllocator();

        if (buffer_allocator == NULL) {
            ALOGE("CreateDmabufHeapBufferAllocator() failed.\n");
            ret = DRM_FAILURE;
            break;
        }

        handle->ion_sbuffer = NULL;
        handle->ifd_data_fd = 0;

        map_fd = DmabufHeapAlloc(buffer_allocator, heap, size, flags, 0);

        if (map_fd < 0) {
            ALOGE("failed to allocate memory %d", map_fd);
            ret = DRM_FAILURE;
            break;
        }

        if (!is_secure) {
            v_addr = (unsigned char *)mmap(NULL, alligned_size,
                                           PROT_READ | PROT_WRITE, MAP_SHARED,
                                           map_fd, 0);
            if (v_addr == MAP_FAILED) {
                ALOGE("mmap failed. errno=%d %s", errno, strerror(errno));
                ret = DRM_FAILURE;
                break;
            }

            buf_sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
            ret = ioctl(map_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
            if (ret) {
                ALOGE("ioctl DMA_BUF_IOCTL_SYNC failed. errno=%d %s", errno,
                      strerror(errno));
                ret = DRM_FAILURE;
                break;
            }
        }

        handle->ifd_data_fd = map_fd;
        handle->ion_sbuffer = v_addr;
        handle->sbuf_len = size;

    } while (0);

    if (buffer_allocator != NULL) {
        FreeDmabufHeapBufferAllocator(buffer_allocator);
    }
    if (ret) {
        if (v_addr != MAP_FAILED) {
            munmap(v_addr, alligned_size);
            handle->ion_sbuffer = NULL;
        }

        if (map_fd >= 0) {
            close(map_fd);
            handle->ifd_data_fd = -1;
        }
    }

    return ret;
}

/*
 * Description: API to de-allocate and unmap a buffer.
 *
 * In/Out:      handle     :  buffer handle
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
static int32_t buffer_dealloc(drm_buf_t *handle)
{
    struct dma_buf_sync buf_sync = {0};
    uint32_t alligned_size = (handle->sbuf_len + 4095) & (~4095);
    int32_t ret = 0;

    buf_sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
    ret = ioctl(handle->ifd_data_fd, DMA_BUF_IOCTL_SYNC, &buf_sync);
    if (ret) {
        ALOGE("DMA_BUF_IOCTL_SYNC start failed. errno=%d %s", errno,
              strerror(errno));
    }

    if (handle->ion_sbuffer) {
        munmap(handle->ion_sbuffer, alligned_size);
        handle->ion_sbuffer = NULL;
    }

    if (handle->ifd_data_fd >= 0) {
        close(handle->ifd_data_fd);
        handle->ifd_data_fd = -1;
    }

    return ret;
}

/*
 * Description: Map memory in qcedev to populate the SMMMU VA.
 *
 * In/Out:      handle     :  buffer handle
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
int32_t buffer_qcedev_map(drm_buf_t *handle)
{
    int32_t ret = DRM_SUCCESS;
    int32_t qcedev_fd = -1;
    struct qcedev_map_buf_req map;

    if (handle == NULL) {
        printf("%s: Error:: null handle received\n", __func__);
        return -1;
    }

    handle->qcedev_fd = -1;
    handle->ce_vaddr = 0;

    qcedev_fd = open("/dev/qce", O_RDONLY);
    if (qcedev_fd < 0) {
        printf("%s: Error:: Cannot open QCEDEV device\n", __func__);
        return -1;
    }
    handle->qcedev_fd = qcedev_fd;

    map.fd[0] = handle->ifd_data_fd;
    map.num_fds = 1;
    map.fd_size[0] = handle->sbuf_len;
    map.fd_offset[0] = 0;

    /* Map the ion buffer in crypto driver */
    ret = ioctl(qcedev_fd, QCEDEV_IOCTL_MAP_BUF_REQ, &map);
    if (ret) {
        printf(
            "%s: Error:: Failed to map ion buf in QCEDEV_IOCTL_MAP_BUF_REQ "
            "call\n",
            __func__);
        goto map_fail;
    }
    handle->ce_vaddr = map.buf_vaddr[0];
    return ret;

map_fail:
    if (handle->qcedev_fd >= 0) {
        close(handle->qcedev_fd);
    }
    return ret;
}

/*
 * Description: Unmap memory in qcedev.
 *
 * In/Out:      handle     :  buffer handle
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
int32_t buffer_qcedev_unmap(drm_buf_t *handle)
{
    struct qcedev_unmap_buf_req unmap;
    int32_t ret = 0;

    unmap.fd[0] = handle->ifd_data_fd;
    unmap.num_fds = 1;

    if (handle->ce_vaddr) {
        /* Unmap the ion buffer in crypto driver */
        ret = ioctl(handle->qcedev_fd, QCEDEV_IOCTL_UNMAP_BUF_REQ, &unmap);
        if (ret) {
            printf(
                "%s: Error:: Unmapping ION Buffer failed in Crypto driver with "
                "ret = %d\n",
                __func__, ret);
            return -1;
        }
        close(handle->qcedev_fd);
    }

    handle->qcedev_fd = -1;
    handle->ce_vaddr = 0;
    return ret;
}

/*
 * Description: API to prepare buffers and call the crypto interface
 *              of the sample DRM implementation
 *
 * In:          crypto_data  :  crypto information for the cipher
 * In:          in_buf_info  :  input buffer information
 * In:          out_buf_info :  output buffer information
 * In:          is_secure    :  secure flag
 * Return:      on success   :- DRM_SUCCESS
 *              on failure   :- Any other error code
 */
int32_t drm_crypto_op(IDRMCryptoInfo *crypto_data, drm_buf_t *in_buf_info,
                      drm_buf_t *out_buf_info)
{
    int32_t ret = DRM_SUCCESS;
    uint64_t size = SIZE_4K;
    uint8_t *pattern_mem = NULL;
    IDRMBufferInfo input_buf = {0, 0, 0}, output_buf = {0, 0, 0};

    /* Check uniformity of buffer handling */
    CHECK_COND_ERR((in_buf_info->is_tzva == out_buf_info->is_tzva),
                   Object_ERROR);

    /* Generate data to be decrypted */
    pattern_mem = (uint8_t *)malloc(size);
    CHECK_COND(!(pattern_mem == NULL));
    memset(pattern_mem, 0xA5, size);

    /* Allocate buffers */
    GUARD(buffer_alloc(in_buf_info, size, in_buf_info->heap,
                       in_buf_info->is_secure));
    GUARD(buffer_alloc(out_buf_info, size, out_buf_info->heap,
                       out_buf_info->is_secure));

    if (in_buf_info->is_secure) {
        /* Prepare the secure input buffer */
        cp_lend_dma_buf(in_buf_info->ifd_data_fd);
    } else {
        memcpy(in_buf_info->ion_sbuffer, pattern_mem, size);
    }

    if (out_buf_info->is_secure) {
        /* Prepare the secure output buffer */
        cp_lend_dma_buf(out_buf_info->ifd_data_fd);
    } else {
        memset(out_buf_info->ion_sbuffer, 0, size);
    }

    if (in_buf_info->is_tzva) {
        /* Wrap fd to get memory objects for the buffers */
        TZCom_getFdObject(in_buf_info->ifd_data_fd, &in_buf_info->mem_obj);
        GUARD(Object_isNull(in_buf_info->mem_obj));

        TZCom_getFdObject(out_buf_info->ifd_data_fd, &out_buf_info->mem_obj);
        GUARD(Object_isNull(out_buf_info->mem_obj));
    } else {
        /* Map the buffers to get the Crypto SMMU VA */
        GUARD(buffer_qcedev_map(in_buf_info));
        GUARD(buffer_qcedev_map(out_buf_info));
    }

    /* Populate input and output buffer info */
    input_buf.va = in_buf_info->ce_vaddr;
    input_buf.size = size;
    input_buf.secure = in_buf_info->is_secure;

    output_buf.va = out_buf_info->ce_vaddr;
    output_buf.size = size;
    output_buf.secure = out_buf_info->is_secure;

    GUARD(IDRMSample_Crypto(g_app_obj, crypto_data, &input_buf, &output_buf,
                            in_buf_info->mem_obj, out_buf_info->mem_obj));

exit:
    GUARD(buffer_qcedev_unmap(out_buf_info));
    GUARD(buffer_qcedev_unmap(in_buf_info));
    GUARD(buffer_dealloc(out_buf_info));
    GUARD(buffer_dealloc(in_buf_info));
    Object_RELEASE_IF(in_buf_info->mem_obj);
    Object_RELEASE_IF(out_buf_info->mem_obj);
    if (pattern_mem) {
        free(pattern_mem);
        pattern_mem = NULL;
    }
    return ret;
}

/*
 * Description: SMMU VA buffer test helper.
 *
 * In/Out:      void
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
int32_t crypto_smmu_va(void)
{
    int32_t ret = DRM_SUCCESS;
    IDRMCryptoInfo crypto_data = {DRM_CIPHER_OP_DECRYPT,
                                  DRM_ENCRYPTION_TYPE_CBCS};
    drm_buf_t in_buf_info = {
        0, NULL, 0, 0, 0, (char *)"qcom,qseecom", false, false, Object_NULL};
    drm_buf_t out_buf_info = {
        0, NULL, 0, 0, 0, (char *)"qcom,system", true, false, Object_NULL};

    GUARD(drm_crypto_op(&crypto_data, &in_buf_info, &out_buf_info));

exit:
    return ret;
}

/*
 * Description: TA VA buffer test helper.
 *
 * In/Out:      void
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
int32_t crypto_tz_va(void)
{
    int32_t ret = DRM_SUCCESS;
    IDRMCryptoInfo crypto_data = {DRM_CIPHER_OP_DECRYPT,
                                  DRM_ENCRYPTION_TYPE_CENC};
    drm_buf_t in_buf_info = {
        0, NULL, 0, 0, 0, (char *)"qcom,qseecom", false, true, Object_NULL};
    drm_buf_t out_buf_info = {
        0, NULL, 0, 0, 0, (char *)"qcom,qseecom", false, true, Object_NULL};

    GUARD(drm_crypto_op(&crypto_data, &in_buf_info, &out_buf_info));

exit:
    return ret;
}

/*
 * Description: Helper function to check if CPZ license is present.
 *
 * In:          file_path : CPZ license path
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
int32_t is_cpz_license_present(const char *file_path)
{
    int result = 0;
    std::string check_file_exists = "test -e ";
    check_file_exists += file_path;
    result = system(check_file_exists.c_str());
    if (result == 0) {
        /* Return success if license is present */
        return DRM_SUCCESS;
    }
    printf("CPZ license not present, Push license \n");
    /* Return failure if license is not present */
    return DRM_FAILURE;
}

/*
 * Description: Helper function to remove CPZ license.
 *
 * In:          file_path : CPZ license path
 *              serial_num : Serial num of license to be removed
 *
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- DRM_FAILURE
 */
int32_t remove_cpz_license_if_present(const char *file_path,
                                      const char *serial_num)
{
    string str = "/vendor/bin/qwes_cli_ndk -s  ";
    std::string check_file_exists = "test -e ";
    std::string remove_cpz_license = "rm ";
    const char *qwes_remove_license;
    int result = 0;

    check_file_exists += file_path;
    remove_cpz_license += file_path;
    str = str + serial_num + " " + "remove";
    qwes_remove_license = str.c_str();

    result = system(check_file_exists.c_str());
    if (result == 0) {
        system(remove_cpz_license.c_str());
        system(qwes_remove_license);
        printf("License is now removed, Now reboot the device \n");
        return DRM_FAILURE;
    }
    return DRM_SUCCESS;
}

/*
 * Description: Helper function to run all the sample DRM tests.
 *
 * In/Out:      void
 * Return:      on success :- DRM_SUCCESS
 *              on failure :- Any other error code
 */
int32_t run_all__drm_tests(void)
{
    int ret = DRM_SUCCESS;

    GUARD(crypto_smmu_va());
    GUARD(crypto_tz_va());
    GUARD(IDRMSample_SFSTest(g_app_obj));
    GUARD(IDRMSample_RPMBTest(g_app_obj));
    GUARD(IDRMSample_HDCPEnforcementTest(g_app_obj));
    printf("All tests passed\n");
exit:
    return ret;
}

/******************************************************************************
 *                   DRM Sample App Implementation
 *****************************************************************************/
int main(int argc, char *argv[])
{
    int32_t ret = DRM_SUCCESS;
    int32_t cmd_id = -1;
    const char *serial_num = "";
    const char *file_path = "";
    const char *app_path = "";

    printf("-------------DRM Sample Client------------\n");

    if (argc < 2 || ((strcmp(argv[1], "-h") != 0) && argc < 5)) {
        printf("Please verify the arguments to be processed, exiting! \n");
        drm_sample_help_usage();
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        if ((strcmp(argv[i], "-c") == 0) && ((i + 1) < argc)) {
            i++;
            cmd_id = atoi(argv[i]);
            if (cmd_id == 7) {
                if (argv[3] == NULL) {
                    drm_sample_help_usage();
                    goto exit;
                }
                file_path = argv[3];
                i += 1;
            } else if (cmd_id == 8) {
                if (argv[3] == NULL || argv[4] == NULL) {
                    drm_sample_help_usage();
                    goto exit;
                }
                serial_num = argv[3];
                file_path = argv[4];
                i += 3;
            }
        } else if ((strcmp(argv[i], "-ta") == 0) && ((i + 1) < argc)) {
            i++;
            app_path = argv[i];
        } else if (strcmp(argv[i], "-h") == 0) {
            drm_sample_help_usage();
            goto exit;
        } else {
            drm_sample_help_usage();
            goto exit;
        }
    }

    GUARD(load_ta_with_smc_invoke(app_path));

    switch (cmd_id) {
        case DRM_SAMPLE_ALL:
            GUARD(run_all__drm_tests());
            break;
        case DRM_SAMPLE_SMMU_VA:
            GUARD(crypto_smmu_va());
            printf("Crypto SMMU VA test passed\n");
            break;
        case DRM_SAMPLE_TZ_VA:
            GUARD(crypto_tz_va());
            printf("Crypto TZ VA test passed\n");
            break;
        case DRM_SAMPLE_SFS:
            GUARD(IDRMSample_SFSTest(g_app_obj));
            printf("SFS test passed\n");
            break;
        case DRM_SAMPLE_RPMB:
            GUARD(IDRMSample_RPMBTest(g_app_obj));
            printf("RPMB test passed\n");
            break;
        case DRM_SAMPLE_HDCP:
            GUARD(IDRMSample_HDCPEnforcementTest(g_app_obj));
            printf("HDCP test passed\n");
            break;
        case DRM_SAMPLE_ACCESS_WITH_CPZ_LICENSE:
            GUARD(is_cpz_license_present(file_path));
            GUARD(IDRMSample_CPZLicenseTest(g_app_obj));
            printf("CPZ test passed with License \n");
            break;
        case DRM_SAMPLE_ACCESS_WITHOUT_CPZ_LICENSE:
            GUARD(remove_cpz_license_if_present(file_path, serial_num));
            ret = IDRMSample_CPZLicenseTest(g_app_obj);
            if (ret == IDRMSample_ERROR_LICENSE_NOT_FOUND) {
                printf("CPZ test failed as expected without license \n");
                ret = DRM_SUCCESS;
            } else {
                printf("License removed, Please reboot device \n");
                ret = DRM_FAILURE;
            }
            break;
        default:
            printf("Please enter a valid cmd_id...\n");
            drm_sample_help_usage();
    }

exit:
    GUARD(unload_ta_with_smc_invoke());
    if (ret) {
        printf("DRM Test FAILED.\n");
    }
    printf("------------------------------------------\n");
    return ret;
}

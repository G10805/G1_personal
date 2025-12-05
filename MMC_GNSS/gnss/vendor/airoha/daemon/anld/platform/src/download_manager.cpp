/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
 * its licensors. Without the prior written permission of Airoha and/or its
 * licensors, any reproduction, modification, use or disclosure of Airoha
 * Software, and information contained herein, in whole or in part, shall be
 * strictly prohibited. You may only use, reproduce, modify, or distribute (as
 * applicable) Airoha Software if you have agreed to and been bound by the
 * applicable license agreement with Airoha ("License Agreement") and been
 * granted explicit permission to do so within the License Agreement ("Permitted
 * User").  If you are not a Permitted User, please cease any access or use of
 * Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
 * UNEQUIVOCALLY ACKNOWLEDGES AND AGREES THAT AIROHA SOFTWARE RECEIVED FROM
 * AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN "AS-IS"
 * BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
 * ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
 * SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
 * AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND ANY
 * SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR SUCH
 * AIROHA SOFTWARE AT ISSUE.
 */
#define LOG_TAG "DLM"
#include "download_manager.h"
#include <assert.h>
#include <inttypes.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include "gpio/GPIOControl.h"
#include "simulation.h"
#include "wakelock/air_wakelock.h"
using airoha::WakeLock;
// #define SKIP_DOWNLOAD_PROGRESS
// #define AIROHA_PM_LOCK_SYSTEM
#define BACKUP_FILE_PERMISSION (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define MAX_SLAVE_PACKET_NUM 4
#define HANDSHAKE_SINGLE_ROUND_TIMEOUT_MS 5000
#define DELAY_WHEN_DOWNLOAD_FINISH
static struct flashInfo AG3335_Flash_Info = {
    0x08000000,
    400000,
};
std::string DownloadManager::sFwBackupPath = "";
#ifdef DA_USE_RACE_CMD
uint32_t DownloadManager::sCRC32Table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
    0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
    0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
    0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
    0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
    0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
    0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
    0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
    0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
    0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
    0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
    0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
    0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
    0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
    0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
    0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
    0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
    0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
    0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
    0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
    0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
    0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};
#endif
// Condition Check
#if defined(SPEED_UP_DOWNLOAD) && !defined(DA_USE_RACE_CMD)
#error "Speed up download should work base on Race cmd"
#endif
#if defined(LOW_BAUD) && defined(UART_SPEED_UP_TO_3M)
#error "LOW_BAUD and 3M should not exist at the same time"
#endif
#if defined(UART_SPEED_UP_TO_3M) && !defined(UART_ENABLE_SOFTWARE_FLOWCONTROL)
// #error "3M baudrate should use flowcontrol"
#endif
void portTaskDelayMs(pTime_t ms) {
#ifdef WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
    return;
}
#define CHECK_BOOL_AND_NOTIFY_RETURN(x, r1, r2, tell) \
    {                                                 \
        if (!x) {                                     \
            LOG_D(tell);                              \
            TRACE_D(tell);                            \
            mDownloadOp->notifyStatus(r1);            \
            envClear();                               \
            return r1;                                \
        }                                             \
        mDownloadOp->notifyStatus(r2);                \
    }
#define DEBUG_GET_LAST_BYTES()              \
    {                                       \
        LOG_D("Stop at Line:%d", __LINE__); \
        for (int i = 0; i < 10; i++) {      \
            plReadU8();                     \
        }                                   \
        while (1)                           \
            ;                               \
    }
#define DEBUG_GET_LAST_BYTES2_10()          \
    {                                       \
        LOG_D("Stop at Line:%d", __LINE__); \
        for (int i = 0; i < 10; i++) {      \
            CharEx rsp = plReadU16();       \
        }                                   \
        portTaskDelayMs(1);                 \
    }
#define DEBUG_GET_LAST_BYTES_10()           \
    {                                       \
        LOG_D("Stop at Line:%d", __LINE__); \
        for (int i = 0; i < 10; i++) {      \
            CharEx rsp = plReadU8();        \
            (void)rsp;                      \
        }                                   \
        portTaskDelayMs(1);                 \
    }
#define DEBUG_ECHO_BYTES_10()               \
    {                                       \
        LOG_D("Stop at Line:%d", __LINE__); \
        for (int i = 0; i < 10; i++) {      \
            CharEx rsp = plReadU8();        \
            plSendU8(~(rsp.dataU8));        \
        }                                   \
        portTaskDelayMs(1);                 \
    }
#define CHECK_RETURN_CHAREX(x)                         \
    {                                                  \
        if (x.error != ErrorCode::DL_ERROR_NO_ERROR) { \
            LOG_E("Invalid Char at %d", __LINE__);     \
            return pFALSE;                             \
        }                                              \
    }
#define plReadU8() mDownloadOp->readU8()
#define plReadU16() mDownloadOp->readU16()
#define plReadU32() mDownloadOp->readU32()
#define plSendU8(x) mDownloadOp->sendU8(x);
#define plSendU16(x) mDownloadOp->sendU16(x);
#define plSendU32(x) mDownloadOp->sendU32(x);
DownloadManager::PlatformType DownloadManager::sPlatformType =
    DownloadManager::DM_PL_INVALID;
#define MAX_SUPPORT_DOWNLOAD_FILE_NUM 6
struct DownloadConfig gConfigList[MAX_SUPPORT_DOWNLOAD_FILE_NUM];
int gBinFileNum = 0;
DownloadManager::DownloadManager() {
    mDownloadRound = 1;
    useOtaBin = false;
}
void DownloadManager::setUseOtaBin(bool enable) {
    useOtaBin = enable;
    return;
}
struct DownloadCfg {
    std::string name;
    std::string file;
    uint32_t beginAddress;
};
void DownloadManager::setFwBackupPath(const std::string &path) {
    sFwBackupPath = path;
}
std::string DownloadManager::getFwBackupPath() {
    return sFwBackupPath;
}
static pBool_t parserDownloadConfig(const char *buf, uint32_t length) {
    std::string downloadcfgdata((char *)buf, length);
    std::string address;
    std::string relativePath = IMAGE_RELATIVE_DIR;
    std::string relativeOtaPath = IMAGE_RELATIVE_OTA_PATH;
    std::string relativeBackupPath = DownloadManager::getFwBackupPath();
    std::string::size_type position_start = 0;
    std::string::size_type position_end = 0;
    std::string::size_type pos_end_rn = 0;
    std::string::size_type pos_end_n = 0;
    DownloadCfg downloadcfg[MAX_SUPPORT_DOWNLOAD_FILE_NUM];
    LOG_D("parser_dwonlaod_cfg ");
    if (downloadcfgdata.size() == 0) {
        assert(0);  // normally will not run here
        return false;
    }
    // find platform
    size_t platformPos = downloadcfgdata.find("platform:");
    platformPos += strlen("platform:") + 1;
    size_t platformEnd = platformPos;
    for (platformEnd = platformPos; downloadcfgdata[platformEnd] != '\r' &&
                                    downloadcfgdata[platformEnd] != '\n';
         platformEnd++)
        ;
    std::string platformString =
        downloadcfgdata.substr(platformPos, platformEnd - platformPos);
    LOG_D("find platform: %s", platformString.c_str());
    if (platformString == "AG3335" || platformString == "ag3335") {
        LOG_D("Platform select:AG3335");
        DownloadManager::setPlatform(DownloadManager::DM_PL_AG3335);
    } else if (platformString == "AG3352" || platformString == "ag3352") {
        LOG_D("Platform select:AG3352");
        DownloadManager::setPlatform(DownloadManager::DM_PL_AG3352);
    } else if (platformString == "AG3365" || platformString == "ag3365") {
        LOG_D("Platform select:AG3365");
        DownloadManager::setPlatform(DownloadManager::DM_PL_AG3365);
    } else {
        DownloadManager::setPlatform(DownloadManager::DM_PL_INVALID);
    }
    // Parser Item
    for (int i = 0; i < MAX_SUPPORT_DOWNLOAD_FILE_NUM; i++) {
        position_start = downloadcfgdata.find("file", position_start);
        if (position_start == downloadcfgdata.npos) {
            LOG_E("can't find file string position, parser finish");
            return true;
        }
        pos_end_rn = downloadcfgdata.find("\r", position_start);
        pos_end_n = downloadcfgdata.find("\n", position_start);
        if (pos_end_rn != std::string::npos) {
            position_end = pos_end_rn;
        } else if (pos_end_n != std::string::npos) {
            position_end = pos_end_n;
        } else {
            LOG_E("can't find   file end string position");
            return false;
        }
        downloadcfg[i].file = downloadcfgdata.substr(
            position_start + 6, (position_end - position_start - 6));
        // LOG_D(" %s position  %lu ",downloadcfg[i].file.c_str(),position);
        position_start = downloadcfgdata.find("name", position_start);
        if (position_start == downloadcfgdata.npos) {
            LOG_E("can't find   name string position");
            return false;
        }
        pos_end_rn = downloadcfgdata.find("\r", position_start);
        pos_end_n = downloadcfgdata.find("\n", position_start);
        if (pos_end_rn != std::string::npos) {
            position_end = pos_end_rn;
        } else if (pos_end_n != std::string::npos) {
            position_end = pos_end_n;
        } else {
            LOG_E("can't find  name end string position");
            return false;
        }
        downloadcfg[i].name = downloadcfgdata.substr(
            position_start + 6, (position_end - position_start - 6));
        // LOG_D(" %s position  %lu ",downloadcfg[i].name.c_str(), position);
        position_start =
            (downloadcfgdata).find("begin_address: ", position_start);
        if (position_start == downloadcfgdata.npos) {
            LOG_E("can't find begin_address  string");
            return false;
        }
        downloadcfg[i].beginAddress =
            std::stoi(downloadcfgdata.substr(position_start + 17, 8), 0, 16);
        LOG_D("  %s beginAddress: %x ", downloadcfg[i].name.c_str(),
              downloadcfg[i].beginAddress);
        gConfigList[gBinFileNum].Name = downloadcfg[i].name;
        std::string nameUpper = gConfigList[gBinFileNum].Name;
        for (size_t j = 0; j < nameUpper.size(); j++) {
            if (nameUpper[j] >= 'a' && nameUpper[j] <= 'z') {
                nameUpper[j] = nameUpper[j] - 'a' + 'A';
            } 
        }
        LOG_D("Name Upper: %s", nameUpper.c_str());
        if (nameUpper == "BOOTLOADER") {
            gConfigList[gBinFileNum].isBootLoader = true;
            LOG_D("Find Bootloader");
        } else {
            gConfigList[gBinFileNum].isBootLoader = false;
        }
        gConfigList[gBinFileNum].fileName = relativePath + downloadcfg[i].file;
        gConfigList[gBinFileNum].beginAddress = downloadcfg[i].beginAddress;
        gConfigList[gBinFileNum].otaFileName =
            relativeOtaPath + downloadcfg[i].file;
        gConfigList[gBinFileNum].beginAddress = downloadcfg[i].beginAddress;
        gConfigList[gBinFileNum].backupfileName =
            relativeBackupPath + downloadcfg[i].file;
        gBinFileNum++;
        if (gBinFileNum >= MAX_SUPPORT_DOWNLOAD_FILE_NUM) {
            break;
        }
    }
    return true;
}
pBool_t DownloadManager::loadDownloadConfig(const char *filename) {
    char sourcebuf[4096];
    FILE *source = NULL;
    uint32_t file_length = 0;
    bool ret = pFALSE;
    source = fopen(filename, "rb");
    if (source == NULL) {
        LOG_E("can not open %s,no need download", filename);
        return ret;
    }
    file_length = getFileSize(filename);
    LOG_D("%s file size is %d", filename, file_length);
    if ((file_length == 0) || (file_length > 4096)) {
        LOG_D("%s  size is not corret, pls check file", filename);
        return ret;
    }
    memset(sourcebuf, 0, sizeof(sourcebuf));
    uint32_t readnummux = (getFileSize(filename)) / 4096 + 1;
    while (readnummux--) {
        if (fread(sourcebuf, 4096, 1, source) == 0) {
            if (feof(source)) LOG_D("CfgFile READ END OF FILE");
            ret = parserDownloadConfig(sourcebuf, file_length);
        }
    }
    // Print Parser Info
    for (int j = 0; j < MAX_SUPPORT_DOWNLOAD_FILE_NUM; j++) {
        LOG_D("ConfigList[%d] name %s,file %s,beginAddress 0x%08x, is_bl: %d", j,
              gConfigList[j].Name.c_str(), gConfigList[j].fileName.c_str(),
              gConfigList[j].beginAddress, gConfigList[j].isBootLoader);
    }
    return ret;
}
void DownloadManager::parserDownloadAgentAttr() {
    if (mIoType == DM_UART) {
#ifdef DA_USE_RACE_CMD
        if (sPlatformType == DM_PL_AG3335) {
            mDaPath = UART_DA_3335_PATH;
            // assert("AG3335 donot support race download" && 0);
            mDaRunAddr = HDL_DA_RUN_ADDR_AG3335_UART;
        } else if (sPlatformType == DM_PL_AG3352) {
            mDaPath = UART_DA_3352_PATH;
            mDaRunAddr = HDL_DA_RUN_ADDR_AG3352;
        } else if (sPlatformType == DM_PL_AG3365) {
            mDaPath = UART_DA_3365_PATH;
            mDaRunAddr = HDL_DA_RUN_ADDR_AG3365_UART;
        } else {
            assert("unsupport type" && 0);
        }
#else
        // only support AG3335
        mDaPath = UART_DA_PATH;
        mDaRunAddr = HDL_DA_RUN_ADDR;
#endif
    } else if (mIoType == DM_SPI) {
#ifdef DA_USE_RACE_CMD
        assert("SPI do not donot support race download" && 0);
        assert(sPlatformType != DM_PL_AG3352 && "SPI has not been tested 3352");
#else
        // only support AG3335
        mDaPath = SPI_DA_PATH;
#endif
#ifdef DA_USE_RACE_CMD
        mDaRunAddr = HDL_DA_RUN_ADDR_AG3335_SPI;
#else
        mDaRunAddr = HDL_DA_RUN_ADDR;
#endif
    }
    LOG_I("da path: %s, da run addr: 0x%08x", mDaPath, mDaRunAddr);
}
void DownloadManager::setPlatform(PlatformType type) { sPlatformType = type; }
pBool_t DownloadManager::registerInterface(
    IDownloadPlatformInterface *instance) {
    mDownloadOp = instance;
    return pTRUE;
}
pBool_t DownloadManager::deregisterInterface() {
    mDownloadOp = nullptr;
    return pTRUE;
}
DownloadStatus DownloadManager::start(int retryTime) {
    // before start download, make sure 3335 is off
    LOG_D("download manager start, retry=%d", retryTime);
    TRACE_D("download manager start, retry=%d", retryTime);
    uint32_t total = mDownloadRound;
    uint32_t success = 0;
    uint32_t failed = 0;
    parserDownloadAgentAttr();
#ifdef AIROHA_PM_LOCK_SYSTEM
    Airoha::GPIO::portLockSystem();
#endif
    WakeLock::getInstnace()->acquireLock(
        WakeLock::WAKELOCK_USER_DOWNLOAD_FIRMWARE);
    while (total > 0) {
        mDownloadOp->powerOff();
        usleep(100000);
        DownloadStatus roundRet = startHostDownload();
        if (roundRet == DownloadStatus::DL_STATUS_FINISH) {
            success++;
        } else {
            failed++;
        }
        // when download finish, power off 3335 again
        mDownloadOp->powerOff();
        total--;
        LOG_I("Download Status: %" PRIu32 " retry: %" PRIu32 " Failed: %" PRIu32
              " Success: %" PRIu32,
              mDownloadRound, retryTime, failed, success);
        if (roundRet != DownloadStatus::DL_STATUS_FINISH && retryTime > 0) {
            // If download error, we will retry again.
            total++;
            retryTime--;
            failed--;
        }
    }
    WakeLock::getInstnace()->releaseLock(
        WakeLock::WAKELOCK_USER_DOWNLOAD_FIRMWARE);
#ifdef AIROHA_PM_LOCK_SYSTEM
    Airoha::GPIO::portUnlockSystem();
#endif
    LOG_D("========== Download Finish ========");
    LOG_D("== Total: %" PRIu32 " Success: %" PRIu32 " Failed: %" PRIu32,
          mDownloadRound, success, failed);
    TRACE_D("== DL FINISH: Total: %" PRIu32 " Success: %" PRIu32 " Failed: %" PRIu32,
          mDownloadRound, success, failed);
    LOG_D("===================================");
    if (failed) {
        return DownloadStatus::DL_STATUS_COMMON_FAILED;
    }
    return DownloadStatus::DL_STATUS_FINISH;
}
DownloadStatus DownloadManager::startHostDownload() {
    TRACE_D("=== Start Host Download ===")
    pBool_t ret = pFALSE;
    pTime_t start = getSystemTickMs();
    if (envSetup() != DL_ERROR_NO_ERROR) {
        return DL_STATUS_PREPARE_FAILED;
    }
    pTime_t bromHandshakeFinish;
    pTime_t bromDaStart;
    pTime_t bromDaEnd;
    pTime_t daFormatStart;
    pTime_t daFormatEnd;
#ifndef SKIP_DOWNLOAD_PROGRESS
    // LOG_D("default timeout: %d", mDownloadOp->defaultIOTimeoutMs);
    ret = bromHandshake();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_BROM_FAILED,
                                 DL_STATUS_BROM_PASS, "Brom Handshake Failed");
    bromHandshakeFinish = getSystemTickMs();
    ret = bromDisableWDT();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_DISABLE_WDT_FAILED,
                                 DL_STATUS_DISABLE_WDT_PASS,
                                 "WDT Disable Failed");
#ifdef RAISE_BROM_BAUDRATE
    ret = bromRaiseBaudrate();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_BROM_RAISE_BAUDRATE_FAILED,
                                 DL_STATUS_BROM_RAISE_BAUDRATE_PASS,
                                 "BromRaiseBaudrate Failed");
#endif
    // DEBUG_GET_LAST_BYTES();
    bromDaStart = getSystemTickMs();
    ret = bromSendDA();
    bromDaEnd = getSystemTickMs();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_SEND_DA_FAILED,
                                 DL_STATUS_SEND_DA_PASS, "Send DA Failed");
    ret = bromJumpDA();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_JUMP_DA_FAILED,
                                 DL_STATUS_JUMP_DA_PASS, "DA Jump Failed");
#ifdef DA_USE_RACE_CMD
    ret = daHandshakeRace();
#else
    ret = DAHandshake();
#endif
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_DA_HANDSHAKE_FAILED,
                                 DL_STATUS_DA_HANDSHAKE_PASS,
                                 "DA HandShake Failed");
    daFormatStart = getSystemTickMs();
#ifdef DA_USE_RACE_CMD
    ret = daformatFlashRace();
#else
    ret = DAformatFlash();
#endif
    daFormatEnd = getSystemTickMs();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_FORMAT_FLASH_FAILED,
                                 DL_STATUS_FORMAT_FLASH_PASS,
                                 "DA Format Failed");
    pTime_t downloadChipStart = getSystemTickMs();
    ret = downloadChip(gConfigList, gBinFileNum);
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_DOWNLOAD_CHIP_FAILED,
                                 DL_STATUS_DOWNLOAD_CHIP_PASS,
                                 "Download Chip Error");
    ret = checkChipImage(gConfigList, gBinFileNum);
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_DOWNLOAD_CHIP_FAILED,
                                 DL_STATUS_DOWNLOAD_CHIP_PASS,
                                 "CHECK CRC Chip Error");
    pTime_t downloadChipEnd = getSystemTickMs();
#ifdef DA_USE_RACE_CMD
    ret = daFinishRace();
    CHECK_BOOL_AND_NOTIFY_RETURN(ret, DL_STATUS_DEINIT_PASS,
                                 DL_STATUS_DEINIT_FAILED,
                                 "DA Disconnect Error");
#endif
#else
    (void)ret;
    usleep(2000000);
#endif
    if (envClear() != DL_ERROR_NO_ERROR) {
        return DL_STATUS_DEINIT_FAILED;
    }
    pTime_t end = getSystemTickMs();
    LOG_D("Download Finish, Cost: %" PRId64 "ms", end - start);
    LOG_D("\t Send DA, Cost: %" PRId64 "ms", bromDaEnd - bromDaStart);
    LOG_D("\t Format, Cost: %" PRId64 "ms", daFormatEnd - daFormatStart);
    LOG_D("\t Download Firmware, Cost: %" PRId64 "ms",
          downloadChipEnd - downloadChipStart);
    LOG_D("\t Brom handshake, Cost: %" PRId64 "ms",
          bromHandshakeFinish - start);
#ifdef DELAY_WHEN_DOWNLOAD_FINISH
    usleep(2000000);
#endif
    return DL_STATUS_FINISH;
}
pBool_t DownloadManager::echoU8(uint8_t sendChar, uint8_t receiveChar) {
    CharEx receive;
    ErrorCode ret = plSendU8(sendChar);
    if (ret != ErrorCode::DL_ERROR_NO_ERROR) {
        return false;
    }
    // portTaskDelayMs(mDownloadOp->getRWDelayMs());
    receive = plReadU8();
    if (receive.dataU8 == receiveChar && receive.error == DL_ERROR_NO_ERROR) {
        return pTRUE;
    }
    return pFALSE;
}
pBool_t DownloadManager::bromWrite16(uint32_t addr, uint16_t value) {
    LOG_D("brom write 16: addr: 0x%08x, value 0x%04x", addr, value);
    CharEx r;
    if (echoU8(0xD2, 0xD2) == pFALSE) {
        return pFALSE;
    }
    if (echoU32(addr, addr) == pFALSE) {
        return pFALSE;
    }
    if (echoU32(1, 1) == pFALSE) {
        return pFALSE;
    }
    r = plReadU16();
    if (r.dataU16 >= BROM_ERROR) {
        LOG_E("Line: %d get status error %" PRIx32 " %d", __LINE__, r.dataU32,
              r.error);
        return pFALSE;
    }
    if (echoU16(value, value) == pFALSE) {
        return pFALSE;
    }
    r = plReadU16();
    if (r.dataU16 >= BROM_ERROR) {
        return pFALSE;
    }
    return pTRUE;
}
pBool_t DownloadManager::echoU16(uint16_t sendChar, uint16_t receiveChar) {
    CharEx receive;
    ErrorCode ret = mDownloadOp->sendU16(sendChar);
    if (ret != ErrorCode::DL_ERROR_NO_ERROR) {
        return false;
    }
    // portTaskDelayMs(mDownloadOp->getRWDelayMs());
    receive = plReadU16();
    if (receive.dataU16 == receiveChar && receive.error == DL_ERROR_NO_ERROR) {
        return pTRUE;
    }
    return pFALSE;
}
pBool_t DownloadManager::echoU32(uint32_t sendChar, uint32_t receiveChar) {
    CharEx receive;
    ErrorCode ret = mDownloadOp->sendU32(sendChar);
    if (ret != ErrorCode::DL_ERROR_NO_ERROR) {
        return false;
    }
    // portTaskDelayMs(mDownloadOp->getRWDelayMs());
    receive = plReadU32();
    if (receive.dataU32 == receiveChar && receive.error == DL_ERROR_NO_ERROR) {
        return pTRUE;
    }
    LOG_D("echo U32 : %" PRIx32 " %" PRIx32 " %" PRIx32, sendChar, receiveChar,
          receive.dataU32);
    return pFALSE;
}
pBool_t DownloadManager::bromHandshake() {
    // ErrorCode err;
    // uint8_t receive;
    TRACE_D("BL Handshake");
    pBool_t firstHandshakeFinish = pFALSE;
    uint8_t handshakePassCount = 0;
    for (int i = 0; i < kHandshakeRetryCount; i++) {
        handshakePassCount = 0;
        firstHandshakeFinish = pFALSE;
        pTime_t current =
            getSystemTickMs();  // should move to here;modify by baohua
        mDownloadOp->powerOn();
        while (pTRUE) {
            if (echoU8(HDL_START_CMD1, HDL_START_CMD1_R)) {
                firstHandshakeFinish = pTRUE;
                break;
            }
            LOG_D("First handshake failed");
            if (getSystemTickMs() - current >
                HANDSHAKE_SINGLE_ROUND_TIMEOUT_MS) {
                break;
            }
        }
        LOG_D("First handshake try i =%d", i);
        if (!firstHandshakeFinish) {
            // First Handshake Failed, should restart download
            mDownloadOp->powerOff();
            mDownloadOp->powerReset();
            // read all data in uart buffer
            while (plReadU8().error == DL_ERROR_NO_ERROR);
            continue;
        }
        LOG_D("First handshake continue try i =%d", i);
        handshakePassCount++;
        // handshake for following cmd
        if (echoU8(HDL_START_CMD2, HDL_START_CMD2_R)) {
            handshakePassCount++;
        }
        if (echoU8(HDL_START_CMD3, HDL_START_CMD3_R)) {
            handshakePassCount++;
        }
        if (echoU8(HDL_START_CMD4, HDL_START_CMD4_R)) {
            handshakePassCount++;
        }
        if (handshakePassCount == 4) {
            break;
        } else {
            LOG_E("handshakePassCount %" PRIu8, handshakePassCount);
            mDownloadOp->powerOff();
            // read all data in uart buffer
             while (plReadU8().error == DL_ERROR_NO_ERROR);
        }
    }
    if (handshakePassCount == 4) {
        LOG_D("brom handshake pass!");
        return pTRUE;
    } else {
        return pFALSE;
    }
}
pBool_t DownloadManager::bromDisableWDT() {
    pBool_t ret = pTRUE;
    if (sPlatformType == DM_PL_AG3335) {
        ret = bromWrite16(0xA2080000, 0x0010) && ret;
    } else if (sPlatformType == DM_PL_AG3352 || sPlatformType == DM_PL_AG3365) {
        ret = bromWrite16(0xA2080000, 0x0010) && ret;
        ret = bromWrite16(0xA2080030, 0x0040) && ret;
    } else {
        assert(0 && "Unknown platform.");
    }
    return ret;
}
pBool_t DownloadManager::bromRaiseBaudrate() {
    CharEx r;
    if (echoU8(0xDC, 0xDC) == pFALSE) {
        return pFALSE;
    }
    if (echoU32(921600, 921600) == pFALSE) {
        return pFALSE;
    }
    r = plReadU16();
    if (r.dataU16 >= BROM_ERROR) {
        LOG_E("raise baudrate error");
        return pFALSE;
    }
    mDownloadOp->raiseSpeed(921600);
    return pTRUE;
}
pBool_t DownloadManager::bromSendDA() {
    LOG_D("Send DA...");
    FILE *fda = NULL;
    fda = fopen(mDaPath, "rb");
    if (fda == NULL) {
        LOG_E("open da file %s,error", mDaPath);
        return pFALSE;
    }
    int len = getFileSize(mDaPath);
    LOG_D("DATA len:%d", len);
    if (echoU8(0xD7, 0xD7) == pFALSE) {
        LOG_E("Write 0xD7 Error");
        fclose(fda);
        return pFALSE;
    }
    // DEBUG_GET_LAST_BYTES2_10();
    echoU32(mDaRunAddr, mDaRunAddr);
    // DEBUG_GET_LAST_BYTES_10();
    // DEBUG_GET_LAST_BYTES2_10();
    echoU32(len, len);
    // echoU32(30000,30000);
    // DEBUG_GET_LAST_BYTES2_10();
    echoU32(0, 0);
    // DEBUG_GET_LAST_BYTES_10();
    CharEx status = plReadU16();
    // DEBUG_GET_LAST_BYTES_10();
    if (status.dataU16 >= BROM_ERROR) {
        LOG_E("status error");
        fclose(fda);
        return pFALSE;
    }
    LOG_D("send da data>>>");
    char buffer[UART_SEND_STEP];
    int total = 0;
    int checksum = 0;
    int readStep = UART_SEND_STEP;
    pBool_t readExit = pFALSE;
    while (1) {
        fread(buffer, readStep, 1, fda);
        // DEBUG_GET_LAST_BYTES_10();
        mDownloadOp->sendBytes((uint8_t *)buffer, readStep);
        portTaskDelayMs(10);
        // LOG_D("Checksum :%x", plReadU16().dataU16);
        // DEBUG_ECHO_BYTES_10()
        checksum ^= computeDAChecksum((uint8_t *)buffer, readStep);
        total += readStep;
        LOG_D("SEND %d bytes OK,total %d,checksum %x", readStep, total,
              checksum);
        if (readExit) {
            break;
        }
        if (len - total < readStep) {
            readStep = len - total;
            readExit = pTRUE;
        }
    }
    LOG_D("checksum ...");
    CharEx checksumR;
    checksumR = plReadU16();
    // while(1){
    //    checksumR = plReadU16();
    //    if(checksumR.dataU16 != 0x662d && checksumR.error ==
    //    DL_ERROR_NO_ERROR) break;
    //}
    CHECK_RETURN_CHAREX(checksumR);
    if (checksumR.dataU16 != checksum || checksumR.error != DL_ERROR_NO_ERROR) {
        LOG_E("checksum: %x,%x", checksum, checksumR.dataU16);
        // DEBUG_GET_LAST_BYTES_10();
        return pFALSE;
    }
    // DEBUG_GET_LAST_BYTES();
    LOG_D("checksum: %x,%x", checksum, checksumR.dataU16);
    status = plReadU16();
    CHECK_RETURN_CHAREX(status);
    if (status.dataU16 >= BROM_ERROR || status.error != DL_ERROR_NO_ERROR) {
        LOG_E("brom status error");
        return pFALSE;
    }
    LOG_D("send DA successful");
    fclose(fda);
    return pTRUE;
}
uint16_t DownloadManager::computeDAChecksum(uint8_t *buf, uint32_t buf_len) {
    uint16_t checksum = 0;
    if (buf == NULL || buf_len == 0) {
        return 0;
    }
    uint32_t i = 0;
    for (i = 0; i < buf_len / 2; i++) {
        checksum ^= *(uint16_t *)(buf + i * 2);
    }
    if ((buf_len % 2) == 1) {
        checksum ^= buf[i * 2];
    }
    return checksum;
}
uint32_t DownloadManager::simpleBinChecksum(uint8_t *buf, uint32_t len) {
    uint32_t checksum = 0;
    if (buf == NULL || len == 0) {
        LOG_E("simpleChecksum error");
        return 0;
    }
    uint32_t i = 0;
    for (i = 0; i < len; i++) {
        checksum += *(buf + i);
    }
    return checksum;
}
pBool_t DownloadManager::bromJumpDA() {
    // portTaskDelayMs(1000);
    if (echoU8(0xD5, 0xD5) == pFALSE) {
        LOG_E("Write 0xD5 Cmd Error");
        return pFALSE;
    }
    // if (echoU8(0xD5, 0xD5) == pFALSE) {
    //    LOG_E("Write 0xD5 Cmd Error");
    //    return pFALSE;
    //}
    LOG_D("jump to DA");
    if (echoU32(mDaRunAddr, mDaRunAddr) == pFALSE) {
        return pFALSE;
    }
    LOG_D("set address");
    uint16_t status = plReadU16().dataU16;
    if (status >= BROM_ERROR) {
        return pFALSE;
    }
    LOG_D("Jump to DA OK");
    return pTRUE;
}
#ifdef DA_USE_RACE_CMD
/*
pBool_t DownloadManager::RaceCMDSendAndRecive(uint16_t ID, uint32_t *pArg1,
uint32_t *pArg2){ switch(ID){ case RACE_DA_GET_FLASH_ADDRESS: RACE_ADDR_SEND
send; send.head_ = 0x05; send.type_ = 0x5A; send.len_ = sizeof(send.id_);
        send.id_ = RACE_DA_GET_FLASH_ADDRESS;
        mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
            break;
        case RACE_DA_GET_FLASH_SIZE:
        RACE_SIZE_SEND send;
        send.head_ = 0x05;
        send.type_ = 0x5A;
        send.len_ = sizeof(send.id_);
        send.id_ = RACE_DA_GET_FLASH_SIZE;
            break
        default:
            break;
        }
}*/
pBool_t DownloadManager::daFlowControl(bool enable) {
    LOG_I("hdl_da_flow_ctrl start");
    // send
    race::FlowCtrlSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_) + sizeof(send.flag_);
    send.id_ = race::RACE_DA_FLOW_CTRL;
    send.flag_ = enable;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::FlowCtrlResPayload res;
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (res.head_ == 0x05 && res.type_ == 0x5B &&
        res.len_ == (sizeof(res.id_) + sizeof(res.status_)) &&
        res.id_ == race::RACE_DA_FLOW_CTRL && res.status_ == DA_S_DONE) {
        LOG_I("hdl_da_flow_ctrl res done");
        return true;
    }
    LOG_E("hdl_da_flow_ctrl res fail");
    return false;
}
pBool_t DownloadManager::daSpeedupBaudrate(uint32_t baudrate) {
    LOG_I("hdl_da_speedup_baudrate start");
    // send
    race::BaudrateSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_) + sizeof(send.rate_);
    send.id_ = race::RACE_DA_BAUDRATE;
    send.rate_ = baudrate;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::BaudrateResPayload res;
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (res.head_ == 0x05 && res.type_ == 0x5B &&
        res.len_ == (sizeof(res.id_) + sizeof(res.status_)) &&
        res.id_ == race::RACE_DA_BAUDRATE && res.status_ == DA_S_DONE) {
        LOG_I("hdl_da_speedup_baudrate res done");
        return pTRUE;
    }
    LOG_E("hdl_da_speedup_baudrate res fail");
    return pFALSE;
}
pBool_t DownloadManager::daGetFlashAdressRace(uint32_t *mFlashBaseAddr) {
    // send
    race::AddrSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = race::RACE_DA_GET_FLASH_ADDRESS;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::AddrResPayload res;
    memset(&res, 0, sizeof(res));
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (res.head_ == 0x05 && res.type_ == 0x5B &&
        res.len_ ==
            (sizeof(res.id_) + sizeof(res.status_) + sizeof(res.addr_)) &&
        res.id_ == race::RACE_DA_GET_FLASH_ADDRESS &&
        res.status_ == DA_S_DONE) {
        *mFlashBaseAddr = res.addr_;
        return true;
    }
    LOG_D("hdl_da_get_flash_address res fail %d", res.len_);
    return pFALSE;
}
pBool_t DownloadManager::daGetFlashSizeRace(uint32_t *mFlashSize) {
    // send
    race::SizeSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = race::RACE_DA_GET_FLASH_SIZE;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::SizeResPayload res;
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (res.head_ == 0x05 && res.type_ == 0x5B &&
        res.len_ ==
            (sizeof(res.id_) + sizeof(res.status_) + sizeof(res.size_)) &&
        res.id_ == race::RACE_DA_GET_FLASH_SIZE && res.status_ == DA_S_DONE) {
        *mFlashSize = res.size_;
        return true;
    }
    LOG_D("hdl_da_get_flash_size res fail");
    return false;
}
pBool_t DownloadManager::daGetFlashIDRace(uint16_t *mFlashManufacturerID,
                                          uint16_t *mFlashID1,
                                          uint16_t *mFlashID2) {
    // send
    race::IdSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_);
    send.id_ = race::RACE_DA_GET_FLASH_ID;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::IdResPayload res;
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (res.head_ == 0x05 && res.type_ == 0x5B &&
        res.len_ ==
            (sizeof(res.id_) + sizeof(res.status_) + sizeof(res.flash_id_)) &&
        res.id_ == race::RACE_DA_GET_FLASH_ID && res.status_ == DA_S_DONE) {
        *mFlashManufacturerID = res.flash_id_[0];
        *mFlashID1 = res.flash_id_[1];
        *mFlashID2 = res.flash_id_[2];
        return true;
    }
    LOG_D("hdl_da_get_flash_id res fail");
    return false;
}
pBool_t DownloadManager::daHandshakeRace() {
    pBool_t ret = pFALSE;
// Speed up to DA init baudrate
    if (mIoType == DM_UART) {
#ifdef RAISE_BROM_BAUDRATE
        {
            ErrorCode res = mDownloadOp->raiseSpeed(115200);
            if (DL_ERROR_NO_ERROR != res) {
                LOG_E("change baud 115200 error");
            }
            usleep(100000);
        }
#endif
#ifndef LOW_BAUD
        LOG_D("Normal Baud Mode: 921600");
        daSpeedupBaudrate(921600);
        ErrorCode res = mDownloadOp->raiseSpeed(921600);
        if (DL_ERROR_NO_ERROR != res) {
            LOG_E("change baud error");
        }
#else
        LOG_D("Low Baud Mode: 115200");
#endif
#ifdef UART_ENABLE_SOFTWARE_FLOWCONTROL
        daFlowControl(true);
        mDownloadOp->setFlowControl(IDownloadPlatformInterface::FC_SOFTWARE);
#endif
#ifdef UART_SPEED_UP_TO_3M
        daSpeedupBaudrate(3000000);
        mDownloadOp->raiseSpeed(3000000);
        usleep(100000);
#endif
    }
    uint32_t flash_base_addr = 0;
    ret = daGetFlashAdressRace(&flash_base_addr);
    if (!ret) return ret;
    uint32_t flash_size = 0;
    ret = daGetFlashSizeRace(&flash_size);
    if (!ret) return ret;
    uint16_t manufacturer_id = 0;
    uint16_t device_id1 = 0;
    uint16_t device_id2 = 0;
    ret = daGetFlashIDRace(&manufacturer_id, &device_id1, &device_id2);
    if (!ret) return ret;
    AG3335_Flash_Info.formatAddress = flash_base_addr;
    AG3335_Flash_Info.formatSize = flash_size;
    ret = pTRUE;
    return ret;
}
uint32_t DownloadManager::crc32(const uint8_t *pData, uint32_t mSize) {
    uint32_t crc32 = 0xFFFFFFFFu;
    for (size_t i = 0; i < mSize; i++) {
        const uint32_t lookupIndex = (crc32 ^ pData[i]) & 0xff;
        crc32 = (crc32 >> 8) ^ DownloadManager::sCRC32Table[lookupIndex];
    }
    crc32 ^= 0xFFFFFFFFu;
    return crc32;
}
uint32_t DownloadManager::crc32Image(struct ImageInfo *img) {
    const uint32_t image_len = img->image_len;
    uint32_t crc32 = 0xFFFFFFFFu;
    uint32_t i = 0;
    uint32_t j = 0;
    const uint32_t buff_size = 1024;
    uint8_t buff[buff_size];
    uint32_t check_length = 0;
    size_t result;
    for (i = 0; i < image_len; i += buff_size) {
        memset(buff, 0xFF, buff_size);
        if ((image_len - i) > buff_size) {
            check_length = buff_size;
        } else {
            check_length = image_len - i;
            uint32_t m = check_length % 4;
            if (m != 0) {
                check_length = check_length + (4 - m);  // must 4-byte
                                                        // alignment.
                LOG_I("Move to 4-byte alignment");
            }
        }
        result = fread(buff, 1, check_length, img->imagefile);
        for (j = 0; j < check_length; j++) {
            const uint32_t lookupIndex = (crc32 ^ buff[j]) & 0xff;
            crc32 = (crc32 >> 8) ^
                    sCRC32Table[lookupIndex];  // CRCTable is an array of 256
                                               // 32-bit constants
        }
    }
    // Finalize the CRC-32 value by inverting all the bits
    crc32 ^= 0xFFFFFFFFu;
    return crc32;
}
pBool_t DownloadManager::daformatFlashRace() {
    pBool_t ret = pTRUE;
    const uint32_t format_begin_address = AG3335_Flash_Info.formatAddress;
    const uint32_t format_size = AG3335_Flash_Info.formatSize;
    const uint32_t format_end_address = format_begin_address + format_size;
    uint32_t format_address = format_begin_address;
    while (format_address < format_end_address) {
        uint32_t block_size = 0;
        if (format_size >= LEN_64K && (format_address % LEN_64K) == 0) {
            block_size = LEN_64K;
        } else {
            block_size = LEN_4K;
        }
        // send
        race::FmSendPayload send;
        send.head_ = 0x05;
        send.type_ = 0x5A;
        send.len_ = sizeof(send.id_) + sizeof(send.addr_) + sizeof(send.size_) +
                    sizeof(send.crc_);
        send.id_ = race::RACE_DA_ERASE_BYTES;
        send.addr_ = format_address;
        send.size_ = block_size;
        send.crc_ = crc32((uint8_t *)&send, sizeof(send) - sizeof(send.crc_));
        mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
        // response
        race::FmResPayload res;
        uint32_t bytesRead = 0;
        mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
        if (!(res.head_ == 0x05 && res.type_ == 0x5B &&
              res.len_ ==
                  (sizeof(res.id_) + sizeof(res.status_) + sizeof(res.addr_)) &&
              res.id_ == race::RACE_DA_ERASE_BYTES &&
              res.status_ == DA_S_DONE && res.addr_ == format_address)) {
            // LOG_D("hdl_format_race res done: addr=0x%X, len=0x%X",
            // format_address, block_size);
            return false;
        }
        format_address += block_size;
        uint8_t progress =
            (format_address - format_begin_address) * 100 / format_size;
        progress = (progress > 100) ? 100 : progress;
        LOG_D("Format percent %d", progress);
    }
    // return true;
    return ret;
}
pBool_t DownloadManager::downloadImageRace(struct ImageInfo img) {
    const uint32_t startAddr = img.slaveAddress;
    const uint32_t imageLen = img.image_len;
    const uint32_t packetNum = ((imageLen - 1) / DA_SEND_PACKET_LEN) + 1;
    uint8_t g_hdl_fw_data_buf[DA_SEND_PACKET_LEN] = {0};
    uint32_t packetSentNum = 0;
    uint64_t *addrStatus;
    addrStatus = (uint64_t *)malloc(sizeof(uint64_t) * packetNum);
    for (uint32_t i = 0; i < packetNum; i++) {
        addrStatus[i] = i * DA_SEND_PACKET_LEN + startAddr;
    }
    uint32_t packetAckNum = 0;
    uint8_t waitResponseNum = 0;
    for (int round = 0; round < 3; round++) {
        packetAckNum = 0;
        packetSentNum = 0;
        // Could not reset this value because it represent the dma status in
        // slave waitResponseNum = 0;
        for (uint32_t i = 0; i < packetNum || packetAckNum < packetNum;
             i = (i < packetNum) ? i + 1 : i) {
            if (i < packetNum) {
                LOG_D("Round:%d, i=%d, status=%" PRId64 ", addr = 0x%08" PRIX64,
                      round, i, addrStatus[i] >> 32,
                      addrStatus[i] & (uint64_t)0xFFFFFFFF);
            } else {
                LOG_D("Round: Last Pointer, waiting for response.");
            }
            if (i < packetNum && (addrStatus[i] & ((uint64_t)1 << 32))) {
                // if receive response, do not need send and ack
                packetSentNum++;
                packetAckNum++;
                continue;
            }
            if (i < packetNum) {
                const uint32_t startOffset = i * DA_SEND_PACKET_LEN;
                const bool isLastPacket = (i == (packetNum - 1));
                const uint32_t curPacketLen =
                    (imageLen - startOffset) < DA_SEND_PACKET_LEN
                        ? (imageLen - startOffset)
                        : DA_SEND_PACKET_LEN;
                memset(g_hdl_fw_data_buf, 0, sizeof(g_hdl_fw_data_buf));
                fseek(img.imagefile, startOffset, SEEK_SET);
                fread(g_hdl_fw_data_buf, curPacketLen, 1, img.imagefile);
                uint8_t *packet_buf = g_hdl_fw_data_buf;
                if (isLastPacket && curPacketLen < DA_SEND_PACKET_LEN) {
                    // DA Must received whole DA_SEND_PACKET_LEN, so Fill 0xFF
                    // to packet_buf if it is last packet
                    memset(packet_buf + curPacketLen, 0xFF,
                           DA_SEND_PACKET_LEN - curPacketLen);
                }
                const uint32_t slave_addr = startAddr + startOffset;
                // send
                race::DownloadSendPayload send;
                send.head_ = 0x05;
                send.type_ = 0x5A;
                send.len_ = sizeof(send.id_) + sizeof(send.addr_) +
                            sizeof(send.size_) + DA_SEND_PACKET_LEN +
                            sizeof(send.crc_);
                send.id_ = race::RACE_DA_WRITE_BYTES;
                send.addr_ = slave_addr;
                send.size_ = DA_SEND_PACKET_LEN;
                memcpy(send.buf_, packet_buf, send.size_);
                send.crc_ =
                    crc32((uint8_t *)&send, sizeof(send) - sizeof(send.crc_));
                mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
                packetSentNum++;
#ifdef SPEED_UP_DOWNLOAD
                waitResponseNum++;
#endif
                LOG_D("==== Packet Status(1) (%d/%d/%d/%d) =====", packetAckNum,
                      packetSentNum, packetNum, waitResponseNum);
                TRACE_D(
                    "==== Packet Status(1) (%d/%d/%d/%d) =====", packetAckNum,
                    packetSentNum, packetNum, waitResponseNum);
#ifdef SPEED_UP_DOWNLOAD
                if ((waitResponseNum < MAX_SLAVE_PACKET_NUM) &&
                    (packetNum - packetSentNum)) {
                    continue;
                }
#endif
            }
            // response
            race::DownloadResPayload res;
            uint32_t bytesRead = 0;
            mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
            if (sizeof(res) != bytesRead) {
#ifdef STRICT_TEST_MODE
                assert(0 &&
                       "do not allow sizeof(res) != bytesRead in test mode");
#endif
#ifdef SPEED_UP_DOWNLOAD
                // if no ack , we assume that something is error, keep send
                // next, and reset window
                waitResponseNum = 0;
#endif
                // if no ack and this is the last packet, should break,
                if (i >= packetNum - 1) {
                    break;
                }
            } else if (!(res.head_ == 0x05 && res.type_ == 0x5B &&
                  res.len_ == (sizeof(res.id_) + sizeof(res.status_) +
                               sizeof(res.addr_)) &&
                  res.id_ == race::RACE_DA_WRITE_BYTES &&
                  res.status_ == DA_S_DONE &&
                  ((res.addr_ - startAddr) < imageLen))) {
                LOG_E("hdl_download_race res failed!: addr=0x%X", res.addr_);
                LOG_W("\tres.head_ == 0x05: %d", res.head_ == 0x05);
                LOG_W("\tres.type_ == 0x5B: %d", res.type_ == 0x5B);
                LOG_W(
                    "\tres.len_ == (sizeof(res.id_) + sizeof(res.status_) + "
                    "sizeof(res.addr_): %d",
                    res.len_ == (sizeof(res.id_) + sizeof(res.status_) +
                                 sizeof(res.addr_)));
                LOG_W("\tres.id_ == race::RACE_DA_WRITE_BYTES: %d",
                      res.id_ == race::RACE_DA_WRITE_BYTES);
                LOG_W("\t((res.addr_ - startAddr) < imageLen): %d, 0x%08X",
                      (res.addr_ - startAddr) < imageLen, res.addr_);
                LOG_W("res.status_ = %d", res.status_);
#ifndef SPEED_UP_DOWNLOAD
                LOG_W("response error!");
#else
                waitResponseNum =
                    waitResponseNum > 0 ? waitResponseNum - 1 : waitResponseNum;
#endif
#ifdef STRICT_TEST_MODE
                assert(0 && "do not allow error in test mode");
#endif
            } else {
                LOG_D("hdl_download_race res done: addr=0x%X", res.addr_);
                int index = (res.addr_ - startAddr) / DA_SEND_PACKET_LEN;
                addrStatus[index] |= ((uint64_t)(0x1) << 32);
                packetAckNum++;
#ifdef SPEED_UP_DOWNLOAD
                waitResponseNum =
                    waitResponseNum > 0 ? waitResponseNum - 1 : waitResponseNum;
#else
#endif
            }
            LOG_D("==== Packet Status(2) (%d/%d/%d/%d) =====", packetAckNum,
                  packetSentNum, packetNum, waitResponseNum);
            TRACE_D("==== Packet Status(2) (%d/%d/%d/%d) =====", packetAckNum,
                    packetSentNum, packetNum, waitResponseNum);
        }
        if (packetAckNum == packetNum) {
            LOG_D("receive enougn response, download finish");
            break;
        }
        LOG_D("Download Round %d Address Check: ===", round);
        for (uint32_t i = 0; i < packetNum; i++) {
            if ((addrStatus[i] & ((uint64_t)1 << 32)) == 0) {
                LOG_E("===> Download Addr Failed: 0x%08X",
                      (uint32_t)(addrStatus[i] & 0xFFFFFFFF));
            }
        }
        LOG_D("Download Round %d Address Check Done. ===", round);
    }
    // Check Download Status
    pBool_t downloadRet = pTRUE;
    for (uint32_t i = 0; i < packetNum; i++) {
        if ((addrStatus[i] & ((uint64_t)1 << 32)) == 0) {
            downloadRet = pFALSE;
            LOG_E("===> Download Addr Failed: 0x%08X",
                  (uint32_t)(addrStatus[i] & 0xFFFFFFFF));
        }
    }
#ifdef SPEED_UP_DOWNLOAD
    free(addrStatus);
#endif
    return downloadRet;
}
pBool_t DownloadManager::checkImageCrcRace(struct ImageInfo img) {
    const uint32_t image_len = img.image_len;
    const uint32_t image_crc = crc32Image(&img);
    // send
    race::CheckCrcSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = sizeof(send.id_) + sizeof(send.data_addr_) +
                sizeof(send.data_len_) + sizeof(send.data_crc_) +
                sizeof(send.crc_);
    send.id_ = race::RACE_DA_DATA_RANGE_CRC;
    send.data_addr_ = img.slaveAddress;
    send.data_len_ = image_len;
    send.data_crc_ = image_crc;
    send.crc_ = 0;
    send.crc_ = crc32((const uint8_t *)&send, sizeof(send) - sizeof(send.crc_));
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    race::CheckCrcResPayload res;
    memset(&res, 0, sizeof(res));
    uint32_t bytesRead = 0;
    ErrorCode rtn =
        mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (rtn != DL_ERROR_NO_ERROR) {
        LOG_E("hdl_da_check_image_crc error, ErrorCode:%d", rtn);
        return false;
    }
    if (!(res.head_ == 0x05 && res.type_ == 0x5B &&
          res.len_ ==
              (sizeof(res.status_) + sizeof(res.crc_) + sizeof(res.id_)) &&
          res.id_ == race::RACE_DA_DATA_RANGE_CRC &&
          res.status_ == DA_S_DONE)) {
        LOG_D("checkImageCrcRace res fail res.len=%d CRC=0x%08X", res.crc_,
              (int)res.len_);
        return false;
    }
    LOG_I(
        "hdl_da_check_image_crc done, image size:%d, response_crc=  0x%08X, "
        "send_crc=0x%08X",
        (int)img.image_len, res.crc_, send.crc_);
    return true;
}
pBool_t DownloadManager::daFinishRace() {
    // send
    race::RstSendPayload send;
    send.head_ = 0x05;
    send.type_ = 0x5A;
    send.len_ = 3;
    send.id_ = race::RACE_DA_FINISH;
    send.flag_ = 0;
    mDownloadOp->sendBytes((uint8_t *)&send, sizeof(send));
    // response
    race::RstResPayload res;
    uint32_t bytesRead = 0;
    mDownloadOp->readBytes((uint8_t *)&res, sizeof(res), &bytesRead);
    if (!(res.head_ == 0x05 && res.type_ == 0x5B && res.len_ == 3 &&
          res.id_ == race::RACE_DA_FINISH && res.status_ == DA_S_DONE)) {
        LOG_D("hdl_finish_race res fail");
        return false;
    }
    LOG_D("=====DA Finish=====");
    TRACE_D("=====DA Finish=====");
    return true;
}
#else
pBool_t DownloadManager::DAHandshake() {
    CharEx r;
    // portTaskDelayMs(1000);
    if (receiveAndSendUint8(0xC0, 0x3F) == pFALSE) return pFALSE;
    LOG_D("handshake step 1");
    if (receiveAndSendUint8(0x0C, 0xF3) == pFALSE) return pFALSE;
    LOG_D("handshake step 2");
    if (receiveAndSendUint8(0x3F, 0xC0) == pFALSE) return pFALSE;
    LOG_D("handshake step 3");
    if (receiveAndSendUint8(0xF3, 0x0C) == pFALSE) return pFALSE;
    LOG_D("handshake with da!");
    r = plReadU8();
    if (r.dataU8 != 0x5A) {
        LOG_E("handshake error 1 %x", r.dataU8);
        return pFALSE;
    }
    // when sync with da, we change data mode
#ifdef USE_NEW_DA
    plSendU8(1);
#else
    plSendU8(0);
#endif
    r = plReadU8();
    if (r.dataU8 != 0x69) {
        LOG_E("wrong sync 1 %x", r.dataU8);
        return pFALSE;
    }
#ifndef USE_NEW_DA
    r = plReadU8();
    if (r.dataU8 != 0x69) {
        LOG_E("wrong sync 2");
        return pFALSE;
    }
    LOG_D("Power latch..");
    plSendU8(0x5A);
    plSendU8(0);
    r = plReadU8();
    if (r.dataU8 != 0x69) {
        return pFALSE;
    }
    plSendU8(0x5A);
    LOG_D("disable power key success");
    if (plReadU8().dataU8 != 0x69) {
        return pFALSE;
    }
    LOG_D("DA Raise MCU.");
    plSendU8(0x5A);
#endif
    if (mIoType == DM_UART) {
        portTaskDelayMs(10);  // wait for transmit
#ifndef LOW_BAUD
        ErrorCode res = mDownloadOp->raiseSpeed(921600);
        if (res != DL_ERROR_NO_ERROR) {
            LOG_E("change baud error");
        }
#endif
        for (int i = 0; i < 20; i++) {
            plSendU8(0xC0);
            r = plReadU8();
            if (r.dataU8 == 0xC0) {
                break;
            }
            LOG_E("WAIT for sync..");
        }
        LOG_D("change baud OK");
        if (echoU8(0x5A, 0x5A) == false) {
            return false;
        }
        r = plReadU8();
        if (r.dataU8 != 0x69) {
            LOG_E("sync error %x", r.dataU8);
            return false;
        }
        plSendU8(0x5A);
    }
    LOG_D("resync with da SUCCESS");
    // get DA report
    int id = plReadU16().dataU16;
    LOG_D("manufacturer id =%x", id);
    id = plReadU16().dataU16;
    LOG_D("flash device id1 =%x", id);
    id = plReadU16().dataU16;
    LOG_D("flash device id2 =%x", id);
#ifndef USE_NEW_DA
    id = plReadU32().dataU32;
    LOG_D("mount status =%x", id);
#endif
    AG3335_Flash_Info.formatAddress = plReadU32().dataU32;
    LOG_D("base addr =%x", AG3335_Flash_Info.formatAddress);
    AG3335_Flash_Info.formatSize = plReadU32().dataU32;
    LOG_D("flash size=%x", AG3335_Flash_Info.formatSize);
#ifndef USE_NEW_DA
    uint8_t da_rx = plReadU8().dataU8;
    if (da_rx != 0x5A) {
        return pFALSE;
    }
    plSendU8(0x5A);
#endif
    LOG_D("HOST SYNC DA SUCCESSFUL!!");
    return pTRUE;
}
pBool_t DownloadManager::DAformatFlash() {
    plSendU8(0xD4);
#ifndef USE_NEW_DA
    plSendU8(0);
#endif
    plSendU32(AG3335_Flash_Info.formatAddress);
    plSendU32(AG3335_Flash_Info.formatSize);
    CharEx r;
#ifndef USE_NEW_DA
    do {
        r = plReadU8();
    } while (r.dataU8 != 0x5A);
    LOG_D("check format range success");
    r = plReadU8();
    if (r.dataU8 != 0x5A) {
        return false;
    }
    LOG_D("check begin address success");
#else
    r = plReadU8();
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        return pFALSE;
    }
    LOG_D("check format range success");
#endif
    while (1) {
        r = plReadU32();
        uint32_t formatStatus = r.dataU32;
        CHECK_RETURN_CHAREX(r);
        if (formatStatus == 0x00) {
            r = plReadU8();
            CHECK_RETURN_CHAREX(r);
            uint8_t progress = r.dataU8;
            LOG_D("Progress = %d", progress);
            plSendU8(0x5A);
            break;
        } else if (formatStatus == 3021) {
            r = plReadU8();
            CHECK_RETURN_CHAREX(r);
            uint8_t progress = r.dataU8;
            plSendU8(0x5A);
            LOG_D("format progress = %d", progress);
        }
    }
    r = plReadU8();
    if (r.dataU8 != 0x5A) {
        return false;
    }
    LOG_D("format flash OK!!");
    return true;
}
pBool_t DownloadManager::downloadImage(struct ImageInfo img) {
    CharEx r;
    const uint32_t imageLen = img.image_len;
    uint32_t img4K = (((imageLen - 1) / 4096) + 1) * 4096;
    LOG_D("Download image...%d 4k %d", imageLen, img4K);
    plSendU8(0xB2);
    plSendU32(img.slaveAddress);
    plSendU32(img4K);
#ifndef USE_NEW_DA
    plSendU32(DA_SEND_PACKET_LEN);
#endif
    r = plReadU8();
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        return pFALSE;
    }
    LOG_D("range OK");
    r = plReadU8();
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        LOG_E("check argument fail..2");
        return pFALSE;
    }
    LOG_D("erase OK");
    fseek(img.imagefile, 0L, SEEK_SET);
    uint32_t sent_len = 0;
    uint8_t sendBuffer[DA_SEND_PACKET_LEN];
    uint32_t image_checksum = 0;
    uint32_t checksum = 0;
    bool padding = false;
    while (sent_len < img4K) {
        if (padding == false) {
            LOG_D("Packet parser,sent:%d", sent_len);
            uint32_t remain_len = imageLen - sent_len;
            if (remain_len <= DA_SEND_PACKET_LEN) {
                // the last packet
                fread(sendBuffer, remain_len, 1, img.imagefile);
                memset(sendBuffer + remain_len, 0xFF,
                       DA_SEND_PACKET_LEN - remain_len);
                sent_len += DA_SEND_PACKET_LEN;
                padding = true;
            } else {
                fread(sendBuffer, DA_SEND_PACKET_LEN, 1, img.imagefile);
                sent_len += DA_SEND_PACKET_LEN;
            }
            checksum = simpleBinChecksum(sendBuffer, DA_SEND_PACKET_LEN);
            image_checksum += checksum;
        } else {
            memset(sendBuffer, 0xFF, DA_SEND_PACKET_LEN);
            sent_len += DA_SEND_PACKET_LEN;
            checksum = simpleBinChecksum(sendBuffer, DA_SEND_PACKET_LEN);
            image_checksum += checksum;
        }
        // usleep(50000);
        // sendPacket(sendBuffer,DA_SEND_PACKET_LEN);
        mDownloadOp->sendBytes(sendBuffer, DA_SEND_PACKET_LEN);
        LOG_D("Packet parser,sent:%d finish", sent_len);
        TRACE_D("Packet Sent: %d/%d", sent_len, img4K);
        LOG_D("sendPacket OK");
        // usleep(50000);
        plSendU32(checksum);
        LOG_D("wait respond..");
        r = plReadU8();
        CHECK_RETURN_CHAREX(r);
        if (r.dataU8 != 0x69) {
            return pFALSE;
        }
#ifdef DOWNLOAD_DEBUG_STEP
        r = plReadU32();
        CHECK_RETURN_CHAREX(r);
        LOG_D("write flash 0x%x", r.dataU32);
#endif
    }
    LOG_D("download full image ok..check");
    TRACE_D("Download full image ok, check checksum");
    r = plReadU8();
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        LOG_D("ACK error");
        return pFALSE;
    }
    plSendU32(image_checksum);
    pTime_t current = getSystemTickMs();
    do {
        r = plReadU8();  // this ack may be cost long time, MAX is 10000
    } while (r.error != ErrorCode::DL_ERROR_NO_ERROR ||
             getSystemTickMs() - current > 10000);
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        r = plReadU32();
        CHECK_RETURN_CHAREX(r);
        uint32_t checksumF = r.dataU32;
        LOG_D("image checksum failed!!! %x,%x", checksumF, image_checksum);
        return pFALSE;
    }
    LOG_D("image checksum OK");
    TRACE_D("Image Checksum OK");
#ifndef USE_NEW_DA
    if (img.is_bootloader) {
        plSendU8(0x5A);
    } else {
        plSendU8(0xA5);
    }
    r = plReadU8();
    CHECK_RETURN_CHAREX(r);
    if (r.dataU8 != 0x5A) {
        LOG_D("set bootloader error");
        return pFALSE;
    }
#endif
    LOG_D("image download OK");
    TRACE_D("=====Image download OK=====");
    return pTRUE;
}
#endif
pBool_t DownloadManager::downloadChip(struct DownloadConfig list[],
                                      uint32_t size) {
    LOG_D("%s...", __FUNCTION__);
    uint32_t i;
    for (i = 0; i < size; i++) {
        FILE *p = NULL;
        p = fopen(list[i].fileName.c_str(), "rb");
        if (p == NULL) {
            LOG_E("can not open %s", list[i].fileName.c_str());
            return false;
        }
        struct ImageInfo imginfo;
        memset(&imginfo, 0, sizeof(imginfo));
        imginfo.imagefile = p;
        imginfo.image_len = getFileSize(list[i].fileName.c_str());
        imginfo.is_bootloader = list[i].isBootLoader;
        imginfo.slaveAddress = list[i].beginAddress;
        TRACE_D("========= Download Image: %s ========",
                list[i].fileName.c_str());
#ifdef DA_USE_RACE_CMD
        bool ret = downloadImageRace(imginfo);
#else
        bool ret = downloadImage(imginfo);
#endif
        if (ret == false) {
            LOG_D("download %s FAILED", list[i].fileName.c_str());
            fclose(p);
            return false;
        }
        LOG_D("download %s SUCCESS", list[i].fileName.c_str());
        // statusNotify(DL_FINISH,0);
        fclose(p);
        // backUpBinFile(list[i].fileName.c_str(),
        // list[i].backupfileName.c_str());
    }
    return true;
}
pBool_t DownloadManager::checkChipImage(struct DownloadConfig list[],
                                        uint32_t size) {
    LOG_D("%s...", __FUNCTION__);
    uint32_t i;
    for (i = 0; i < size; i++) {
        FILE *p = NULL;
        p = fopen(list[i].fileName.c_str(), "rb");
        if (p == NULL) {
            LOG_E("can not open %s", list[i].fileName.c_str());
            return false;
        }
        struct ImageInfo imginfo;
        memset(&imginfo, 0, sizeof(imginfo));
        imginfo.imagefile = p;
        imginfo.image_len = getFileSize(list[i].fileName.c_str());
        imginfo.is_bootloader = list[i].isBootLoader;
        imginfo.slaveAddress = list[i].beginAddress;
        TRACE_D("========= Check Image: %s ========", list[i].fileName.c_str());
        bool ret = checkImageCrcRace(imginfo);
        if (ret == false) {
            LOG_D("Check %s FAILED", list[i].fileName.c_str());
            fclose(p);
            return false;
        }
        LOG_D("Check %s SUCCESS", list[i].fileName.c_str());
        TRACE_D("Check %s SUCCESS", list[i].fileName.c_str());
        // statusNotify(DL_FINISH,0);
        fclose(p);
        backUpBinFile(list[i].fileName.c_str(), list[i].backupfileName.c_str());
    }
    return true;
}
pBool_t DownloadManager::receiveAndSendUint8(uint8_t a, uint8_t b) {
    CharEx r = plReadU8();
    if (r.dataU8 == a && r.error == DL_ERROR_NO_ERROR) {
        plSendU8(b);
        return pTRUE;
    }
    return pFALSE;
}
// int64_t getSystemTickNs() {
//    struct timespec ts;
//    clock_gettime(CLOCK_REALTIME, &ts);
//    return ts.tv_sec * 1000000000 + ts.tv_nsec;
//}
int64_t getSystemTickMs() {
    int64_t timeMs;
#ifdef _MSC_VER
    timeMs = clock();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    timeMs = (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
#endif
    return timeMs;
}
size_t getFileSize(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return 0;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fclose(f);
    return size;
}
ErrorCode DownloadManager::envSetup() {
    mDownloadOp->openPort();
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode DownloadManager::envClear() {
    mDownloadOp->closePort();
    return ErrorCode::DL_ERROR_NO_ERROR;
}
void DownloadManager::backUpBinFile(const char *sourcefile,
                                    const char *destfile) {
    LOG_D("%s...", __FUNCTION__);
    char buff[4096];
    int len;
    FILE *read, *write;
    read = fopen(sourcefile, "rb");
    if (read == NULL) {
        LOG_D("open sourcefile error %s [%s]", sourcefile, strerror(errno));
        return;
    }
    write = fopen(destfile, "wb+");
    if (write == NULL) {
        LOG_D("open backupfile error %s [%s]", destfile, strerror(errno));
        fclose(read);
        return;
    }
    while (0 != (len = fread(buff, 1, sizeof(buff), read))) {
        fwrite(buff, 1, len, write);
    }
    fseek(read, 0L, SEEK_SET);
    fseek(write, 0L, SEEK_SET);
    if (getFileSize(sourcefile) != getFileSize(destfile)) {
        LOG_E("%s file backup error,need check!!!!", sourcefile);
    } else {
        LOG_D("%s file backup OK !!", sourcefile);
    }
    LOG_D("sourcefile size %zu  destfile size %zu !!", getFileSize(sourcefile),
          getFileSize(destfile));
    int syncRet = fsync(fileno(write));
    LOG_D("sync ret: %s, %d", destfile, syncRet);
    fclose(read);
    fclose(write);
    chmod(destfile, BACKUP_FILE_PERMISSION);
#ifndef AIROHA_SIMULATION
    // 1021 = gps, 1000 = system
    chown(destfile, 1021, 1000);
#endif
}
IDownloadPlatformInterface::~IDownloadPlatformInterface() {}
ErrorCode IDownloadPlatformInterface::setFlowControl(FlowControl fc) {
    (void)fc;
    return ErrorCode::DL_ERROR_NO_ERROR;
};
ErrorCode IDownloadPlatformInterface::raiseSpeed(int baudrate) {
    (void)baudrate;
    return ErrorCode::DL_ERROR_NO_ERROR;
}
void DownloadManager::setIoType(InterfaceType value) {
    mIoType = value;
    return;
}
void DownloadManager::setDownloadRound(uint32_t downloadRound) {
    mDownloadRound = downloadRound;
}

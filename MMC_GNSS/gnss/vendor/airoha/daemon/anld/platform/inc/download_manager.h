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
#ifndef PLATFORM_INC_DOWNLOAD_MANAGER_H_
#define PLATFORM_INC_DOWNLOAD_MANAGER_H_
#include <stdint.h>
#include <stdio.h>
#include <string>
// Download Config
#define DA_USE_RACE_CMD
#define SPEED_UP_DOWNLOAD
#define UART_SPEED_UP_TO_3M
// #define UART_ENABLE_SOFTWARE_FLOWCONTROL
#define RAISE_BROM_BAUDRATE
// Condtion Check
// #define STRICT_TEST_MODE
// #define USE_NEW_DA
// #define DL_WITH_UART
// #define DOWNLOAD_DEBUG_STEP
#define DA_BOOTUP_WITH_3M
#define pTRUE 1
#define pFALSE 0
#ifdef DA_USE_RACE_CMD
namespace race {
enum RaceId {
    RACE_DA_GET_FLASH_ADDRESS = (0x210E),
    RACE_DA_GET_FLASH_SIZE = (0x210F),
    RACE_DA_GET_FLASH_ID = (0x2110),
    RACE_DA_BAUDRATE = (0x2115),
    RACE_DA_FLOW_CTRL = (0x2116),
    RACE_DA_WRITE_BYTES = (0x2100),
    RACE_DA_READ_BYTES = (0x2101),
    RACE_DA_ERASE_BYTES = (0x2104),
    RACE_DA_FINISH = (0x2106),
    RACE_DA_DATA_RANGE_CRC = (0x211A),
};
#define LEN_4K (0x1000)
#define LEN_64K (0x10000)
#define DA_S_DONE 0x00
#define DA_FW_PACKET_LEN (0x1000)
#define DA_SEND_PACKET_LEN (DA_FW_PACKET_LEN)
#define DA_RECV_PACKET_LEN (DA_FW_PACKET_LEN)
#pragma pack(push, 1)
struct FlowCtrlSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t flag_;
};
struct FlowCtrlResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
};
struct BaudrateSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t rate_;
};
struct BaudrateResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
};
struct IdSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
};
struct IdResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint8_t flash_id_[3];
};
struct SizeSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
};
struct SizeResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t size_;
};
struct AddrSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
};
struct AddrResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
};
struct FmSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint32_t size_;
    uint32_t crc_;
};
struct FmResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
};
struct DownloadSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint16_t size_;
    uint8_t buf_[DA_SEND_PACKET_LEN];
    uint32_t crc_;
};
struct DownloadResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
};
struct ReadbackSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t addr_;
    uint16_t size_;
    uint32_t crc_;
};
struct ReadbackResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t addr_;
    uint32_t crc_;
    uint8_t buf_[DA_RECV_PACKET_LEN];
};
struct RstSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t flag_;
};
struct RstResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
};
struct CheckCrcSendPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint32_t data_addr_;
    uint32_t data_len_;
    uint32_t data_crc_;
    uint32_t crc_;
};
struct CheckCrcResPayload {
    uint8_t head_;
    uint8_t type_;
    uint16_t len_;
    uint16_t id_;
    uint8_t status_;
    uint32_t crc_;
};
#pragma pack(pop)
}  // namespace race
#endif
typedef uint8_t pBool_t;
typedef int64_t pTime_t;
typedef size_t dSize_t;
typedef enum download_status {
    DL_STATUS_FINISH = 0,
    DL_STATUS_PREPARE_PASS,
    DL_STATUS_BROM_PASS,
    DL_STATUS_DISABLE_WDT_PASS,
    DL_STATUS_BROM_RAISE_BAUDRATE_PASS,
    DL_STATUS_SEND_DA_PASS,
    DL_STATUS_JUMP_DA_PASS,
    DL_STATUS_DA_HANDSHAKE_PASS,
    DL_STATUS_FORMAT_FLASH_PASS,
    DL_STATUS_DOWNLOAD_CHIP_PASS,
    DL_STATUS_DEINIT_PASS,
    DL_STATUS_START = 1000,
    DL_STATUS_PREPARE_FAILED,
    DL_STATUS_BROM_FAILED,
    DL_STATUS_DISABLE_WDT_FAILED,
    DL_STATUS_BROM_RAISE_BAUDRATE_FAILED,
    DL_STATUS_SEND_DA_FAILED,
    DL_STATUS_JUMP_DA_FAILED,
    DL_STATUS_DA_HANDSHAKE_FAILED,
    DL_STATUS_FORMAT_FLASH_FAILED,
    DL_STATUS_DOWNLOAD_CHIP_FAILED,
    DL_STATUS_DEINIT_FAILED,
    DL_STATUS_COMMON_FAILED,
} DownloadStatus;
typedef enum error_code {
    DL_ERROR_NO_ERROR,
    DL_ERROR_READ_TIMEOUT,
    DL_ERROR_DATA_INVALID,
    DL_ERROR_PARAMETER_ERROR,
    DL_ERROR_NO_MEMORY,
    DL_ERROR_LENGTH_TOO_LONG,
    DL_ERROR_COMMON_FAULT,
    DL_ERROR_IO_ERROR,
    DL_ERROR_FATAL_ERROR,
    DL_ERROR_SPI_READ_ERROR,
    DL_ERROR_SPI_WRITE_ERROR,
    DL_ERROR_SPI_SYSTEM_ERROR,
    DL_ERROR_SPI_WRITE_CFG_ERROR,
    DL_ERROR_SPI_WRITE_DATA_ERROR,
    DL_ERROR_SPI_READ_CFG_ERROR,
    DL_ERROR_SPI_READ_DATA_ERROR,
    DL_ERROR_SPI_READ_STATUS_TIMEOUT,
    DL_ERROR_SPI_WRITE_STATUS_TIMEOUT,
} ErrorCode;
struct CharEx {
    union {
        uint8_t dataU8;
        uint16_t dataU16;
        uint32_t dataU32;
    };
    ErrorCode error;
    CharEx() {
        dataU32 = 0xFFFFFFFF;
        error = ErrorCode::DL_ERROR_READ_TIMEOUT;
    }
};
struct ImageInfo {
    FILE *imagefile;
    uint32_t slaveAddress;
    uint32_t image_len;
    bool is_bootloader;
};
struct DownloadConfig {
	std::string Name;
    std::string fileName;
    std::string backupfileName;
    std::string otaFileName;
    bool isBootLoader;
    uint32_t beginAddress;
};
class IDownloadPlatformInterface {
 public:
    enum FlowControl { FC_NONE = 0, FC_SOFTWARE = 1, FC_HARDWARE = 2 };
    virtual ErrorCode powerOn() = 0;
    virtual ErrorCode powerOff() = 0;
    virtual ErrorCode powerReset() = 0;
    virtual ErrorCode openPort() = 0;
    virtual ErrorCode closePort() = 0;
    virtual CharEx readU8() = 0;
    virtual CharEx readU16() = 0;
    virtual CharEx readU32() = 0;
    virtual ErrorCode readBytes(void *buffer, dSize_t length,
                                uint32_t *bytesRead) = 0;
    virtual ErrorCode sendU8(uint8_t dataU8) = 0;
    virtual ErrorCode sendU16(uint16_t dataU16) = 0;
    virtual ErrorCode sendU32(uint32_t dataU32) = 0;
    virtual ErrorCode sendBytes(const void *buffer, dSize_t length) = 0;
    // only use in uart
    virtual ErrorCode raiseSpeed(int baudrate);
    virtual ErrorCode setFlowControl(FlowControl fc);
    virtual int32_t getHandshakeDelayMs() = 0;
    virtual int32_t getRWDelayMs() = 0;
    virtual ~IDownloadPlatformInterface();
    virtual void notifyStatus(DownloadStatus status) = 0;
};
class DownloadManager {
 public:
    enum InterfaceType {
        DM_UART = 0,
        DM_SPI = 1,
    };
    enum PlatformType {
        DM_PL_AG3335,
        DM_PL_AG3352,
        DM_PL_AG3365,
        DM_PL_INVALID = 0xFF,
    };
    DownloadManager();
    pBool_t registerInterface(IDownloadPlatformInterface *);
    pBool_t deregisterInterface();
    DownloadStatus start(int retryTime = 0);
    void setIoType(InterfaceType value);
    void setDownloadRound(uint32_t downloadRound);
    void setUseOtaBin(bool enable);
    static pBool_t loadDownloadConfig(const char *filename);
    static void setFwBackupPath(const std::string &path);
    static void setPlatform(PlatformType);
    static std::string getFwBackupPath();
 private:
    DownloadStatus startHostDownload();
    ErrorCode envSetup();
    ErrorCode envClear();
    pBool_t bromWrite16(uint32_t addr, uint16_t value);
    pBool_t echoU8(uint8_t send, uint8_t receive);
    pBool_t echoU16(uint16_t send, uint16_t receive);
    pBool_t echoU32(uint32_t send, uint32_t receive);
    pBool_t receiveAndSendUint8(uint8_t a, uint8_t b);
    // Brom Function
    pBool_t bromHandshake();
    pBool_t bromDisableWDT();
    pBool_t bromSendDA();
    uint16_t computeDAChecksum(uint8_t *buf, uint32_t buf_len);
    uint32_t simpleBinChecksum(uint8_t *buf, uint32_t len);
    pBool_t bromJumpDA();
    pBool_t bromRaiseBaudrate();
// DA Function
#ifdef DA_USE_RACE_CMD
    pBool_t daHandshakeRace();
    pBool_t daformatFlashRace();
    pBool_t downloadImageRace(struct ImageInfo img);
    pBool_t checkImageCrcRace(struct ImageInfo img);
    pBool_t checkChipImage(struct DownloadConfig list[], uint32_t size);
    pBool_t daFinishRace();
    // for uart baudrate
    pBool_t daFlowControl(bool enable);
    pBool_t daSpeedupBaudrate(uint32_t baudrate);
    pBool_t daGetFlashAdressRace(uint32_t *mFlashBaseAddr);
    pBool_t daGetFlashSizeRace(uint32_t *mFlashSize);
    pBool_t daGetFlashIDRace(uint16_t *mFlashManufacturerID,
                             uint16_t *mFlashID1, uint16_t *mFlashID2);
    // pBool_t RaceCMDSendAndRecive(uint16_t ID, uint32_t *pArg1, uint32_t
    // *pArg2);
    static uint32_t crc32(const uint8_t *pData, uint32_t mSize);
    static uint32_t crc32Image(struct ImageInfo *img);
    // uint16_t mFlashManufacturerID;
    // uint16_t mFlashID1;
    // uint16_t mFlashID2;
    // uint32_t mFlashBaseAddr;
    // uint32_t mFlashSize;
    // uint8_t *pData;
    // uint32_t mSize;
    static uint32_t sCRC32Table[256];
#else
    pBool_t DAHandshake();
    pBool_t DAformatFlash();
    pBool_t downloadImage(struct ImageInfo img);
#endif
    pBool_t downloadChip(struct DownloadConfig list[], uint32_t size);
    void backUpBinFile(const char *sourcefile, const char *destfile);
    void parserDownloadAgentAttr();
    IDownloadPlatformInterface *mDownloadOp;
    static const int kHandshakeRetryCount = 5;
    InterfaceType mIoType;
    static PlatformType sPlatformType;
    static std::string sFwBackupPath;
    const char *mDaPath;
    uint32_t mDaRunAddr;
    uint32_t mDownloadRound;
    bool useOtaBin;
};
#define HDL_START_CMD1 0xA0
#define HDL_START_CMD1_R 0x5F
#define HDL_START_CMD2 0x0A
#define HDL_START_CMD2_R 0xF5
#define HDL_START_CMD3 0x50
#define HDL_START_CMD3_R 0xAF
#define HDL_START_CMD4 0x05
#define HDL_START_CMD4_R 0xFA
#define HDL_WDT_REG 0xA2080000
#define HDL_WDT_VAL 0x0010
#if defined USE_NEW_DA || defined DA_USE_RACE_CMD
#define HDL_DA_RUN_ADDR_AG3352 0x04200000
#define HDL_DA_RUN_ADDR_AG3335_UART 0x04000000
#define HDL_DA_RUN_ADDR_AG3335_SPI 0x04200000
#define HDL_DA_RUN_ADDR_AG3365_UART 0x04000000
#else
#define HDL_DA_RUN_ADDR 0x04204000
#endif
#define BROM_ERROR 0x1000
#define UART_SEND_STEP 1024
// #define DA_SEND_PACKET_LEN 4096
// some driver do not support 4096 bytes
#ifdef DA_USE_RACE_CMD
#define UART_DA_3335_PATH "/vendor/etc/gnss/ag3335_da.bin"
#define UART_DA_3365_PATH "/vendor/etc/gnss/ag3365_da.bin"
#define UART_DA_3352_PATH "/vendor/etc/gnss/ag3352_da.bin"
#else
#define DA_SEND_PACKET_LEN 2048
#define UART_DA_PATH "/vendor/etc/gnss/slave_da_UART.bin"
#endif
#define SPI_DA_PATH "/vendor/etc/gnss/slave_da_SPI.bin"
#define IMAGE_RELATIVE_DIR "/vendor/etc/gnss/"
#define IMAGE_RELATIVE_OTA_PATH "/vendor/etc/gnss/ota/"
#define IMAGE_RELATIVE_BACKUP_PATH "/data/vendor/airoha/"
#define IMAGE_GNSS_FLASH_DOWNLOAD_CFG_PATH     "/vendor/etc/gnss/flash_download.cfg"
#define IMAGE_PARTITION_BACKUP_PATH \
    "/data/vendor/airoha/partition_table_backup.bin"
#define IMAGE_BOOTLOADER_BACKUP_PATH \
    "/data/vendor/airoha/ag3335_bootloader_backup.bin"
#define IMAGE_GNSS_DEMO_BACKUP_PATH "/data/vendor/airoha/gnss_demo_backup.bin"
#define IMAGE_GNSS_CONFIG_BACKUP_PATH \
    "/data/vendor/airoha/gnss_config_backup.bin"
// Gnss OTA Bin
#define IMAGE_GNSS_OTA_PARTITION_PATH \
    "/data/vendor/airoha/ota/partition_table.bin"
#define IMAGE_GNSS_OTA_BOOTLOADER_PATH "/data/vendor/airoha/ota/ag3335_bootloader.bin"
#define IMAGE_GNSS_OTA_GNSS_DEMO_PATH "/data/vendor/airoha/ota/gnss_demo.bin"
#define IMAGE_GNSS_OTA_CONFIG_PATH "/data/vendor/airoha/ota/gnss_config.bin"
#define USE_OTA_TO_DOWNLOAD_FILE "/data/vendor/airoha/ota_download"
struct flashInfo {
    uint32_t formatAddress;
    uint32_t formatSize;
};
/**
 * @brief Get the System Tick in nano-second
 *
 * @return pTime_t
 *
 *  Platform function for porting
 */
pTime_t getSystemTickNs();
/**
 * @brief Get the System Tick in millisecond
 *
 * @return pTime_t
 *
 *  Platform function for porting
 */
pTime_t getSystemTickMs();
void portTaskDelayMs(pTime_t ms);
size_t getFileSize(const char *filename);
#endif

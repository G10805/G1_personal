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
#ifndef DRIVER_COMM_SPI_DRIVER_H_
#define DRIVER_COMM_SPI_DRIVER_H_
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <atomic>
#include <list>
#include <string>
#include "system_timer.h"
namespace airoha {
namespace communicator {
typedef size_t spi_size_t;
typedef uint8_t SpiSlaveStatus;
class SpiDriver : SysTimer {
    typedef void (*spiRxCallback)(uint8_t *buffer, spi_size_t length);
 public:
    enum SpiDriverStatus {
        SPI_STATUS_OK,
        SPI_STATUS_COMMON_FAILED = 1000,
        SPI_STATUS_SYSTEM_IO_ERROR,
        SPI_STATUS_SLAVE_STATUS_INVALID_VALUE,
        SPI_STATUS_SLAVE_STATUS_NOT_MATCH,
        SPI_STATUS_SLAVE_ERROR_OCCUR,
        SPI_STATUS_SLAVE_ABNORMAL_POWER_OFF,
        SPI_STATUS_TIMEOUT,
        SPI_STATUS_WAIT_FOR_PADDING_ERROR,
        SPI_STATUS_SLAVE_POWER_ON_ERROR,
        SPI_STATUS_SLAVE_POWER_OFF_ERROR,
        SPI_STATUS_SLAVE_TX_BUFFER_EMPTY,
        SPI_STATUS_MSG_THREAD_NOT_RUNNING,
        SPI_STATUS_SLAVE_RX_BUFFER_FULL,
        SPI_STATUS_SLAVE_POWER_STATUS_INVALID,
        SPI_STATUS_ASYNC_SUCCESS = 5000,
        SPI_STATUS_ASYNC_ERROR = 5001,
    };
    enum SpiReadMode {
        SRM_POLLING,
        SRM_SIGNALING,
    };
    SpiDriver(const char *devName);
    ~SpiDriver();
    /**
     * @brief Set the Read Mode object, this should be set BEFORE open spi
     *
     * @param mode
     */
    void setReadMode(SpiReadMode mode);
    /**
     * trigger the driver to read for multiple time.
     * 
     * \param readTimes how many time to read
     * \param retryMs retry ms
     * \return 
     * This function is hot thread-safe
     */
    SpiDriverStatus triggerRead(int32_t readTimes, int64_t retryMs);
    SpiDriverStatus triggerReadConsecutive(bool enable, int64_t retryMs) = delete;
    SpiDriverStatus startRead();
    SpiDriverStatus stopRead();
    SpiDriverStatus asyncWrite(const void *buf, spi_size_t length);
    SpiDriverStatus asyncRead();
    SpiDriverStatus setForceReverse(bool enable);
    bool open();
    bool close();
    void setCallback(spiRxCallback callback, void *userdata);
    void setAsynckMode(bool enable);
    /**
     * Write Data, if async is enable, this function will return
     * SPI_STATUS_ASYNC_SUCCESS if success. if not, this function will return
     * SPI_STATUS_OK if success. \param data \param length \param bytesWrite
     * \return
     */
    SpiDriverStatus write(const uint8_t *data, spi_size_t length,
                          spi_size_t *bytesWrite);
    /**
     * Query tx buffer size.
     *
     * \return return how many byte in tx buffer
     */
    uint32_t querySlaveTxBufferSize();
    /**
     * Query rx buffer.
     *
     * \return return the free space in rx buffer
     */
    uint32_t querySlaveRxBufferSize();
    std::string readAll();
    SpiDriverStatus read(void *buffer, spi_size_t length,
                         spi_size_t *bytesRead);
    /**
     * If the power-off of the GPS chip occurs before the shutdown of the SPI
     * driver, there's a chance that the SPI read operation could be stuck in a
     * loop for a long time (as it's not achieving a certain status). Therefore,
     * we've implemented an API that allows the upper layer to inform the SPI
     * driver about the power-down. In this way, the SPI driver can better
     * maintain its internal state.
     */
    int notifySlavePowerState(bool powerOn);

 protected:
    virtual void onDataIn(const uint8_t *data, spi_size_t length) = 0;
    void onExpire(void *userdata) override;

 private:
    enum SpiDriverMessageID {
        SDM_CLOSE_DRIVER,
        SDM_READ_ONCE,
        SDM_SEND_DATA,
        SDM_TRY_RECOVER,
        SDM_OPEN_DRIVER,
    };
    struct SpiDriverMessage {
        explicit SpiDriverMessage(SpiDriverMessageID id, const void *data,
                                  spi_size_t length);
        ~SpiDriverMessage();
        SpiDriverMessageID mId;
        void *mData;
        spi_size_t mLength;
    };
    static void *spiPollThread(void *vp);
    pthread_t mPollThd;
    pthread_mutex_t mIoOperationMutex;
    pthread_mutex_t mSignalMutex;
    pthread_cond_t mSignalCond;
    SpiReadMode mReadMode;
    char mDevName[200];
    std::list<SpiDriverMessage *> mMessageQueue;
    SpiDriverStatus sendInternalMessage(SpiDriverMessageID id, const void *data,
                                        spi_size_t length);
    SpiDriverStatus sendReadRequest();
    SpiDriverStatus sendWriteRequest(const void *buf, spi_size_t length);
    void handleReadRequest(const SpiDriverMessage *);
    void handleWriteRequest(const SpiDriverMessage *);
    int spiFd;
    // SPI Protocal Flow
    // There functions are not thread-safe
    bool slavePowerOn();
    bool slavePowerOff();
    uint32_t getSlaveRxFreeBufferSize();
    uint32_t getSlaveTxBufferSize();
    // SPI Read/Write Flow Function
    bool spiPowerOn();
    bool spiPowerOff();
    SpiDriverStatus spiReadReg32(uint32_t regAddr, void *buffer,
                                 spi_size_t readLength, spi_size_t *bytesRead);
    SpiDriverStatus spiWriteReg32(uint32_t regAddr, const void *buffer,
                                  spi_size_t writeLength,
                                  spi_size_t *bytesWrite);
    /**
     * Read Data with Airoha Protocal.
     *
     * \param buffer
     * \param len
     * \param readLen
     * \return
     */
    SpiDriverStatus spiReadData(uint8_t *buffer, spi_size_t len,
                                spi_size_t *readLen);
    /**
     * Write Data with Airoha Protocal.
     *
     * \param buffer
     * \param len
     * \return
     */
    SpiDriverStatus spiWriteData(const void *buffer, spi_size_t len,
                                 spi_size_t *bytesWrite);
    SpiSlaveStatus spiQueryStatus();
    SpiDriverStatus spiQuertStatusWithRetry(SpiSlaveStatus bit_mask,
                                            SpiSlaveStatus bit_value,
                                            uint32_t retry_counter,
                                            bool inPowerOff = false);
    SpiDriverStatus spiClearErrorStatus(SpiSlaveStatus statusToClear);
    SpiDriverStatus spiSendPaddingChar(spi_size_t length, uint8_t ch);
    /**
     * This is an internal function.
     * To wait rx has enough buffer.
     *
     * \param threshold
     * \return
     */
    SpiDriverStatus spiWaitRxBufferEmpty(spi_size_t threshold);
    bool spiTransfer(uint8_t *txBuffer, uint8_t *rxBuffer, spi_size_t len);
    // SPI Driver Config
    const static spi_size_t kSpiPacketSize = 1024;
    const static uint32_t kPowerOnRetryCount = 5;
    const static uint32_t kPowerOffRetryCount = 5;
    const static uint32_t kQueryStatusRetryCount = 1000;
    static const int64_t kWaitBufferEmptyTimeRetryMs = 1;
    static const int64_t kWaitBufferEmptyTimeCount = 10;
    static const spi_size_t kRxBufferThreshold = 2048;
    static const spi_size_t kPaddingThreshold = 2;
    static const uint8_t kPaddingChar = 0xFF;
    uint8_t mTxBuffer[4097];
    uint8_t mRxBuffer[4097];
    // SPI Cmd
    const static uint8_t kInvalidSlaveStatus = 0xFF;
    const static uint8_t kSpisCfgRdCmd = 0x0a;
    const static uint8_t kSpisRdCmd = 0x81;
    const static uint8_t kSpisCfgWrCmd = 0x0c;
    const static uint8_t kSpisWrCmd = 0x0e;
    const static uint8_t kSpisWsCmd = 0x08;
    const static uint8_t kSpisRsCmd = 0x06;
    const static uint8_t kSpisPowerOnCmd = 0x04;
    const static uint8_t kSpisPowerOffCmd = 0x02;
    const static uint8_t kSpisCtCmd = 0x10;
    // virtual addr
    const static uint32_t kTxLengthAddr = 0x08;
    const static uint32_t kTxBufferAddr = 0x2000;
    const static uint32_t kRxLengthAddr = 0x04;
    const static uint32_t kRxBufferAddr = 0x1000;
    const static uint32_t kSpisSlavevOnOffset = 0;
    const static SpiSlaveStatus kSpisSlavevOnMask =
        (0x1 << kSpisSlavevOnOffset);
    const static uint32_t kSpisSlaveFifoReadyOffset = 2;
    const static SpiSlaveStatus kSpisSlaveFifoReadyMask =
        (0x1 << kSpisSlaveFifoReadyOffset);
    const static uint32_t kSpisSlaveXferFinishOffset = 5;
    const static SpiSlaveStatus kSpisSlaveXferFinishMask =
        (0x1 << kSpisSlaveXferFinishOffset);
    const static uint32_t kSpisSlaveReadErrorOffset = 3;
    const static SpiSlaveStatus kSpisSlaveReadErrorMask =
        (0x1 << kSpisSlaveReadErrorOffset);
    const static uint32_t kSpisSlaveWriteErrorOffset = 4;
    const static SpiSlaveStatus kSpisSlaveWriteErrorMask =
        (0x1 << kSpisSlaveWriteErrorOffset);
    const static SpiSlaveStatus kNotAllowPowerOffMask =
        (kSpisSlaveXferFinishMask | kSpisSlaveFifoReadyMask);
    static bool isSlavePowerOn(SpiSlaveStatus status);
    spiRxCallback mCallback;
    void *mUserdata;
    bool mAsyncMode;
    SysTimer *mSpiReadTimer;
    bool mIsDriverThreadRunning;
    bool mContinuouslyRead;
    uint32_t mReadTimes;
    bool mForceBitReverse;
    std::atomic_bool mSlavePowerOn;
    std::atomic_bool mIsCloseProcedure;
};
}  // namespace communicator
}  // namespace airoha
#endif  // DRIVER_COMM_SPI_DRIVER_H_

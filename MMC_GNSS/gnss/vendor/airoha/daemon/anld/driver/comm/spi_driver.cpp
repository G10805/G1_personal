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
#define LOG_TAG "Spi_D"
#include "spi_driver.h"
#include <assert.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include "simulation.h"
// #define SPI_DRIVER_DEBUG
#define SPI_SUPPORT_LSB_CONFIG
using airoha::communicator::spi_size_t;
using airoha::communicator::SpiDriver;
using airoha::communicator::SpiSlaveStatus;
using SpiDriverStatus = airoha::communicator::SpiDriver::SpiDriverStatus;
SpiDriver::SpiDriver(const char *devName) : SysTimer() {
    memset(mDevName, 0, sizeof(mDevName));
    pthread_mutex_init(&mSignalMutex, nullptr);
    pthread_mutex_init(&mIoOperationMutex, nullptr);
    pthread_cond_init(&mSignalCond, nullptr);
    strncpy(mDevName, devName, sizeof(mDevName));
    mCallback = nullptr;
    mUserdata = nullptr;
    mAsyncMode = false;
    mSpiReadTimer = nullptr;
    mIsDriverThreadRunning = false;
    mContinuouslyRead = false;
    mReadTimes = 0;
    mForceBitReverse = false;
    spiFd = -1;
    mSlavePowerOn = false;
    mIsCloseProcedure = false;
    memset(mRxBuffer, 0xFE, sizeof(mRxBuffer));
}
SpiDriver::~SpiDriver() {
    if (mIsDriverThreadRunning) {
        close();
    }
}
void SpiDriver::setReadMode(SpiDriver::SpiReadMode mode) {
    mReadMode = mode;
    return;
}
SpiDriverStatus SpiDriver::startRead() {
    mContinuouslyRead = true;
    return sendReadRequest();
}
SpiDriverStatus SpiDriver::stopRead() {
    mContinuouslyRead = false;
    SysTimer::stop();
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiDriverStatus SpiDriver::asyncWrite(const void *buf, spi_size_t length) {
    SpiDriverStatus status = sendWriteRequest(buf, length);
    return status;
}
SpiDriverStatus SpiDriver::asyncRead() {
    SpiDriverStatus status = sendReadRequest();
    return status;
}
SpiDriverStatus SpiDriver::triggerRead(int32_t readTimes, int64_t retryMs) {
    mReadTimes = readTimes;
    SysTimer::stop();
    SysTimer::setDuration(retryMs);
    SysTimer::start();
    return SpiDriverStatus::SPI_STATUS_OK;
}
void SpiDriver::onDataIn(const uint8_t *data, spi_size_t length) {
    (void)data;
    (void)length;
}
SpiDriverStatus SpiDriver::sendInternalMessage(SpiDriverMessageID id,
                                               const void *data,
                                               spi_size_t length) {
    bool putFront = false;
    pthread_mutex_lock(&mSignalMutex);
    if (mIsDriverThreadRunning == false) {
        pthread_mutex_unlock(&mSignalMutex);
        return SpiDriverStatus::SPI_STATUS_MSG_THREAD_NOT_RUNNING;
    }
    if (id == SpiDriverMessageID::SDM_CLOSE_DRIVER) {
        mIsDriverThreadRunning = false;
        putFront = true;
    }
    SpiDriverMessage *msg = new SpiDriverMessage(id, data, length);
    if (putFront) {
        mMessageQueue.push_front(msg);
    } else {
        mMessageQueue.push_back(msg);
    }
    pthread_cond_signal(&mSignalCond);
    pthread_mutex_unlock(&mSignalMutex);
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiDriver::SpiDriverMessage::SpiDriverMessage(SpiDriverMessageID id,
                                              const void *data,
                                              spi_size_t length) {
    mId = id;
    if (data) {
        assert(length > 0);
        mData = malloc(length);
        memcpy(mData, data, length);
    } else {
        mData = nullptr;
    }
    mLength = length;
}
SpiDriver::SpiDriverMessage::~SpiDriverMessage() {
    if (mData) {
        free(mData);
    }
}
void *SpiDriver::spiPollThread(void *vp) {
    SpiDriver *instance = static_cast<SpiDriver *>(vp);
    bool running = true;
    LOG_D("spi driver %p thread running", vp);
    while (running) {
        pthread_mutex_lock(&instance->mSignalMutex);
        while (instance->mMessageQueue.size() == 0) {
            pthread_cond_wait(&instance->mSignalCond, &instance->mSignalMutex);
        }
        SpiDriverMessage *msg = instance->mMessageQueue.front();
        LOG_D("spi message: %d", msg->mId);
        instance->mMessageQueue.pop_front();
        pthread_mutex_unlock(&instance->mSignalMutex);
        switch (msg->mId) {
            case SpiDriverMessageID::SDM_CLOSE_DRIVER: {
                running = false;
                break;
            }
            case SpiDriverMessageID::SDM_SEND_DATA: {
                instance->handleWriteRequest(msg);
                break;
            }
            case SpiDriverMessageID::SDM_READ_ONCE: {
                instance->handleReadRequest(msg);
                break;
            }
            default:
                break;
        }
        delete (msg);
    }
    while (instance->mMessageQueue.size() > 0) {
        // delete item remain in message queue
        SpiDriverMessage *msg = instance->mMessageQueue.front();
        instance->mMessageQueue.pop_front();
        delete (msg);
    }
    LOG_D("spi driver %p thread close", vp);
    return nullptr;
}
SpiDriverStatus SpiDriver::sendReadRequest() {
    SpiDriverStatus status =
        sendInternalMessage(SpiDriverMessageID::SDM_READ_ONCE, nullptr, 0);
    return status;
}
SpiDriverStatus SpiDriver::sendWriteRequest(const void *buf,
                                            spi_size_t length) {
    SpiDriverStatus status =
        sendInternalMessage(SpiDriverMessageID::SDM_SEND_DATA, buf, length);
    return status;
}
void SpiDriver::handleReadRequest(const SpiDriverMessage *msg) {
    (void)msg;
    std::string data = readAll();
    onDataIn(reinterpret_cast<const uint8_t *>(data.data()), data.size());
    if (!mContinuouslyRead) {
        return;
    }
    if (data.size() > 0) {
        sendReadRequest();
    } else {
        SysTimer::setDuration(500);
        SysTimer::start();
    }
}
void SpiDriver::handleWriteRequest(const SpiDriverMessage *msg) {
    spi_size_t bytesWrite = 0;
    write(static_cast<const uint8_t *>(msg->mData), msg->mLength, &bytesWrite);
    if (bytesWrite != msg->mLength) {
        LOG_E("spi write request failed");
    }
    return;
}
bool SpiDriver::spiPowerOn() {
    LOG_D("spi power on");
    uint8_t powerOnCmd[1] = {kSpisPowerOnCmd};
    uint32_t counter = 0;
    for (counter = 0; counter < kPowerOnRetryCount; counter++) {
        spiTransfer(powerOnCmd, mRxBuffer, 1);
        SpiDriverStatus ret = spiQuertStatusWithRetry(
            kSpisSlavevOnMask, kSpisSlavevOnMask, kQueryStatusRetryCount);
        if (ret == SpiDriverStatus::SPI_STATUS_OK) {
            return true;
        }
    }
    return false;
}
bool SpiDriver::spiPowerOff() {
    LOG_D("spi power off");
    uint8_t powerOffCmd[1] = {kSpisPowerOffCmd};
    uint32_t counter = 0;
    for (counter = 0; counter < kPowerOffRetryCount; counter++) {
        spiTransfer(powerOffCmd, mRxBuffer, 1);
        SpiDriverStatus ret = spiQuertStatusWithRetry(kSpisSlavevOnMask, 0,
                                                      kQueryStatusRetryCount, true);
        if (ret == SpiDriverStatus::SPI_STATUS_OK) {
            return true;
        }
    }
    return false;
}
SpiDriverStatus SpiDriver::spiReadReg32(uint32_t regAddr, void *buffer,
                                        spi_size_t readLength,
                                        spi_size_t *bytesRead) {
#ifdef SPI_DRIVER_DEBUG
    LOG_D("spi read reg32: 0x%08x, %zu", regAddr, readLength);
#endif
    bool ret = false;
    SpiDriverStatus status;
    *bytesRead = 0;
    spi_size_t toRead =
        (readLength > kSpiPacketSize) ? kSpiPacketSize : readLength;
    // Step 1: Config Read
    uint8_t requestCmd[9] = {0};
    requestCmd[0] = kSpisCfgRdCmd;
    requestCmd[1] = regAddr & 0xff;
    requestCmd[2] = (regAddr >> 8) & 0xff;
    requestCmd[3] = (regAddr >> 16) & 0xff;
    requestCmd[4] = (regAddr >> 24) & 0xff;
    requestCmd[5] = (toRead - 1) & 0xff;
    requestCmd[6] = ((toRead - 1) >> 8) & 0xff;
    requestCmd[7] = ((toRead - 1) >> 16) & 0xff;
    requestCmd[8] = ((toRead - 1) >> 24) & 0xff;
    ret = spiTransfer(requestCmd, mRxBuffer, 9);
    if (!ret) {
        return SpiDriverStatus::SPI_STATUS_SYSTEM_IO_ERROR;
    }
    // Step 2: Wait Data Prepare Done
    status = spiQuertStatusWithRetry(kSpisSlaveFifoReadyMask,
                                     kSpisSlaveFifoReadyMask,
                                     kQueryStatusRetryCount);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("spi cfg rd error: %d", status);
        return status;
    }
    // Step 3: read data
    mTxBuffer[0] = kSpisRdCmd;
    ret = spiTransfer(mTxBuffer, mRxBuffer, toRead + 1);
    if (!ret) {
        return SpiDriverStatus::SPI_STATUS_SYSTEM_IO_ERROR;
    }
    status = spiQuertStatusWithRetry(kSpisSlaveXferFinishMask,
                                     kSpisSlaveXferFinishMask,
                                     kQueryStatusRetryCount);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("spi read error: %d", status);
        return status;
    }
    memcpy(buffer, &(mRxBuffer[1]), toRead);
    *bytesRead = toRead;
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiDriverStatus SpiDriver::spiWriteReg32(uint32_t regAddr, const void *buffer,
                                         spi_size_t writeLength,
                                         spi_size_t *bytesWrite) {
    LOG_D("spi write reg: %x, %p, %zu", regAddr, buffer, writeLength);
    bool ret = false;
    spi_size_t toWrite =
        (writeLength > kSpiPacketSize) ? kSpiPacketSize : writeLength;
    // Step 1: Config Read
    uint8_t requestCmd[9] = {0};
    requestCmd[0] = kSpisCfgWrCmd;
    requestCmd[1] = regAddr & 0xff;
    requestCmd[2] = (regAddr >> 8) & 0xff;
    requestCmd[3] = (regAddr >> 16) & 0xff;
    requestCmd[4] = (regAddr >> 24) & 0xff;
    requestCmd[5] = (toWrite - 1) & 0xff;
    requestCmd[6] = ((toWrite - 1) >> 8) & 0xff;
    requestCmd[7] = ((toWrite - 1) >> 16) & 0xff;
    requestCmd[8] = ((toWrite - 1) >> 24) & 0xff;
    ret = spiTransfer(requestCmd, mRxBuffer, 9);
    if (!ret) {
        return SpiDriverStatus::SPI_STATUS_SYSTEM_IO_ERROR;
    }
    // Step 2: Wait Data Prepare Done
    SpiDriverStatus status = spiQuertStatusWithRetry(kSpisSlaveFifoReadyMask,
                                                     kSpisSlaveFifoReadyMask,
                                                     kQueryStatusRetryCount);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("spi cfg wr error: %d", status);
        return status;
    }
    // Step 3: read data
    mTxBuffer[0] = kSpisWrCmd;
    memcpy(&(mTxBuffer[1]), buffer, toWrite);
    ret = spiTransfer(mTxBuffer, mRxBuffer, toWrite + 1);
    if (!ret) {
        return SpiDriverStatus::SPI_STATUS_SYSTEM_IO_ERROR;
    }
    status = spiQuertStatusWithRetry(kSpisSlaveXferFinishMask,
                                     kSpisSlaveXferFinishMask,
                                     kQueryStatusRetryCount);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("spi write error: %d", status);
        return status;
    }
    *bytesWrite = toWrite;
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiDriverStatus SpiDriver::spiReadData(uint8_t *buffer, spi_size_t len,
                                       spi_size_t *readLen) {
    // pass to spi Read Reg for check
    // uint32_t validLength = getSlaveTxBufferSize();
    // if (validLength == 0) {
    //    return SpiDriverStatus::SPI_STATUS_SLAVE_TX_BUFFER_EMPTY;
    //}
    // validLength = validLength > len ? len : validLength;
    SpiDriverStatus status = spiReadReg32(kTxBufferAddr, buffer, len, readLen);
    return status;
}
SpiDriverStatus SpiDriver::spiWriteData(const void *buffer, spi_size_t len,
                                        spi_size_t *bytesWrite) {
    // When receive data, 3335 SPI driver may try to fill an array from index 0
    // to index n. Regardless of whether the previous data is read by the
    // application layer, the data pointer will only shift backwards. To make
    // sure the data is transfer, we use a loop to send message Before send
    // message, if the remain buffer is less than or equal 2, we will send 0xFF
    // to try to reset buffer.
    spi_size_t bytesWriteToReg = 0;
    // spi_size_t totalWrite = 0;
    SpiDriverStatus status = SpiDriverStatus::SPI_STATUS_OK;
    uint32_t validLength = getSlaveRxFreeBufferSize();
// Should not add padding here because of the following case.
// 1. spi buffer remain 4096 bytes
// 2. first send 4094 and then send 2 bytes.
// Result: There are extra 2 bytes in the data because of padding.
#if 0
    bool needPadding = false;
    if (validLength < kPaddingThreshold) {
        needPadding = true;
        spiSendPaddingChar(validLength, kPaddingChar);
        status = spiWaitRxBufferEmpty(kRxBufferThreshold);
        if (status != SpiDriverStatus::SPI_STATUS_OK) {
            LOG_E("spi driver error: write data failed");
            return SpiDriverStatus::SPI_STATUS_WAIT_FOR_PADDING_ERROR;
        }
    }
    if (needPadding) {
        // because slave rx pointer is wrap to front.
        // get free buffer size again
        validLength = getSlaveRxFreeBufferSize();
    }
#endif
    LOG_D("rx free buffer : %u", validLength);
    if (validLength == 0) {
        return SpiDriverStatus::SPI_STATUS_SLAVE_RX_BUFFER_FULL;
    }
    validLength = validLength > len ? len : validLength;
    status =
        spiWriteReg32(kRxBufferAddr, buffer, validLength, &bytesWriteToReg);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("spi write data failed");
        return status;
    }
    *bytesWrite = bytesWriteToReg;
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiSlaveStatus SpiDriver::spiQueryStatus() {
    uint8_t status_cmd[2] = {kSpisRsCmd, 0};
    uint8_t status_receive[2] = {0};
    /* Note:
     * The value of receive_length is the valid number of bytes received plus
     * the number of bytes to send. For example, here the valid number of bytes
     * received is 1 byte, and the number of bytes to send also is 1 byte, so
     * the receive_length is 2.
     */
    if (spiTransfer(status_cmd, status_receive, 2) != true) {
        LOG_E("[SPIM] SPI master query status of slaver failed");
        return kInvalidSlaveStatus;
    }
    // LOG_D("[SPIM] Status receive: 0x%x", status_receive[1]);
    return status_receive[1];
}
SpiDriverStatus SpiDriver::spiQuertStatusWithRetry(SpiSlaveStatus bitMask,
                                                   SpiSlaveStatus bitValue,
                                                   uint32_t retryCounter,
                                                   bool inPowerOff) {
    SpiSlaveStatus status = 0xFF;
    while (retryCounter > 0) {
        status = spiQueryStatus();
#ifdef SPI_DRIVER_DEBUG
        LOG_D("spiQuertStatusWithRetry status: 0x%02x", status);
#endif
        if (!mSlavePowerOn || mIsCloseProcedure) {
            return SpiDriverStatus::SPI_STATUS_SLAVE_POWER_STATUS_INVALID;
        }
        if (status == kInvalidSlaveStatus && inPowerOff) {
            // If slave power off, it's possible for host to read 0xFF
            return SpiDriverStatus::SPI_STATUS_OK;
        } else if (status == kInvalidSlaveStatus) {
            // do nothing
        } else if ((status & kSpisSlaveReadErrorMask) ||
                   (status & kSpisSlaveWriteErrorMask)) {
            spiClearErrorStatus(status);
            LOG_E(
                "spiQuertStatusWithRetry status: SPI_STATUS_SLAVE_ERROR_OCCUR");
            return SpiDriverStatus::SPI_STATUS_SLAVE_ERROR_OCCUR;
        } else if ((bitMask & status) == bitValue) {
            LOG_D("spiQuertStatusWithRetry status: SPI_STATUS_OK");
            return SpiDriverStatus::SPI_STATUS_OK;
        } else if ((bitMask & kNotAllowPowerOffMask) &&
                   (!isSlavePowerOn(status))) {
            // if bitMask waiting for a mask that not allow poweroff
            LOG_E(
                "spiQuertStatusWithRetry status: "
                "SPI_STATUS_SLAVE_ABNORMAL_POWER_OFF");
            return SpiDriverStatus::SPI_STATUS_SLAVE_ABNORMAL_POWER_OFF;
        }
        retryCounter--;
        // sleep 1ms for retry
        usleep(1000);
    }
    LOG_E("spiQuertStatusWithRetry failed, last status: 0x%02x", status);
    return SpiDriverStatus::SPI_STATUS_SLAVE_STATUS_NOT_MATCH;
}
SpiDriverStatus SpiDriver::spiClearErrorStatus(SpiSlaveStatus statusToClear) {
    uint8_t clearCmd[2];
    clearCmd[0] = kSpisWsCmd;
    clearCmd[1] = statusToClear;
    if (spiTransfer(clearCmd, mRxBuffer, 2) == true) {
        return SpiDriverStatus::SPI_STATUS_OK;
    }
    return SpiDriverStatus::SPI_STATUS_SYSTEM_IO_ERROR;
}
SpiDriverStatus SpiDriver::spiSendPaddingChar(spi_size_t length, uint8_t ch) {
    uint8_t *buf = (uint8_t *)malloc(length);
    spi_size_t writeBytes;
    memset(buf, ch, length);
    SpiDriverStatus status =
        spiWriteReg32(kRxBufferAddr, buf, length, &writeBytes);
    free(buf);
    return status;
}
SpiDriverStatus SpiDriver::spiWaitRxBufferEmpty(spi_size_t threshold) {
    LOG_D("Wait buffer empty");
    int64_t retryCount = kWaitBufferEmptyTimeCount;
    while (retryCount > 0) {
        spi_size_t free = getSlaveRxFreeBufferSize();
        if (free >= threshold) {
            return SpiDriverStatus::SPI_STATUS_OK;
        }
        retryCount--;
        usleep(kWaitBufferEmptyTimeRetryMs * 1000);
    }
    LOG_E("wait buffer empty timeout");
    return SpiDriverStatus::SPI_STATUS_TIMEOUT;
}
static uint8_t bitReverseU8(uint8_t byte) {
    byte = ((byte & 0x55) << 1) | ((byte & 0xAA) >> 1);
    byte = ((byte & 0x33) << 2) | ((byte & 0xCC) >> 2);
    byte = ((byte & 0x0F) << 4) | ((byte & 0xF0) >> 4);
    return byte;
}
bool SpiDriver::spiTransfer(uint8_t *txBuffer, uint8_t *rxBuffer,
                            spi_size_t len) {
#ifndef AIROHA_SIMULATION
    if (mForceBitReverse) {
        for (size_t i = 0; i < len; i++) {
            // LOG_D("spiTransfer send[%d] 0x%x", i, txBuffer[i]);
            txBuffer[i] = bitReverseU8(txBuffer[i]);
            // txBuffer[i] = BitReverseTable256[txBuffer[i]];
            // LOG_D("spiTransfer send after reverse     0x%x", txBuffer[i]);
        }
    }
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)txBuffer,
        .rx_buf = (unsigned long)rxBuffer,
        .len = static_cast<uint32_t>(len),
        .delay_usecs = 10,
        .speed_hz = 2000000,
        .bits_per_word = 8,
        .tx_nbits = 1,
        .rx_nbits = 1,
    };
    int ret = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (mForceBitReverse) {
        for (size_t i = 0; i < len; i++) {
            // LOG_D("spiTransfer send[%d] 0x%x", i, rxBuffer[i]);
            rxBuffer[i] = bitReverseU8(rxBuffer[i]);
            // txBuffer[i] = BitReverseTable256[txBuffer[i]];
            // LOG_D("spiTransfer send after reverse     0x%x", txBuffer[i]);
        }
    }
    if (ret < 0) {
        LOG_E("Func:%s %d,%d", __FUNCTION__, ret, errno);
        return false;
    }
#else  // AIROHA_SIMULATION
    (void)txBuffer;
    (void)rxBuffer;
    (void)len;
    (void)bitReverseU8;
#endif  // AIROHA_SIMULATION
    return true;
}
bool SpiDriver::open() {
    mIsCloseProcedure = false;
#ifndef AIROHA_SIMULATION
    spiFd = ::open(mDevName, O_RDWR);
    if (spiFd == -1) {
        return false;
    }
#ifdef SPI_SUPPORT_LSB_CONFIG
    uint32_t mode = SPI_LSB_FIRST;
    if (ioctl(spiFd, SPI_IOC_WR_MODE32, &mode) == -1) {
        LOG_E("set mode failed");
        return false;
    }
    uint8_t lsb_seting = 1;
    if (ioctl(spiFd, SPI_IOC_WR_LSB_FIRST, &lsb_seting) == -1) {
        LOG_E("lsb_seting failed");
        return false;
    }
#else
    mForceBitReverse = true;
#endif
    // start thread
    pthread_create(&mPollThd, nullptr, spiPollThread, this);
    pthread_mutex_lock(&mSignalMutex);
    mIsDriverThreadRunning = true;
    pthread_mutex_unlock(&mSignalMutex);
    LOG_D("spi driver open done");
#endif
    startRead();
    return true;
}
bool SpiDriver::close() {
    mIsCloseProcedure = true;
    LOG_D("spi driver close");
    if (spiFd == -1) {
        LOG_W("spi driver already close");
        return true;
    }
    sendInternalMessage(SpiDriverMessageID::SDM_CLOSE_DRIVER, nullptr, 0);
    pthread_join(mPollThd, nullptr);
    ::close(spiFd);
    return true;
}
void SpiDriver::setCallback(spiRxCallback callback, void *userdata) {
    mCallback = callback;
    mUserdata = userdata;
}
void SpiDriver::setAsynckMode(bool enable) {
    mAsyncMode = enable;
    return;
}
SpiDriverStatus SpiDriver::write(const uint8_t *data, spi_size_t length,
                                 spi_size_t *bytesWrite) {
    spi_size_t totalWrite = 0;
    spi_size_t writeLength = 0;
    *bytesWrite = 0;
    LOG_D("spi write %p, %zu", data, length);
    pthread_mutex_lock(&mIoOperationMutex);
    SpiDriverStatus status = SpiDriverStatus::SPI_STATUS_OK;
    if (slavePowerOn() == false) {
        pthread_mutex_unlock(&mIoOperationMutex);
        return SpiDriverStatus::SPI_STATUS_SLAVE_POWER_ON_ERROR;
    }
    while (totalWrite < length) {
        status =
            spiWriteData(data + totalWrite, length - totalWrite, &writeLength);
        if (status != SpiDriverStatus::SPI_STATUS_OK) {
            slavePowerOff();
            pthread_mutex_unlock(&mIoOperationMutex);
            return status;
        }
        totalWrite += writeLength;
        *bytesWrite += writeLength;
    }
    if (slavePowerOff() == false) {
        pthread_mutex_unlock(&mIoOperationMutex);
        return SpiDriverStatus::SPI_STATUS_SLAVE_POWER_OFF_ERROR;
    }
    pthread_mutex_unlock(&mIoOperationMutex);
    return SpiDriverStatus::SPI_STATUS_OK;
}
SpiDriverStatus SpiDriver::read(void *buffer, spi_size_t length,
                                spi_size_t *bytesRead) {
    spi_size_t totalRead = 0;
    SpiDriverStatus status = SpiDriverStatus::SPI_STATUS_OK;
    spi_size_t readLength = 0;
    *bytesRead = 0;
    pthread_mutex_lock(&mIoOperationMutex);
    slavePowerOn();
    uint8_t *p = static_cast<uint8_t *>(buffer);
    while (totalRead < length) {
        status = spiReadData(p + totalRead, length - totalRead, &readLength);
        // read data return empty when read length is 0
        if (status != SpiDriverStatus::SPI_STATUS_OK) {
            slavePowerOff();
            pthread_mutex_unlock(&mIoOperationMutex);
            return status;
        }
        totalRead += readLength;
        *bytesRead += readLength;
    }
    slavePowerOff();
    pthread_mutex_unlock(&mIoOperationMutex);
    return SpiDriverStatus::SPI_STATUS_OK;
}
int SpiDriver::notifySlavePowerState(bool powerOn) {
    mSlavePowerOn = powerOn;
    return 0;
}
std::string SpiDriver::readAll() {
    uint32_t rxBufferSize = querySlaveTxBufferSize();
    spi_size_t bytesRead;
    std::string str;
    if (rxBufferSize == 0) {
        return std::string();
    }
    LOG_D("readall %u", rxBufferSize);
    char *buffer = (char *)malloc(rxBufferSize);
    SpiDriverStatus status = read(buffer, rxBufferSize, &bytesRead);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        free(buffer);
        return std::string();
    }
    str = std::string(buffer, bytesRead);
    free(buffer);
    return str;
}
uint32_t SpiDriver::querySlaveTxBufferSize() {
    bool ret = false;
    pthread_mutex_lock(&mIoOperationMutex);
    ret = slavePowerOn();
    if (!ret) {
        LOG_D("query tx, slave power on failed");
        pthread_mutex_unlock(&mIoOperationMutex);
        return 0;
    }
    uint32_t length = getSlaveTxBufferSize();
    slavePowerOff();
    if (!ret) {
        LOG_D("query tx, slave power off failed");
        pthread_mutex_unlock(&mIoOperationMutex);
        return 0;
    }
    pthread_mutex_unlock(&mIoOperationMutex);
    return length;
}
uint32_t SpiDriver::querySlaveRxBufferSize() {
    bool ret = false;
    pthread_mutex_lock(&mIoOperationMutex);
    ret = slavePowerOn();
    if (!ret) {
        LOG_D("query rx, slave power on failed");
        pthread_mutex_unlock(&mIoOperationMutex);
        return 0;
    }
    uint32_t length = getSlaveRxFreeBufferSize();
    slavePowerOff();
    if (!ret) {
        LOG_D("query rx, slave power off failed");
        pthread_mutex_unlock(&mIoOperationMutex);
        return 0;
    }
    pthread_mutex_unlock(&mIoOperationMutex);
    return length;
}
bool SpiDriver::isSlavePowerOn(SpiSlaveStatus status) {
    if (status & kSpisSlavevOnMask) {
        return true;
    }
    return false;
}
bool SpiDriver::slavePowerOn() { return spiPowerOn(); }
bool SpiDriver::slavePowerOff() { return spiPowerOff(); }
uint32_t SpiDriver::getSlaveRxFreeBufferSize() {
    uint32_t rxFreeSize = 0;
    spi_size_t bytesRead = 0;
    SpiDriverStatus status = spiReadReg32(kRxLengthAddr, &rxFreeSize,
                                          sizeof(rxFreeSize), &bytesRead);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("%s Err: %d", __FUNCTION__, status);
    }
    return rxFreeSize;
}
uint32_t SpiDriver::getSlaveTxBufferSize() {
    uint32_t txFreeSize = 0;
    spi_size_t bytesRead = 0;
    SpiDriverStatus status = spiReadReg32(kTxLengthAddr, &txFreeSize,
                                          sizeof(txFreeSize), &bytesRead);
    if (status != SpiDriverStatus::SPI_STATUS_OK) {
        LOG_E("%s Err: %d", __FUNCTION__, status);
    }
    return txFreeSize;
}
// Function for SysTimer
void SpiDriver::onExpire(void *userdata) {
    (void)userdata;
    sendReadRequest();
    if (mReadTimes > 0) {
        SysTimer::start();
        mReadTimes--;
    }
    return;
}
SpiDriverStatus SpiDriver::setForceReverse(bool enable) {
    mForceBitReverse = enable;
    return SpiDriverStatus::SPI_STATUS_OK;
}

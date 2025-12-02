
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
#include "download_interface.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "native_call.h"
// for spi
#include <linux/spi/spidev.h>
#include "GPIOControl.h"
#include "simulation.h"
#define MIN_INT(x, y) (((x) > (y)) ? (y) : (x))
#if defined(UART_SPEED_UP_FOR_3M) &&             \
    (UART_RX_SUPPORT_RINGBUFFER == 0 ||          \
     (UART_SUPPORT_HARDWARE_FLOW_CONTROL == 0 && \
      UART_SUPPORT_SOFTWARE_FLOW_CONTROL == 0))
#error \
    "3M Baudrate must be use when ringbuffer support and sw/hw flowcontrol enable"
#endif
#if UART_SUPPORT_HARDWARE_FLOW_CONTROL
#error "Hardware FlowControl Not Support Now."
#endif
#define SPI_DOWNLOAD_CONFIG_SUPPORT_LSB
// #define QCOM_UART_CLOCK_FUNC
/**
 * ========================
 * Interface With Power ON/OFF
 * =========================
 */
ErrorCode BasicInterface::powerOn() {
    Airoha::GPIO::portPowerOnChip();
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode BasicInterface::powerOff() {
    Airoha::GPIO::portPowerOffChip();
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode BasicInterface::powerReset() {
    Airoha::GPIO::portResetChip();
    return ErrorCode::DL_ERROR_NO_ERROR;
}
/**
 * ========================
 * Interface With Power ON/OFF  End =========
 * =========================
 */
/**

 * ===========================
 *   Uart Download Interface ==========================
 * ===========================
 */
#define UART_DOWNLOAD_HAND_BAUD B115200
#define UART_DOWNLOAD_DA_BAUD B921600
#ifdef UART_RX_SUPPORT_RINGBUFFER
void *UartDownloadInterface::uartReadThd(void *p) {
    UartDownloadInterface *ins = (UartDownloadInterface *)p;
    struct pollfd pollFd[2];
    LOG_D("uart rx run, %s", ins->mDevName);
    uint8_t buffer[1024];
    uint8_t dstBuffer[1024];
    pollFd[0].fd = ins->mUartFd;
    pollFd[0].events = POLLIN;
    pollFd[1].fd = ins->mSocketPair[0];
    pollFd[1].events = POLLIN;
    while (1) {
        poll(pollFd, 2, -1);
        if (pollFd[1].revents & POLLIN) {
            break;
        } else if (pollFd[0].revents & POLLIN) {
            int k = read(ins->mUartFd, buffer, 1023);
            LOG_D("rx buffer read: %d", k);
            if (k > 0) {
                buffer[k] = 0;
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL
                if (ins->mFlowControl == FC_SOFTWARE) {
                    int escapedChar = 0;
                    int result = ins->uartSwFcRxConvert(
                        buffer, k, dstBuffer, sizeof(dstBuffer), &escapedChar);
                    if ((size_t) escapedChar > sizeof(dstBuffer)) {
                        LOG_E("abnormal reverse-escape operation");
                    }
                    ins->mUartBuffer->write(dstBuffer, result, true, 100);
                } else {
                    ins->mUartBuffer->write(buffer, k, true, 100);
                }
#else
                ins->mUartBuffer->write(buffer, k, true, 100);
#endif
                // p->mCallback(buffer, k, p->mUserdata);
            }
        }
    }
    LOG_D("close signal fd, %p", p);
    LOG_D("uart rx end, %s", ins->mDevName);
    fflush(stdout);
    return NULL;
}
#endif
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL
int UartDownloadInterface::uartSwFcTxConvert(const void *data, int length,
                                             void *dstBuffer, int dstLength,
                                             int *escapedLength) {
    if (length < 0 || data == nullptr || dstBuffer == nullptr ||
        dstLength < 0) {
        return -1;
    }
    const uint8_t *p = (const uint8_t *)data;
    uint8_t *pdst = (uint8_t *)dstBuffer;
    int i = 0;
    int j = 0;
    for (i = 0; i < length && j < dstLength; i++) {
        if (p[i] == 0x77) {
            // avoid that dst buffer is not enough
            if (dstLength - j >= 2) {
                pdst[j] = 0x77;
                pdst[j + 1] = 0x88;
                j += 2;
            } else
                break;
        } else if (p[i] == 0x13) {
            if (dstLength - j >= 2) {
                pdst[j] = 0x77;
                pdst[j + 1] = 0xEC;
                j += 2;
            } else
                break;
        } else if (p[i] == 0x11) {
            if (dstLength - j >= 2) {
                pdst[j] = 0x77;
                pdst[j + 1] = 0xEE;
                j += 2;
            } else
                break;
        } else {
            pdst[j] = p[i];
            j++;
        }
    }
    *escapedLength = i;
    return j;
}
int UartDownloadInterface::uartSwFcRxConvert(const void *data, int length,
                                             void *dstBuffer, int dstLength,
                                             int *escapedLength) {
    int i = 0;
    int j = 0;
    const uint8_t *arr = (const uint8_t *)data;
    uint8_t *dst = (uint8_t *)dstBuffer;
    // memcpy(p,)
    for (i = 0; i < length && j < dstLength; i++) {
        if (arr[i] == 0x77) {
            mEscapedMarker = true;
            continue;
        }
        if (mEscapedMarker) {
            if (arr[i] == 0x88) {
                dst[j] = 0x77;
            } else if (arr[i] == 0xEE) {

                dst[j] = 0x11;
            } else if (arr[i] == 0xEC) {
                dst[j] = 0x13;
            } else {
                LOG_E("unrecognized char 0x%" PRIx8, (uint8_t)arr[i]);
            }
            mEscapedMarker = false;
        } else {
            dst[j] = arr[i];
        }
        j++;
    }
    *escapedLength = i;
    return j;
}
#endif
UartDownloadInterface::UartDownloadInterface(const char *devName) {
    strncpy(mDevName, devName, sizeof(mDevName));
    mUartFd = -1;
    mFlowControl = FlowControl::FC_NONE;
    mU8Timeout = 50;
#if UART_RX_SUPPORT_RINGBUFFER
    mUartBuffer = new RBuffer(1024);
#endif
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL
    mEscapedMarker = false;
#endif
    mEnableQcPlatform = false;
}
ErrorCode UartDownloadInterface::openPort() {
    struct termios tty_config;
    int result;
    int fd = open(mDevName, O_RDWR | O_NONBLOCK);
    if (mEnableQcPlatform) {
        LOG_I("Lock uart in QC platform");
        result = ioctl(fd, 0x544D, 0);
        if (result != 0) {
            LOG_E("lock uart error: %d, %s", errno, strerror(errno));
        }
    }
    if (fd < 0) {
        LOG_D("open %s error: %s", mDevName, strerror(errno));
        return ErrorCode::DL_ERROR_FATAL_ERROR;
    }
    LOG_D("open tty %s successful", mDevName);
    // int flags = fcntl(fd, F_GETFL,0);
    // flags &= ~(O_NONBLOCK);
    // if (fcntl (fd, F_SETFL, flag) < 0){
    //     return false;
    // }
    result = tcgetattr(fd, &tty_config);
    if (result != 0) {
        LOG_E("get attr failed!!%d,%s", result, strerror(errno));
        return ErrorCode::DL_ERROR_FATAL_ERROR;
    }
    cfsetispeed(&tty_config, kHandshakeBaudrate);
    cfsetospeed(&tty_config, kHandshakeBaudrate);
    tty_config.c_lflag &=
        ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | NOFLSH | ISIG);
    tty_config.c_oflag &= ~(ONLCR | OPOST);
    tty_config.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                            ICRNL | IXON | IXOFF | IXANY);
    tty_config.c_cflag |= CS8;
    tty_config.c_cflag &= (~CRTSCTS);
    // tty_config.c_cc[VTIME] = 1;
    // tty_config.c_cc[VMIN] = 0;
    result = tcsetattr(fd, TCSANOW, &tty_config);
    if (result != 0) {
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    mUartFd = fd;
    tcflush(fd, TCIOFLUSH);
    mFlowControl = FC_NONE;
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL
    mEscapedMarker = false;
#endif
#if UART_RX_SUPPORT_RINGBUFFER
    int socRet = socketpair(AF_UNIX, SOCK_STREAM, 0, mSocketPair);
    if (socRet != 0) {
        LOG_E("Create Socket Error!");
        return ErrorCode::DL_ERROR_FATAL_ERROR;
    }
    LOG_D("vreate thread %p", this);
    pthread_create(&mReadThreadHdr, nullptr, uartReadThd, this);
#endif
    LOG_D("Set U8 Timeout to 50ms");
    mU8Timeout = 50;
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode UartDownloadInterface::closePort() {
    if (mEnableQcPlatform) {
        LOG_I("Unlock uart in QC platform");
        ioctl(mUartFd, 0x544E, 0);
    }
#if UART_RX_SUPPORT_RINGBUFFER
    LOG_D("send signal to read thread");
    native_safe_write(mSocketPair[1], "Close", 5);
    pthread_join(mReadThreadHdr, NULL);
    close(mSocketPair[0]);
    close(mSocketPair[1]);
    LOG_D("close buffer success");
#endif
    close(mUartFd);
	mUartFd = -1;
    LOG_D("close port succeed");
    return ErrorCode::DL_ERROR_NO_ERROR;
}
CharEx UartDownloadInterface::readU8() {
    // for u8 , because of handshake, we use timeout machanism
    CharEx r;
#if UART_RX_SUPPORT_RINGBUFFER
    uint8_t buf[1];
    int res = mUartBuffer->read(buf, 1, true, mU8Timeout);
    LOG_D("uart read u8 %d %02X", res, (res == 1) ? buf[0] : 0xFF);
    if (res == 0) {
        r.error = ErrorCode::DL_ERROR_READ_TIMEOUT;
    } else {
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
        r.dataU8 = buf[0];
    }
#else
    int res;
    uint8_t buf[1];
    // Step 1: Try Read
    res = read(mUartFd, buf, 1);
    if (res == 1) {
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
        r.dataU8 = buf[0];
        LOG_D("read u8 %x", r.dataU8);
        return r;
    }
    // Step 2: if no responde, just wait and read
    struct pollfd poll_fd;
    poll_fd.fd = mUartFd;
    poll_fd.events = POLLIN;
    int poll_ret = poll(&poll_fd, 1, mU8Timeout);
    if (poll_ret <= 0) {
        r.error = ErrorCode::DL_ERROR_READ_TIMEOUT;
        LOG_E("read U8 timeout,%d,%" PRIu64, errno, mU8Timeout);
        return r;
    }
    res = read(mUartFd, buf, 1);
    r.error = ErrorCode::DL_ERROR_NO_ERROR;
    r.dataU8 = buf[0];
    LOG_D("read u8 %x", r.dataU8);
#endif  // UART_RX_SUPPORT_RINGBUFFER
    return r;
}
CharEx UartDownloadInterface::readU16() {
    uint16_t result;
    CharEx r;
    if (uartReadBytes(&result, 2) == ErrorCode::DL_ERROR_NO_ERROR) {
        reverse(reinterpret_cast<uint8_t *>(&result), 2);
        r.dataU16 = result;
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
        LOG_D("read16: 0x%x", r.dataU16);
    } else {
        r.error = ErrorCode::DL_ERROR_IO_ERROR;
    }
    return r;
}
CharEx UartDownloadInterface::readU32() {
    uint32_t result;
    CharEx r;
    if (uartReadBytes(&result, 4) == ErrorCode::DL_ERROR_NO_ERROR) {
        reverse(reinterpret_cast<uint8_t *>(&result), 4);
        r.dataU32 = result;
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
    } else {
        r.error = ErrorCode::DL_ERROR_IO_ERROR;
    }
    return r;
}
ErrorCode UartDownloadInterface::readBytes(void *buffer, size_t length,
                                           uint32_t *bytesRead) {
    if (uartReadBytes(buffer, length) == ErrorCode::DL_ERROR_NO_ERROR) {
        *bytesRead = length;
        return ErrorCode::DL_ERROR_NO_ERROR;
    }
    return ErrorCode::DL_ERROR_IO_ERROR;
}
ErrorCode UartDownloadInterface::sendU8(uint8_t dataU8) {
    reverse(reinterpret_cast<uint8_t *>(&dataU8), 1);
    LOG_D("send u8: 0x%02x", dataU8);
    ErrorCode ret = uartWriteBytes(&dataU8, 1);
    return ret;
}
ErrorCode UartDownloadInterface::sendU16(uint16_t dataU16) {
    reverse(reinterpret_cast<uint8_t *>(&dataU16), 2);
    LOG_D("send u16: 0x%04x", dataU16);
    ErrorCode ret = uartWriteBytes(&dataU16, 2);
    return ret;
}
ErrorCode UartDownloadInterface::sendU32(uint32_t dataU32) {
    reverse(reinterpret_cast<uint8_t *>(&dataU32), 4);
    LOG_D("send u32: 0x%08x", dataU32);
    ErrorCode ret = uartWriteBytes(&dataU32, 4);
    return ret;
}
ErrorCode UartDownloadInterface::sendBytes(const void *buffer, size_t length) {
    ErrorCode ret = ErrorCode::DL_ERROR_NO_ERROR;
    size_t written = 0;
    while (written < length) {
        size_t roundWritten = 0;
        ret = uartWriteBytes(((uint8_t *)buffer) + written, length - written, &roundWritten);
        if (ret != ErrorCode::DL_ERROR_NO_ERROR) {
            break;
        }
        written += roundWritten;
    }
    if (written != length) {
        LOG_E("something occur in send data: %zu, %zu", written, length);
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    return ret;
}
ErrorCode UartDownloadInterface::raiseSpeed(int baudrate) {
    struct termios tty_config;
    int result;
    result = tcgetattr(mUartFd, &tty_config);
    if (result < 0) {
        LOG_E("Get tty attr error");
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    int baud = B115200;
    switch (baudrate) {
        case 115200:
            baud = B115200;
            break;
        case 460800:
            baud = B460800;
            break;
        case 921600:
            baud = B921600;
            break;
        case 3000000:
            baud = B3000000;
            break;
        default:
            baud = B115200;
    }
    cfsetispeed(&tty_config, baud);
    cfsetospeed(&tty_config, baud);
    result = tcsetattr(mUartFd, TCSADRAIN, &tty_config);
    if (result < 0) {
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    // for wait for baudrate stable
    portTaskDelayMs(100);
    return ErrorCode::DL_ERROR_NO_ERROR;
};
ErrorCode UartDownloadInterface::setFlowControl(FlowControl fc) {
    struct termios tty_config;
    int result;
    result = tcgetattr(mUartFd, &tty_config);
    if (result < 0) {
        LOG_E("Get tty attr error");
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    mFlowControl = fc;
    if (mFlowControl == FC_SOFTWARE) {
        LOG_D("software control enable");
        tty_config.c_iflag |= (IXON | IXOFF);
        tty_config.c_cc[VSTART] = 0x11;
        tty_config.c_cc[VSTOP] = 0x13;
    } else if (mFlowControl == FC_HARDWARE) {
        // not support now.
    } else {
        tty_config.c_iflag &= ~(IXON | IXOFF);
        tty_config.c_cflag &= ~(CRTSCTS);
    }
    result = tcsetattr(mUartFd, TCSADRAIN, &tty_config);
    if (result < 0) {
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    // for flowcontrol setting stable
    portTaskDelayMs(100);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
int32_t UartDownloadInterface::getHandshakeDelayMs() { return 50; }
int32_t UartDownloadInterface::getRWDelayMs() { return 5; }
UartDownloadInterface::~UartDownloadInterface() {
#if UART_RX_SUPPORT_RINGBUFFER
    delete (mUartBuffer);
#endif  // UART_RX_SUPPORT_RINGBUFFER
}
void UartDownloadInterface::reverse(uint8_t *to_reverse, dSize_t length) {
    dSize_t i = 0;
    dSize_t j = length - 1;
    uint8_t tmp;
    while (i < j) {
        tmp = to_reverse[i];
        to_reverse[i] = to_reverse[j];
        to_reverse[j] = tmp;
        i++;
        j--;
    }
}
ErrorCode UartDownloadInterface::uartReadBytes(void *buffer, dSize_t length) {
    uint8_t *p = static_cast<uint8_t *>(buffer);
    dSize_t has_read = 0;
    // struct pollfd poll_fd;
    // poll_fd.events = POLLIN;
    // poll_fd.fd = mUartFd;
#if UART_RX_SUPPORT_RINGBUFFER
    has_read = mUartBuffer->read(p + has_read, length - has_read, true,
                                 kDefaultTimeoutMs);
#else
    pTime_t current = getSystemTickMs();
    while ((has_read < length) &&
           ((getSystemTickMs() - current) < kDefaultTimeoutMs)) {
        int reads = read(mUartFd, p + has_read, length - has_read);
        if (reads > 0) {
            has_read += (dSize_t)reads;
        } else if (reads == -1 && errno == EAGAIN) {
            continue;
        } else {
            break;
        }
    }
#endif  // UART_RX_SUPPORT_RINGBUFFER
    LOG_D("has read: %zu, length %zu", has_read, length);
    if (has_read == length) {
        return ErrorCode::DL_ERROR_NO_ERROR;
    }
    return ErrorCode::DL_ERROR_IO_ERROR;
}
ErrorCode UartDownloadInterface::uartWriteBytes(const void *buffer,
                                                dSize_t length,
                                                dSize_t *bytesWritten) {
    dSize_t has_write = 0;
    static uint8_t sDstBuffer[kUartTxEscapeBuffer];
    dSize_t toSend = length;
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL || UART_SUPPORT_HARDWARE_CONTROL
    int escapedLength;
    int escapedResultLen;
    if (mFlowControl == FC_SOFTWARE) {
        escapedLength = 0;
        escapedResultLen = uartSwFcTxConvert(
            buffer, length, sDstBuffer, sizeof(sDstBuffer), &escapedLength);
        toSend = escapedResultLen;
    } else {
        toSend = length > sizeof(sDstBuffer) ? sizeof(sDstBuffer) : length;
        memcpy(sDstBuffer, buffer, toSend);
        escapedLength = toSend;
    }
#else
    memcpy(sDstBuffer, buffer,
           length > sizeof(sDstBuffer) ? sizeof(sDstBuffer) : length);
    toSend = length > sizeof(sDstBuffer) ? sizeof(sDstBuffer) : length;
#endif
    int64_t current = getSystemTickMs();
    // assume that write function should be finish in 1s.
    while (has_write < toSend && (getSystemTickMs() - current) < 1000) {
        dSize_t write_step = (kDefaultWriteStep > (toSend - has_write))
                                 ? (toSend - has_write)
                                 : kDefaultWriteStep;
        int writes = write(mUartFd, sDstBuffer + has_write, write_step);
        // For Error Check
        // int mock = rand() % 25;
        // LOG_D("uart send bytes: %d, %d", writes, mock);
        
        // if (writes == 4096 && mock == 0) {
        //     usleep(2000000);
        //     mock = 1;
        // }
        // // print data send
        // // if (writes < 50) {
        // //     for (int i = 0; i < writes; i++) {
        // //         LOG_D("0x%x", sDstBuffer[i]);
        // //     }
        // // }
        if (writes > 0) {
            has_write += writes;
        }
    }
    if (toSend > has_write) {
        LOG_E("uart write bytes error!!! %zu, %zu", length, has_write);
        return ErrorCode::DL_ERROR_IO_ERROR;
    }

    if (bytesWritten != nullptr) {
#if UART_SUPPORT_SOFTWARE_FLOW_CONTROL || UART_SUPPORT_HARDWARE_CONTROL
        *bytesWritten = escapedLength;
#else
        *bytesWritten = toSend;
#endif
    }
    return ErrorCode::DL_ERROR_NO_ERROR;
}
void UartDownloadInterface::notifyStatus(DownloadStatus status) {
    switch (status) {
        case DownloadStatus::DL_STATUS_JUMP_DA_PASS: {
            mU8Timeout = 1000;
            break;
        }
        default:
            break;
    }
}
UartDownloadInterface::RBuffer::RBuffer(int bufferSize) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] construct: %d", bufferSize);
#endif
    int ret = 0;
    mIsObjectInvalid = true;
    mWrap = 0;
    mWp = 0;
    mRp = 0;
    mBuf = nullptr;
    mCapacity = bufferSize;
    mCalcellationMarker = CANCELLATION_MARK_FALSE;
    ret |= pthread_condattr_init(&mCondAttr);
    ret |= pthread_condattr_setclock(&mCondAttr, CLOCK_MONOTONIC);
    ret |= pthread_cond_init(&mWaitDataCond, &mCondAttr);
    ret |= pthread_cond_init(&mWaitSpaceCond, &mCondAttr);
    ret |= pthread_mutex_init(&mRwMutex, nullptr);
    if (ret) {
        LOG_E("Rbuffer %p initialize error!!", this);
        mIsObjectInvalid = false;
        return;
    }
    mBuf = (uint8_t *)malloc(mCapacity);
    memset(mBuf, 0xFF, mCapacity);
}
UartDownloadInterface::RBuffer::~RBuffer() {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] destroy: %p", this);
#endif
    pthread_condattr_destroy(&mCondAttr);
    pthread_cond_destroy(&mWaitDataCond);
    pthread_cond_destroy(&mWaitSpaceCond);
    pthread_mutex_destroy(&mRwMutex);
    mIsObjectInvalid = false;
    if (mBuf) {
        free(mBuf);
    }
}
int UartDownloadInterface::RBuffer::write(const void *buf, int length,
                                          bool fullWrite, int timeoutMs) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] write: %p, %d, %d, %d", buf, length, fullWrite, timeoutMs);
#endif
    bool cancellationHappen = false;
    int dataPutRet = 0;
    if (!mIsObjectInvalid) {
        return RB_ERROR_SYSTEM_ERROR;
    }
    if ((timeoutMs < 0) && (timeoutMs != -1)) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    if (length < 0 || buf == nullptr) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    pthread_mutex_lock(&mRwMutex);
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        // do nothing
        cancellationHappen = true;
    } else if (timeoutMs == -1) {
        while ((fullWrite) ? (getFreeSize() < length) : (getFreeSize() == 0)) {
            pthread_cond_wait(&mWaitSpaceCond, &mRwMutex);
        }
    } else {
        struct timespec ts;
        struct timespec tsNow;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        int64_t monoNs = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;
        monoNs += (int64_t)timeoutMs * 1000000;
        ts.tv_sec = monoNs / 1000000000;
        ts.tv_nsec = monoNs % 1000000000;
        while ((fullWrite) ? (getFreeSize() < length) : (getFreeSize() == 0)) {
            pthread_cond_timedwait(&mWaitSpaceCond, &mRwMutex, &ts);
            // Check if timeout
            clock_gettime(CLOCK_MONOTONIC, &tsNow);
            if ((int64_t)tsNow.tv_nsec + (int64_t)tsNow.tv_sec * 1000000000 > monoNs) {
                break;
            }
        }
    }
    dataPutRet = getFreeSize();
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        cancellationHappen = true;
    } else if ((dataPutRet < length) && fullWrite) {
        dataPutRet = 0;
    } else {
        dataPutRet = pureWrite(buf, length);
    }
    if (dataPutRet > 0) {
        pthread_cond_signal(&mWaitDataCond);
    }
    pthread_mutex_unlock(&mRwMutex);
    if (cancellationHappen) {
        return RB_ERROR_CANCELLATION_RECEIVED;
    }
    return dataPutRet;
}
int UartDownloadInterface::RBuffer::read(void *buf, int toRead, bool fullRead,
                                         int timeoutMs) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] read: %p, %d, %d", buf, toRead, timeoutMs);
#endif
    bool cancellationHappen = false;
    int dataGet = 0;
    if (!mIsObjectInvalid) {
        return RB_ERROR_SYSTEM_ERROR;
    }
    if ((timeoutMs < 0) && (timeoutMs != -1)) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    if (toRead < 0 || buf == nullptr) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    pthread_mutex_lock(&mRwMutex);
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        // do nothing
        cancellationHappen = true;
    } else if (timeoutMs == -1) {
        while (fullRead ? (getUsedSize() < toRead) : (getUsedSize() == 0)) {
            pthread_cond_wait(&mWaitDataCond, &mRwMutex);
        }
    } else {
        struct timespec ts;
        struct timespec tsNow;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        int64_t monoNs = (int64_t)ts.tv_sec * 1000000000 + (int64_t)ts.tv_nsec;
        monoNs += (int64_t)timeoutMs * 1000000;
        ts.tv_sec = monoNs / 1000000000;
        ts.tv_nsec = monoNs % 1000000000;
        while (fullRead ? (getUsedSize() < toRead) : (getUsedSize() == 0)) {
            pthread_cond_timedwait(&mWaitDataCond, &mRwMutex, &ts);
            // Check if timeout
            clock_gettime(CLOCK_MONOTONIC, &tsNow);
            if ((int64_t)tsNow.tv_nsec + (int64_t)tsNow.tv_sec * 1000000000 > monoNs) {
                break;
            }
        }
    }
    dataGet = getUsedSize();
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        cancellationHappen = true;
    } else if (dataGet < toRead && fullRead) {
        dataGet = 0;
    } else {
        dataGet = pureRead(buf, toRead);
    }
    if (dataGet > 0) {
        pthread_cond_signal(&mWaitSpaceCond);
    }
    pthread_mutex_unlock(&mRwMutex);
    if (cancellationHappen) {
        return RB_ERROR_CANCELLATION_RECEIVED;
    }
    return dataGet;
}
int UartDownloadInterface::RBuffer::waitForData(int timeoutMs) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] waitForData: %d", timeoutMs);
#endif
    bool cancellationHappen = false;
    int dataGet = 0;
    if (!mIsObjectInvalid) {
        return RB_ERROR_SYSTEM_ERROR;
    }
    if ((timeoutMs < 0) && (timeoutMs != -1)) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    pthread_mutex_lock(&mRwMutex);
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        // do nothing
        cancellationHappen = true;
    } else if (timeoutMs == -1) {
        while (getUsedSize() == 0) {
            pthread_cond_wait(&mWaitDataCond, &mRwMutex);
        }
    } else {
        if (getUsedSize() == 0) {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            int64_t monoNs = ts.tv_sec * 1000000000 + ts.tv_nsec;
            monoNs += (int64_t)timeoutMs * 1000000;
            ts.tv_sec = monoNs / 1000000000;
            ts.tv_nsec = monoNs % 1000000000;
            pthread_cond_timedwait(&mWaitDataCond, &mRwMutex, &ts);
        }
    }
    dataGet = getUsedSize();
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        cancellationHappen = true;
    }
    pthread_mutex_unlock(&mRwMutex);
    if (cancellationHappen) {
        return RB_ERROR_CANCELLATION_RECEIVED;
    }
    return dataGet;
}
int UartDownloadInterface::RBuffer::waitForFree(int timeoutMs) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] waitForFree: %d", timeoutMs);
#endif
    bool cancellationHappen = false;
    int dataGet = 0;
    if (!mIsObjectInvalid) {
        return RB_ERROR_SYSTEM_ERROR;
    }
    if ((timeoutMs < 0) && (timeoutMs != -1)) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    pthread_mutex_lock(&mRwMutex);
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        // do nothing
        cancellationHappen = true;
    } else if (timeoutMs == -1) {
        while (getFreeSize() == 0) {
            pthread_cond_wait(&mWaitSpaceCond, &mRwMutex);
        }
    } else {
        if (getFreeSize() == 0) {
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            int64_t monoNs = ts.tv_sec * 1000000000 + ts.tv_nsec;
            monoNs += (int64_t)timeoutMs * 1000000;
            ts.tv_sec = monoNs / 1000000000;
            ts.tv_nsec = monoNs % 1000000000;
            pthread_cond_timedwait(&mWaitSpaceCond, &mRwMutex, &ts);
        }
    }
    dataGet = getFreeSize();
    if (mCalcellationMarker == CANCELLATION_MARK_TRUE) {
        cancellationHappen = true;
    }
    pthread_mutex_unlock(&mRwMutex);
    if (cancellationHappen) {
        return RB_ERROR_CANCELLATION_RECEIVED;
    }
    return dataGet;
}
int UartDownloadInterface::RBuffer::space() {
    pthread_mutex_lock(&mRwMutex);
    int result = getFreeSize();
    pthread_mutex_unlock(&mRwMutex);
    return result;
}
int UartDownloadInterface::RBuffer::used() {
    pthread_mutex_lock(&mRwMutex);
    int result = getUsedSize();
    pthread_mutex_unlock(&mRwMutex);
    return result;
}
int UartDownloadInterface::RBuffer::setCancel(bool enable) {
    pthread_mutex_lock(&mRwMutex);
    mCalcellationMarker =
        enable ? CANCELLATION_MARK_TRUE : CANCELLATION_MARK_FALSE;
    pthread_cond_broadcast(&mWaitDataCond);
    pthread_cond_broadcast(&mWaitSpaceCond);
    pthread_mutex_unlock(&mRwMutex);
    return 0;
}
int UartDownloadInterface::RBuffer::getUsedSize() {
    return mWp + mWrap * mCapacity - mRp;
}
int UartDownloadInterface::RBuffer::getFreeSize() {
    return mCapacity - getUsedSize();
}
int UartDownloadInterface::RBuffer::pureRead(void *buffer, int readLength) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] pureRead: wp=%d, rp=%d, wrap=%d", mWp, mRp, mWrap);
#endif
    if (readLength < 0 || buffer == nullptr) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    int used = getUsedSize();
    if (used == 0) {
        return 0;
    }
    int actualRead = used > readLength ? readLength : used;
    if (actualRead == 0) {
        return 0;
    }
    if (mWp > mRp) {
        if (mWp - mRp != used) {
            LOG_E("condition failed! mWp - mRp != used");
            return RB_ERROR_FATEL_SYSETM_ERROR;
        }
        memcpy(buffer, mBuf + mRp, actualRead);
        mRp += actualRead;
    } else if (mWp <= mRp) {
        if (mWrap == 0) {
            LOG_E("condition failed! mWrap == 0");
            return RB_ERROR_FATEL_SYSETM_ERROR;
        }
        int firstRead = mCapacity - mRp;
        int firstTry = firstRead > actualRead ? actualRead : firstRead;
        memcpy(buffer, mBuf + mRp, firstTry);
        mRp += firstTry;
        if (mRp == mCapacity) {
            mRp = 0;
            mWrap = 0;
        }
        if (actualRead - firstTry > 0) {
            // call this function again to perform a normal read.
            pureRead((uint8_t *)buffer + firstTry, actualRead - firstTry);
        }
    }
    return actualRead;
}
int UartDownloadInterface::RBuffer::pureWrite(const void *buffer,
                                              int writeLength) {
#ifdef UART_RING_BUFFER_DEBUG
    LOG_D("[RB] pureWrite: wp=%d, rp=%d, wrap=%d", mWp, mRp, mWrap);
#endif
    if (writeLength < 0 || buffer == nullptr) {
        return RB_ERROR_PARAMETER_ERROR;
    }
    int space = getFreeSize();
    if (space == 0) {
        return 0;
    }
    int actualWrite = space > writeLength ? writeLength : space;
    if (actualWrite == 0) {
        return 0;
    }
    if (mWp >= mRp) {
        int firstTry = MIN_INT(mCapacity - mWp, actualWrite);
        memcpy(mBuf + mWp, buffer, firstTry);
        mWp += firstTry;
        if (mWp == mCapacity) {
            mWrap = 1;
            mWp = 0;
        }
        if (actualWrite - firstTry > 0) {
            pureWrite((uint8_t *)buffer + firstTry, actualWrite - firstTry);
        }
    } else {
        if (((mRp - mWp) != space) || (mWrap != 1)) {
            LOG_E("condition failed! (mRp - mWp) != space");
            return RB_ERROR_FATEL_SYSETM_ERROR;
        }
        memcpy(mBuf + mWp, buffer, actualWrite);
        ;
        mWp += actualWrite;
    }
    return actualWrite;
}
void UartDownloadInterface::setQcPlatfrom(bool enable) {
    mEnableQcPlatform = enable;
}
/**
 * ===========================
 *   Uart Download Interface End ======================
 * ===========================
 */
/**
 * ===========================
 *   SPI Download Interface ==========================
 * ===========================
 */
SpiDownloadInterface::SpiDownloadInterface(const char *devName) {
    spiFd = -1;
    strncpy(mDevName, devName, sizeof(mDevName));
    (void)kSpisCfgRdCmd;
    (void)kSpisRdCmd;
    (void)kSpisCfgWrCmd;
    (void)kSpisWrCmd;
    (void)kSpisWsCmd;
    (void)kSpisRsCmd;
    (void)kSpisPowerOnCmd;
    (void)kSpisPowerOffCmd;
    (void)kSpisCtCmd;
    (void)kSpiAddress;
    (void)kSpisSlavevOnOffset;
    (void)kSpisSlavevOnMask;
    (void)kSpisSlaveFifoReadyOffset;
    (void)kSpisSlaveFifoReadyMask;
    (void)kSpisSlaveXferFinishOffset;
    (void)kSpisSlaveXferFinishMask;
    (void)kSpisSlaveReadErrorOffset;
    (void)kSpisSlaveReadErrorMask;
    (void)kSpisSlaveWriteErrorOffset;
    (void)kSpisSlaveWriteErrorMask;
    mForceReverseBit = false;
}
ErrorCode SpiDownloadInterface::openPort() {
    spiFd = open(mDevName, O_RDWR);
    if (spiFd == -1) {
        LOG_E("open port %s failed", mDevName);
        return ErrorCode::DL_ERROR_SPI_SYSTEM_ERROR;
    }
#ifdef SPI_DOWNLOAD_CONFIG_SUPPORT_LSB
    uint32_t mode = SPI_LSB_FIRST;
#ifndef AIROHA_SIMULATION
    if (ioctl(spiFd, SPI_IOC_WR_MODE32, &mode) == -1) {
#else
    if (ioctl(spiFd, SPI_IOC_WR_MODE, &mode) == -1) {
#endif
        LOG_E("set mode failed");
        return ErrorCode::DL_ERROR_SPI_SYSTEM_ERROR;
    }
    uint8_t lsb_seting = 1;
    if (ioctl(spiFd, SPI_IOC_WR_LSB_FIRST, &lsb_seting) == -1) {
        LOG_E("lsb_seting failed");
        return ErrorCode::DL_ERROR_SPI_SYSTEM_ERROR;
    }
#else  // SPI_DOWNLOAD_CONFIG_SUPPORT_LSB
    mForceReverseBit = true;
#endif  // SPI_DOWNLOAD_CONFIG_SUPPORT_LSB
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::closePort() {
    close(spiFd);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
CharEx SpiDownloadInterface::readU8() {
    CharEx r;
    uint8_t spiResult;
    ErrorCode err = spiReadBytes(&spiResult, 1);
    LOG_D("read u8 0x%02x, %d", spiResult, err);
    if (err == ErrorCode::DL_ERROR_NO_ERROR) {
        r.dataU8 = spiResult;
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
    } else {
        r.error = ErrorCode::DL_ERROR_SPI_READ_ERROR;
    }
    return r;
}
CharEx SpiDownloadInterface::readU16() {
    CharEx r;
    uint16_t spiResult;
    ErrorCode err = spiReadBytes(&spiResult, 2);
    LOG_D("read u16 0x%04x, %d", spiResult, err);
    if (err == ErrorCode::DL_ERROR_NO_ERROR) {
        r.dataU16 = spiResult;
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
    } else {
        r.error = ErrorCode::DL_ERROR_SPI_READ_ERROR;
    }
    return r;
}
CharEx SpiDownloadInterface::readU32() {
    CharEx r;
    uint32_t spiResult;
    ErrorCode err = spiReadBytes(&spiResult, 4);
    LOG_D("read u32 0x%08x, %d", spiResult, err);
    if (err == ErrorCode::DL_ERROR_NO_ERROR) {
        r.dataU32 = spiResult;
        r.error = ErrorCode::DL_ERROR_NO_ERROR;
    } else {
        r.error = ErrorCode::DL_ERROR_SPI_READ_ERROR;
    }
    return r;
}
ErrorCode SpiDownloadInterface::readBytes(void *buffer, dSize_t length,
                                          uint32_t *bytesRead) {
    if (spiReadBytes(buffer, length) == ErrorCode::DL_ERROR_NO_ERROR) {
        *bytesRead = length;
        return ErrorCode::DL_ERROR_NO_ERROR;
    }
    return ErrorCode::DL_ERROR_SPI_READ_ERROR;
}
ErrorCode SpiDownloadInterface::sendU8(uint8_t dataU8) {
    LOG_D("send U8 0x%02x", dataU8);
    spiWriteBytes(&dataU8, 1);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::sendU16(uint16_t dataU16) {
    LOG_D("send U16 0x%04x", dataU16);
    spiWriteBytes(&dataU16, 2);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::sendU32(uint32_t dataU32) {
    LOG_D("send U32 0x%08x", dataU32);
    spiWriteBytes(&dataU32, 4);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::sendBytes(const void *buffer, dSize_t length) {
    ErrorCode err = spiWriteBytes(buffer, length);
    return err;
}
int32_t SpiDownloadInterface::getHandshakeDelayMs() { return 5; }
int32_t SpiDownloadInterface::getRWDelayMs() { return 5; }
SpiDownloadInterface::~SpiDownloadInterface() {}
void SpiDownloadInterface::notifyStatus(DownloadStatus status) {
    (void)status;
    return;
}
ErrorCode SpiDownloadInterface::spiReadBytes(void *buffer, dSize_t length) {
    uint8_t configReadCmd[9];
    uint8_t status_receive = 0;
    configReadCmd[0] = kSpisCfgRdCmd;
    configReadCmd[1] = kSpiAddress & 0xff;
    configReadCmd[2] = (kSpiAddress >> 8) & 0xff;
    configReadCmd[3] = (kSpiAddress >> 16) & 0xff;
    configReadCmd[4] = (kSpiAddress >> 24) & 0xff;
    configReadCmd[5] = (length - 1) & 0xff;
    configReadCmd[6] = ((length - 1) >> 8) & 0xff;
    configReadCmd[7] = ((length - 1) >> 16) & 0xff;
    configReadCmd[8] = ((length - 1) >> 24) & 0xff;
    LOG_D("spi read bytes %p,%zu", buffer, length);
    pTime_t setTime = getSystemTickMs();
    do {
        // set 5s for timeout
        if (getSystemTickMs() - setTime > 5000) {
            return ErrorCode::DL_ERROR_SPI_READ_STATUS_TIMEOUT;
        }
        if (spiTransfer(configReadCmd, mRxBuffer, 9) !=
            ErrorCode::DL_ERROR_NO_ERROR) {
            LOG_E("[SPIM] SPI master send CR command failed");
            return ErrorCode::DL_ERROR_SPI_READ_CFG_ERROR;
        }
        portTaskDelayMs(5);
        // portTaskDelayMs(100);
        spiQuerySlaveStatus(&status_receive);
    } while ((status_receive & static_cast<uint8_t>(kSpisSlaveFifoReadyMask)) !=
             static_cast<uint8_t>(kSpisSlaveFifoReadyMask));
    mTxBuffer[0] = kSpisRdCmd;
    memset(buffer, 0, length);
    if (spiTransfer(mTxBuffer, mRxBuffer, length + 1) !=
        ErrorCode::DL_ERROR_NO_ERROR) {
        LOG_E("[SPIM] SPI master send dma failed");
        return ErrorCode::DL_ERROR_SPI_READ_DATA_ERROR;
    }
    setTime = getSystemTickMs();
    do {
        if (getSystemTickMs() - setTime > 5000) {
            return ErrorCode::DL_ERROR_SPI_READ_STATUS_TIMEOUT;
        }
        spiQuerySlaveStatus(&status_receive);
    } while ((status_receive & (uint8_t)kSpisSlaveXferFinishMask) !=
             (uint8_t)kSpisSlaveXferFinishMask);
    memcpy(buffer, &(mRxBuffer[1]), length);
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::spiWriteBytes(const void *buffer,
                                              dSize_t length) {
    uint8_t configWriteCmd[9];
    uint8_t status_receive = 0;
    bool statusPass = false;
    configWriteCmd[0] = kSpisCfgWrCmd;
    configWriteCmd[1] = kSpiAddress & 0xff;
    configWriteCmd[2] = (kSpiAddress >> 8) & 0xff;
    configWriteCmd[3] = (kSpiAddress >> 16) & 0xff;
    configWriteCmd[4] = (kSpiAddress >> 24) & 0xff;
    configWriteCmd[5] = (length - 1) & 0xff;
    configWriteCmd[6] = ((length - 1) >> 8) & 0xff;
    configWriteCmd[7] = ((length - 1) >> 16) & 0xff;
    configWriteCmd[8] = ((length - 1) >> 24) & 0xff;
    pTime_t setTime = getSystemTickMs();
    while (!statusPass) {
        if (spiTransfer(configWriteCmd, mRxBuffer, 9) !=
            ErrorCode::DL_ERROR_NO_ERROR) {
            return ErrorCode::DL_ERROR_SPI_WRITE_CFG_ERROR;
        }
        for (int i = 0; i < 10; i++) {
            portTaskDelayMs(5);
            spiQuerySlaveStatus(&status_receive);
            if ((status_receive != 0xFF) &&
                ((status_receive &
                  static_cast<uint8_t>(kSpisSlaveFifoReadyMask)) ==
                 static_cast<uint8_t>(kSpisSlaveFifoReadyMask))) {
                statusPass = true;
                break;
            }
        }
        LOG_E("[SPIM] spiWriteBytes: fifo not ready");
        // return ErrorCode::DL_ERROR_IO_ERROR;
        if (getSystemTickMs() - setTime > 5000) {
            break;
        }
    }
    if (!statusPass) {
        return DL_ERROR_SPI_WRITE_STATUS_TIMEOUT;
    }
    LOG_D("spi write config success");
    mTxBuffer[0] = kSpisWrCmd;
    memcpy(&(mTxBuffer[1]), buffer, length);
    if (spiTransfer(mTxBuffer, mRxBuffer, length + 1) !=
        ErrorCode::DL_ERROR_NO_ERROR) {
        LOG_E("[SPIM] SPI master send dma failed");
        return ErrorCode::DL_ERROR_SPI_WRITE_DATA_ERROR;
    }
    setTime = getSystemTickMs();
    do {
        spiQuerySlaveStatus(&status_receive);
        LOG_D("write data status:0x%x, 0x%x",
              (status_receive & (uint8_t)kSpisSlaveXferFinishMask),
              kSpisSlaveXferFinishMask);
        if (getSystemTickMs() - setTime > 5000) {
            return ErrorCode::DL_ERROR_SPI_WRITE_DATA_ERROR;
        }
    } while ((status_receive & (uint8_t)kSpisSlaveXferFinishMask) !=
             (uint8_t)kSpisSlaveXferFinishMask);
    LOG_D("spi write data success");
    return ErrorCode::DL_ERROR_NO_ERROR;
}
ErrorCode SpiDownloadInterface::spiQuerySlaveStatus(uint8_t *status) {
    uint8_t status_cmd[2] = {kSpisRsCmd, 0};
    uint8_t status_receive[2] = {0};
    /* Note:
     * The value of receive_length is the valid number of bytes received plus
     * the number of bytes to send. For example, here the valid number of bytes
     * received is 1 byte, and the number of bytes to send also is 1 byte, so
     * the receive_length is 2.
     */
    if (spiTransfer(status_cmd, status_receive, 2) !=
        ErrorCode::DL_ERROR_NO_ERROR) {
        LOG_E("[SPIM] SPI master query status of slaver failed");
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    LOG_D("[SPIM] Status receive: 0x%x", status_receive[1]);
    *status = status_receive[1];
    return ErrorCode::DL_ERROR_NO_ERROR;
    ;
}
static uint8_t bitReverseU8(uint8_t byte) {
    byte = ((byte & 0x55) << 1) | ((byte & 0xAA) >> 1);
    byte = ((byte & 0x33) << 2) | ((byte & 0xCC) >> 2);
    byte = ((byte & 0x0F) << 4) | ((byte & 0xF0) >> 4);
    return byte;
}
ErrorCode SpiDownloadInterface::spiTransfer(uint8_t *tx_buffer,
                                            uint8_t *rx_buffer, size_t len) {
    (void)mTxBuffer;
    (void)mRxBuffer;
#ifndef AIROHA_SIMULATION
    if (mForceReverseBit) {
        for (size_t i = 0; i < len; i++) {
            // LOG_D("spiTransfer send[%d] 0x%x", i, tx_buffer[i]);
            tx_buffer[i] = bitReverseU8(tx_buffer[i]);
            // txBuffer[i] = BitReverseTable256[txBuffer[i]];
            // LOG_D("spiTransfer send after reverse     0x%x", txBuffer[i]);
        }
    }
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer,
        .rx_buf = (unsigned long)rx_buffer,
        .len = static_cast<uint32_t>(len),
        .delay_usecs = 10,
        .speed_hz = 1000000,
        .bits_per_word = 8,
        .tx_nbits = 1,
        .rx_nbits = 1,
    };
#else
    struct spi_ioc_transfer tr;
    tr.tx_buf = (unsigned long)tx_buffer;
    tr.rx_buf = (unsigned long)rx_buffer;
    tr.len = static_cast<uint32_t>(len);
#endif
    int ret = ioctl(spiFd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        LOG_E("Spi Transfer error %d,%d", ret, errno);
        return ErrorCode::DL_ERROR_IO_ERROR;
    }
    if (mForceReverseBit) {
        for (size_t i = 0; i < len; i++) {
            // LOG_D("spiTransfer send[%d] 0x%x", i, rx_buffer[i]);
            rx_buffer[i] = bitReverseU8(rx_buffer[i]);
            // txBuffer[i] = BitReverseTable256[txBuffer[i]];
            // LOG_D("spiTransfer send after reverse     0x%x", txBuffer[i]);
        }
    }
    return ErrorCode::DL_ERROR_NO_ERROR;
}
/**
 * ===========================
 *   SPI Download Interface End ======================
 * ===========================
 */

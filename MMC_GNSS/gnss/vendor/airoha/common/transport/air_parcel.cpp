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
#include "air_parcel.h"
#include <assert.h>
#include <inttypes.h>
#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "simulation.h"
using airoha::Parcel;
void Parcel::initState() {
    this->mWriteBuffer = nullptr;
    this->mWriteCapacity = 0;
    this->mWriteOffset = 0;
    this->mReadBuffer = nullptr;
    this->mReadCapacity = 0;
    this->mReadOffset = 0;
    this->mIsReadBufferCopy = false;
    this->mReadBufferConst = nullptr;
}
Parcel::Parcel() {
    initState();
    mWriteBuffer = (uint8_t *)malloc(kDefaultWriteBuffer);
    mWriteCapacity = kDefaultWriteBuffer;
}
Parcel::Parcel(int32_t desiredLength) {
    initState();
    mWriteBuffer = (uint8_t *)malloc(desiredLength);
    mWriteCapacity = desiredLength;
}
Parcel::Parcel(const uint8_t *buffer, int32_t length, bool deepcopy) {
    initState();
    if (deepcopy) {
        copyReadBuffer(buffer, length);
    } else {
        this->mReadBufferConst = buffer;
        this->mReadCapacity = length;
    }
}
Parcel::Parcel(const Parcel &c) { copyMem(c); }
Parcel &Parcel::operator=(const Parcel &c) {
    copyMem(c);
    return *this;
}
Parcel::~Parcel() {
    assert(isReadFinish() &&
           "Parcel read not finish. Please check if need readFinish().");
    clearMem();
}
void Parcel::clearMem() {
    if (this->mWriteBuffer) {
        free(this->mWriteBuffer);
        this->mWriteBuffer = nullptr;
    }
    if (this->mReadBuffer) {
        free(this->mReadBuffer);
        this->mReadBuffer = nullptr;
    }
}
void Parcel::copyMem(const Parcel &c) {
    clearMem();
    initState();
    this->mWriteCapacity = c.mWriteCapacity;
    this->mReadCapacity = c.mReadCapacity;
    if (this->mWriteCapacity) {
        this->mWriteBuffer = (uint8_t *)malloc(this->mWriteCapacity);
        memcpy(this->mWriteBuffer, c.mWriteBuffer, this->mWriteCapacity);
    }
    if (this->mReadCapacity) {
        this->mReadBuffer = (uint8_t *)malloc(this->mReadCapacity);
        memcpy(this->mReadBuffer, c.mReadBuffer, this->mReadCapacity);
        // When copy buffer, we set const read pointer the same as read pointer
        this->mReadBufferConst = this->mReadBuffer;
    }
    restartRead();
    restartWrite();
}
void Parcel::copyReadBuffer(const void *data, int32_t length) {
    // First we need to check if should release buffer first.
    if (this->mReadBuffer) {
        free(this->mReadBuffer);
        this->mReadBuffer = nullptr;
    }
    this->mReadCapacity = length;
    if (this->mReadCapacity) {
        this->mReadBuffer = (uint8_t *)malloc(this->mReadCapacity);
        memcpy(this->mReadBuffer, data, this->mReadCapacity);
        // When copy buffer, we set const read pointer the same as read pointer
        this->mReadBufferConst = this->mReadBuffer;
    }
    restartRead();
}
void Parcel::restartWrite() { this->mWriteOffset = 0; }
void Parcel::restartRead() { this->mReadBuffer = 0; }
bool Parcel::readBytes(void *buffer, int32_t toRead) const {
    if (mReadCapacity - mReadOffset < toRead) {
        return false;
    }
    memcpy(buffer, mReadBufferConst + mReadOffset, toRead);
    mReadOffset += toRead;
    return true;
}
const void *Parcel::readBytes(int32_t toRead) const {
    if (mReadCapacity - mReadOffset < toRead) {
        return nullptr;
    }
    const void *p = mReadBufferConst + mReadOffset;
    mReadOffset += toRead;
    return p;
}
bool Parcel::writeBytes(const void *buffer, int32_t toWrite) {
    if (INT32_MAX - mWriteOffset < toWrite) {
        return false;
    }
    if (mWriteCapacity < toWrite + mWriteOffset) {
        if (!tryEnlarge(toWrite + mWriteOffset)) {
            return false;
        }
    }
    assert(mWriteCapacity >= toWrite + mWriteOffset);
    memcpy(mWriteBuffer + mWriteOffset, buffer, toWrite);
    mWriteOffset += toWrite;
    return true;
}
// unsigned int read
uint8_t Parcel::readUint8() const {
    uint8_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
uint16_t Parcel::readUint16() const {
    uint16_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
uint32_t Parcel::readUint32() const {
    uint32_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
uint64_t Parcel::readUint64() const {
    uint64_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
// signed int read
int8_t Parcel::readInt8() const {
    int8_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
int16_t Parcel::readInt16() const {
    int16_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
int32_t Parcel::readInt32() const {
    int32_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
int64_t Parcel::readInt64() const {
    int64_t val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
bool Parcel::readBool() const {
    int8_t intVal = readInt8();
    return (intVal == 1) ? true : false;
}
float Parcel::readFloat() const {
    float val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
double Parcel::readDouble() const {
    double val = 0;
    readBytes(&val, sizeof(val));
    return val;
}
// unsigned int write
bool Parcel::writeUint8(uint8_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeUint16(uint16_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeUint32(uint32_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeUint64(uint64_t val) { return writeBytes(&val, sizeof(val)); }
// signed int write
bool Parcel::writeInt8(int8_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeInt16(int16_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeInt32(int32_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeInt64(int64_t val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeBool(bool val) {
    int8_t intVal = 0;
    if (val) intVal = 1;
    return writeInt8(intVal);
}
bool Parcel::writeFloat(float val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeDouble(double val) { return writeBytes(&val, sizeof(val)); }
bool Parcel::writeBinary(const void *data, int32_t length) {
    bool ret = true;
    ret = writeInt32(length) && ret;
    ret = writeBytes(data, length) && ret;
    return true;
}
int32_t Parcel::readBinary(uint8_t *data, int32_t capacity) const {
    int32_t len = readInt32();
    if (len == 0) {
        return -1;
    }
    if (len > capacity) {
        LOG_E("read binary with not enough capacity: %d, %d", capacity, len);
        moveRead(-(int32_t)sizeof(int32_t));
        return -1;
    }
    bool ret = readBytes(data, len);
    if (!ret) {
        LOG_E("read binary error:%p,cap=%d,toRead=%d", data, capacity, len);
        moveRead(-(int32_t)sizeof(int32_t));
        return -1;
    }
    return len;
}
std::vector<uint8_t> Parcel::readBinary() const {
    int32_t len = readInt32();
    if (len == 0) {
        return std::vector<uint8_t>();
    }
    const uint8_t *p = (const uint8_t *)readBytes(len);
    return std::vector<uint8_t>(p, p + len);
}
int32_t Parcel::readNextBinaryLen() {
    const int32_t *len = (const int32_t *)(mReadBufferConst + mReadOffset);
    return *len;
}
bool Parcel::writeString(const char *c_str) {
    if (c_str == nullptr) {
        // avoid pass null pointer
        c_str = "";
    }
    if ((mWriteCapacity - mWriteOffset) < (int32_t)(strlen(c_str) + 1)) {
        return false;
    }
    writeInt32(strlen(c_str) + 1);
    writeBytes(c_str, strlen(c_str) + 1);
    return true;
}
const char *Parcel::readString() {
    int32_t len = readInt32();
    if (len == -1) {
        return nullptr;
    }
    const char *p = (const char *)readBytes(len);
    return p;
}
const uint8_t *Parcel::data() { return mWriteBuffer; }
int32_t Parcel::size() { return mWriteOffset; }
void Parcel::moveRead(int32_t count) const {
    mReadOffset += count;
    assert(mReadOffset >= 0);
}
void Parcel::prettyDump(const void *data, int32_t length) {
    char tmp[512] = {0};
    int offset = 0;
    int32_t i = 0;
    LOG_D("============ Pretty MESSAGE DUMP[%p][%" PRId32
          "] START ==============",
          data, length);
    if (data != nullptr) {
        while (i < length) {
            if (i != 0 && i % 100 == 0) {
                LOG_D("[hexdump][%zu] %s", (size_t)i, tmp);
                offset = 0;
            }
            offset += snprintf(tmp + offset, sizeof(tmp) - offset, "%02x ",
                               ((uint8_t *)data)[i]);
            i++;
        }
        if (offset > 0) {
            LOG_D("[hexdump][%zu] %s", (size_t)i, tmp);
        }
    }
    LOG_D("============ Pretty MESSAGE DUMP FINISH ==============");
    return;
}
void Parcel::dumpRead() {
    LOG_D("Dump read: data=%p, data_const=%p, read_capacity=%d, read_offset=%d",
          mReadBuffer, mReadBufferConst, mReadCapacity, mReadOffset);
    prettyDump(mReadBufferConst, mReadCapacity);
}
void Parcel::dumpWrite() {
    LOG_D("Dump write: data=%p, read_capacity=%d, read_offset=%d", mWriteBuffer,
          mWriteCapacity, mWriteOffset);
    prettyDump(mWriteBuffer, mWriteOffset);
}
bool Parcel::isReadFinish() {
    assert(mReadOffset <= mReadCapacity);
    return (mReadOffset == mReadCapacity);
}
void Parcel::finishRead() { mReadOffset = mReadCapacity; }
bool Parcel::tryEnlarge(int32_t desired) {
    if (desired > MAX_AIROHA_PARCEL_SIZE) {
        return false;
    }
    uint32_t uDesired = desired;
    uDesired = uDesired - 1;
    uDesired |= uDesired >> 1;
    uDesired |= uDesired >> 2;
    uDesired |= uDesired >> 4;
    uDesired |= uDesired >> 8;
    uDesired |= uDesired >> 16;
    uDesired = uDesired + 1;
    mWriteCapacity = (int32_t)uDesired;
    uint8_t *buffer = (uint8_t *)malloc(mWriteCapacity);
    if (mWriteOffset > 0) {
        memcpy(buffer, mWriteBuffer, mWriteOffset);
    }
    if (mWriteBuffer) {
        free(mWriteBuffer);
        mWriteBuffer = nullptr;
    }
    mWriteBuffer = buffer;
    return true;
}
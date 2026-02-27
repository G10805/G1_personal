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
 * granted explicit permission to do; so within the License Agreement
 * ("Permitted User").  If you are not a Permitted User, please cease any access
 * or use of Airoha Software immediately. BY OPENING THIS FILE, RECEIVER HEREBY
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
#pragma once
#include <stdint.h>
#include <vector>
namespace airoha {
#define MAX_AIROHA_PARCEL_SIZE (1024 * 512)
namespace transport {
enum ParcelHashKey : uint32_t {
    PARCEL_HASH_KEY_STAR = 0x5000,
    PARCEL_HASH_KEY_WLAN_INFO,
    PARCEL_HASH_KEY_SUPL_NI_MESSAGE,
};
}  // namespace transport
class Parcel {
 public:
    Parcel();
    Parcel(int32_t desiredLength);
    Parcel(const uint8_t *buffer, int32_t length, bool deepcopy = false);
    Parcel(const Parcel &);
    Parcel &operator=(const Parcel &);
    ~Parcel();
    void initState();
    void restartWrite();
    void restartRead();
    // unsigned int read
    uint8_t readUint8() const;
    uint16_t readUint16() const;
    uint32_t readUint32() const;
    uint64_t readUint64() const;
    // signed int read
    int8_t readInt8() const;
    int16_t readInt16() const;
    int32_t readInt32() const;
    int64_t readInt64() const;
    bool readBool() const;
    float readFloat() const;
    double readDouble() const;
    // unsigned int write
    bool writeUint8(uint8_t val);
    bool writeUint16(uint16_t val);
    bool writeUint32(uint32_t val);
    bool writeUint64(uint64_t val);
    // signed int write
    bool writeInt8(int8_t val);
    bool writeInt16(int16_t val);
    bool writeInt32(int32_t val);
    bool writeInt64(int64_t val);
    bool writeBool(bool val);
    bool writeFloat(float val);
    bool writeDouble(double val);
    bool writeBinary(const void *data, int32_t length);
    int32_t readBinary(uint8_t *data, int32_t capacity) const;
    std::vector<uint8_t> readBinary() const;
        int32_t readNextBinaryLen();
    bool writeString(const char *c_str);
    const char *readString();
    const uint8_t *data();
    int32_t size();
    void dumpRead();
    void dumpWrite();
    bool isReadFinish();
    void finishRead();
    bool tryEnlarge(int32_t desired);
    void clearMem();
    void copyMem(const Parcel &);
    void copyReadBuffer(const void *data, int32_t length);
    bool readBytes(void *buffer, int32_t toRead) const;
    void moveRead(int32_t count) const;
    void prettyDump(const void *data, int32_t length);

 private:
    const void *readBytes(int32_t toRead) const;
    bool writeBytes(const void *buffer, int32_t toWrite);
    const static int32_t kDefaultWriteBuffer = 4096;
    const uint8_t *mReadBufferConst;
    uint8_t *mReadBuffer;
    uint8_t *mWriteBuffer;
    int32_t mReadCapacity;
    mutable int32_t mReadOffset;
    int32_t mWriteCapacity;
    int32_t mWriteOffset;
    bool mIsReadBufferCopy;
};
}  // namespace airoha
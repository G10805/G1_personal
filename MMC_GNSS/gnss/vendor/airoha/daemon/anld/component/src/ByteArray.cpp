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
#define LOG_TAG "ByteArray"
#include "ByteArray.h"
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include "simulation.h"
ByteArray::ByteArray() { initMember(); }
ByteArray::ByteArray(const ByteArray &c) {
    initMember();
    if (c.mData && c.mLength > 0) {
        this->mData = malloc(c.mLength);
        memcpy(this->mData, c.mData, c.mLength);
        this->mLength = c.mLength;
    }
}

ByteArray::ByteArray(ByteArray &&m) {
    mData = m.mData;
    m.mData = nullptr;
    mLength = m.mLength;
    m.mLength = 0;
}
ByteArray ByteArray::operator=(const ByteArray &c) {
    if (this == &c) {
        return *this;
    }
    clearMem();
    if (c.mData && c.mLength > 0) {
        this->mData = malloc(c.mLength);
        memcpy(this->mData, c.mData, c.mLength);
        this->mLength = c.mLength;
    }
    return *this;
}
ByteArray::ByteArray(const void *data, size_t length) {
    initMember();
    if (data && length > 0) {
        this->mData = malloc(length);
        memcpy(this->mData, data, length);
        this->mLength = length;
    }
}
ByteArray::~ByteArray() {
    if (mData) {
        assert(mLength > 0);
        free(mData);
        mData = nullptr;
    }
}
bool ByteArray::setData(const void *data, size_t length) {
    clearMem();
    return copyMem(data, length);
}
const void *ByteArray::data() { return mData; }
size_t ByteArray::size() { return mLength; }
void ByteArray::initMember() {
    mData = nullptr;
    mLength = 0;
}
void ByteArray::clearMem() {
    if (mData) {
        free(mData);
        mData = nullptr;
    }
    mLength = 0;
}
bool ByteArray::copyMem(const void *data, size_t length) {
    if (data && length > 0) {
        mData = malloc(length);
        if (!mData) return false;
        memcpy(mData, data, length);
        mLength = length;
    }
    return true;
}
void ByteArray::dump() {
    LOG_D("ByteArray: %p, %p, %zu", this, mData, mLength);
    char output[1024];
    output[0] = 0;
    char *buf = output;
    size_t i;
    for (i = 0; i < mLength; i++) {
        if ((size_t)(buf - (char *)output) > (sizeof(output) - 5)) break;
        buf += sprintf(buf, "%02X ", ((uint8_t *)mData)[i]);
    }
    LOG_D("DATA(%zu): %s", mLength, output);
}
void ByteArray::easyDump(const void *data, size_t length, const char *comment) {
    char tmp[512] = {0};
    int offset = 0;
    size_t i = 0;
    LOG_D("============ Easy MESSAGE DUMP [%s][%zu] START ==============", comment,
       length);
    while (i < length) {
        if (i != 0 && i % 100 == 0) {
            LOG_D("[hexdump][%zu] %s", i, tmp);
            offset = 0;
        }
        offset += snprintf(tmp + offset, sizeof(tmp) - offset, "%02x",
                           ((uint8_t *)data)[i]);
        i++;
    }
    if (offset > 0) {
        LOG_D("[hexdump][%zu] %s", i, tmp);
    }
    LOG_D("============ Easy MESSAGE DUMP [%s] FINISH ==============", comment);
    return;
}
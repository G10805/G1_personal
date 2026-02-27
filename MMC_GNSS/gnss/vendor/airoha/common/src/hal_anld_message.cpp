/* Copyright Statement:
 *
 * (C) 2020  Airoha Technology Corp. All rights reserved.
 *
 * This software/firmware and related documentation ("Airoha Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or its licensors.
 * Without the prior written permission of Airoha and/or its licensors,
 * any reproduction, modification, use or disclosure of Airoha Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 * You may only use, reproduce, modify, or distribute (as applicable) Airoha Software
 * if you have agreed to and been bound by the applicable license agreement with
 * Airoha ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of Airoha Software immediately.
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT AIROHA SOFTWARE RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL ALSO NOT BE RESPONSIBLE FOR ANY AIROHA
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
 */

#define LOG_TAG "ANLD_MESSAGE"

#include <unistd.h>
#include <string>
#include <memory.h>
#include "stdio.h"
#include "stdint.h"
#include "hal_anld_message.h"

namespace Airoha{
namespace IPC{
static const uint8_t DECODE_STATE_IDLE = 0;
static const uint8_t DECODE_STATE_PRE = 1;
static const uint8_t DECODE_STATE_LENGTH = 2;
static const uint8_t DECODE_STATE_DATA = 3;
static const uint8_t DECODE_STATE_END = 4;
static void dumpBuffer(const uint8_t *buffer, size_t len){
    std::string s;
    for(size_t i=0;i<len;i++){
        char t[3];
        sprintf(t,"%x",buffer[i]);
        s += t;
        s += " ";
    }
    print_ipc_log(LOG_LEVEL_DEBUG,"%s",s.c_str());
    
}
static void hexdump_anld_hidl_message(const void* buff, size_t length,
                                 const char* comment) {
    char tmp[512] = {0};
    int offset = 0;
    size_t i = 0;
    print_ipc_log(
        LOG_LEVEL_INFO,
        "============ HIDL MESSAGE DUMP [%s][%zu] START ==============",
        comment, length);
    while (i < length) {
        if (i != 0 && i % 100 == 0) {
            print_ipc_log(LOG_LEVEL_INFO, "[hexdump][%s][%d] %s", comment, i,
                          tmp);
            offset = 0;
        }
        offset += snprintf(tmp + offset, sizeof(tmp) - offset, "%02x",
                           ((uint8_t*)buff)[i]);
        i++;
    }
    if (offset > 0) {
        print_ipc_log(LOG_LEVEL_INFO, "[hexdump][%s][%d] %s", comment, i, tmp);
    }
    print_ipc_log(LOG_LEVEL_INFO, "============ HIDL MESSAGE DUMP [%s] FINISH ==============",
                  comment);
    return;
}
// #define DECODE_STRICT_MODE
anld_status_t decode_anld_hidl_message(const uint8_t* msgBuf, size_t len,
                                       decodeIPCCallback callback,
                                       size_t* comsumeLength) {
    print_ipc_log(LOG_LEVEL_DEBUG, "msgbuf 0x%p,len 0x%lx, callback %p", msgBuf,
                  len, callback);
    // dumpBuffer(msgBuf,len);
    uint8_t state = DECODE_STATE_IDLE;
    anld_status_t ret = ANLD_STATUS_OK;
    size_t packetLen = 0;
    uint8_t* datap = NULL;
    *comsumeLength = 0;
    bool premebleFound = false;
    if (len < HIDL_ANLD_PACKET_CONTROL_SIZE) {
        print_ipc_log(LOG_LEVEL_INFO, "packet too short: %zu", len);
#ifdef DECODE_STRICT_MODE
        assert(0);
#endif
        return ANLD_STATUS_DATA_PACKET_INCOMPLETED;

    }
    for (size_t i = 0; i < len && ret == 0;) {
        switch (state) {
            case DECODE_STATE_IDLE:
                if (msgBuf[i] == PREAMBLE_WORD_W1) {
                    state = DECODE_STATE_PRE;
                }
                i++;
                break;
            case DECODE_STATE_PRE:
                if (msgBuf[i] == PREAMBLE_WORD_W2) {
                    state = DECODE_STATE_LENGTH;
                    premebleFound = true;
                } else {
                    dumpBuffer(msgBuf, len);
#ifdef DECODE_STRICT_MODE
                    assert(0);
#endif
                    print_ipc_log(1, "error state %d", state);
                }
                i++;
                break;
            case DECODE_STATE_LENGTH:
                memcpy((uint8_t*)&packetLen, &(msgBuf[i]), LENGTH_FIELD_LENGTH);
                i += LENGTH_FIELD_LENGTH;
                if (packetLen + i + END_WORD_LENGTH >
                    len) {  // pointer + length + endword
                    ret = ANLD_STATUS_DATA_PACKET_INCOMPLETED;
                    print_ipc_log(LOG_LEVEL_WARNING,
                                  "datapacket is not ready,%ld/%ld",
                                  packetLen + i + END_WORD_LENGTH, len);
#ifdef DECODE_STRICT_MODE
                    assert(0);
#endif
                }
                state = DECODE_STATE_DATA;
                break;
            case DECODE_STATE_DATA:
                datap = (uint8_t*)malloc(sizeof(uint8_t) * packetLen);
                if (datap == NULL) {
                    print_ipc_log(LOG_LEVEL_ERROR, "alloc buffer error!");
                    ret = ANLD_STATUS_MEMORY_ALLOC_FAILED;
                    break;
                }
                memcpy(datap, &(msgBuf[i]), packetLen);
                state = DECODE_STATE_END;
                i += packetLen;
                break;
            case DECODE_STATE_END:
                if (msgBuf[i] == END_WORD_W1 && msgBuf[i + 1] == END_WORD_W2) {
                    // print_ipc_log(LOG_LEVEL_INFO,"parser ok!");
                    callback(datap, packetLen);
                    free(datap);  // release buffer
                } else {
                    print_ipc_log(LOG_LEVEL_ERROR,
                                  "parser error: endword not match");
                    ret = ANLD_STATUS_DATA_PACKET_FORMAT_ERROR;
                    free(datap);  // release buffer
                    datap = NULL;
#ifdef DECODE_STRICT_MODE
                    assert(0);
#endif
                }
                i += 2;
                *comsumeLength = i;
                state = DECODE_STATE_IDLE;
        }
    }
    if (state != DECODE_STATE_IDLE && ret == 0) {
#ifdef DECODE_STRICT_MODE
        assert(0);
#endif
        ret = ANLD_STATUS_DATA_PACKET_INCOMPLETED;
    }
    if (!premebleFound) {
        print_ipc_log(LOG_LEVEL_INFO, "ANLD_STATUS_PREAMBLE_NOT_FOUND");
        hexdump_anld_hidl_message(msgBuf, len, "Rcv");
#ifdef DECODE_STRICT_MODE
        assert(0);
#endif
        return ANLD_STATUS_PREAMBLE_NOT_FOUND;
    }
    return ret;
}
anld_status_t encode_anld_hidl_message(uint8_t *buffer,
                    size_t bufferLen, uint8_t *userData, 
                    size_t userDataLen, size_t *encodeLen){
    //anld_status_t ret = ANLD_STATUS_OK;
    print_ipc_log(LOG_LEVEL_DEBUG,"encode_anld_hidl_message");
    
    *encodeLen = 0;
    if(bufferLen < PREAMBLE_WORD_LENGTH + LENGTH_FIELD_LENGTH + END_WORD_LENGTH + userDataLen){
        return ANLD_STATUS_NO_SPACE;
    }
    if(buffer == NULL || userData == NULL || encodeLen == NULL){
        return ANLD_STATUS_PARAMETER_INVALID;
    }
    size_t encodePointer = 0;

    buffer[encodePointer] = PREAMBLE_WORD_W1;
    encodePointer++;
    buffer[encodePointer] = PREAMBLE_WORD_W2;
    encodePointer++;
    memcpy(buffer + encodePointer, &userDataLen, LENGTH_FIELD_LENGTH);
    encodePointer += LENGTH_FIELD_LENGTH;
    if(userDataLen > 0){
        memcpy(buffer + encodePointer, userData, userDataLen);
    }
    encodePointer += userDataLen;
    buffer[encodePointer] = END_WORD_W1;
    encodePointer++;
    buffer[encodePointer] = END_WORD_W2;
    encodePointer ++;
    *encodeLen = encodePointer;
    //dumpBuffer(buffer,*encodeLen);
    return ANLD_STATUS_OK;  

}

anld_status_t decode_message_payload(const uint8_t *msgBuf, size_t len, decodeModuleMsgCallback callback){
    print_ipc_log(LOG_LEVEL_DEBUG,"decode_message_payload, buf %p,size %ld",
                            msgBuf,
                            len);
    if(len < sizeof(anld_msg_hdr_t)){
        return ANLD_STATUS_PARAMETER_INVALID;
    }
    anld_msg_hdr_t *hdr = (anld_msg_hdr_t*)msgBuf;
    uint8_t *data_p = NULL;
    if(hdr->payload_size > 0){
        if(len < sizeof(anld_msg_hdr_t) + hdr->payload_size){
            return ANLD_STATUS_PARAMETER_INVALID;
        }
        data_p = (uint8_t *)malloc(sizeof(uint8_t) * hdr->payload_size);
        if(data_p == NULL){
            print_ipc_log(LOG_LEVEL_DEBUG,"malloc memory failed ,line %ld",__LINE__);
            return ANLD_STATUS_MEMORY_ALLOC_FAILED;
        }
        memcpy(data_p,msgBuf + sizeof(anld_msg_hdr_t),hdr->payload_size);
        
    }
    callback(hdr->module_id,hdr->msg_id,data_p, hdr->payload_size);
    if(data_p){
        free(data_p);
    }
    return ANLD_STATUS_OK;
}
anld_status_t encode_message_payload(uint8_t *msgBuf, size_t len, anld_message_t *source, size_t *total_len){
    print_ipc_log(LOG_LEVEL_DEBUG,"encode_message_payload, mid %d,msgid %d,size %ld,userdata %p",
                            source->header.module_id,
                            source->header.msg_id,
                            source->header.payload_size,
                            source->userdata);
    if(len < sizeof(anld_msg_hdr_t)+source->header.payload_size){
        return ANLD_STATUS_PARAMETER_INVALID;
    }
    if(source->header.payload_size != 0 && source->userdata == NULL){
        return ANLD_STATUS_PARAMETER_INVALID;
    }
    size_t i = 0;
    memcpy(msgBuf,source,sizeof(anld_msg_hdr_t));
    i += sizeof(anld_msg_hdr_t);
    if(source->header.payload_size > 0){
        memcpy(msgBuf + i,source->userdata,source->header.payload_size);       
    }
    i += source->header.payload_size;
    *total_len = i;
    // dumpBuffer(msgBuf,i);
    return ANLD_STATUS_OK;
}
static constexpr size_t kGetAlignMask(int alignBits) {
    return ~((size_t)(1 << alignBits) - 1);
}
static constexpr size_t kGetAlignSize(size_t size, int alignBits) {
    return (size + (1 << alignBits) - 1) & kGetAlignMask(alignBits);
}
// Add Test Case
static_assert(kGetAlignSize(3, HIDL_ANLD_MSG_ALIGN_BIT) == 8,
              "Align for size 3 error");
static_assert(kGetAlignSize(0, HIDL_ANLD_MSG_ALIGN_BIT) == 0,
              "Align for size 0 error");
static_assert(kGetAlignSize(8, HIDL_ANLD_MSG_ALIGN_BIT) == 8,
              "Align for size 8 error");
static_assert(kGetAlignSize(1, HIDL_ANLD_MSG_ALIGN_BIT) == 8,
              "Align for size 8 error");
static_assert(kGetAlignSize(521, HIDL_ANLD_MSG_ALIGN_BIT) == 528,
              "Align for size 528 error");
anld_status_t encode_anld_hidl_message_v2(uint8_t *buffer, size_t bufferLen,
                                          anld_msg_hdr_v2_t *hdr,
                                          const void *userData,
                                          size_t *encodeLen) {
    (void)buffer;
    (void)bufferLen;
    (void)hdr;
    (void)userData;
    (void)encodeLen;
    size_t total_size = hdr->payload_size + sizeof(anld_msg_hdr_v2_t);
    size_t align_size = kGetAlignSize(total_size, HIDL_ANLD_MSG_ALIGN_BIT);
    if (bufferLen < align_size) {
        return ANLD_STATUS_FAIL;
    }
    anld_msg_hdr_v2_t *p_hdr = (anld_msg_hdr_v2_t *)buffer;
    p_hdr->pre1 = PREAMBLE_WORD_W1;
    p_hdr->pre2 = PREAMBLE_WORD_W2;
    p_hdr->hash = INTERGRITY_HASH;
    p_hdr->module_id = hdr->module_id;
    p_hdr->msg_id = hdr->msg_id;
    p_hdr->payload_size = hdr->payload_size;
    p_hdr->padding_size = (uint32_t)(align_size - total_size);
    memcpy(p_hdr->data, userData, p_hdr->payload_size);
    *encodeLen = align_size;
    return ANLD_STATUS_OK;
}
anld_status_t decode_anld_hidl_message_v2(const uint8_t *buffer,
                                          size_t bufferLen,
                                          decodeModuleMsgCallback callback,
                                          size_t *comsume) {
    size_t i = 0;
    for (i = 0; i < bufferLen;) {
        if ((bufferLen - i) < sizeof(anld_msg_hdr_v2_t)) {
            return ANLD_STATUS_DATA_PACKET_INCOMPLETED;
        }
        anld_msg_hdr_v2_t *p_hdr = (anld_msg_hdr_v2_t *)(buffer + i);
        size_t total_size = p_hdr->payload_size + sizeof(anld_msg_hdr_v2_t) +
                            p_hdr->padding_size;
        if (total_size > (bufferLen - i)) {
            print_ipc_log(LOG_LEVEL_INFO, "packet not complete %zu/%zu",
                          total_size, bufferLen - i);
            return ANLD_STATUS_DATA_PACKET_INCOMPLETED;
        }
        if (p_hdr->hash != INTERGRITY_HASH) {
            return ANLD_STATUS_DATA_PACKET_FORMAT_ERROR;
        }
        if (p_hdr->pre1 != PREAMBLE_WORD_W1 ||
            p_hdr->pre2 != PREAMBLE_WORD_W2) {
            return ANLD_STATUS_PREAMBLE_NOT_FOUND;
        }
        callback(p_hdr->module_id, p_hdr->msg_id, p_hdr->data,
                 p_hdr->payload_size);
        i += total_size;
        *comsume = i;
    }
    return ANLD_STATUS_OK;
}
anld_status_t decode_anld_hidl_message_v2a(
    const uint8_t *buffer, size_t bufferLen,
    std::function<void(msg_module_id_t, uint32_t, const uint8_t *, size_t)>
        callback,
    size_t *comsume) {
    size_t i = 0;
    for (i = 0; i < bufferLen;) {
        if ((bufferLen - i) < sizeof(anld_msg_hdr_v2_t)) {
            return ANLD_STATUS_DATA_PACKET_INCOMPLETED;
        }
        anld_msg_hdr_v2_t *p_hdr = (anld_msg_hdr_v2_t *)(buffer + i);
        size_t total_size = p_hdr->payload_size + sizeof(anld_msg_hdr_v2_t) +
                            p_hdr->padding_size;
        if (total_size > (bufferLen - i)) {
            print_ipc_log(LOG_LEVEL_INFO, "packet not complete %zu/%zu",
                          total_size, bufferLen - i);
            return ANLD_STATUS_DATA_PACKET_INCOMPLETED;
        }
        if (p_hdr->hash != INTERGRITY_HASH) {
            return ANLD_STATUS_DATA_PACKET_FORMAT_ERROR;
        }
        if (p_hdr->pre1 != PREAMBLE_WORD_W1 ||
            p_hdr->pre2 != PREAMBLE_WORD_W2) {
            return ANLD_STATUS_PREAMBLE_NOT_FOUND;
        }
        callback(p_hdr->module_id, p_hdr->msg_id, p_hdr->data,
                 p_hdr->payload_size);
        i += total_size;
        *comsume = i;
    }
    return ANLD_STATUS_OK;
}
} // IPC
} // Airoha


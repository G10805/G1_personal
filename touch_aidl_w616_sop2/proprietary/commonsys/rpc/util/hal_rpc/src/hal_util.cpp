/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "hal_util.h"
#include "someip_util.h"

using ::qti::hal::rpc::SomeipMessage;

/* min header size: service_id (uint16_t, 2byte) + instance_id (uint16_t, 2 byte) */
#define MIN_PACKET_HEADER_SIZE  4

void* CreateMessage(const std::shared_ptr<SomeipMessage>& msg)
{
    return new SomeipMessage(msg->getMessage());
}

void DeleteMessage(void* buf)
{
    SomeipMessage* msg = static_cast<SomeipMessage*> (buf);
    delete msg;
}

bool SerializePayload(uint16_t service_id, uint16_t instance_id,
    const vector<uint8_t>& payload, vector<uint8_t>& data)
{
    size_t size = payload.size();

    data.clear();
    data.resize(size + MIN_PACKET_HEADER_SIZE);

    /* header: service_id (2 byte, byte0 + byte1) */
    data.push_back((uint8_t) (service_id & 0xFF));
    data.push_back((uint8_t) ((service_id >> 8) & 0xFF));

    /* header: instance_id (2 byte, byte2 + byte3) */
    data.push_back((uint8_t) (instance_id & 0xFF));
    data.push_back((uint8_t) ((instance_id >> 8) & 0xFF));

    /* payload */
    data.insert(data.end(), payload.begin(), payload.end());
    return true;
}

bool ParsePayload(uint8_t* data, size_t length, uint16_t& service_id,
    uint16_t& instance_id, uint8_t* &payload, size_t& size)
{
    if (!data || (length < MIN_PACKET_HEADER_SIZE))
        return false;

    /* header: service_id (2 byte, byte0 + byte1) */
    service_id = ((uint16_t) data[0]) | (((uint16_t) data[1]) << 8);

    /* header: instance_id (2 byte, byte2 + byte3) */
    instance_id = ((uint16_t) data[2]) | (((uint16_t) data[3]) << 8);

    /* payload */
    payload = (length > MIN_PACKET_HEADER_SIZE) ? data + MIN_PACKET_HEADER_SIZE : NULL;
    size = length - MIN_PACKET_HEADER_SIZE;
    return true;
}
/* This is an auto-generated source file. */

#pragma once

#include <stdio.h>
#include <unistd.h>

typedef uint16_t AudioMsg;

#define BLUETOOTH_HANDSFREE_AUDIO_REQ_BASE                                          ((AudioMsg)(0x0000))

#define BLUETOOTH_HANDSFREE_AUDIO_START_REQ                                         (AudioMsg)(BLUETOOTH_HANDSFREE_AUDIO_REQ_BASE + 0x0000)
#define BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ                                          (AudioMsg)(BLUETOOTH_HANDSFREE_AUDIO_REQ_BASE + 0x0001)

#define BLUETOOTH_HANDSFREE_AUDIO_REQ_COUNT                                         (BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ - BLUETOOTH_HANDSFREE_AUDIO_START_REQ + 1)
#define IS_BLUETOOTH_HANDSFREE_AUDIO_REQ(t)                                         (((t) >= BLUETOOTH_HANDSFREE_AUDIO_START_REQ) && ((t) <= BLUETOOTH_HANDSFREE_AUDIO_STOP_REQ))
/*!
 * @file IPasrManager.hal
 *
 * @cr
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * @services Defines the external interface for PASR Manager.
 */


package vendor.qti.memory.pasrmanager;

import vendor.qti.memory.pasrmanager.PasrInfo;
import vendor.qti.memory.pasrmanager.PasrPriority;
import vendor.qti.memory.pasrmanager.PasrSrc;
import vendor.qti.memory.pasrmanager.PasrState;
import vendor.qti.memory.pasrmanager.PasrStatus;
import android.hardware.common.Ashmem;

/*
 * Extend 1.0 interfaces and add the additional ones here
 */
// Interface inherits from vendor.qti.memory.pasrmanager@1.0::IPasrManager but AIDL does not support interface inheritance (methods have been flattened).
@VintfStability
interface IPasrManager {
    // Ignoring method attemptOffline from 1.0::IPasrManager since a newer alternative is available.
    // Ignoring method attemptOnline from 1.0::IPasrManager since a newer alternative is available.
    // Ignoring method attemptOfflineAll from 1.0::IPasrManager since a newer alternative is available.
    // Ignoring method attemptOnlineAll from 1.0::IPasrManager since a newer alternative is available.
    // Ignoring method stateEnter from 1.0::IPasrManager since a newer alternative is available.
    // Ignoring method stateExit from 1.0::IPasrManager since a newer alternative is available.

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from attemptOffline_1_1 to attemptOffline
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Attempt to Offline one block
     *
     * Selects a candidate block which has the least number of bytes allocated and attempts to Offline it.
     * This makes sure least amount of page migration happens when we trigger Offline.
     * Returns OFFLINE if Offline succeeds, ERROR if Offline fails
     *
     * @param    PasrSrc    The source ID of the initiator
     * @param    PasrPriority    The priority level of pasr trigger
     * @param out    PasrStatus    Returns OFFLINE if Offline succeeds, ERROR if Offline fails
     */
    PasrStatus attemptOffline(in PasrSrc srcId, in PasrPriority pri);

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from attemptOfflineAll_1_1 to attemptOfflineAll
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Attempt to Offline *all* the blocks
     *
     * This lists through all the removable blocks in the system and attempts to Offline them all
     * Returns OFFLINE if offlining all blocks succeeds, ERROR if failed to offline some or all blocks
     *
     * @param    PasrSrc    The source ID of the initiator
     * @param    PasrPriority    The priority level of pasr trigger
     * @param out   PasrStatus    Returns OFFLINE if offlining all blocks succeeds, ERROR if failed to offline some or all blocks
     */
    PasrStatus attemptOfflineAll(in PasrSrc srcId, in PasrPriority pri);

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from attemptOnline_1_1 to attemptOnline
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Attempt to Online one block
     *
     * Selects a candidate block and attempts to Online it.
     * Returns ONLINE if Online succeeds, ERROR if Online fails
     *
     * @param    PasrSrc    The source ID of the initiator
     * @param    PasrPriority    The priority level of pasr trigger
     * @param out    PasrStatus    Returns ONLINE if Online succeeds, ERROR if Online fails
     */
    PasrStatus attemptOnline(in PasrSrc srcId, in PasrPriority pri);

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from attemptOnlineAll_1_1 to attemptOnlineAll
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Attempt to Online *all* the blocks
     *
     * This lists through all the removable blocks in the system and attempts to Online them all
     * Returns ONLINE if onlining all blocks succeeds, ERROR if failed to online some or all blocks
     *
     * @param    PasrSrc    The source ID of the initiator
     * @param    PasrPriority    The priority level of pasr trigger
     * @param out   PasrStatus    Returns ONLINE if onlining all blocks succeeds, ERROR if failed to online some or all blocks
     */
    PasrStatus attemptOnlineAll(in PasrSrc srcId, in PasrPriority pri);

    // Adding return type to method instead of out param int blockCount since there is only one return value.
    /**
     * Get number of blocks currently Offline
     *
     * Gets the total numbers of blocks under PASR region that are currently Offline
     *
     * @return	int	Returns number of blocks currently Offline
     */
    int getOfflineCount();

    // Adding return type to method instead of out param int blockCount since there is only one return value.
    /**
     * Get number of blocks currently Online
     *
     * Gets the total numbers of blocks under PASR region that are currently Online
     *
     * @return	int	Returns number of blocks currently Online
     */
    int getOnlineCount();

    // Adding return type to method instead of out param PasrInfo info since there is only one return value.
    /**
     * Get the PASR info of the system
     *
     * Gets the information such as DDR size, PASR granule, number of removable blocks in the system
     *
     * @return	PasrInfo	Returns pasr info of the system
     */
    PasrInfo getPasrInfo();

    // Adding return type to method instead of out param int allocMem since there is only one return value.
    /**
     * Get allocated memory of a given segment in MBs
     *
     * Gets the allocated memory of a given pasr segment by using the
     * 'allocated_bytes' node in the kernel.
     *
     * @param    segNum    The pasr segment number
     * @param out    allocMem The allocated memory in the given segment
     */
    int getSegmentAllocatedMem(in int segNum);

    // FIXME: AIDL does not allow int to be an out parameter.
    // Move it to return, or add it to a Parcelable.
    /**
     * Get allocated memory of each segments in MBs
     *
     * Gets the allocated memory of each segment by looping through the
     * 'allocated_bytes' node in the kernel. This data is populated into
     * hidl_memory buffer and passed over to the requesting client.
     *
     * @param out    mem hidl_memory buffer which has info of allocated memory of each segments
     * @param out    numSegs The number of segments for which the data is held in mem
     */
    int getSegmentAllocatedMemAll(out Ashmem mem);

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from stateEnter_1_1 to stateEnter
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Request to enter priority state for trigerring Online/Offline
     *
     * @param    srcId  The source ID of the initiator
     * @param    pri    The priority level for which source wants to enter into state
     * @param    mode   The mode of state, the source wants to enter
     * @param out   status Returns SUCCESS if source entered into given state, ERROR if failed to enter the give state
     */
    PasrStatus stateEnter(in PasrSrc srcId, in PasrPriority pri, in PasrState  mode);

    // FIXME: AIDL has built-in status types. Do we need the status type here?
    // Changing method name from stateExit_1_1 to stateExit
    // Adding return type to method instead of out param PasrStatus status since there is only one return value.
    /**
     * Request to exit the source from previous put state
     *
     * @param    srcId  The source ID of the initiator
     * @param out   status Returns SUCCESS if source exited from the previous state it was put in, ERROR if failed to exit the state
     */
    PasrStatus stateExit(in PasrSrc srcId);
}

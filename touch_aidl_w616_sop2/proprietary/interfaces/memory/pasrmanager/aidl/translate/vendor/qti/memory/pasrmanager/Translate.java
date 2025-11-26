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


// FIXME Remove this file if you don't need to translate types in this backend.

package vendor.qti.memory.pasrmanager;

public class Translate {
static public vendor.qti.memory.pasrmanager.PasrInfo h2aTranslate(vendor.qti.memory.pasrmanager.V1_0.PasrInfo in) {
    vendor.qti.memory.pasrmanager.PasrInfo out = new vendor.qti.memory.pasrmanager.PasrInfo();
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.ddr_size > 2147483647 || in.ddr_size < 0) {
        throw new RuntimeException("Unsafe conversion between signed and unsigned scalars for field: in.ddr_size");
    }
    out.ddr_size = in.ddr_size;
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.granule > 2147483647 || in.granule < 0) {
        throw new RuntimeException("Unsafe conversion between signed and unsigned scalars for field: in.granule");
    }
    out.granule = in.granule;
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.num_blocks > 2147483647 || in.num_blocks < 0) {
        throw new RuntimeException("Unsafe conversion between signed and unsigned scalars for field: in.num_blocks");
    }
    out.num_blocks = in.num_blocks;
    // FIXME This requires conversion between signed and unsigned. Change this if it doesn't suit your needs.
    if (in.min_free_mem > 9223372036854775807L || in.min_free_mem < 0) {
        throw new RuntimeException("Unsafe conversion between signed and unsigned scalars for field: in.min_free_mem");
    }
    out.min_free_mem = in.min_free_mem;
    return out;
}

}

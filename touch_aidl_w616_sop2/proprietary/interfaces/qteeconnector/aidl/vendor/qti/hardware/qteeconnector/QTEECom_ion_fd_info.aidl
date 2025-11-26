/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qteeconnector;

import vendor.qti.hardware.qteeconnector.QTEECom_ion_fd_data;

@VintfStability
parcelable QTEECom_ion_fd_info {
    /**
     * data array of the 4 QTEECom_ion_fd_data entries
     */
    QTEECom_ion_fd_data[4] data;
}

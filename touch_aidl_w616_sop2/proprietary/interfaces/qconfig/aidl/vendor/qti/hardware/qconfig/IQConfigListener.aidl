/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

import vendor.qti.hardware.qconfig.ChangedType;
import vendor.qti.hardware.qconfig.QConfigUserParam;

@VintfStability
interface IQConfigListener {
    /**
     * !
     * @brief       Notify the client with the changed configs
     *
     * @description When the values of any config in the specified module are changed
     *              (added, modified or removed), notify the client with
     *              the new key-value pairs
     *
     * @input       moduleName      The module name to which the changed configs belong
     * @input       type            The type to indicate the configs are added, modified
     *                              or removed.
     * @input       configs         The changed key-value pairs
     *
     *
     * @param out      void
     *
     */
    void onConfigChanged(in String moduleName, in ChangedType type,
        in QConfigUserParam[] configs);
}

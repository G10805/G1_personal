/*
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.hardware.qconfig;

import vendor.qti.hardware.qconfig.IQConfigListener;
import vendor.qti.hardware.qconfig.QConfigPresetParam;
import vendor.qti.hardware.qconfig.QConfigUserParam;
import vendor.qti.hardware.qconfig.Result;
import vendor.qti.hardware.qconfig.QConfigPresetIdResult;

@VintfStability
interface IQConfig {
    /**
     * !
     * @brief       Add the callback for the interested modules to monitor their changed configs.
     *
     * @description The client could register one listener for multiple interested modules.
     *              When the module's configs are changed, the client will get the callback.
     *
     * @input       moduleNameList  The list of the interested module names
     * @input       cb              The callback object for the changed configs
     *
     * @return      result          Indicate the result of API
     *
     */
    Result addUserConfigListener(in String[] moduleNameList, in IQConfigListener cb);

    /**
     * !
     * @brief       Clear the config for the user in the specified module.
     *
     * @description Remove the config (key-value pair) in the specified module.
     *
     *
     * @input       moduleName      Indicates the module field
     * @input       key             It is the key to the previously sent QConfigUserParam
     *
     * @return      result          Indicate the result of API
     *
     */
    Result clearUserConfig(in String moduleName, in String key);

    /**
     * !
     * @brief       Get the preset list based on <module name and preset id>.
     *
     * @description The preset parameters are preloaded by QConfigService. Each module has
     *              a list of presetIds and each presetId whill contain the list of QConfigPresetParam.
     *              This function could query the list of QConfigPresetParam by the module name
     *              and preset id.
     *
     * @input       moduleName      The name of module
     * @input       presetId        The preset id
     *
     * @output   presets        The vector of key-value pairs for the parameters.
     *                             if Result is NOT Success, the size of presets is zero.
     * @return result              Indicate the result of API
     *
     */
    Result getPresets(in String moduleName, in String presetId, out QConfigPresetParam[] presets);

    /**
     * !
     * @brief       Get the value based on <module name and user key id>.
     *
     *
     * @input       moduleName      The name of module
     * @input       key             It is the key to the previously sent QConfigUserParam
     *
     * @return      QConfigPresetIdResult    Indicate the result of API and the string value of preset id
     *
     */
    QConfigPresetIdResult getUserConfigValue(in String moduleName, in String key);

    /**
     * !
     * @brief       Remove the callback from service
     *
     * @description The client could remove the callback from service side.
     *              When the module's config is changed, the client will not get the callback.
     *
     * @input       cb              The callback object to be removed
     *
     * @return      result          Indicate the result of API
     *
     */
    Result removeUserConfigListener(in IQConfigListener cb);

    /**
     * !
     * @brief       Set the configs for the users in the specified module.
     *
     * @description Register the vector of key-value pairs in the specified module.
     *              They will be persisted in QConfig service. If the QConfig service
     *              dies, they will be lost and the client should invoke this API to
     *              register them again.
     *              This API will overwrite the value of the previous configs with
     *              the same module name and the key.
     *
     * @input       moduleName      Indicate the module field
     * @input       configs         The set of key-value pairs
     *
     *
     * @return      result          Indicate the result of API
     *
     */
    Result setUserConfigs(in String moduleName, in QConfigUserParam[] configs);
}

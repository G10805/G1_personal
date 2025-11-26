/*
  * Copyright (c) 2023 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package vendor.qti.hardware.secureprocessor.device;

import vendor.qti.hardware.secureprocessor.common.ErrorCode;
import android.hardware.common.NativeHandle;
import vendor.qti.hardware.secureprocessor.device.sessionIDout;


/**
 * Secure data processor interface.
 *
 * This interface allows processing of secure data on various secure
 * destinations. e.g.
 *   - TEE (Trusted Execution Engine),
 *   - GuestVM,
 *   - Hypervisor,
 *   - DSP (Digital Signal Processor) etc.
 * The secure data access is limited to secure destinations hence disallowed
 * from non-secure entities.
 * The interface is currently limited to secure image data processing.
 */
@VintfStability
interface ISecureProcessor {
    /**
     * createSession:
     *
     * Create a new secure data processor session for image data processing.
     * It creates and returns a unique session identifier for subsequent
     * interactions to this session.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         New session created successfully.
     *     SECURE_PROCESSOR_FAIL:
     *         New session creation failed.
     *
     * @param out sessionId New session identifier.
     *
     */
    ErrorCode createSession(
        out sessionIDout sessionId);

    /**
     * deleteSession:
     *
     * Delete a previously allocated session.
     *
     * @param sessionId Session identifier.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Session started successfully.
     *     SECURE_PROCESSOR_FAIL:
     *         Session start failed.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid parameter passed.
     *
     */
    ErrorCode deleteSession(in int sessionId);

    /**
     * getConfig:
     *
     * Get session configuration. The input configuration buffer (inConfig)
     * contains set of required tag entries. The output configuration buffer
     * (outConfig) is populated with set of pairs having <tag, value> entries
     * for requested configuration tags. Both input and output configuration
     * buffers are expected to be prepared using SecureProcessorConfig common
     * helper class.
     *
     * @param sessionId Session identifier.
     *
     * @param inConfig Input configuration buffer.
     *     It contains required tags to be queried.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Session configuration get successful.
     *     SECURE_PROCESSOR_FAIL:
     *         Session configuration get failed.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid configuration parameter passed.
     *
     * @param out outConfig Output configuration buffer.
     *     It contains set of <tag, value> pairs for requested tags.
     *
     */
    ErrorCode getConfig(in int sessionId, in byte[] inConfig,
        out byte[] outConfig);

    /**
     * processImage:
     *
     * Process secure image data on selected secure destination.
     *
     * Additionally, the API allows set/get of the image specific configuration.
     * The input configuration buffer (inConfig) contains configuration data
     * associated with current image and the output configuration buffer
     * (outConfig) is expected to be populated with new configuration request to
     * be applied to current secure data capture (source) session based on
     * current image data processing on secure destination.
     *
     * The input/output configuration buffers are expected to be prepared
     * using SecureProcessorConfig common helper class.
     *
     * @param sessionId Session identifier.
     *
     * @param image Image handle for secure image data buffer.
     *
     * @param inConfig Input configuration buffer.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Image processing successful.
     *     SECURE_PROCESSOR_FAIL:
     *         Image processing failed.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid parameter passed.
     *     SECURE_PROCESSOR_NEED_CALIBRATE:
     *         Image processing successful. Additionally, client need to
     *         process outConfig for image source calibration.
     *
     * @param out outConfig Output configuration buffer.
     *
     */
    ErrorCode processImage(in int sessionId, in NativeHandle image, in byte[] inConfig,
        out byte[] outConfig);

    /**
     * setConfig:
     *
     * Set session configuration. The configuration buffer (inConfig) contains
     * set of pairs having <tag, value> entries.
     * The configuration buffer is expected to be prepared using
     * SecureProcessorConfig common helper class.
     *
     * @param sessionId Session identifier.
     *
     * @param inConfig Input configuration buffer.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Session configuration applied successfully.
     *     SECURE_PROCESSOR_FAIL:
     *         Session configuration failed to apply.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid configuration parameter passed.
     *
     */
    ErrorCode setConfig(in int sessionId,
        in byte[] inConfig);

    /**
     * startSession:
     *
     * Start requested session. This API allocates essential resources on secure
     * destination and makes them ready for secure data processing.
     * The mandatory session configs are expected to be set before calling
     * this API.
     *
     * @param sessionId Session identifier.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Session started successfully.
     *     SECURE_PROCESSOR_FAIL:
     *         Session start failed.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid parameter passed.
     *
     */
    ErrorCode startSession(in int sessionId);

    /**
     * stopSession:
     *
     * Stop requested session. This API releases resources on secure destination
     * which were allocated during startSession call.
     * No secure data processing is allowed post this API call.
     * This API is expected to be called after completely stopping the
     * secure data capture requests on source.
     *
     * @param sessionId Session identifier.
     *
     * @return ErrorCode Return status of this operation:
     *     SECURE_PROCESSOR_OK:
     *         Session started successfully.
     *     SECURE_PROCESSOR_FAIL:
     *         Session start failed.
     *     SECURE_PROCESSOR_BAD_VAL:
     *         Invalid parameter passed.
     *
     */
    ErrorCode stopSession(in int sessionId);
}

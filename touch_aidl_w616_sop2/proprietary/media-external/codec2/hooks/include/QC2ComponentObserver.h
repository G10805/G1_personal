/*
 **************************************************************************************************
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_COMPONENT_OBSERVER_H_
#define _QC2_COMPONENT_OBSERVER_H_

#include <memory>
#include "QC2.h"
#include "QC2Constants.h"

namespace qc2 {

/// @addtogroup hooks Customization hooks
/// @{

/**
 * @brief Observer for component's lifecycle and state transitions
 *
 * Optional and customizable hook to listen to component's lifecycle and state-transitions.
 * Can be used to optionally plug-in product-specific handling (taking perf-locks etc).
 */
class QC2ComponentObserver {
 public:
    virtual ~QC2ComponentObserver();

    /**
     * @brief notification when component is started
     */
    virtual void onStarted() = 0;

    /**
     * @brief notification when component is stopped
     */
    virtual void onStopped() = 0;

    /**
     * @brief notification when component is tripped
     */
    virtual void onTripped() = 0;

    /**
     * @brief notification when component transitions to error state
     *
     * @param[in]  errorCode one of the status codes specifying the exact error
     * @param[in]  errorMsg  error description
     */
    virtual void onError(QC2Status errorCode, const char *errorMsg) = 0;

 public:
    /**
     * @brief Create a component observer
     *
     * @param[in]  componentName  component name
     * @param[in]  kind           component kind (ENCODER/DECODER/FILTER)
     * @param[in]  id             unique identifier of the component
     * @param[out] observer       Observer object or nullptr
     *
     * @return QC2_OK Observer was created successfully
     * @return QC2_OMITTED Observer was not created intentionally
     * @return QC2_ERROR something went wrong and observer was not created
     */
    static QC2Status Create(
            const std::string& componentName,
            ComponentKind kind,
            uint32_t id,
            std::unique_ptr<QC2ComponentObserver> *observer);
};

/// @}

};   // namespace qc2

#endif  // _QC2_COMPONENT_OBSERVER_H_

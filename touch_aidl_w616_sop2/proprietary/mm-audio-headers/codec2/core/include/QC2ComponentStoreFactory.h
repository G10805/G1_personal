/*
 **************************************************************************************************
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **************************************************************************************************
*/
#ifndef _QC2_COMPONENT_STORE_FACTORY_H_
#define _QC2_COMPONENT_STORE_FACTORY_H_

#include <C2Component.h>
#include <memory>

namespace qc2audio{}

struct QC2ComponentStoreFactory {
    virtual ~QC2ComponentStoreFactory() = default;
    virtual std::shared_ptr<C2ComponentStore> getInstance() = 0;
};

// symbol name for getting the factory (library = libqc2audio_core.so)
static constexpr const char * kFn_QC2AudioComponentStoreFactoryGetter = "QC2AudioComponentStoreFactoryGetter";

using QC2ComponentStoreFactoryGetter_t
        = QC2ComponentStoreFactory * (*)(int majorVersion, int minorVersion);

#endif  // _QC2_COMPONENT_STORE_FACTORY_H_

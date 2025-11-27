#ifndef __VISTEON_VSTRVC_SERVICE_H
#define __VISTEON_VSTRVC_SERVICE_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <binder/IInterface.h>

namespace VSTRvc {
// ----------------------------------------------------------------------------

using namespace android;

class IVSTRvcService : public IInterface
{
public:
    DECLARE_META_INTERFACE(VSTRvcService);
    
    virtual void setRvcBounds(const int32_t left, const int32_t top, const int32_t right, const int32_t bottom) = 0;
    virtual void startRvcCamera() = 0;
    virtual void stopRvcCamera() = 0;
    virtual bool hasStarted() = 0;
};

// ----------------------------------------------------------------------------

class BnVSTRvcService: public BnInterface<IVSTRvcService> {
public:
    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

}; // namespace VSTRvc

#endif

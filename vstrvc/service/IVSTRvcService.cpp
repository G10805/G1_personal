/**
*
*/

#define LOG_TAG "VSTRvcService"

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include "VSTRvcService.h"

namespace VSTRvc {
	
using namespace android;

enum {
    SET_RVC_BOUNDS = IBinder::FIRST_CALL_TRANSACTION,
    START_RVC_CAMERA,
    STOP_RVC_CAMERA,
    HAS_RVC_CAMERA_STARTED,
};

class BpVSTRvcService : public BpInterface<IVSTRvcService>
{
public:
    BpVSTRvcService(const sp<IBinder>& impl)
        : BpInterface<IVSTRvcService>(impl)
    {
    }
    virtual void setRvcBounds(int32_t left, int32_t top, int32_t right, int32_t bottom)
    {
      Parcel data, reply;
      data.writeInterfaceToken(IVSTRvcService::getInterfaceDescriptor());
      data.writeInt32(left);
      data.writeInt32(top);
      data.writeInt32(right);
      data.writeInt32(bottom);
      status_t transact_result=remote()->transact(SET_RVC_BOUNDS, data, &reply);
	    if(transact_result != NO_ERROR) {
		     ALOGE("transact failed with error code: %d",transact_result);
	    }
    }

    virtual void startRvcCamera()
    {
      Parcel data, reply;
      data.writeInterfaceToken(IVSTRvcService::getInterfaceDescriptor());
      status_t transact_result=remote()->transact(START_RVC_CAMERA, data, &reply);
      ALOGE("transcat_result %d",transact_result);
	    if(transact_result != NO_ERROR) {
		     ALOGE("transact failed with error code: %d",transact_result);
	  }
    }

    virtual void stopRvcCamera()
    {
      Parcel data, reply;
      data.writeInterfaceToken(IVSTRvcService::getInterfaceDescriptor());
      status_t transact_result=remote()->transact(STOP_RVC_CAMERA, data, &reply); //added statement
	    if(transact_result != NO_ERROR) {
		     ALOGE("transact failed with error code: %d",transact_result);
	  }
    }

    virtual bool hasStarted()
    {
      Parcel data, reply;
      data.writeInterfaceToken(IVSTRvcService::getInterfaceDescriptor());
      status_t transact_result=remote()->transact(HAS_RVC_CAMERA_STARTED, data, &reply);
	    if(transact_result != NO_ERROR) {
		     ALOGE("transact failed with error code: %d",transact_result);
	  }
      bool state = reply.readBool();
      return state;
    }
};

IMPLEMENT_META_INTERFACE(VSTRvcService, "android.visteon.rvc.IRvcService");

status_t BnVSTRvcService::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
  switch(code) {
    case SET_RVC_BOUNDS: {
      CHECK_INTERFACE(IVSTRvcService, data, reply);
      int32_t left = data.readInt32();
      int32_t top = data.readInt32();
      int32_t right = data.readInt32();
      int32_t bottom = data.readInt32();
      setRvcBounds(left, top, right, bottom);
      return NO_ERROR;
    }
    case START_RVC_CAMERA: {
      CHECK_INTERFACE(IVSTRvcService, data, reply);
      startRvcCamera();
      return NO_ERROR;
    }
    case STOP_RVC_CAMERA: {
      CHECK_INTERFACE(IVSTRvcService, data, reply);
      stopRvcCamera();     
      return NO_ERROR;
    }
    case HAS_RVC_CAMERA_STARTED: {
        CHECK_INTERFACE(IVSTRvcService, data, reply);
        reply->writeNoException();
        bool state = hasStarted();
        reply->writeBool(state);
        return NO_ERROR;
    }
    default:
      return BBinder::onTransact(code, data, reply, flags);
    }
}

}; // namespace VSTRvc

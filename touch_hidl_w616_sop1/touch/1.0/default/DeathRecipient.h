#include <hidl/HidlTransportSupport.h>  // Required for HIDL death recipient
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <utils/Log.h>
#include <functional>
#include <log/log.h>

namespace vendor::visteon::hardware::interfaces::touch::implementation {

using android::hardware::hidl_death_recipient;
using android::hardware::Return;
using android::sp;
using android::wp;
using android::hidl::base::V1_0::IBase;

class TouchDeathRecipient : public hidl_death_recipient {
private:
	sp<ITouchCallback> mClient = nullptr;
	sp<Touch> mTouch = nullptr;
	
	public:
	/*!
        \brief Overloaded Constructor of the class
        \param callback Address of the ITouch object
    */

	TouchDeathRecipient(const sp<ITouchCallback>& callback, Touch *touch)
	{
		ALOGD(" Touch::TouchDeathRecipient constructor Called");
		mTouch = touch;
		mClient = callback;
		
		if(mClient != nullptr)
		{
			mClient->linkToDeath(this, 0 /*cookie*/);
		}
		ALOGD(" Touch::TouchDeathRecipient constructor End");
	}
	virtual void serviceDied(uint64_t /*cookie*/, const android::wp<::android::hidl::base::V1_0::IBase>& /*who*/)
	{
		ALOGD(" TouchDeathRecipient::DeathHandler serviceDied start");
		mTouch->handleClientDeath();
		mClient = nullptr;
		mTouch = nullptr;
		delete this;
	}

};
}  // namespace vendor::visteon::hardware::interfaces::touch::implementation
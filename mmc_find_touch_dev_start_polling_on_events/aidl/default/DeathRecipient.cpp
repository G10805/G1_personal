#include "DeathRecipient.h"
#include "Touch.h"

namespace vendor::visteon::hardware::interfaces::touch::implementation {

TouchDeathRecipient::TouchDeathRecipient(std::shared_ptr<ITouchCallback> callback, Touch* touch)
    : mClient(callback), mTouch(touch) {
    ALOGD("TouchDeathRecipient constructor called");

    mDeathRecipient = AIBinder_DeathRecipient_new(TouchDeathRecipient::onBinderDied);
    if (mClient && mDeathRecipient) {
        AIBinder_linkToDeath(mClient->asBinder().get(), mDeathRecipient, this);
    }
}

void TouchDeathRecipient::onBinderDied(void* cookie) {
    auto* self = static_cast<TouchDeathRecipient*>(cookie);
    ALOGD("TouchDeathRecipient::onBinderDied called");

    if (self && self->mTouch) {
        self->mTouch->handleClientDeath();
    }

    delete self;
}

} // namespace

/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "EvsStateControl.h"
#include "RenderDirectView.h"
#include "RenderTopView.h"
#include "RenderPixelCopy.h"
#include "FormatConvert.h"

#include <stdio.h>
#include <string.h>

#include <android-base/logging.h>
#include <inttypes.h>
#include <utils/SystemClock.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>

//max time can be taken for displaying each frame initial value 33ms
//TODO:Set this value based on sensor's FPS
static int gMaxTimeToDisplay = 33;
void GetSystemTime(unsigned long long* pnCurrentTime) {
    struct timespec t;
    if (pnCurrentTime) {
        clock_gettime(CLOCK_MONOTONIC, &t);
        // convert milliseconds
        *pnCurrentTime = (unsigned long long)((t.tv_sec * 1000) + (t.tv_nsec / 1000000));
    }
}

using ::android::hardware::automotive::evs::V1_0::EvsResult;
using EvsDisplayState = ::android::hardware::automotive::evs::V1_0::DisplayState;
using BufferDesc_1_0  = ::android::hardware::automotive::evs::V1_0::BufferDesc;
using BufferDesc_1_1  = ::android::hardware::automotive::evs::V1_1::BufferDesc;

static bool isSfReady() {
    const android::String16 serviceName("SurfaceFlinger");
    return android::defaultServiceManager()->checkService(serviceName) != nullptr;
}

// TODO:  Seems like it'd be nice if the Vehicle HAL provided such helpers (but how & where?)
inline constexpr VehiclePropertyType getPropType(VehicleProperty prop) {
    return static_cast<VehiclePropertyType>(
            static_cast<int32_t>(prop)
            & static_cast<int32_t>(VehiclePropertyType::MASK));
}


EvsStateControl::EvsStateControl(android::sp <IVehicle>       pVnet,
                                 android::sp <IEvsEnumerator> pEvs,
                                 android::sp <IEvsDisplay>    pDisplay,
                                 const ConfigManager&         config) :
    mVehicle(pVnet),
    mEvs(pEvs),
    mDisplay(pDisplay),
    mConfig(config),
    mCurrentState(OFF) {

    // Initialize the property value containers we'll be updating (they'll be zeroed by default)
    static_assert(getPropType(VehicleProperty::GEAR_SELECTION) == VehiclePropertyType::INT32,
                  "Unexpected type for GEAR_SELECTION property");
    static_assert(getPropType(VehicleProperty::TURN_SIGNAL_STATE) == VehiclePropertyType::INT32,
                  "Unexpected type for TURN_SIGNAL_STATE property");

    mGearValue.prop       = static_cast<int32_t>(VehicleProperty::GEAR_SELECTION);
    mTurnSignalValue.prop = static_cast<int32_t>(VehicleProperty::TURN_SIGNAL_STATE);

    // This way we only ever deal with cameras which exist in the system
    // Build our set of cameras for the states we support
    LOG(DEBUG) << "Requesting camera list";
    mEvs->getCameraList_1_1(
        [this, &config](hidl_vec<CameraDesc> cameraList) {
            LOG(INFO) << "Camera list callback received " << cameraList.size() << "cameras.";
            for (auto&& cam: cameraList) {
                LOG(INFO) << "Found camera " << cam.v1.cameraId;
                bool cameraConfigFound = false;

                // Check our configuration for information about this camera
                // Note that a camera can have a compound function string
                // such that a camera can be "right/reverse" and be used for both.
                // If more than one camera is listed for a given function, we'll
                // list all of them and let the UX/rendering logic use one, some
                // or all of them as appropriate.
                for (auto&& info: config.getCameras()) {
                    if (cam.v1.cameraId == info.cameraId) {
                        // We found a match!
                        if (info.function.find("reverse") != std::string::npos) {
                            mCameraList[State::REVERSE].emplace_back(info);
                            mCameraDescList[State::REVERSE].emplace_back(cam);
                        }
                        if (info.function.find("right") != std::string::npos) {
                            mCameraList[State::RIGHT].emplace_back(info);
                            mCameraDescList[State::RIGHT].emplace_back(cam);
                        }
                        if (info.function.find("left") != std::string::npos) {
                            mCameraList[State::LEFT].emplace_back(info);
                            mCameraDescList[State::LEFT].emplace_back(cam);
                        }
                        if (info.function.find("park") != std::string::npos) {
                            mCameraList[State::PARKING].emplace_back(info);
                            mCameraDescList[State::PARKING].emplace_back(cam);
                        }
                        cameraConfigFound = true;
                        break;
                    }
                }
                if (!cameraConfigFound) {
                    LOG(WARNING) << "No config information for hardware camera "
                                 << cam.v1.cameraId;
                }
            }
        }
    );

    LOG(DEBUG) << "State controller ready";
}


bool EvsStateControl::startUpdateLoop() {
    // Create the thread and report success if it gets started
    mRenderThread = std::thread([this](){ updateLoop(); });
    return mRenderThread.joinable();
}

void EvsStateControl::terminateUpdateLoop() {
    // Join a rendering thread
    if (mRenderThread.joinable()) {
        mRenderThread.join();
    }
}

void EvsStateControl::postCommand(const Command& cmd, bool clear) {
    // Push the command onto the queue watched by updateLoop
    mLock.lock();
    if (clear) {
        std::queue<Command> emptyQueue;
        std::swap(emptyQueue, mCommandQueue);
    }

    mCommandQueue.push(cmd);
    mLock.unlock();

    // Send a signal to wake updateLoop in case it is asleep
    mWakeSignal.notify_all();
}


void EvsStateControl::updateLoop() {
    LOG(DEBUG) << "Starting EvsStateControl update loop";

    bool run = true;
    unsigned long long timerBeforeGfx = 0, timerBeforeDis = 0, timerAfter = 0,
                  timerDiffGfx = 0, timerDiffDis = 0;
    while (run) {
        // Process incoming commands
        {
            std::lock_guard <std::mutex> lock(mLock);
            while (!mCommandQueue.empty()) {
                const Command& cmd = mCommandQueue.front();
                switch (cmd.operation) {
                case Op::EXIT:
                    run = false;
                    break;
                case Op::CHECK_VEHICLE_STATE:
                    // Just running selectStateForCurrentConditions below will take care of this
                    break;
                case Op::TOUCH_EVENT:
                    // Implement this given the x/y location of the touch event
                    break;
                }
                mCommandQueue.pop();
            }
        }

        // Review vehicle state and choose an appropriate renderer
        if (!selectStateForCurrentConditions()) {
            LOG(ERROR) << "selectStateForCurrentConditions failed so we're going to die";
            break;
        }

        // If we have an active renderer, give it a chance to draw
        if (mCurrentRenderer) {
            // Get the output buffer we'll use to display the imagery
            BufferDesc_1_0 tgtBuffer = {};
            mDisplay->getTargetBuffer([&tgtBuffer](const BufferDesc_1_0& buff) {
                                          tgtBuffer = buff;
                                      }
            );

            if (tgtBuffer.memHandle == nullptr) {
                LOG(ERROR) << "Didn't get requested output buffer -- skipping this frame.";
            } else {
                // Generate our output image
                GetSystemTime(&timerBeforeGfx);
                if (!mCurrentRenderer->drawFrame(convertBufferDesc(tgtBuffer))) {
                    // If drawing failed, we want to exit quickly so an app restart can happen
                    run = false;
                }

                GetSystemTime(&timerBeforeDis);
                timerDiffGfx = timerBeforeDis - timerBeforeGfx;
                // Send the finished image back for display
                mDisplay->returnTargetBufferForDisplay(tgtBuffer);
                GetSystemTime(&timerAfter);
                timerDiffDis = timerAfter - timerBeforeDis;
                if ((timerDiffDis+timerDiffGfx)>gMaxTimeToDisplay) {
                    LOG(ERROR) <<"Frame drop in EVS app, frame processing took "<<(timerDiffDis+timerDiffGfx)<<"ms!!!!"
                        <<" GFX ops "<<timerDiffGfx<<"ms, Display ops "<<timerDiffDis<<"ms";
                }
            }
        } else if (run) {
            // No active renderer, so sleep until somebody wakes us with another command
            // or exit if we received EXIT command
            std::unique_lock<std::mutex> lock(mLock);
            mWakeSignal.wait(lock);
        }
    }

    LOG(WARNING) << "EvsStateControl update loop ending";

    if (mCurrentRenderer) {
        // Deactive the renderer
        mCurrentRenderer->deactivate();
    }

    printf("Shutting down app due to state control loop ending\n");
    LOG(ERROR) << "Shutting down app due to state control loop ending";
}


bool EvsStateControl::selectStateForCurrentConditions() {
    static int32_t sDummyGear   = int32_t(VehicleGear::GEAR_REVERSE);
    static int32_t sDummySignal = int32_t(VehicleTurnSignal::NONE);

    if (mVehicle != nullptr) {
        // Query the car state
        if (invokeGet(&mGearValue) != StatusCode::OK) {
            LOG(ERROR) << "GEAR_SELECTION not available from vehicle.  Exiting.";
            return false;
        }
        if ((mTurnSignalValue.prop == 0) || (invokeGet(&mTurnSignalValue) != StatusCode::OK)) {
            // Silently treat missing turn signal state as no turn signal active
            mTurnSignalValue.value.int32Values.setToExternal(&sDummySignal, 1);
            mTurnSignalValue.prop = 0;
        }
    } else {
        // While testing without a vehicle, behave as if we're in reverse for the first 20 seconds
        static const int kShowTime = 20;    // seconds

        // See if it's time to turn off the default reverse camera
        static std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() > kShowTime) {
            // Switch to drive (which should turn off the reverse camera)
            sDummyGear = int32_t(VehicleGear::GEAR_PARK);
        }

        // Build the dummy vehicle state values (treating single values as 1 element vectors)
        mGearValue.value.int32Values.setToExternal(&sDummyGear, 1);
        mTurnSignalValue.value.int32Values.setToExternal(&sDummySignal, 1);
    }

    // Choose our desired EVS state based on the current car state
    // TODO:  Update this logic, and consider user input when choosing if a view should be presented
    State desiredState = OFF;

#ifndef EVS_CAM_POS
    /* To switch in between the cameras, please set property of camera position.
     * Camera positions are right, left, reverse and park.
     * For example, to stream right camera, set the property as "setprop evs.qcom.camera.pos right".
     */
    char value[PROPERTY_VALUE_MAX];
    // To get the system property to switch among cameras
    property_get("evs.qcom.camera.pos", value, "0");
    if (strncmp(value, "reverse", 7) == 0) {
        desiredState = REVERSE;
    } else if (strncmp(value, "right", 5) == 0) {
        desiredState = RIGHT;
    } else if (strncmp(value, "left", 4) == 0) {
        desiredState = LEFT;
    } else if ((strncmp(value, "park", 4) == 0) || sDummyGear == int32_t(VehicleGear::GEAR_PARK)) {
        desiredState = PARKING;
    } else {
        desiredState = REVERSE;
        LOG(ERROR) << "Wrong value entered, please choose among reverse, left, right and park. Setting default to REVERSE";
    }
#else
    if (mGearValue.value.int32Values[0] == int32_t(VehicleGear::GEAR_REVERSE)) {
        desiredState = REVERSE;
    } else if (mTurnSignalValue.value.int32Values[0] == int32_t(VehicleTurnSignal::RIGHT)) {
        desiredState = RIGHT;
    } else if (mTurnSignalValue.value.int32Values[0] == int32_t(VehicleTurnSignal::LEFT)) {
        desiredState = LEFT;
    } else if (mGearValue.value.int32Values[0] == int32_t(VehicleGear::GEAR_PARK)) {
        desiredState = PARKING;
    }
#endif
    // Apply the desire state
    return configureEvsPipeline(desiredState);
}


StatusCode EvsStateControl::invokeGet(VehiclePropValue *pRequestedPropValue) {
    StatusCode status = StatusCode::TRY_AGAIN;

    // Call the Vehicle HAL, which will block until the callback is complete
    mVehicle->get(*pRequestedPropValue,
                  [pRequestedPropValue, &status]
                  (StatusCode s, const VehiclePropValue& v) {
                       status = s;
                       if (s == StatusCode::OK) {
                           *pRequestedPropValue = v;
                       }
                  }
    );

    return status;
}


bool EvsStateControl::configureEvsPipeline(State desiredState) {
    static bool isGlReady = false;

    if (mCurrentState == desiredState) {
        // Nothing to do here...
        return true;
    }

    LOG(INFO) << "Switching to state " << desiredState;
    LOG(INFO) << "  Current state " << mCurrentState
               << " has " << mCameraList[mCurrentState].size() << " cameras";
    LOG(INFO) << "  Desired state " << desiredState
               << " has " << mCameraList[desiredState].size() << " cameras";

    if (!isGlReady && !isSfReady()) {
        // Graphics is not ready yet; using CPU renderer.
        if (mCameraList[desiredState].size() >= 1) {
            mDesiredRenderer = std::make_unique<RenderPixelCopy>(mEvs,
                                                                 mCameraList[desiredState][0]);
            if (!mDesiredRenderer) {
                LOG(ERROR) << "Failed to construct Pixel Copy renderer.  Skipping state change.";
                return false;
            }
        } else {
            LOG(DEBUG) << "Unsupported, desiredState " << desiredState
                       << " has " << mCameraList[desiredState].size() << " cameras.";
        }
    } else {
        // Assumes that SurfaceFlinger is available always after being launched.

        // Do we need a new direct view renderer?
        if (mCameraList[desiredState].size() == 1) {
            // We have a camera assigned to this state for direct view.
            mDesiredRenderer = std::make_unique<RenderDirectView>(mEvs,
                                                                  mCameraDescList[desiredState][0],
                                                                  mConfig);
            if (!mDesiredRenderer) {
                LOG(ERROR) << "Failed to construct direct renderer.  Skipping state change.";
                return false;
            }
        } else if (mCameraList[desiredState].size() > 1 || desiredState == PARKING) {
            //TODO(b/140668179): RenderTopView needs to be updated to use new
            //                   ConfigManager.
            mDesiredRenderer = std::make_unique<RenderTopView>(mEvs,
                                                               mCameraList[desiredState],
                                                               mConfig);
            if (!mDesiredRenderer) {
                LOG(ERROR) << "Failed to construct top view renderer.  Skipping state change.";
                return false;
            }
        } else {
            LOG(DEBUG) << "Unsupported, desiredState " << desiredState
                       << " has " << mCameraList[desiredState].size() << " cameras.";
        }

        // GL renderer is now ready.
        isGlReady = true;
    }

    // Since we're changing states, shut down the current renderer
    if (mCurrentRenderer != nullptr) {
        mCurrentRenderer->deactivate();
        mCurrentRenderer = nullptr; // It's a smart pointer, so destructs on assignment to null
    }

    // Now set the display state based on whether we have a video feed to show
    if (mDesiredRenderer == nullptr) {
        LOG(DEBUG) << "Turning off the display";
        mDisplay->setDisplayState(EvsDisplayState::NOT_VISIBLE);
    } else {
        mCurrentRenderer = std::move(mDesiredRenderer);

        // Start the camera stream
        LOG(DEBUG) << "EvsStartCameraStreamTiming start time: "
                   << android::elapsedRealtime() << " ms.";
        if (!mCurrentRenderer->activate()) {
            LOG(ERROR) << "New renderer failed to activate";
            return false;
        }

        // Activate the display
        LOG(DEBUG) << "EvsActivateDisplayTiming start time: "
                   << android::elapsedRealtime() << " ms.";
        Return<EvsResult> result = mDisplay->setDisplayState(EvsDisplayState::VISIBLE_ON_NEXT_FRAME);
        if (result != EvsResult::OK) {
            LOG(ERROR) << "setDisplayState returned an error "
                       << result.description();
            return false;
        }
    }

    // Record our current state
    LOG(INFO) << "Activated state " << desiredState;
    mCurrentState = desiredState;

    return true;
}

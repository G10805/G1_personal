#ifndef __VST_RVC_IMPL_H__
#define __VST_RVC_IMPL_H__
#ifdef __cplusplus
extern "C" {
#endif

typedef struct display_parameters {
    int mDisplayWidth;
    int mDisplayHeight;
    int mXStart;
    int mYStart;
    bool initialized;
} display_parameters;

typedef enum tcameraState {

    CAMERA_STATE_IDLE = 0,
    CAMERA_STATE_INITIALIZED,
    CAMERA_STATE_PAUSED,
    CAMERA_STATE_RESUMED,
    CAMERA_STATE_STOPPED,
} cameraState;

int start_camera_rvc(display_parameters *mDisplayParameters);
int clean_camera_rvc();
int stop_camera_rvc();
bool is_home_ready();

#ifdef __cplusplus
}
#endif

#endif // __VST_RVC_IMPL_H__


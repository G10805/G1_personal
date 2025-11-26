#ifndef __CAMPOSTPROCESSING_H_
#define __CAMPOSTPROCESSING_H_

#include "qcarcam.h"

/**
 * @file post_process.h
 *
 * @brief This header file defines the interface for the post processing library
 *
 * Copyright (c) 2021 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

/* ===========================================================================
                        INCLUDE FILES FOR MODULE
=========================================================================== */
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define POST_PROCESS_VERSION         ((POST_PROCESS_VERSION_MAJOR << 16) | POST_PROCESS_VERSION_MINOR)
#define POST_PROCESS_VERSION_MAJOR   1
#define POST_PROCESS_VERSION_MINOR   1

#define PP_MAX_BUFFERLISTS           3

typedef void* pp_ctxt_t;

typedef struct
{
    unsigned long long mem_handle;
    void* ptr;  //virtual address
} pp_buffer_t;

typedef struct
{
    pp_buffer_t  buffers[QCARCAM_MAX_NUM_BUFFERS];
    qcarcam_color_fmt_t color_format;
    unsigned int num_planes;
    unsigned int width;
    unsigned int height;
    unsigned int n_buffers;
    unsigned int stride[3];
    unsigned int offset[3]; //offset to each plane from start of buffer
    unsigned int size[3];   //plane size
}pp_bufferlist_t;


typedef struct
{
    pp_bufferlist_t src_bufferlists[PP_MAX_BUFFERLISTS];
    unsigned int num_src;
    pp_bufferlist_t tgt_bufferlists[PP_MAX_BUFFERLISTS];
    unsigned int num_tgt;
}pp_init_t;

/**
 * Buffer state
 *
 * The process frame function sets the buffer state as processed or not
 */
typedef enum
{
    PP_BUFFER_STATE_PROCESSED,     // buffer was processed
    PP_BUFFER_STATE_UNUSED,        // buffer was not processed
}pp_buffer_state_t;

typedef struct
{
    /*in parameters*/
    unsigned long long job_id;
    unsigned int frame_id;
    unsigned int src_buf_idx[PP_MAX_BUFFERLISTS]; //source buffer indices
    unsigned int tgt_buf_idx[PP_MAX_BUFFERLISTS]; //target buffer indices

    /*out parameters*/
    //filled by process_frame if the tgt buffer was processed or not
    pp_buffer_state_t  tgt_buf_state[PP_MAX_BUFFERLISTS];

}pp_job_t;

/**
 * This structure defines an interface for post processing
 * and methods.
 */
typedef struct
{
    // POST_PROCESS_VERSION
    int version;

    //Opens instance of library
    pp_ctxt_t (*qcarcam_post_process_open)(void);
    //Closes instance of library
    void (*qcarcam_post_process_close)(pp_ctxt_t);

    /**
     * Initializes the library
     *
     * @return 0 SUCCESS and -1 otherwise
     */
    int (*qcarcam_post_process_init)(pp_ctxt_t, pp_init_t*);

    // Uninitialize post processing.
    int (*qcarcam_post_process_deinit)(pp_ctxt_t);

    // Update settings such as color format or which algorithm to run.
    int (*qcarcam_post_process_update)(pp_ctxt_t);

    // Process the image data and output processed frame.
    // The implementation should fill the tgt_buf_state if the output buffer was
    // processed or not.
    int (*qcarcam_post_process_frame)(pp_ctxt_t, pp_job_t*);

} IPostProcessing;

/*
 * Defined signature for public GetCameraConfigInterface
 */
typedef IPostProcessing const* (*GetPostProcessingInterfaceType)(void);

#define GET_POST_PROCESSING_INTERFACE       "GetPostProcessingInterface"


#ifdef __cplusplus
} // extern "C"
#endif  // __cplusplus

#endif

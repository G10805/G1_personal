/*--------------------------------------------------------------------------
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Copyright (c) 2010 - 2013, 2016 - 2017, The Linux Foundation. All rights reserved.
 * Copyright (c) 2011 Benjamin Franzke
 * Copyright (c) 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
--------------------------------------------------------------------------*/
/*
 * Copyright (C) 2015 Advanced Driver Information Technology Joint Venture GmbH
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define LOG_TAG "OMX-VDEC-RENDER"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "omx_vdec_render_linux.h"

#ifdef USE_ION
bool first_display = true;
char gpiovalue[128] = "/sys/class/gpio/gpio109/value";
int gpio_interval = 100 * 1000; //100ms
bool close_display_thread = false;
bool surface_registered = false;
int per_frame_byte = 0;

int number_of_surfaces;
uint32_t id_ivisurf;
pthread_mutex_t mutex;
pthread_cond_t  waiterVariable = PTHREAD_COND_INITIALIZER;
#endif

#ifdef WL_DISPLAY
struct display *display;
struct window *window;
GHashTable *buffer_table;
pthread_t open_display_thread_id;
#endif

#ifdef USE_ION
struct vdec_ion *outPort_ion;
struct gbm_device *gbm = NULL;
int drm_fd = -1;
static bool drm_ready = false;
static drmModeConnector *connector;
static drmModeRes *resources;
static drmModePlaneRes *plane_res;
static drmModeCrtc *crtc;
static struct gbm_bo *crtc_bo = NULL;
static int crtc_id = -1;
static uint32_t fbid_current;
static int plane_id = 0;
static bool gbm_created = false;

static t_ilm_uint screenWidth;
static t_ilm_uint screenHeight;
static t_ilm_uint layer;
#endif



static void destroy_buffer(void *data) {
  struct queue_buffer *queue_buffer = (struct queue_buffer *)data;
  if (queue_buffer == NULL)
    return;
  if (queue_buffer->buffer)
     free(queue_buffer->buffer);
  free(queue_buffer);
}

static void configure_ilm_surface(t_ilm_uint id, t_ilm_uint width, t_ilm_uint height)
{
    ilm_surfaceSetDestinationRectangle(id, 0, 0, screenWidth, screenHeight);
    DEBUGT_PRINT("SetDestinationRectangle: surface ID (%d), Width (%u), Height (%u)", id, screenWidth, screenHeight);
    ilm_surfaceSetSourceRectangle(id, 0, 0, width, height);
    DEBUGT_PRINT("SetSourceRectangle     : surface ID (%d), Width (%u), Height (%u)", id, width, height);
    ilm_surfaceSetVisibility(id, ILM_TRUE);
    DEBUGT_PRINT("SetVisibility          : surface ID (%d), ILM_TRUE", id);
    ilm_layerAddSurface(layer,id);
    DEBUGT_PRINT("layerAddSurface        : surface ID (%d) is added to layer ID (%d)", id, layer);
    ilm_commitChanges();
    pthread_cond_signal( &waiterVariable );
}

static void surfaceCallbackFunction(t_ilm_uint id, struct ilmSurfaceProperties* sp, t_ilm_notification_mask m)
{
    if ((unsigned)m & ILM_NOTIFICATION_CONFIGURED)
    {
        configure_ilm_surface(id, sp->origSourceWidth, sp->origSourceHeight);
    }
}
static void callbackFunction(ilmObjectType object, t_ilm_uint id, t_ilm_bool created, void *user_data)
{
    (void)user_data;
    struct ilmSurfaceProperties sp;

    if (object == ILM_SURFACE)
    {
        if (created)
        {
            if (number_of_surfaces > 0)
            {
                number_of_surfaces--;
                DEBUGT_PRINT("surface                : %d created",id);
                ilm_getPropertiesOfSurface(id, &sp);

                if ((sp.origSourceWidth != 0) && (sp.origSourceHeight !=0))
                {   // surface is already configured
                    configure_ilm_surface(id, sp.origSourceWidth, sp.origSourceHeight);
                }
                else
                {
                    // wait for configured event
                    ilm_surfaceAddNotification(id,&surfaceCallbackFunction);
                    ilm_commitChanges();
                }
            }
        }
        else
        {
            if(!created)
            {
                DEBUGT_PRINT("surface: %d destroyed",id);
            }
        }
    }
    else
    {
        if (object == ILM_LAYER)
        {
            if (created)
            {
                DEBUGT_PRINT("layer: %d created",id);
            }
            else
            {
                if(!created)
                {
                    DEBUGT_PRINT("layer: %d destroyed",id);
                }
            }
        }
    }
}

void init(){
    return;
}

void deInit(){
    return;
}

void init_queue(){
    g_queue_init (&queue_buffer_list);
}

void clear_queue(){
    g_queue_clear(&queue_buffer_list);
}

void clear_all_queue(){
    g_queue_foreach (&queue_buffer_list, (GFunc) destroy_buffer, NULL);
    g_queue_clear(&queue_buffer_list);
}

bool is_empty_queue(){
    return g_queue_is_empty(&queue_buffer_list);
}

struct queue_buffer* pop_queue(){
    GList *item = g_queue_pop_head_link (&queue_buffer_list);
    return (struct queue_buffer *)item->data;
}

void push_queue(struct queue_buffer *queue_buffer){
    g_queue_push_tail (&queue_buffer_list, queue_buffer);
}

int queue_length(){
    return g_queue_get_length(&queue_buffer_list);
}

int open_display()
{
#ifdef WL_DISPLAY
 int major = 0, minor = 0;
  int idx = -1;
  display = create_display();
  if (!display) {
    DEBUGT_PRINT_ERROR("create_display failed\n");
    goto display_error;
  }
  window = create_window(display, 480, 360);
  if (!window) {
    DEBUGT_PRINT_ERROR("create_window failed\n");
    goto window_error;
  }
  DEBUGT_PRINT("display-ivi_application %d\n",display->ivi_application);
  isdisplayopened = true;
  if (display->ivi_application)
    {
        layer = 1;
        number_of_surfaces = 1;

        pthread_mutexattr_t a;
        if (pthread_mutexattr_init(&a) != 0)
        {
            DEBUGT_PRINT("pthread_mutexattr_init error\n");
            goto window_error;
        }
        if (pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE) != 0)
        {
            DEBUGT_PRINT("pthread_mutexattr_settype error\n");
            pthread_mutexattr_destroy(&a);
            goto window_error;
        }

        if (pthread_mutex_init(&mutex, &a) != 0)
        {
            pthread_mutexattr_destroy(&a);
            DEBUGT_PRINT("failed to initialize pthread_mutex");
            goto window_error;
        }

        pthread_mutexattr_destroy(&a);

        struct ilmScreenProperties screenProperties;
        t_ilm_layer renderOrder[1];
        renderOrder[0] = layer;
        ilm_init();
        ilm_getPropertiesOfScreen(0, &screenProperties);
        screenWidth = screenProperties.screenWidth;
        screenHeight = screenProperties.screenHeight;
        ilm_layerCreateWithDimension(&layer, screenWidth, screenHeight);
        DEBUGT_PRINT("CreateWithDimension: layer ID (%d), Width (%u), Height (%u)", layer, screenWidth, screenHeight);
        ilm_layerSetVisibility(layer,ILM_TRUE);
        DEBUGT_PRINT("SetVisibility      : layer ID (%d), ILM_TRUE", layer);
        ilm_displaySetRenderOrder(0,renderOrder,1);
        ilm_commitChanges();
        ilm_registerNotification(callbackFunction, NULL);

        while (number_of_surfaces > 0) {
            DEBUGT_PRINT("number_of_surfaces %d\n", number_of_surfaces);
            pthread_mutex_lock(&mutex);
            DEBUGT_PRINT("pthread_mutex_lock\n");
            pthread_cond_wait( &waiterVariable, &mutex);
            DEBUGT_PRINT("pthread_cond_wait\n");
        }
    }

  return 0;
window_error:
  DEBUGT_PRINT("window error\n");
  destroy_display(display);
display_error:
  DEBUGT_PRINT("display error\n");
  return -1;

#endif
}

void close_display()
{
    if(enable_display == RENDER_DRM){
        drm_close_display();
    }else{
        weston_close_display();
    }
}

static void weston_close_display()
{
#ifdef WL_DISPLAY
  destroy_window(window);
  destroy_display(display);
#endif
}

#ifdef WL_DISPLAY
void* open_display_thread(void* pArg) {

  int ion_device_fd =-1;
  place_marker((char*)"ethcam === drm open");
  if(protocol == USE_GBM_PROTOCOL){
    ion_device_fd = open (GBM_MEM_DEVICE,  O_RDWR | O_CLOEXEC);
    if (ion_device_fd < 0) {
       DEBUGT_PRINT_ERROR("opening ion device failed with ion_device_fd = %d", ion_device_fd);
       return NULL;
    }
    if (enable_display == RENDER_WESTON)
    {
       if (drmDropMaster(ion_device_fd))
       {
         DEBUGT_PRINT("Drop master failed\n");
       }
       else
       {
         DEBUGT_PRINT("Master dropped\n");
       }
    }
    drm_fd = ion_device_fd;
    printf("drm_fd %d\n", drm_fd);
    gbm = gbm_create_device(ion_device_fd);
    if (gbm == NULL)
    {
       DEBUGT_PRINT_ERROR("gbm_create_device failed\n");
       return NULL;
    }
  }
  gbm_created = true;
  if (enable_display == RENDER_DRM) {
    if(drm_open_display() != 0){
      DEBUGT_PRINT_ERROR(" Error opening display! Video won't be displayed...");
    } else {
       isdisplayopened = TRUE;
    }
  }
  else {
    if (open_display() != 0) {
       DEBUGT_PRINT_ERROR(" Error opening display! Video won't be displayed...");
    } else {
       isdisplayopened = TRUE;
    }
  }
}
#endif

void *create_gbm_thread(void *arg)
{
    int ion_device_fd =-1;

    if(protocol == USE_GBM_PROTOCOL){
       ion_device_fd = open (GBM_MEM_DEVICE,  O_RDWR | O_CLOEXEC);
       if (ion_device_fd < 0) {
         DEBUGT_PRINT_ERROR("opening ion device failed with ion_device_fd = %d", ion_device_fd);
         return NULL;
       }
       if (enable_display == RENDER_WESTON)
       {
           if (drmDropMaster(ion_device_fd))
           {
             DEBUGT_PRINT("Drop master failed\n");
           }
           else
           {
             DEBUGT_PRINT("Master dropped\n");
           }
       }
       drm_fd = ion_device_fd;
       DEBUGT_PRINT("drm_fd %d\n", drm_fd);
       gbm = gbm_create_device(ion_device_fd);
       if (gbm == NULL)
       {
           DEBUGT_PRINT_ERROR("gbm_create_device failed\n");
           return NULL;
       }
    }
    gbm_created = true;
}

void *check_gpio_thread(void *arg)
{
  int retry = 0;
  while(1)
  {
    FILE *fp = fopen(gpiovalue, "r");
    if(fp)
    {
       fscanf(fp, "%d", &gpio_value);
       DEBUGT_PRINT("gpio value %d\n", gpio_value);
       switch(gpio_value)
       {
         case 0:
          if(isdisplayopened)
          {
            if(enable_display == RENDER_DRM)
            {
              DEBUGT_PRINT("drm close display\n");
              drm_close_display();
            }
            if(enable_display == RENDER_WESTON)
            {
              DEBUGT_PRINT("weston close display\n");
              weston_close_display();
            }
            isdisplayopened = false;
          }
          if(first_start)
            first_start = false;
          break;
         case 1:
          if(first_start)
          {
            enable_display = RENDER_DRM;
          }
          else
          {
            enable_display = RENDER_WESTON;
          }
          DEBUGT_PRINT("enable display %d, displayYuv %d, isdisplayopened %d, gbm_created %d\n",enable_display, displayYuv, isdisplayopened, gbm_created);
          if (displayYuv && !isdisplayopened)
          {
             if(0 != pthread_create(&open_display_thread_id, NULL, open_display_thread, NULL))
             {
               DEBUGT_PRINT_ERROR(" Error in Creating open_display_thread\n ");
               return NULL;
             }
             while(!isdisplayopened && retry < 100)
             {
               usleep(5000);
               retry ++;
             }
          }
         break;
        default:
          break;
        }

        fclose(fp);
     }
     else
     {
        if(first_start)
        {
            enable_display = RENDER_DRM;
        }
        else
        {
            enable_display = RENDER_WESTON;
        }
        if (displayYuv && !isdisplayopened)
        {
          if(0 != pthread_create(&open_display_thread_id, NULL, open_display_thread, NULL))
          {
            DEBUGT_PRINT_ERROR(" Error in Creating open_display_thread \n");
            return NULL;
          }
          while(!isdisplayopened && retry < 100)
          {
            usleep(5000);
            retry ++;
          }
        }
    }
    usleep(gpio_interval);
    }

    return NULL;
}

void display_marker() {
#ifdef WL_DISPLAY
      if (displayYuv && !isdisplayopened && gpio_value == 1)
      {
        place_marker((char *)"ethcam ===display not ready");
        do {
          DEBUGT_PRINT_ERROR("display has not been ready!");
          usleep(8000);

        } while (!isdisplayopened && gpio_value == 1);
        place_marker((char *)"ethcam ===display is ready");
      }
#endif
}

int display_render(struct OMX_BUFFERHEADERTYPE *pBuffer, int frameWidth, int frameHeight, uint32_t end_playback) {
    (void) end_playback;
    int ret = 0;
#ifdef WL_DISPLAY
    DEBUGT_PRINT("begin render\n");
    if(gpio_value == 1 && isdisplayopened)
    {
        if(enable_display == RENDER_DRM)
            ret = drm_buf_render(pBuffer);
        else
            ret = wl_buf_render(pBuffer);
    }
#endif
    return ret;
}

#ifdef WL_DISPLAY
static gpointer
mm_vidc_display_thread_run (gpointer data)
{
    int ret =0;
    struct display *display =(struct display *) data;
    struct pollfd fds[] = {
        { wl_display_get_fd(display->display), POLLIN },
    };
    close_display_thread = false;
    while(gpio_value && !close_display_thread) {
     while (wl_display_prepare_read_queue (display->display, display->queue) != 0)
     {
       ret = wl_display_dispatch_queue_pending (display->display, display->queue);
       if (ret == -1) {
         bRtpReceiverStopped = TRUE;
         bOutputEosReached = TRUE;
         return NULL;
       }
     }

     wl_display_flush (display->display);
     ret = poll(fds, 1, 50);
     if (ret > 0)
     {
       wl_display_read_events (display->display);
     }
     else
     {
       DEBUGT_PRINT("poll failed ret %d",ret);
       wl_display_cancel_read (display->display);
       if(bRtpReceiverStopped || bOutputEosReached  || bInputEosReached)
            break;
     }
     ret = wl_display_dispatch_queue_pending (display->display, display->queue);
     if (ret == -1){
        bRtpReceiverStopped = TRUE;
        bOutputEosReached = TRUE;
        return NULL;
     }
   }

   printf("exit mm_vidc_display_thread_run\n");
   return NULL;
}
static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
  struct display *d = (struct display *)data;

  d->formats |= (1 << format);
}

struct wl_shm_listener shm_listener = {
  shm_format
};

static void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
}

static void handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
    handle_ping,
    handle_configure,
    handle_popup_done};

static void
xdg_shell_ping(void *data, struct xdg_shell *shell, uint32_t serial)
{
  xdg_shell_pong(shell, serial);
}

static const struct xdg_shell_listener xdg_shell_listener = {
  xdg_shell_ping,
};
#define XDG_VERSION 5 /* The version of xdg-shell that we implement */
static void dmabuf_format(void *data, struct zlinux_dmabuf *zlinux_dmabuf, uint32_t format)
{
    struct display_wl *d = (struct display_wl *)data;
}
static const struct zlinux_dmabuf_listener dmabuf_listener = {
    dmabuf_format
};
static void
gbm_buffer_release(void *data, struct wl_buffer *buffer)
{
        struct listen_buffer *mybuf = (struct listen_buffer *)data;

        mybuf->busy = 0;
        DEBUGT_PRINT("buffer_release mybuf->busy %d\n", mybuf->busy);
}

static const struct wl_buffer_listener gbm_buffer_listener = {
        gbm_buffer_release
};
static const unsigned int pagesize = 4096;

static void
gbm_create_succeeded(void *data,
                 struct gbm_buffer_params *params,
                 struct wl_buffer *new_buffer)
{
        struct listen_buffer *buffer = (struct listen_buffer *)data;

        buffer->wlbuf = new_buffer;
        wl_proxy_set_queue((struct wl_proxy*)new_buffer, display->queue);
        wl_buffer_add_listener(buffer->wlbuf, &gbm_buffer_listener, buffer);

        gbm_buffer_params_destroy(params);
}

static void
gbm_create_failed(void *data, struct gbm_buffer_params *params)
{
        struct listen_buffer *buffer = (struct listen_buffer *)data;

        buffer->wlbuf = NULL;

        gbm_buffer_params_destroy(params);

        fprintf(stderr, "Error:gbm_buffer_params_create.create failed.\n");
}

static const struct gbm_buffer_params_listener gbm_params_listener = {
        gbm_create_succeeded,
        gbm_create_failed
};
static void
buffer_release(void *data, struct wl_buffer *buffer)
{
  struct listen_buffer *listen_buf =(listen_buffer *)data;
  DEBUGT_PRINT("buffer_release listen_buf = %p",listen_buf);
  listen_buf->busy = 0;
}

static const struct wl_buffer_listener buffer_listener = {
  buffer_release
};
static void create_succeeded(void *data, struct zlinux_buffer_params *params,
struct wl_buffer *new_buffer)
{
    struct listen_buffer *buffer = (struct listen_buffer *)data;
    DEBUGT_PRINT("buffer->wlbuf %p create_succeeded \n",new_buffer);
    buffer->wlbuf = new_buffer;
    wl_proxy_set_queue((struct wl_proxy*)new_buffer, display->queue);
    wl_buffer_add_listener(new_buffer, &buffer_listener, buffer);

    zlinux_buffer_params_destroy(params);
}
static void create_failed(void *data, struct zlinux_buffer_params *params)
{
    struct listen_buffer *buffer = (struct listen_buffer *)data;

    buffer->wlbuf = NULL;
    DEBUGT_PRINT_ERROR("wl buffer create_failed \n");
    zlinux_buffer_params_destroy(params);

}

static const struct zlinux_buffer_params_listener params_listener = {
    create_succeeded,
    create_failed
};

static void
registry_handle_global(void *data, struct wl_registry *registry,
           uint32_t id, const char *interface, uint32_t version)
{
  struct display *d = (struct display *)data;
  if (strcmp(interface, "wl_compositor") == 0) {
    d->compositor =(struct wl_compositor *)
        wl_registry_bind(registry,
        id, &wl_compositor_interface, 1);
 /* } else if (strcmp(interface, "xdg_shell") == 0) {
    d->shell = (struct xdg_shell *)wl_registry_bind(registry,
        id, &xdg_shell_interface, 1);
    xdg_shell_use_unstable_version(d->shell, XDG_VERSION);

    xdg_shell_add_listener(d->shell, &xdg_shell_listener, d);

 */ } else if (strcmp(interface, "wl_shm") == 0) {
    d->shm = (struct wl_shm *)wl_registry_bind(registry,
      id, &wl_shm_interface, 1);
    wl_shm_add_listener(d->shm, &shm_listener, d);
  } else if (strcmp(interface, "ivi_application") == 0) {
    d->ivi_application = (struct ivi_application *)
       wl_registry_bind(registry, id,
       &ivi_application_interface, 1);
  }else if (strcmp(interface, "gbm_buffer_backend") == 0) {
        d->gbmbuf = (gbm_buffer_backend*)wl_registry_bind(registry,
            id, &gbm_buffer_backend_interface, 1);
  } else if (strcmp(interface, "zlinux_dmabuf") == 0) {
        d->dmabuf = (struct zlinux_dmabuf *)wl_registry_bind (registry,
            id, &zlinux_dmabuf_interface, 1);
        zlinux_dmabuf_add_listener(d->dmabuf, &dmabuf_listener, d);
    } else if (strcmp(interface, "wl_shell") == 0)
    {
        d->wlshell = (wl_shell*)wl_registry_bind(registry, id, &wl_shell_interface, 1);
    }
  surface_registered = true;
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
          uint32_t name)
{
  surface_registered = false;
}

static const struct wl_registry_listener registry_listener = {
 registry_handle_global,
 registry_handle_global_remove
};

static struct display *create_display(void)
{
  struct display *display;
  GError *err = NULL;
  display = (struct display *)g_malloc(sizeof (*display));
  if (display == NULL) {
    DEBUGT_PRINT_ERROR("display alloc error");
    return NULL;
  }
  memset(display, 0, sizeof(*display));
  display->display = NULL;

  /*wait for display being connected*/
  while (display->display == NULL)
  {
    display->display = wl_display_connect(NULL);
    if(display->display == NULL)
    {
      DEBUGT_PRINT_ERROR("wl_display_connect is NULL reconnecting ....");
      usleep(10000);
    }
  }

  display->formats = 0;
  display->queue = wl_display_create_queue(display->display);
  display->registry = wl_display_get_registry(display->display);
  wl_registry_add_listener(display->registry,
      &registry_listener, display);
  DEBUGT_PRINT("create_display roundtrip enter  1 \n");
  wl_display_roundtrip(display->display);
  DEBUGT_PRINT("create_display roundtrip out  1 \n");
  if (display->shm == NULL) {
    DEBUGT_PRINT_ERROR("display->shm not exist");
    return NULL;
  }
  DEBUGT_PRINT("create_display roundtrip enter  2 \n");
  wl_display_roundtrip(display->display);
  DEBUGT_PRINT("create_display roundtrip out  2 \n");
  if (!(display->formats & (1 << WL_SHM_FORMAT_XRGB8888))) {
    DEBUGT_PRINT_ERROR("WL_SHM_FORMAT_XRGB8888 not support");
    return NULL;
  }
  display->thread = g_thread_try_new("mm-vidc-display", mm_vidc_display_thread_run, display, &err);
  if(err){
    DEBUGT_PRINT_ERROR("g_thread_try_new mm-vidc-display error");
    return NULL;
  }

  return display;
}

static void
destroy_display(struct display *display)
{
  printf("weston destroy display\n");
  close_display_thread = true;
  if (buffer_table)
     g_hash_table_remove_all(buffer_table);
  if (buffer_table)
    g_hash_table_destroy(buffer_table);
  buffer_table = NULL;
  if (display->ivi_application)
    {
        ilm_layerSetVisibility(layer, ILM_FALSE);
        ilm_layerRemoveSurface(layer, id_ivisurf);
        ilm_layerRemove(layer);
        ilm_unregisterNotification();
        ilm_destroy();
    }
  if(gbm){
    gbm_device_destroy(gbm);
    gbm = NULL;
    gbm_created = false;
    DEBUGT_PRINT("weston destroy display gbm destroy\n");
  }
  if(drm_fd > 0 && protocol == USE_GBM_PROTOCOL){
    close(drm_fd);
    drm_fd = -1;
    DEBUGT_PRINT("weston destroy display close drm\n");
  }
   if (display->thread)
  {
    g_thread_join (display->thread);
    DEBUGT_PRINT("weston destroy display thread join\n");
  }
  if (display->shm){
    wl_shm_destroy(display->shm);
    display->shm = NULL;
  }
  if (display->shell){
    xdg_shell_destroy(display->shell);
    display->shell = NULL;
  }
  if (display->wlshell){
    wl_shell_destroy(display->wlshell);
    display->wlshell = NULL;
  }

  if (display->compositor){
    wl_compositor_destroy(display->compositor);
    display->compositor = NULL;
  }
  if (display->dmabuf){
    zlinux_dmabuf_destroy(display->dmabuf);
    display->dmabuf = NULL;
  }
  if (display->registry){
    wl_registry_destroy(display->registry);
    display->registry = NULL;
  }
  if (display->queue){
    wl_event_queue_destroy (display->queue);
	display->queue = NULL;
  }

  if (display->display) {
    wl_display_flush(display->display);
    wl_display_disconnect(display->display);
    display->display = NULL;
  }

  g_free(display);
  display = NULL;
  DEBUGT_PRINT("weston free display\n");
}
static void
sync_callback (void *data, struct wl_callback *callback, uint32_t serial)
{
  bool *done = (bool *)data;
  *done = TRUE;
}

static const struct wl_callback_listener sync_listener = {
  sync_callback
};

static gint
vdec_wl_display_roundtrip (struct display * self)
{
  struct wl_callback *callback;
  gint ret = 0;
  gboolean done = FALSE;

  g_return_val_if_fail (self != NULL, -1);

  /* We don't own the display, process only our queue */
  callback = wl_display_sync (self->display);
  wl_callback_add_listener (callback, &sync_listener, &done);
  wl_proxy_set_queue ((struct wl_proxy *) callback, self->queue);
  while (ret != -1 && !done)
  {
    ret=wl_display_dispatch_queue_pending (display->display, display->queue);
    usleep(10);
  }

  wl_callback_destroy (callback);

  return ret;
}
#define AL(a,b) (((a)+(b)-1)&~((b)-1))
static  struct wl_buffer *
gst_wayland_sink_create_wl_buffer(int w,int h, int fd, int meta_fd, int offset)
{
    struct listen_buffer *lstbuf =(struct listen_buffer *) g_malloc (sizeof *lstbuf);
    if(protocol == USE_GBM_PROTOCOL){
        struct gbm_buffer_params *params;
        uint64_t modifier;
        uint32_t flags;
        int bo_fd = fd;
        modifier = 0;
        flags = 1;
        DEBUGT_PRINT("w %d, h %d, bo_fd %d, meta_fd %d\n", w, h, bo_fd, meta_fd);
        params = gbm_buffer_backend_create_params(display->gbmbuf);
        DEBUGT_PRINT("params %d, gbmbuf %d \n", params, display->gbmbuf);
        gbm_buffer_params_add_listener(params, &gbm_params_listener, lstbuf);
        flags |= GBM_BUFFER_PARAMS_FLAGS_EARLY_DISPLAY;
        gbm_buffer_params_create(params,
                                 bo_fd,
                                 meta_fd,
                                 w,
                                 h,
                                 DRM_FORMAT_NV12,
                                 flags);
         DEBUGT_PRINT("gst_wayland_sink_create_wl_buffer roundtrip enter  1 \n");
         wl_display_roundtrip(display->display);
         DEBUGT_PRINT("gst_wayland_sink_create_wl_buffer roundtrip out  1 \n");
         wl_display_dispatch_queue_pending (display->display, display->queue);
         DEBUGT_PRINT("wlbuf %d\n", lstbuf->wlbuf);
   }
   else{
        struct zlinux_buffer_params *params;
        uint64_t modifier = 0;
        uint32_t flags = 0;
        int32_t align_w = AL(w,128);
        int32_t align_h = AL(h,32);
        printf("",display->dmabuf,params);
        params = zlinux_dmabuf_create_params(display->dmabuf);
        DEBUGT_PRINT("create wlbuf from fd %d offset %d w %d h %d",fd,offset ,w, h);
        zlinux_buffer_params_add(params,
                                 fd,
                                 0, /* plane_idx */
                                 offset, /* offset */
                                 align_w,
                                 modifier >> 32,
                                 modifier & 0xffffffff);
        zlinux_buffer_params_add(params,
                         fd,
                         1, /* plane_idx */
                         align_w * align_h, /* offset */
                         align_w >> 2,
                         modifier >> 32,
                         modifier & 0xffffffff);
        {
          zlinux_buffer_params_add_listener(params, &params_listener, lstbuf);
          zlinux_buffer_params_create(params,
                                      w,
                                      h,
                                      DRM_FORMAT_NV12,
                                      flags);
         wl_display_roundtrip(display->display);
        }
   }
   return lstbuf->wlbuf;
  }

static void
destroy_wl_buffer(gpointer data)
{
  struct wl_buffer *wlbuf = (struct wl_buffer *)data;
  struct listen_buffer *lstenbuf = (struct listen_buffer *)wl_buffer_get_user_data(wlbuf);
  DEBUGT_PRINT("destroy_listen_buffer = %p",lstenbuf);
  g_free(lstenbuf);
  wl_buffer_destroy(wlbuf);
}
static struct wl_buffer *
gst_wayland_sink_get_wl_buffer(int w, int h, int fd, int meta_fd, int offset)
{
  struct wl_buffer *wlbuf;
  gint64 key;
  if (!buffer_table) {
    buffer_table = g_hash_table_new_full (g_int64_hash, g_int64_equal, g_free, destroy_wl_buffer);
  }
  /* put fd and offset into a int64 key*/
  key = ((gint64)(fd & 0xFFFFFFFF) << 32) | (offset & 0xFFFFFFFF);

  wlbuf = (struct wl_buffer*) g_hash_table_lookup(buffer_table, &key);
  if ( !wlbuf) {
    /* create a new wl_buffer */
    gint64 *bufkey = (gint64 *)g_malloc(sizeof(*bufkey));
    *bufkey = key;
    wlbuf = gst_wayland_sink_create_wl_buffer(w, h, fd, meta_fd, offset);
    if (wlbuf)
      g_hash_table_insert(buffer_table, bufkey, wlbuf);
  }
  else {
    listen_buffer *lstbuf = (struct listen_buffer *)wl_buffer_get_user_data(wlbuf);
    if(lstbuf->busy ==TRUE)
    {
      DEBUGT_PRINT_ERROR("listen_buf %p is busy",lstbuf);
      return NULL;
    }
  }
  return wlbuf;
}
static void
frame_redraw_callback (void *data, struct wl_callback *callback, uint32_t time)
{
  DEBUGT_PRINT("frame_redraw_callback");

  wl_callback_destroy (callback);
}

static const struct wl_callback_listener frame_callback_listener = {
  frame_redraw_callback
};
static int
wl_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr)
{
  struct wl_surface *surface = window->surface;;
  struct wl_buffer  *wlbuf;

  OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *pPMEMInfo = NULL;

  pPMEMInfo  = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)
              ((OMX_QCOM_PLATFORM_PRIVATE_LIST *)
                pBufHdr->pPlatformPrivate)->entryList->entry;
  if (pPMEMInfo == NULL) {
    DEBUGT_PRINT_ERROR("pmem_info is null");
    return -1;
  }
  DEBUGT_PRINT("wl_buf_render pmem_fd =%d width height %d %d",pPMEMInfo->pmem_fd,width,height);

  wlbuf = gst_wayland_sink_get_wl_buffer(width, height, pPMEMInfo->pmem_fd,pPMEMInfo->pmeta_fd, pPMEMInfo->offset);
  if (!wlbuf)
  {
    DEBUGT_PRINT_ERROR("wlbuf is null");
    goto done;
  }
  DEBUGT_PRINT("wl_surface_attach\n");
  wl_surface_attach (surface, wlbuf, 0, 0);
  wl_surface_damage (surface, 0, 0, width, height);

  window->callback = wl_surface_frame (surface);
  wl_proxy_set_queue((struct wl_proxy *) window->callback, display->queue);
  wl_callback_add_listener (window->callback, &frame_callback_listener, window);

  wl_surface_commit (surface);
  {
    listen_buffer *lstbuf = (struct listen_buffer *)wl_buffer_get_user_data(wlbuf);
    lstbuf->busy  = TRUE;
  }
  wl_display_flush (display->display);

done:
  return 0;
}
#endif

int drm_buf_render(struct OMX_BUFFERHEADERTYPE *pBufHdr)
{
    union gbm_bo_handle bo_handle;
    struct gbm_import_fd_data buf_data;
    struct gbm_bo *bo = NULL;
    uint32_t handles[4] = {0};
    uint32_t pitches[4] = {0};
    uint32_t offsets[4] = {0};
    uint32_t flags = 0;
    uint32_t fbid;
    struct drm_mode_fb_cmd2 args;
    unsigned int dst_w, dst_h;
    unsigned int bo_width, bo_height;
    OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *pPMEMInfo = NULL;

    pPMEMInfo  = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)
              ((OMX_QCOM_PLATFORM_PRIVATE_LIST *)
                pBufHdr->pPlatformPrivate)->entryList->entry;
    if (pPMEMInfo == NULL) {
      DEBUGT_PRINT_ERROR("pmem_info is null");
      return -1;
    }
    /* create bo */

    buf_data.fd = pPMEMInfo->pmem_fd;
    buf_data.width = width;
    buf_data.height = height;

    buf_data.format = GBM_FORMAT_NV12;
    bo = gbm_bo_import(gbm, GBM_BO_IMPORT_FD, &buf_data, GBM_BO_USE_RENDERING);

    /*get the fb of display from bo */
    bo_width = gbm_bo_get_width(bo);
    bo_height = gbm_bo_get_height(bo);
    bo_handle = gbm_bo_get_handle(bo);
    handles[0] = bo_handle.u32;
    handles[1] = handles[0];
    pitches[0] = gbm_bo_get_stride(bo);
    pitches[1] = pitches[0];
    offsets[0] = 0;
    offsets[1] = pitches[0] * VENUS_Y_SCANLINES(COLOR_FMT_NV12, height);

    if (drmModeAddFB2(drm_fd, bo_width, bo_height, DRM_FORMAT_NV12, handles, pitches, offsets, &fbid, 0))
    {
        DEBUGT_PRINT("failed to add fb: %s, drm_fd %d, bo_width %d, bo_height %d, handles[0] %d, pitches[0] %d, offset[0] %d fbid %d", strerror(errno), drm_fd, bo_width, bo_height,
        handles[0], pitches[0], offsets[0], fbid);
        goto failed_with_bo;
    }

    dst_w = connector->modes[0].hdisplay;
    dst_h = connector->modes[0].vdisplay;

    if (drmModeSetPlane(drm_fd, plane_id, crtc_id, fbid, 0, 0, 0, dst_w, dst_h, 0, 0, bo_width << 16, bo_height << 16))
    {
        DEBUGT_PRINT("drm_fd=%d, crtc_id=%d, fbid=%d, failed to set plane.errno=%s", drm_fd, crtc_id, fbid, strerror(errno));
        goto failed_with_bo;
    }

    if (fbid_current)
        drmModeRmFB(drm_fd, fbid_current);
    fbid_current = fbid;

    if (first_display)
    {
        place_marker((char*)"ethcam === DRM First Frame on Screen");
        first_display = false;
    }

    gbm_bo_destroy(bo);
    return 0;

failed_with_bo:
    gbm_bo_destroy(bo);
    return -1;
}

#ifdef WL_DISPLAY
#define IVI_SURFACE_ID 9000
static void
handle_surface_configure(void *data, struct xdg_surface *surface,
         int32_t width, int32_t height,
         struct wl_array *states, uint32_t serial)
{
    xdg_surface_ack_configure(surface, serial);
}

static void
handle_surface_delete(void *data, struct xdg_surface *xdg_surface)
{
//    running = 0;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    handle_surface_configure,
    handle_surface_delete,
};

static void
handle_ivi_surface_configure(void *data, struct ivi_surface *ivi_surface,
                             int32_t width, int32_t height)
{
}

static const struct ivi_surface_listener ivi_surface_listener = {
        handle_ivi_surface_configure,
};
static struct window *
create_window(struct display *display, int width, int height)
{
  struct window *window;
  int retry_count = 0;
  window =(struct window *) calloc(1, sizeof *window);
  if (!window)
  return NULL;

  window->callback = NULL;
  window->display = display;
  window->width = width;
  window->height = height;
  window->surface = wl_compositor_create_surface(display->compositor);
  while(!surface_registered && retry_count < 10){
    usleep(1000);
    retry_count ++;
  }
  if (display->shell) {
     printf("display->shell\n");
     window->xdg_surface =
         xdg_shell_get_xdg_surface(display->shell,
         window->surface);

     g_assert(window->xdg_surface);

     xdg_surface_add_listener(window->xdg_surface,
         &xdg_surface_listener, window);

     xdg_surface_set_title(window->xdg_surface, "mm-vidc-display");
     xdg_surface_set_fullscreen(window->xdg_surface,NULL);
     wl_display_roundtrip(display->display);

  } else if (display->ivi_application) {
    printf("display->ivi_application\n");
    uint32_t id_ivisurf = IVI_SURFACE_ID + (uint32_t)getpid();
    window->ivi_surface =
    ivi_application_surface_create(display->ivi_application,
        id_ivisurf, window->surface);

    if (window->ivi_surface == NULL) {
       DEBUGT_PRINT_ERROR("Failed to create ivi_client_surface\n");
       return NULL;
    }

    ivi_surface_add_listener(window->ivi_surface,
    &ivi_surface_listener, window);
  } else if (display->wlshell) {
    printf("display->wlshell\n");
    window->shell_surface = wl_shell_get_shell_surface(display->wlshell, window->surface);
    if (!window->shell_surface)
    {
       DEBUGT_PRINT("wl_shell_get_shell_surface failed.");
    }

    wl_shell_surface_add_listener(window->shell_surface, &shell_surface_listener, NULL);
    wl_shell_surface_set_fullscreen(window->shell_surface, WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE, 0, NULL);
   }else {
     g_assert(0);
  }

  return window;
}
static void
destroy_window(struct window *window)
{
  printf("weston destroy window\n");
  if (window->xdg_surface)
     xdg_surface_destroy(window->xdg_surface);
  if (window->display->ivi_application) {
     ivi_surface_destroy(window->ivi_surface);
     ivi_application_destroy(window->display->ivi_application);
     printf("weston destroy window ivi_application\n");
  }
  if (window->surface)
     wl_surface_destroy(window->surface);
   if (window->shell_surface)
        {
            wl_shell_surface_destroy(window->shell_surface);
           window->shell_surface = NULL;
        }
  if (window)
    free(window);

  printf("weston free window\n");
}
#endif

static int set_crtc(unsigned int color_format)
{
    union gbm_bo_handle bo_handle;
    uint32_t handles[4] = {0};
    uint32_t pitches[4] = {0};
    uint32_t offsets[4] = {0};
    uint32_t flags = 0;
    uint32_t fbid;
    struct drm_mode_fb_cmd2 args;
    unsigned char *back_buffer;
    unsigned int buffer_size = 0;
    unsigned int bo_width, bo_height;

    bo_width = connector->modes[0].hdisplay;
    bo_height = connector->modes[0].vdisplay;
    /* create bo */
    crtc_bo = gbm_bo_create(gbm, bo_width, bo_height, color_format, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING | GBM_BO_USE_WRITE);
    if (crtc_bo == NULL)
    {
        DEBUGT_PRINT("Failed to create GBM bo");
        goto failed;
    }

    /*get the fb of display from bo */
    bo_width = gbm_bo_get_width(crtc_bo);
    bo_height = gbm_bo_get_height(crtc_bo);
    bo_handle = gbm_bo_get_handle(crtc_bo);
    handles[0] = bo_handle.u32;
    handles[1] = handles[0];
    pitches[0] = gbm_bo_get_stride(crtc_bo);
    pitches[1] = pitches[0];
    offsets[0] = 0;
    offsets[1] = pitches[0] * bo_height;

    buffer_size = pitches[0] * bo_height;
    back_buffer = (unsigned char*)malloc(buffer_size);
    DEBUGT_PRINT("buffer_size = %d", buffer_size);
    if (!back_buffer)
    {
        goto failed_with_bo;
    }

    memset(back_buffer, 0xff, buffer_size);

    if (gbm_bo_write(crtc_bo, back_buffer, per_frame_byte))
    {
        DEBUGT_PRINT("gbm_bo_write calling failed");
        goto failed_with_buffer;
    }

    free(back_buffer);
    back_buffer = NULL;

    if (drmModeAddFB2(drm_fd, bo_width, bo_height, color_format, handles, pitches, offsets, &fbid, 0))
    {
        DEBUGT_PRINT("failed to add fb: %s", strerror(errno));
        goto failed_with_bo;
    }

    /* set fb to display */
    if(drmModeSetCrtc(drm_fd, crtc_id, fbid, 0, 0, &connector->connector_id, 1, &connector->modes[0]))
    {
        DEBUGT_PRINT("drm_fd=%d, crtc_id=%d, fbid=%d, connector_id=%d,failed to set mode.errno=%s", drm_fd, crtc_id, fbid, connector->connector_id, strerror(errno));
        goto failed_with_bo;
    }

    fbid_current = fbid;
    return 0;

failed_with_buffer:
    free(back_buffer);
    back_buffer = NULL;

failed_with_bo:
    gbm_bo_destroy(crtc_bo);

failed:
    return -1;
}

static bool format_support(const drmModePlanePtr ovr, uint32_t fmt)
{
    unsigned int i;

    for (i = 0; i < ovr->count_formats; ++i)
    {
        if (ovr->formats[i] == fmt)
            return true;
    }

    return false;
}

static int find_crtc_for_connector(int fd, drmModeRes *resources, drmModeConnector *connector)
{
    drmModeEncoder *encoder;
    uint32_t possible_crtcs;
    int i, j;

    for (j = 0; j < connector->count_encoders; j++)
    {
        encoder = drmModeGetEncoder(fd, connector->encoders[j]);
        if (encoder == NULL)
        {
            DEBUGT_PRINT("Failed to get encoder.\n");
            return -1;
        }
        possible_crtcs = encoder->possible_crtcs;
        drmModeFreeEncoder(encoder);

        for (i = 0; i < resources->count_crtcs; i++)
        {
            if (possible_crtcs & (1 << i))
                return i;
        }
    }

    return -1;
}

int drm_open_display()
{
    int idx = -1, i = 0;
    drmModePlane *ovr;
    place_marker((char*)"ethcam === drm open");

    drm_ready = false;
    per_frame_byte = width * height *3/2;//VENUS_BUFFER_SIZE(COLOR_FMT_NV12, width, height);

    while (!gbm_created)
    {
        DEBUGT_PRINT("gbm device not created, wait 5ms");
        usleep(5000);
    }

    resources = drmModeGetResources(drm_fd);
    if (!resources)
    {
        DEBUGT_PRINT("drmModeGetResources failed");
        goto failed_created;
    }

    for (i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (!connector)
        {
            DEBUGT_PRINT("failed to get connector");
            goto failed_resourced;
        }

        if (connector->connector_type == DRM_MODE_CONNECTOR_HDMIA)
        {
            DEBUGT_PRINT("get HDMI connctor succeed!");
            break;
        }
    }

    if (i == resources->count_connectors)
    {
        DEBUGT_PRINT("failed to detect HDMI connctor");
        goto failed_resourced;
    }

    idx = find_crtc_for_connector(drm_fd, resources, connector);
    if (idx < 0)
    {
        DEBUGT_PRINT("No usable crtc/encoder pair for connector");
        goto failed_with_connector;
    }

    crtc =  drmModeGetCrtc(drm_fd, resources->crtcs[idx]);

    if (!crtc)
    {
        DEBUGT_PRINT("failed to get crtc");
        goto failed_with_connector;
    }

    crtc_id = crtc->crtc_id;
    if (crtc_id < 0)
    {
        DEBUGT_PRINT("failed to get crtc id");
        goto failed_with_crtc;
    }
    DEBUGT_PRINT("crtc id is %d for connector %d", crtc_id, i);

    drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

    plane_res = drmModeGetPlaneResources(drm_fd);
    if (!plane_res)
    {
        DEBUGT_PRINT("drmModeGetPlaneResources failed");
        goto failed_created;
    }

    for (i = 0; i < plane_res->count_planes; i++)
    {
        ovr = drmModeGetPlane(drm_fd, plane_res->planes[i]);
        if (!ovr)
        {
            DEBUGT_PRINT("drmModeGetPlane failed");
            continue;
        }

        if (!format_support(ovr, DRM_FORMAT_NV12))
            continue;

        if (ovr->possible_crtcs & (1 << idx))
        {
            plane_id = ovr->plane_id;
            break;
        }
    }

    DEBUGT_PRINT("plane id is %d", plane_id);

    drmSetMaster(drm_fd);
    if(set_crtc(DRM_FORMAT_NV12))
    {
        DEBUGT_PRINT("set crtc failed");
        goto failed_with_crtc;
    }
    else
        DEBUGT_PRINT("set crtc succeeds");

    drm_ready = true;
    place_marker((char*)"ethcam === DRM open_display done");

    return 0;

failed_with_crtc:
    drmModeFreeCrtc(crtc);

failed_with_connector:
    drmModeFreeConnector(connector);

failed_resourced:
    drmModeFreeResources(resources);
    drmModeFreePlaneResources(plane_res);

failed_created:
    gbm_device_destroy(gbm);

failed:
    close(drm_fd);
    drm_fd = -1;

    return -1;
}

void drm_close_display()
{
    drmModeFreeCrtc(crtc);
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    drmModeFreePlaneResources(plane_res);
    if (fbid_current)
        drmModeRmFB(drm_fd, fbid_current);
    gbm_bo_destroy(crtc_bo);
    gbm_device_destroy(gbm);
    gbm = NULL;
    gbm_created = false;
    close(drm_fd);
    drm_fd = -1;
}

void getFreePmem()
{
#ifndef USE_ION
   int ret = -1;
   /*Open pmem device and query free pmem*/
   int pmem_fd = open (PMEM_DEVICE,O_RDWR);

   if(pmem_fd < 0) {
     ALOGE("Unable to open pmem device");
     return;
   }
   struct pmem_freespace fs;
   ret = ioctl(pmem_fd, PMEM_GET_FREE_SPACE, &fs);
   if(ret) {
     ALOGE("IOCTL to query pmem free space failed");
     goto freespace_query_failed;
   }
   ALOGE("Available free space %lx largest chunk %lx", fs.total, fs.largest);
freespace_query_failed:
   close(pmem_fd);
#endif
}

#ifdef USE_ION
int alloc_map_ion_memory( OMX_U32 buffer_size,struct vdec_ion *vdec,OMX_U32 alignment, int flag)
{
  if(protocol == USE_GBM_PROTOCOL){
     struct gbm_bo *bo = NULL;
     int bo_fd = -1, meta_fd = -1;
     int size = buffer_size;
     void *data = NULL;
     size = (size + 4096 - 1) & ~(4096 - 1);
     DEBUGT_PRINT("buffer_size %d, size %d\n", buffer_size, size);

     bo = gbm_bo_create(gbm, width, height, GBM_FORMAT_NV12, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
     if(bo == NULL)
         DEBUGT_PRINT("Create bo failed \n");
     vdec->bo = bo;
     bo_fd = gbm_bo_get_fd(bo);
     if(bo_fd < 0)
         DEBUGT_PRINT("Get bo fd failed \n");

     gbm_perform(GBM_PERFORM_GET_METADATA_ION_FD, bo, &meta_fd);
     if(meta_fd < 0)
         DEBUGT_PRINT("Get bo meta fd failed \n");

     data = (OMX_U8 *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, bo_fd, 0);
     if (data == MAP_FAILED) {
        DEBUGT_PRINT("mmap failed\n");
        return 0;
     }
     vdec->meta_fd = meta_fd;
     vdec->bo_fd = bo_fd;
     return bo_fd;
  }
  else{
     int fd = -EINVAL;
     int rc = -EINVAL;
     int ion_dev_flag;
     struct vdec_ion ion_buf_info;
     int secure_mode = 0;
     struct ion_allocation_data *alloc_data = &(vdec->ion_alloc_data);
     struct ion_fd_data *fd_data = &(vdec->fd_ion_data);

     if (!alloc_data || buffer_size <= 0 || !fd_data) {
       DEBUGT_PRINT_ERROR("Invalid arguments to alloc_map_ion_memory");
       return -EINVAL;
     }
     ion_dev_flag = O_RDONLY;
     fd = open (MEM_DEVICE, ion_dev_flag);
     if (fd < 0) {
      DEBUGT_PRINT_ERROR("opening ion device failed with fd = %d", fd);
      return fd;
    }
    alloc_data->flags = 0;
    if(!secure_mode && (flag & ION_FLAG_CACHED))
    {
      alloc_data->flags |= ION_FLAG_CACHED;
    }
    alloc_data->len = buffer_size;
    alloc_data->align = clip2(alignment);
    if (alloc_data->align < 4096)
    {
      alloc_data->align = 4096;
    }
    if ((secure_mode) && (flag & ION_SECURE))
      alloc_data->flags |= ION_SECURE;

    alloc_data->heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID);
    if (secure_mode)
      alloc_data->heap_id_mask = ION_HEAP(MEM_HEAP_ID);
    rc = ioctl(fd,ION_IOC_ALLOC,alloc_data);
    if (rc || !alloc_data->handle) {
      DEBUGT_PRINT_ERROR(" ION ALLOC memory failed ");
      alloc_data->handle = 0;
      close(fd);
      fd = -ENOMEM;
      return fd;
    }
    fd_data->handle = alloc_data->handle;

    rc = ioctl(fd,ION_IOC_MAP,fd_data);
    if (rc) {
      DEBUGT_PRINT_ERROR(" ION MAP failed ");
      ion_buf_info.ion_alloc_data = *alloc_data;
      ion_buf_info.ion_device_fd = fd;
      ion_buf_info.fd_ion_data = *fd_data;
      free_ion_memory(&ion_buf_info);
      fd_data->fd =-1;
      close(fd);
      fd = -ENOMEM;
    }
    return fd;
  }
}

void free_ion_memory(struct vdec_ion *buf_ion_info) {

     if(!buf_ion_info) {
       DEBUGT_PRINT_ERROR(" ION: free called with invalid fd/allocdata");
       return;
     }
     if(protocol == USE_GBM_PROTOCOL){
         if (buf_ion_info->bo)
             gbm_bo_destroy(buf_ion_info->bo);
         buf_ion_info->bo = NULL;
         buf_ion_info->bo_fd = -1;
         buf_ion_info->meta_fd = -1;
         buf_ion_info->ion_device_fd = -1;
     }
     else{
         if(ioctl(buf_ion_info->ion_device_fd,ION_IOC_FREE,
                 &buf_ion_info->ion_alloc_data.handle)) {
           DEBUGT_PRINT_ERROR(" ION: free failed" );
         }
         close(buf_ion_info->ion_device_fd);
         buf_ion_info->ion_device_fd = -1;
         buf_ion_info->ion_alloc_data.handle = 0;
         buf_ion_info->fd_ion_data.fd = -1;
     }
}

void free_ion(int index){
    free_ion_memory(&outPort_ion[index]);
}
void free_outport_ion(){
    if (outPort_ion) {
        free(outPort_ion);
        outPort_ion = NULL;
    }
}
#endif

OMX_ERRORTYPE use_output_buffer_multiple_fd ( OMX_COMPONENTTYPE *dec_handle,
                                  OMX_BUFFERHEADERTYPE  ***pBufHdrs,
                                  OMX_U32 nPortIndex,
                                  OMX_U32 bufSize,
                                  long bufCntMin)
{
    DEBUGT_PRINT("Inside %s ", __FUNCTION__);
    OMX_ERRORTYPE error=OMX_ErrorNone;
    long bufCnt=0;
    OMX_U8* pvirt = NULL;
//#ifndef USE_OUTPUT_BUFFER
#ifndef USE_ION
    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE)* bufCntMin);
    if(*pBufHdrs == NULL){
        DEBUGT_PRINT_ERROR(" m_inp_heap_ptr Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }
    pPlatformList = (OMX_QCOM_PLATFORM_PRIVATE_LIST *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_LIST)* bufCntMin);

    if(pPlatformList == NULL){
        DEBUGT_PRINT_ERROR(" pPlatformList Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    pPlatformEntry = (OMX_QCOM_PLATFORM_PRIVATE_ENTRY *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_ENTRY)* bufCntMin);

    if(pPlatformEntry == NULL){
        DEBUGT_PRINT_ERROR(" pPlatformEntry Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    pPMEMInfo = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO)* bufCntMin);

    if(pPMEMInfo == NULL){
        DEBUGT_PRINT_ERROR(" pPMEMInfo Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    //output_use_buffer = true;
    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        // allocate input buffers
      DEBUGT_PRINT("OMX_UseBuffer_multiple_fd No %d %d ", bufCnt, bufSize);

      pPlatformEntry[bufCnt].type       = OMX_QCOM_PLATFORM_PRIVATE_PMEM;
      pPlatformEntry[bufCnt].entry      = &pPMEMInfo[bufCnt];
      // Initialize the Platform List
      pPlatformList[bufCnt].nEntries    = 1;
      pPlatformList[bufCnt].entryList   = &pPlatformEntry[bufCnt];
      pPMEMInfo[bufCnt].offset          =  0;
      pPMEMInfo[bufCnt].pmem_fd = open(PMEM_DEVICE,O_RDWR);
      if(pPMEMInfo[bufCnt].pmem_fd < 0) {
          DEBUGT_PRINT_ERROR(" open failed %s",PMEM_DEVICE);
          return OMX_ErrorInsufficientResources;
      }
#ifndef USE_ION
      /* TBD - this commenting is dangerous */
      align_pmem_buffers(pPMEMInfo[bufCnt].pmem_fd, bufSize,
                                  8192);
#endif
      DEBUGT_PRINT(" allocation size %d pmem fd 0x%x",bufSize,pPMEMInfo[bufCnt].pmem_fd);
      pvirt = (unsigned char *)mmap(NULL,bufSize,PROT_READ|PROT_WRITE,
                      MAP_SHARED,pPMEMInfo[bufCnt].pmem_fd,0);
      getFreePmem();
      DEBUGT_PRINT(" Virtual Address %p Size %d pmem_fd=0x%x",pvirt,bufSize,pPMEMInfo[bufCnt].pmem_fd);
      if (pvirt == MAP_FAILED) {
        DEBUGT_PRINT_ERROR(" mmap failed for buffers");
        return OMX_ErrorInsufficientResources;
      }
      use_buf_virt_addr[bufCnt] = pvirt;
      error = OMX_UseBuffer(dec_handle, &((*pBufHdrs)[bufCnt]),
                            nPortIndex, &pPlatformList[bufCnt], bufSize, pvirt);
    }
#else
    int ion_device_fd =-1;
    struct ion_allocation_data ion_alloc_data;
    struct ion_fd_data fd_ion_data;
    struct vdec_ion vdec;

    while(!gbm_created)
    {
      usleep(1000);
    }
    vdec.ion_device_fd = drm_fd;
    *pBufHdrs= (OMX_BUFFERHEADERTYPE **)
                   malloc(sizeof(OMX_BUFFERHEADERTYPE)* bufCntMin);
    if(*pBufHdrs == NULL){
        DEBUGT_PRINT_ERROR(" m_inp_heap_ptr Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }
    pPlatformList = (OMX_QCOM_PLATFORM_PRIVATE_LIST *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_LIST)* bufCntMin);

    if(pPlatformList == NULL){
        DEBUGT_PRINT_ERROR(" pPlatformList Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    pPlatformEntry = (OMX_QCOM_PLATFORM_PRIVATE_ENTRY *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_ENTRY)* bufCntMin);

    if(pPlatformEntry == NULL){
        DEBUGT_PRINT_ERROR(" pPlatformEntry Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }

    pPMEMInfo = (OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO *)
        malloc(sizeof(OMX_QCOM_PLATFORM_PRIVATE_PMEM_INFO)* bufCntMin);

    if(pPMEMInfo == NULL){
        DEBUGT_PRINT_ERROR(" pPMEMInfo Allocation failed ");
        return OMX_ErrorInsufficientResources;
     }
    outPort_ion = (struct vdec_ion *)calloc(sizeof(struct vdec_ion), bufCntMin);

    //output_use_buffer = true;
    for(bufCnt=0; bufCnt < bufCntMin; ++bufCnt) {
        // allocate input buffers
      DEBUGT_PRINT("OMX_UseBuffer_multiple_fd No %d %d ", bufCnt, bufSize);

      pPlatformEntry[bufCnt].type       = OMX_QCOM_PLATFORM_PRIVATE_PMEM;
      pPlatformEntry[bufCnt].entry      = &pPMEMInfo[bufCnt];
      // Initialize the Platform List
      pPlatformList[bufCnt].nEntries    = 1;
      pPlatformList[bufCnt].entryList   = &pPlatformEntry[bufCnt];
      pPMEMInfo[bufCnt].offset          =  0;

      ion_device_fd = alloc_map_ion_memory(bufSize, &vdec, 4096, 0);
      if (ion_device_fd < 0) {
          return OMX_ErrorInsufficientResources;
      }
      if(protocol == USE_GBM_PROTOCOL) {
          pPMEMInfo[bufCnt].pmem_fd  = vdec.bo_fd;
          pPMEMInfo[bufCnt].pmeta_fd = vdec.meta_fd;
          outPort_ion[bufCnt].ion_device_fd = ion_device_fd;
          outPort_ion[bufCnt].bo_fd= vdec.bo_fd;
          outPort_ion[bufCnt].meta_fd= vdec.meta_fd;
      }
      else{
          pPMEMInfo[bufCnt].pmem_fd  = vdec.fd_ion_data.fd;
          outPort_ion[bufCnt].ion_device_fd = ion_device_fd;
          outPort_ion[bufCnt].ion_alloc_data = vdec.ion_alloc_data;
          outPort_ion[bufCnt].fd_ion_data = vdec.fd_ion_data;
      }
      DEBUGT_PRINT(" allocation size %d pmem fd 0x%x",bufSize,pPMEMInfo[bufCnt].pmem_fd);
      pvirt = (unsigned char *)mmap(NULL,bufSize,PROT_READ|PROT_WRITE,
            MAP_SHARED,pPMEMInfo[bufCnt].pmem_fd,0);
      DEBUGT_PRINT(" Virtual Address %p Size %d pmem_fd=0x%x",pvirt,bufSize,pPMEMInfo[bufCnt].pmem_fd);
      if (pvirt == MAP_FAILED) {
         DEBUGT_PRINT_ERROR(" mmap failed for buffers");
         return OMX_ErrorInsufficientResources;
      }
      use_buf_virt_addr[bufCnt] = pvirt;
      error = OMX_UseBuffer(dec_handle, &((*pBufHdrs)[bufCnt]),
                            nPortIndex, &pPlatformList[bufCnt], bufSize, pvirt);
    }
#endif
    return error;
}



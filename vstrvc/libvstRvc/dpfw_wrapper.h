#ifndef FASTRVC_MDP_DPFW_H
#define FASTRVC_MDP_DPFW_H

#include <unistd.h>
#include <sync/sync.h>


typedef struct dpfw_mdp_device {
    unsigned int src_w; // out_buff.width
    unsigned int src_h;
    unsigned int src_fmt;
    unsigned int src_crop_w;
    unsigned int src_crop_h;
    void *src_pa;
    int src_fd;
    unsigned int dst_w; // cap_buff.width
    unsigned int dst_h;
    unsigned int dst_fmt;
    unsigned int dst_crop_w;
    unsigned int dst_crop_h;
    void *dst_pa;
    int dst_fd;
    unsigned int dst_rotation;
    // private
    unsigned int shp_enable; // PQ sharpness_enable
    unsigned int shp_level; // PQ sharpness_level
    unsigned int dc_enable; // PQ dynamic_contrast_enable
} dpfw_mdp_device;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int dpfw_mdp_create(void);
int dpfw_mdp_delete(void);
int dpfw_mdp_make_frame(void* in_va, void* out_va, dpfw_mdp_device* mdp_dev);


#ifdef __cplusplus
}
#endif

#endif /* __DPFW_H__ */
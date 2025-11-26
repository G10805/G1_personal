#ifndef _DSP_AUDIO_MICRO_SERVICE_H_
#define _DSP_AUDIO_MICRO_SERVICE_H_

/*===========================================================================
Copyright (c) 2019,2022-2023 Qualcomm Technologies, Inc.
All rights reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================ */

/****************************************************************************
 * DSP Audio Micro Service (AMS) APIS                                       *
 ****************************************************************************/

/**
  Major DSP AMS API version.
*/
#define DSP_AMS_MAJOR_VERSION (1)

/**
  Minor DSP AMS API version.
*/
#define DSP_AMS_MINOR_VERSION (0)

/**
  Status code for DSP AMS.
*/
#define DSP_AMS_STATUS_OK (0x00)
#define DSP_AMS_STATUS_GENERAL_ERROR (0x01)
#define DSP_AMS_STATUS_BAD_PARAM (0x02)
#define DSP_AMS_STATUS_UNSUPPORTED (0x03)
#define DSP_AMS_STATUS_ALREADY (0x04)
#define DSP_AMS_STATUS_NO_RESOURCE (0x05)
#define DSP_AMS_STATUS_BAD_STATE (0x06)

/**
  Gets the DSP AMS version.

  @payload
  None.

  @return
  DSP_AMS_CMDRSP_GET_VERSION
*/
#define DSP_AMS_CMD_GET_VERSION (0x00013367)

/**
  Response to the DSP_AMS_CMD_GET_VERSION command.

  @payload
  dsp_ams_cmdrsp_get_version_t.

  @return
  None.
*/
#define DSP_AMS_CMDRSP_GET_VERSION (0x00013368)

typedef struct dsp_ams_cmdrsp_get_version_t dsp_ams_cmdrsp_get_version_t;

struct dsp_ams_cmdrsp_get_version_t
{
  uint32_t major_version;
  /**< DSP AMS major version. */
  uint32_t minor_version;
  /**< DSP AMS minor version. */
};

/**
  Opens a graph on DSP AMS.

  @payload
  dsp_ams_cmd_open_graph_t.

  @return
  DSP_AMS_CMDRSP_OPEN_GRAPH
*/
#define DSP_AMS_CMD_OPEN_GRAPH (0x00013369)

/** Exclusive endpoint - endpoint managed only by AMS. */
#define DSP_AMS_ENDPOINT_EXCLUSIVE (0)

/**
  Shared endpoint - endpoint shared between AMS and AVS framework, where the
  AVS framework outputs to this endpoint. The configuration of the HW interface
  is done by AVS framework.
*/
#define DSP_AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT (1)

/**
  Shared endpoint - endpoint shared between AMS and AVS framework, where the
  AVS framework gets input from this endpoint. The configuration of the HW
  interface is done by AVS framework.
*/
#define DSP_AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT (2)

/** Source endpoint type. */
#define DSP_AMS_ENDPOINT_TYPE_SOURCE (0)

/** Sink endpoint type. */
#define DSP_AMS_ENDPOINT_TYPE_SINK (1)

/** TDM 1 HW interface ID. */
#define DSP_AMS_HW_INTERFACE_TDM1 (1)

/** TDM 2 HW interface ID. */
#define DSP_AMS_HW_INTERFACE_TDM2 (2)

/** TDM 3 HW interface ID. */
#define DSP_AMS_HW_INTERFACE_TDM3 (3)

/** TDM 4 HW interface ID. */
#define DSP_AMS_HW_INTERFACE_TDM4 (4)

/** TDM 5 HW interface ID. */
#define DSP_AMS_HW_INTERFACE_TDM5 (5)

/** Linear PCM data*/
#define DSP_AMS_TDM_LINEAR_PCM_DATA (0)

/** Short (one-bit) synchronization mode. */
#define DSP_AMS_TDM_SHORT_SYNC_BIT_MODE (0)

/** Long synchronization mode. */
#define DSP_AMS_TDM_LONG_SYNC_MODE (1)

/** Short (one-slot) synchronization mode. */
#define DSP_AMS_TDM_SHORT_SYNC_SLOT_MODE (2)

/** Synchronization source is external. */
#define DSP_AMS_TDM_SYNC_SRC_EXTERNAL (0)

/** Synchronization source is internal. */
#define DSP_AMS_TDM_SYNC_SRC_INTERNAL (1)

/** Zero-bit clock cycle synchronization data delay. */
#define DSP_AMS_TDM_DATA_DELAY_0_BCLK_CYCLE (0)

/** One-bit clock cycle synchronization data delay. */
#define DSP_AMS_TDM_DATA_DELAY_1_BCLK_CYCLE (1)

/** Two-bit clock cycle synchronization data delay. */
#define DSP_AMS_TDM_DATA_DELAY_2_BCLK_CYCLE (2)

/** Disable sharing of the data-out signal. */
#define DSP_AMS_TDM_CTRL_DATA_OE_DISABLE (0)

/** Enable sharing of the data-out signal. */
#define DSP_AMS_TDM_CTRL_DATA_OE_ENABLE (1)

/** Normal synchronization. */
#define DSP_AMS_TDM_SYNC_NORMAL (0)

/** Invert the synchronization. */
#define DSP_AMS_TDM_SYNC_INVERT (1)

typedef struct dsp_ams_endpoint_t dsp_ams_endpoint_t;

struct dsp_ams_endpoint_t
{
  uint32_t id;
  /**< User-assigned unique identifier. */
  uint32_t type;
  /**< Specifies whether endpoint is source or sink.
       - DSP_AMS_ENDPOINT_TYPE_SOURCE
       - DSP_AMS_ENDPOINT_TYPE_SINK */
  uint32_t channel_mask;
  /**< Channel enablement mask. */
  uint16_t channel_type[32];
  /**< Channel type as defined by 80-NF775-1. */
  uint32_t q_factor;
  /**< Number of fractional bits in sample. */
  uint32_t flags;
  /**< - DSP_AMS_ENDPOINT_EXCLUSIVE
       - DSP_AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT
       - DSP_AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT */
  struct
  {
    uint32_t hw_interface_id;
    /**< TDM interface ID.
         - DSP_AMS_HW_INTERFACE_TDM1
         - DSP_AMS_HW_INTERFACE_TDM2
         - DSP_AMS_HW_INTERFACE_TDM3
         - DSP_AMS_HW_INTERFACE_TDM4
         - DSP_AMS_HW_INTERFACE_TDM5 */
    uint8_t num_channels;
    /**< Total number of channels = number of active lanes * number of active slots. */
    uint8_t bit_width;
    /**< Bit width of the sample. */
    uint16_t data_format;
    /**< Data format supported by the TDM frame.
         - DSP_AMS_TDM_LINEAR_PCM_DATA */
    uint16_t sync_mode;
    /**< TDM Synchronization mode setting.
         - DSP_AMS_TDM_SHORT_SYNC_BIT_MODE
         - DSP_AMS_TDM_LONG_SYNC_MODE
         - DSP_AMS_TDM_SHORT_SYNC_SLOT_MODE */
    uint16_t sync_src;
    /**< Synchronization source.
         - DSP_AMS_TDM_SYNC_SRC_EXTERNAL
         - DSP_AMS_TDM_SYNC_SRC_INTERNAL */
    uint16_t nslots_per_frame;
    /**< Number of slots per frame. */
    uint16_t ctrl_data_out_enable;
    /**< Specifies whether the TDM hardware block shares the data-out
         signal to the driver with other masters.
         - DSP_AMS_TDM_CTRL_DATA_OE_DISABLE
         - DSP_AMS_TDM_CTRL_DATA_OE_ENABLE */
    uint16_t ctrl_invert_sync_pulse;
    /**< Specifies whether to invert synchronization.
         - DSP_AMS_TDM_SYNC_NORMAL
         - DSP_AMS_TDM_SYNC_INVERT */
    uint16_t ctrl_sync_data_delay;
    /**< Number of bit-clock cycles for delaying the data for
         synchronization.
         - DSP_AMS_TDM_DATA_DELAY_0_BCLK_CYCLE
         - DSP_AMS_TDM_DATA_DELAY_1_BCLK_CYCLE
         - DSP_AMS_TDM_DATA_DELAY_2_BCLK_CYCLE */
    uint16_t slot_width;
    /**< Width of the slot in a TDM frame: 16, 24, 32. */
    uint32_t slot_mask;
    /**< Position of the active slots: 1 to 2^32-1.
         Bits 0 to 31 correspond to slots 0 to 31. When a bit is set, the
         corresponding slot is active. The number of active slots can be
         inferred from the number of bits set in the mask.

         32 individual bits can be enabled. */
  } tdm_params;
};

/** Module's processing is done in DMA IST context. */
#define DSP_AMS_INLINE_PROCESSING_MODE (0)

/** Module's processing is done in worker thread context. */
#define DSP_AMS_DEFERRED_PROCESSING_MODE (1)

/** An element in the connection is module. */
#define DSP_AMS_CONNECTION_ELEMENT_TYPE_MODULE (0)

/** An element in the connection is endpoint. */
#define DSP_AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT (1)

typedef struct dsp_ams_module_t dsp_ams_module_t;

struct dsp_ams_module_t
{
  uint32_t id;
  /**< User-assigned unique identifier. */
  uint32_t flags;
  /**< - DSP_AMS_INLINE_PROCESSING_MODE
       - DSP_AMS_DEFERRED_PROCESSING_MODE */
  struct
  {
    uint32_t id;
    /**< CAPIv2 module ID. */
    char shared_obj_filename[32];
    /**< Shared object file name for dynamic module loading; set to
         "" for statically linked modules. */
    char tag[32];
    /**< Tag for dynamic module loading; set to "" for statically
         linked modules. */
  } capiv2_info;
};

/**
  Connection between a source and a destination element.
  An element may be a module or endpoint.
*/
typedef struct dsp_ams_connection_t dsp_ams_connection_t;

struct dsp_ams_connection_t
{
  uint8_t num_channels;
  uint8_t bit_width;

  /** Source info. */
  struct
  {
    uint32_t type;
    /**< Specifies whether the source element is a module or endpoint.
         - DSP_AMS_CONNECTION_ELEMENT_TYPE_MODULE
         - DSP_AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT */
    union
    {
      /** Source module info. */
      struct
      {
        uint32_t id;
        /**< ID for source module. */
        uint16_t port_index;
        /**< 0-based port index. */
      } module;
      /** Source endpoint info. */
      struct
      {
        uint32_t id;
        /**< ID for source endpoint. */
      } endpoint;
    };
  } source;

  /** Destination info. */
  struct
  {
    uint32_t type;
    /**< Specifies whether the destination element is a module or endpoint.
         - DSP_AMS_CONNECTION_ELEMENT_TYPE_MODULE
         - DSP_AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT */
    union
    {
      /** Destination module. */
      struct
      {
        uint32_t id;
        /**< ID for destination module. */
        uint16_t port_index;
        /**< 0-based port index. */
      } module;
      /** Destination endpoint. */
      struct
      {
        uint32_t id;
        /**< ID for destination endpoint. */
      } endpoint;
    };
  } destination;
};

/** Command payload for DSP_AMS_CMD_OPEN_GRAPH */
typedef struct dsp_ams_cmd_open_graph_t dsp_ams_cmd_open_graph_t;

struct dsp_ams_cmd_open_graph_t
{
  uint32_t sample_rate;
  /**< Sample rate in Hz. */
  uint32_t block_size;
  /**< Number of samples per processing block. */
  uint32_t flags;
  /**< Reserved, set to 0. */
  uint32_t num_endpoints;
  /**< Number of endpoints in the graph. */
  uint32_t num_modules;
  /**< Number of modules in the graph. */
  uint32_t num_connections;
  /**< Number of connections in the graph. */

  /** Followed by arrays below.
      The start address for each of the arrays must be 4-byte aligned.
      dsp_ams_endpoint_t endpoints[num_endpoints];
      dsp_ams_module_t modules[num_modules];
      dsp_ams_connection_t connections[num_connections]; */
};

/**
  Response to the DSP_AMS_CMD_OPEN_GRAPH command.

  @payload
  dsp_ams_cmdrsp_open_graph_t.

  @return
  None.
*/
#define DSP_AMS_CMDRSP_OPEN_GRAPH (0x0001336A)

typedef struct dsp_ams_cmdrsp_open_graph_t dsp_ams_cmdrsp_open_graph_t;

struct dsp_ams_cmdrsp_open_graph_t
{
  uint32_t status;
  /**< Status of the open graph command. */
  uint32_t handle;
  /**< Graph handle, valid when status is DSP_AMS_STATUS_OK. */
};

/**
  Closes a graph on DSP AMS.

  @payload
  dsp_ams_cmd_close_graph_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_CLOSE_GRAPH (0x0001336B)

typedef struct dsp_ams_cmd_close_graph_t dsp_ams_cmd_close_graph_t;

struct dsp_ams_cmd_close_graph_t
{
  uint32_t handle;
  /**< Graph handle. */
};

/**
  Starts a graph on DSP AMS.

  @payload
  dsp_ams_cmd_start_graph_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_START_GRAPH (0x0001336C)

/** Command payload for DSP_AMS_CMD_START_GRAPH */
typedef struct dsp_ams_cmd_start_graph_t dsp_ams_cmd_start_graph_t;

struct dsp_ams_cmd_start_graph_t
{
  uint32_t handle;
  /**< Graph handle. */
};

/**
  Stops a graph on DSP AMS.

  @payload
  dsp_ams_cmd_stop_graph_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_STOP_GRAPH (0x0001336D)

typedef struct dsp_ams_cmd_stop_graph_t dsp_ams_cmd_stop_graph_t;

struct dsp_ams_cmd_stop_graph_t
{
  uint32_t handle;
  /**< Graph handle. */
};

/**
  Maps shared memory with DSP AMS.

  @payload
  dsp_ams_cmd_mem_map_t.

  @return
  DSP_AMS_CMDRSP_MEP_MAP
*/
#define DSP_AMS_CMD_MEM_MAP (0x0001336E)

/** Memory region. */
typedef struct dsp_ams_mem_region_t dsp_ams_mem_region_t;

struct dsp_ams_mem_region_t
{
  uint32_t addr_lsw;
  /**< Lower 32 bits of the memory address to map. */
  uint32_t addr_msw;
  /**< Higher 32 bits of the memory address to map. */
  uint32_t size_bytes;
  /**< Size of the memory in bytes. */
};

/** Command payload for DSP_AMS_CMD_MEM_MAP */
typedef struct dsp_ams_cmd_mem_map_t dsp_ams_cmd_mem_map_t;

struct dsp_ams_cmd_mem_map_t
{
  uint32_t num_regions;
  /**< Number of regions to map. */

  /** Followed by array below.
      The start address for the array must be 4-byte aligned.
      dsp_ams_mem_region_t regions[num_regions]; */
};

/**
  Response to the DSP_AMS_CMD_MEM_MAP command.

  @payload
  dsp_ams_cmdrsp_mem_map_t.

  @return
  None.
*/
#define DSP_AMS_CMDRSP_MEM_MAP (0x0001336F)

typedef struct dsp_ams_cmdrsp_mem_map_t dsp_ams_cmdrsp_mem_map_t;

struct dsp_ams_cmdrsp_mem_map_t
{
  uint32_t status;
  /**< Status of memory map command. */
  uint32_t mem_handle;
  /**< Memory map handle, valid when status is DSP_AMS_STATUS_OK. */
};

/**
  Un-maps memory with DSP AMS.

  @payload
  dsp_ams_cmd_mem_unmap_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_MEM_UNMAP (0x00013370)

typedef struct dsp_ams_cmd_mem_unmap_t dsp_ams_cmd_mem_unmap_t;

struct dsp_ams_cmd_mem_unmap_t
{
  uint32_t mem_handle;
  /**< Memory map handle. */
};

/**
  Sets a single module parameter on an AMS graph.

  @payload
  dsp_ams_cmd_set_param_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_SET_PARAM (0x00013371)

typedef struct dsp_ams_cmd_set_param_t dsp_ams_cmd_set_param_t;

struct dsp_ams_cmd_set_param_t
{
  uint32_t handle;
  /**< Graph handle. */
  uint32_t module_id;
  /**< Module ID: user assigned ID in dsp_ams_module_t
       Note this is not the CAPIv2 module ID. */
  uint32_t param_id;
  /**< Parameter ID. */
  uint32_t param_size;
  /**< Parameter data size in bytes. */
  uint32_t mem_map_handle;
  /**< Memory map handle if the parameter data is in shared memory.
       The shared memory contains uint8_t data[param_size].
       Set to 0 if the parameter is in-band. */
  uint32_t address_lsw;
  /**< Lower 32 bit of the parameter data address if data is in shared
       memory. */
  uint32_t address_msw;
  /**< Higher 32 bit of the parameter data address if data is in shared
       memory. */

  /** Followed by array below (in-band case only).
      The start address for the array must be 4-byte aligned.
      uint8_t data[param_size]; */
};

/**
  Gets a single module parameter from an AMS graph.

  @payload
  dsp_ams_cmd_get_param_t.

  @return
  DSP_AMS_CMDRSP_GET_PARAM
*/
#define DSP_AMS_CMD_GET_PARAM (0x00013372)

typedef struct dsp_ams_cmd_get_param_t dsp_ams_cmd_get_param_t;

struct dsp_ams_cmd_get_param_t
{
  uint32_t handle;
  /**< Graph handle. */
  uint32_t module_id;
  /**< Module ID: user assigned ID in dsp_ams_module_t
       Note this is not the CAPIv2 module ID. */
  uint32_t param_id;
  /**< Parameter ID. */
  uint32_t param_max_size;
  /**< Maximum size of the parameter data based on the module ID/parameter ID
       combination.
       For out-of-band mode, the shared memory provided must be at least this
       size. */
  uint32_t mem_map_handle;
  /**< Memory map handle if shared memory is provided. Parameter data
       (dsp_ams_param_data_t param_data) will be written into the memory.
       Set to 0 if the parameter is to be returned in-band. */
  uint32_t address_lsw;
  /**< Lower 32 bit of the memory provided. */
  uint32_t address_msw;
  /**< Higher 32 bit of the memory provided. */
};

/**
  Response to the DSP_AMS_CMD_GET_PARAM command.

  @payload
  dsp_ams_cmdrsp_get_param_t.

  @return
  None.
*/
#define DSP_AMS_CMDRSP_GET_PARAM (0x00013373)

/** Parameter data. */
typedef struct dsp_ams_param_data_t dsp_ams_param_data_t;
struct dsp_ams_param_data_t
{
  uint32_t module_id;
  /**< Module ID: user assigned ID in dsp_ams_module_t
       Note this is not the CAPIv2 module ID. */
  uint32_t param_id;
  /**< Parameter ID. */
  uint32_t param_size;
  /** Followed by array below.
      The start address for the array must be 4-byte aligned.
      uint8_t data[param_size]; */
};

/** Response payload for DSP_AMS_CMDRSP_GET_PARAM */
typedef struct dsp_ams_cmdrsp_get_param_t dsp_ams_cmdrsp_get_param_t;

struct dsp_ams_cmdrsp_get_param_t
{
  uint32_t status;
  /** If status is DSP_AMS_STATUS_OK, followed by param_data (in-band case
      only). The start address for the param_data must be 4-byte aligned.
      dsp_ams_param_data_t param_data; */
};

/**
  Provides the shared resource configurations to DSP AMS.

  @payload
  dsp_ams_cmd_shared_resource_config_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/

/** Macro for the LPAIF DMA data types. */
#define AMS_LPAIF_DEFAULT_DMA_TYPE 0x0 // Default DMA data type
#define AMS_LPAIF_HDMI_DMA_TYPE 0x1    // HDMI DMA data type
#define AMS_LPAIF_VA_CORE_DMA_TYPE 0x2 // VA Core DMA data type
#define AMS_LPAIF_AXI_DMA_TYPE 0x3     // AXI DMA data type
#define AMS_LPAIF_WSA_DMA_TYPE 0x4     // WSA DMA data type
#define AMS_LPAIF_RXTX_DMA_TYPE 0x5    // RXTX DMA data type

#define DSP_AMS_CMD_SHARED_RESOURCE_CONFIG (0x00013374)

typedef struct dsp_ams_cmd_shared_resource_config_t dsp_ams_cmd_shared_resource_config_t;

struct dsp_ams_cmd_shared_resource_config_t
{
  uint32_t lpm_start_addr_lsw;
  /**< Lower 32 bits of LPM start phyical address available for AMS usage. */
  uint32_t lpm_start_addr_msw;
  /**< Higher 32 bits of LPM start phyical address available for AMS
       usage. */
  uint32_t lpm_length;
  /**< LPM length in bytes available for AMS usage. */
  uint32_t num_dma_type;

  /** Followed by below payload for each dma_type*/

  /**< Specifies the DMA resource type.
   uint32_t dma_type;
   @values #AMS_LPAIF_DEFAULT_DMA_TYPE
               #AMS_LPAIF_HDMI_DMA_TYPE
               #AMS_LPAIF_VA_CORE_DMA_TYPE
               #AMS_LPAIF_AXI_DMA_TYPE
               #AMS_LPAIF_WSA_DMA_TYPE
               #AMS_LPAIF_RXTX_DMA_TYPE

   uint16_t num_rddma;
   Number of read DMA channels available for AMS usage.
   uint16_t num_wrdma;
   Number of write DMA channels available for AMS usage.

   Followed by arrays below.
       The start address for each of the arrays must be 4-byte aligned.
       uint32_t rddma_idx[num_rddma];
       uint32_t wrdma_idx[num_wrdma]; */
};

/**
  Release all the shared resource provided via the
  DSP_AMS_CMD_SHARED_RESOURCE_CONFIG command.

  @payload
  None.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_RELEASE_SHARED_RESOURCE (0x0001337D)

/**
  Sets a single prop on AMS service.

  @payload
  dsp_ams_cmd_set_prop_t.

  @return
  APRV2_IBASIC_RSP_RESULT (refer to 80-N1463-2,80-NA601-5).
*/
#define DSP_AMS_CMD_SET_PROP (0x000133BE)

/** AMS prop id for enabling/disbaling end point pcm logging. */
#define DSP_AMS_PROP_ID_END_POINT_PCM_LOGGING (0x000133BF)

/** AMS PCM data logging channel_mask for all channels. */
#define DSP_AMS_PCM_LOG_ALL_CHANNEL (0xFFFFFFFF)

/** AMS PCM data logging channel_mask for zero channels. */
#define DSP_AMS_PCM_LOG_NO_CHANNEL (0x0)

/* Payload of the DSP_AMS_PROP_ID_END_POINT_PCM_LOGGING prop data */
typedef struct dsp_ams_ep_pcm_logging_t dsp_ams_ep_pcm_logging_t;

struct dsp_ams_ep_pcm_logging_t
{
  uint16_t enable;
  /**< Specifies whether end+point PCM logging is enabled.

       @values
       - 0 -- Disabled
       - 1 -- Enabled @tablebulletend */

  uint16_t log_id;
  /**< Specifies the PCM logging tap point */

  uint32_t ch_mask;
  /**< Specifies the PCM logging channel mask */
};

/** AMS prop id for building timestamp message link. */
#define DSP_AMS_PROP_ID_BUILD_TIMESTAMP_MSG_LINK (0x000133C0)

/** module input port type. */
#define DSP_AMS_MODULE_INPUT_PORT (1)

/** module output port type. */
#define DSP_AMS_MODULE_OUTPUT_PORT (2)

typedef struct dsp_ams_msg_link_connection_t dsp_ams_msg_link_connection_t;

struct dsp_ams_msg_link_connection_t
{
  struct
  {
    uint32_t hw_interface_id;
    /**< TDM interface ID for exclusive end point only
         - DSP_AMS_HW_INTERFACE_TDM1
         - DSP_AMS_HW_INTERFACE_TDM2
         - DSP_AMS_HW_INTERFACE_TDM3
         - DSP_AMS_HW_INTERFACE_TDM4
         - DSP_AMS_HW_INTERFACE_TDM5 */
    uint32_t type;
    /**< Specifies whether endpoint is source or sink.
         - DSP_AMS_ENDPOINT_TYPE_SOURCE
         - DSP_AMS_ENDPOINT_TYPE_SINK */
  } source;

  struct
  {
    uint32_t id;
    /**< User-assigned unique identifier. */
    uint32_t module_id;
    /**< CAPIv2 module ID.*/
    uint16_t port_type;
    /**< Specifies whether port is input or output.
         - DSP_AMS_MODULE_INPUT_PORT
         - DSP_AMS_MODULE_OUTPUT_PORT */
    uint16_t port_index;
    /**< 0-based port index. */
  } destination;
};

/* Payload of the DSP_AMS_PROP_ID_BUILD_TIMESTAMP_MSG_LINK prop data */
typedef struct dsp_ams_build_msg_link_t dsp_ams_build_msg_link_t;

struct dsp_ams_build_msg_link_t
{
  uint32_t handle;
  /**< Graph handle */

  uint32_t num_link;

  /**< follow num_link of
       dsp_ams_msg_link_connection_t
       structures
  */
};

/** AMS prop id for destroying timestamp message link. */
#define DSP_AMS_PROP_ID_DESTROY_TIMESTAMP_MSG_LINK (0x000133C1)

/* Payload of the DSP_AMS_PROP_ID_DESTROY_TIMESTAMP_MSG_LINK prop data */
typedef struct dsp_ams_destroy_msg_link_t dsp_ams_destroy_msg_link_t;
struct dsp_ams_destroy_msg_link_t
{
  uint32_t handle;
  /**< Graph handle */
};

/** AMS prop id for multi-lane configuration. */
#define DSP_AMS_PROP_ID_TDM_LANE_CFG (0x000133C2)

typedef struct dsp_ams_tdm_lane_cfg_t dsp_ams_tdm_lane_cfg_t;

struct dsp_ams_tdm_lane_cfg_t
{
  uint32_t handle;
  /**< Graph handle */
  uint32_t hw_interface_id;
  /**< TDM interface ID.
       - DSP_AMS_HW_INTERFACE_TDM1
       - DSP_AMS_HW_INTERFACE_TDM2
       - DSP_AMS_HW_INTERFACE_TDM3
       - DSP_AMS_HW_INTERFACE_TDM4
       - DSP_AMS_HW_INTERFACE_TDM5 */
  uint32_t type;
  /**< Specifies whether endpoint is source or sink.
       - DSP_AMS_ENDPOINT_TYPE_SOURCE
       - DSP_AMS_ENDPOINT_TYPE_SINK */
  uint16_t lane_mask;
  /**< Specifies the lane mask: 1 to 2^N-1
      Position of the active lanes.
      Bits 0 to N-1 correspond to lanes 0 to N-1.
      1 to 2^N-1 When a bit is set,
      the corresponding lane is active.
      The number of active lanes can be inferred
      from the number of bits set in the mask.
      Where N, is the number of physical lanes
      supported by the hardware.
  */
  uint16_t reserved;
  /**< Client should set this field to zero. */
};

typedef struct dsp_ams_cmd_set_prop_t dsp_ams_cmd_set_prop_t;

struct dsp_ams_cmd_set_prop_t
{
  uint32_t prop_id;
  /**< AMS PROP ID.
       - DSP_AMS_PROP_ID_END_POINT_PCM_LOGGING
  */

  uint32_t prop_size;
  /**< Prop data size in bytes which is matched with above prop_id. */

  /** Followed by array below (in-band case only).
      The start address for the array must be 4-byte aligned.
      uint8_t data[prop_size]; */
};

#endif /* _DSP_AUDIO_MICRO_SERVICE_H_ */

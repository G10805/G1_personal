/*
 * Copyright (c) 2019,2022 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef AMS_H
#define AMS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct ams_graph_basic_params
{
  uint32_t processor_id;
  /**< processor ID:0 for modem DSP. */
  uint32_t sample_rate;
  /**< sample rate in Hz. */
  uint32_t block_size;
  /**< number of samples per processing block.*/
  uint32_t flags;
  /**< reserved */
} ams_graph_basic_params_t;

typedef enum
{
  AMS_HW_INTERFACE_TDM1 = 1,
  /**< TDM 1 HW interface ID. */
  AMS_HW_INTERFACE_TDM2,
  /**< TDM 2 HW interface ID. */
  AMS_HW_INTERFACE_TDM3,
  /**< TDM 3 HW interface ID. */
  AMS_HW_INTERFACE_TDM4,
  /**< TDM 4 HW interface ID. */
  AMS_HW_INTERFACE_TDM5
  /**< TDM 5 HW interface ID. */
} ams_hw_interface_id_t;

/** Linear PCM data*/
#define AMS_TDM_LINEAR_PCM_DATA (0)

/** Short (one-bit) synchronization mode. */
#define AMS_TDM_SHORT_SYNC_BIT_MODE (0)

/** Long synchronization mode. */
#define AMS_TDM_LONG_SYNC_MODE (1)

/** Short (one-slot) synchronization mode. */
#define AMS_TDM_SHORT_SYNC_SLOT_MODE (2)

/** Synchronization source is external. */
#define AMS_TDM_SYNC_SRC_EXTERNAL (0)

/** Synchronization source is internal. */
#define AMS_TDM_SYNC_SRC_INTERNAL (1)

/** Zero-bit clock cycle synchronization data delay. */
#define AMS_TDM_DATA_DELAY_0_BCLK_CYCLE (0)

/** One-bit clock cycle synchronization data delay. */
#define AMS_TDM_DATA_DELAY_1_BCLK_CYCLE (1)

/** Two-bit clock cycle synchronization data delay. */
#define AMS_TDM_DATA_DELAY_2_BCLK_CYCLE (2)

/** Disable sharing of the data-out signal. */
#define AMS_TDM_CTRL_DATA_OE_DISABLE (0)

/** Enable sharing of the data-out signal. */
#define AMS_TDM_CTRL_DATA_OE_ENABLE (1)

/** Normal synchronization. */
#define AMS_TDM_SYNC_NORMAL (0)

/** Invert the synchronization. */
#define AMS_TDM_SYNC_INVERT (1)

/** Exclusive endpoint - endpoint managed only by AMS. */
#define AMS_ENDPOINT_EXCLUSIVE (0)

/**
  Shared endpoint - endpoint shared between AMS and AVS framework, where the
  AVS framework outputs to this endpoint. The configuration of the HW interface
  is done by AVS framework.
*/
#define AMS_ENDPOINT_SHARED_WITH_ADSP_OUTPUT (1)

/**
  Shared endpoint - endpoint shared between AMS and AVS framework, where the
  AVS framework gets input from this endpoint. The configuration of the HW
  interface is done by AVS framework.
*/
#define AMS_ENDPOINT_SHARED_WITH_ADSP_INPUT (2)

typedef enum
{
  AMS_ENDPOINT_TYPE_SOURCE = 0,
  AMS_ENDPOINT_TYPE_SINK
} ams_endpoint_type_t;

typedef struct ams_endpoint
{
  uint32_t id;
  /**< User-assigned unique identifier. */
  ams_endpoint_type_t type;
  /**< Specifies whether endpoint is source or sink.
       - AMS_ENDPOINT_TYPE_SOURCE
       - AMS_ENDPOINT_TYPE_SINK */
  uint32_t channel_mask;
  /**< Channel enablement mask. */
  uint16_t channel_type[32];
  /**< Channel type as defined by 80-NF775-1. */
  uint32_t q_factor;
  /**< Number of fractional bits in sample. */
  uint32_t flags;
  /**< - AMS_ENDPOINT_EXCLUSIVE
       - AMS_ENDPOINT_SHARED */
  struct
  {
    ams_hw_interface_id_t hw_interface_id;
    /**< TDM interface ID.
         - AMS_HW_INTERFACE_TDM1
         - AMS_HW_INTERFACE_TDM2
         - AMS_HW_INTERFACE_TDM3
         - AMS_HW_INTERFACE_TDM4
         - AMS_HW_INTERFACE_TDM5 */
    uint8_t num_channels;
    /**< Total number of channels = number of active lanes * number of active slots. */
    uint8_t bit_width;
    /**< Bit width of the sample. */
    uint16_t data_format;
    /**< Data format supported by the TDM frame.
         - AMS_TDM_LINEAR_PCM_DATA */
    uint16_t sync_mode;
    /**< TDM Synchronization mode setting.
         - AMS_TDM_SHORT_SYNC_BIT_MODE
         - AMS_TDM_LONG_SYNC_MODE
         - AMS_TDM_SHORT_SYNC_SLOT_MODE */
    uint16_t sync_src;
    /**< Synchronization source.
         - AMS_TDM_SYNC_SRC_EXTERNAL
         - AMS_TDM_SYNC_SRC_INTERNAL */
    uint16_t nslots_per_frame;
    /**< Number of slots per frame. */
    uint16_t ctrl_data_out_enable;
    /**< Specifies whether the TDM hardware block shares the data-out
         signal to the driver with other masters.
         - AMS_TDM_CTRL_DATA_OE_DISABLE
         - AMS_TDM_CTRL_DATA_OE_ENABLE */
    uint16_t ctrl_invert_sync_pulse;
    /**< Specifies whether to invert synchronization.
         - AMS_TDM_SYNC_NORMAL
         - AMS_TDM_SYNC_INVERT */
    uint16_t ctrl_sync_data_delay;
    /**< Number of bit-clock cycles for delaying the data for
         synchronization.
         - AMS_TDM_DATA_DELAY_0_BCLK_CYCLE
         - AMS_TDM_DATA_DELAY_1_BCLK_CYCLE
         - AMS_TDM_DATA_DELAY_2_BCLK_CYCLE */
    uint16_t slot_width;
    /**< Width of the slot in a TDM frame: 16, 24, 32. */
    uint32_t slot_mask;
    /**< Position of the active slots: 1 to 2^32-1.
         Bits 0 to 31 correspond to slots 0 to 31. When a bit is set, the
         corresponding slot is active. The number of active slots can be
         inferred from the number of bits set in the mask.
         32 individual bits can be enabled. */
  } tdm_params;
} ams_endpoint_t;

/** Module's processing is done in DMA IST context. */
#define AMS_INLINE_PROCESSING_MODE (0)

/** Module's processing is done in worker thread context. */
#define AMS_DEFERRED_PROCESSING_MODE (1)

typedef struct ams_module
{
  uint32_t id;
  /**< User-assigned unique identifier. */
  uint32_t flags;
  /**< - AMS_INLINE_PROCESSING_MODE
       - AMS_DEFERRED_PROCESSING_MODE */
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
} ams_module_t;

typedef enum
{
  AMS_CONNECTION_ELEMENT_TYPE_MODULE = 0,
  AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT
} ams_connection_element_type_t;

/**
  Connection between a source and a destination element.
  An element may be a module or endpoint.
*/
typedef struct ams_connection
{
  uint8_t num_channels;
  uint8_t bit_width;

  // Source info
  struct
  {
    ams_connection_element_type_t type;
    /**< Specifies whether the source element is a module or endpoint.
         - AMS_CONNECTION_ELEMENT_TYPE_MODULE
         - AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT */
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
    ams_connection_element_type_t type;
    /**< Specifies whether the destination element is a module or endpoint.
         - AMS_CONNECTION_ELEMENT_TYPE_MODULE
         - AMS_CONNECTION_ELEMENT_TYPE_ENDPOINT */
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

} ams_connection_t;

typedef enum
{
  DSP_AMS_GRAPH_INIT = 0,
  DSP_AMS_GRAPH_OPENED = 1,
  DSP_AMS_GRAPH_STARTED
} ams_graph_state_t;

typedef enum
{
  AMS_STATUS_SUCCESS = 0,
  AMS_STATUS_GENERAL_ERROR = 1,
  AMS_STATUS_INPUT_ERROR,
  AMS_STATUS_STATE_ERROR,
  AMS_STATUS_GRAPH_TOPO_ERROR
} ams_status_t;

typedef void *ams_graph_handle_t;

typedef struct ams_session *ams_session_t;

/**
  Init ams session.

  @input/output
  Pointer to ams sesion handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_init(ams_session_t *amss);

/**
  De-init ams session.

  @input/output
  Pointer to ams sesion handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_deinit(ams_session_t *amss);

/**
  Create graph.

  @input
    - ams sesion handle
    - basic param pointer
  @output
   - graph handle initialized

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_create_graph(
    ams_session_t amss,
    ams_graph_basic_params_t *p, // [in]
    ams_graph_handle_t *gh);     // [out]

/**
  Add endpoint.

  @input
    - ams sesion handle
    - graph handle
    - endpoint pointer
  @output
   - graph handle initialized

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_graph_add_endpont(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_endpoint_t *e);    // [in]

/**
  Add module.

  @input
    - ams sesion handle
    - graph handle
    - module pointer
  @output
   - graph handle initialized

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_graph_add_module(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_module_t *m);      // [in]

/**
  Add connection.

  @input
    - ams sesion handle
    - graph handle
    - connection pointer
  @output
   - graph handle initialized

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_graph_add_connection(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    ams_connection_t *c);  // [in]

/**
  Destroy graph.

  @input
    - ams sesion handle
  @input/output
   - graph handle will be NULL-ed

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_destroy_graph(
    ams_session_t amss,
    ams_graph_handle_t *gh); // [in]

/**
  Open graph.

  @input
    - ams sesion handle
    - graph handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_open_graph(
    ams_session_t amss,     // [in]
    ams_graph_handle_t gh); // [out]

/**
  Start graph.

  @input
    - ams sesion handle
    - graph handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_start_graph(
    ams_session_t amss,     // [in]
    ams_graph_handle_t gh); // [in]

/**
  Stop graph.

  @input
    - ams sesion handle
    - graph handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_stop_graph(
    ams_session_t amss,     // [in]
    ams_graph_handle_t gh); // [in]

/**
  Close graph.

  @input
    - ams sesion handle
    - graph handle

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_close_graph(
    ams_session_t amss,
    ams_graph_handle_t gh); // [in]

/**
  Set graph parameter.

  @input
    - ams sesion handle
    - graph handle
    - module id
    - paramter id
    - parameter buffer size
  @output
    - parameter buffer pointer

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_set_param(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    uint32_t module_id,    // [in]
    uint32_t param_id,     // [in]
    uint32_t param_size,   // [in]
    void *data);           // [in]

/**
  Get graph parameter.

  @input
    - ams sesion handle
    - graph handle
    - module id
    - paramter id
  @input/output
    - pointer to parameter buffer size
    - parameter buffer pointer

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_get_param(
    ams_session_t amss,
    ams_graph_handle_t gh, // [in]
    uint32_t module_id,    // [in]
    uint32_t param_id,     // [in]
    uint32_t *param_size,  // [in/out]
    void *data);           // [out]

typedef enum
{
  AMS_EVENT_SSR_STARTED = 1,
  AMS_EVENT_SSR_COMPLETED = 2,
} ams_event_id_t;

typedef void (*ams_event_handler_t)(ams_event_id_t e, void *data);

/**
  Register callback.

  @input
    - ams sesion handle
    - handler functoin
    - pointer to data for handler

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_register_callback(
    ams_session_t amss,
    ams_event_handler_t func,
    void *data);

/**
  De-register callback.

  @input
    - ams sesion handle
    - handler functoin

  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_deregister_callback(
    ams_session_t amss,
    ams_event_handler_t h);

enum AMS_GRAPH_PROPERTY_ID
{ /** AMS Graph property ID
    - AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR */
  AMS_GRAPH_PROPERTY_ID_FIRST = 1,
  AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR = AMS_GRAPH_PROPERTY_ID_FIRST,
  AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_BUILD,
  AMS_GRAPH_PROPERTY_ID_TIMESTAMP_MESSAGE_LINK_DESTROY,
  AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG,
  AMS_GRAPH_PROPERTY_ID_LAST = AMS_GRAPH_PROPERTY_ID_EXCLV_EP_TDM_LANE_CFG,
};

/** module input port type. */
#define AMS_MODULE_INPUT_PORT (1)

/** module output port type. */
#define AMS_MODULE_OUTPUT_PORT (2)

typedef struct ams_msg_link_connection
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
} ams_msg_link_connection_t;

typedef struct ams_graph_property
{
  enum AMS_GRAPH_PROPERTY_ID prop_id;
  /** AMS GRAPH Property ID
  - AMS_GRAPH_PROPERTY_ID_EXCLV_EP_CLK_ATTR
  */
  uint32_t appy_after_start;
  /** Apply property flag
      - 0: Apply property before graph is started
      - 1: Apply property after graph is started
      - 2: Apply property before graph is stopped
      - 3: Apply property after graph is stopped
  */
  union
  {
    struct
    {
      int16_t exclv_ep_id;
      /** Exclusive endpoint ID */
      int16_t clk_invert;
      /** Clk invert property of the exclusinve endpoint */
    } exclv_ep_clk_attr;
    /** Exclusive endpoint clk attribute
      It will be applied before graph is started
    */

    struct
    {
      uint32_t num_link;

      ams_msg_link_connection_t *msg_link_ptr;

      /**< point to a piece of memory which contains num_link of
      ams_msg_link_connection_t
      structures */

    } ams_build_msg_link;
    /** for message link build
        It will be applied before graph is started
      */

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
      uint32_t lane_mask;
      /**< Specifies the lane mask: 1 to 2^N-1
        Position of the active lanes.
        Bits 0 to N-1 correspond to lanes 0 to N-1.
        1 to 2^N-1 When a bit is set,
        the corresponding lane is active.
        The number of active lanes can be inferred
        from the number of bits set in the mask.
        Where N, is the number of physical lanes
        supported by the hardware.*/
    } exclv_ep_tdm_lane_cfg;
    /** Exclusive endpoint tdm lane cfg
      It will be applied before graph is started
    */
  } u;
} ams_graph_property_t;
/**
  Set graph property.

  @input
    - ams sesion handle
    - graph handle
    - graph property
  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_graph_set_property(
    ams_session_t amss,
    ams_graph_handle_t gh,
    ams_graph_property_t *prop);

typedef enum
{
  AMS_SESSION_NEW = 0,
  AMS_SESSION_RESTARTED
} ams_session_type_t;

/**
  AMS restart session

  @input
  @output
    - ams sesion handle
    - ams session type
  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
  If restart not possible ams_init will be called and new session will
  be returned. In this case session type will set to AMS_SESSION_NEW.
  ams_restart must be followed by ams_register callback!
*/
ams_status_t ams_restart(
    ams_session_t *amss,
    ams_session_type_t *session_type);

/**
  AMS query graph list

  @input
    - ams sesion handle
  @output
    - graph list
    - graph list elements number
  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
*/
ams_status_t ams_query_graph_list(
    ams_session_t amss,
    ams_graph_handle_t *gh,
    uint32_t max_num_gh,
    uint32_t *num_gh);

typedef enum
{
  AMS_GRAPH_OPENED = 0,
  AMS_GRAPH_STARTED
} ams_session_graph_state_t;
#define AMS_GRAPH_INFO_EL_MAX 8
typedef struct
{
  ams_session_graph_state_t state;
  uint32_t modules_num;
  uint32_t ep_num;
  uint32_t modules_ids[AMS_GRAPH_INFO_EL_MAX];
  uint32_t ep_ids[AMS_GRAPH_INFO_EL_MAX];
} ams_graph_info_t;

/**
  AMS query graph info

  @input
    - ams sesion handle
    - graph handle
  @output
    - graph info
  @return
  Error code( AMS_STATUS_SUCCESS = 0 ).
  User is responsible to free graph_info!
*/
ams_status_t ams_query_graph_info(
    ams_session_t amss,
    ams_graph_handle_t gh,
    ams_graph_info_t *graph_info);

#endif

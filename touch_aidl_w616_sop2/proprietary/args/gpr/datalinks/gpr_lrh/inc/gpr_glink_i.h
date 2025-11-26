#ifndef _GPR_GLINK_I_H_
#define _GPR_GLINK_I_H_
/*=============================================================================
Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/******************************************************************************
 * Includes                                                                   *
 *****************************************************************************/
#include "gpr_comdef.h"
#include "glink.h"
#include "gpr_ids_domains.h"
/******************************************************************************
 * Defines                                                                    *
 *****************************************************************************/

#define GPR_GLINK_MAX_PORT_NAME_SIZE (16)
/**< Max port name size allowed. */

#define GPR_GLINK_MAX_DOMAIN_NAME_SIZE (16)

#define GPR_GLINK_APPS_REMOTE_SS_ID (0x00011318)
/**< Apps remote subsystem ID. */

#define GPR_GLINK_MODEM_REMOTE_SS_ID (0x00011319)
/**< Modem remote subsystem ID. */

#define GPR_GLINK_QDSP_REMOTE_SS_ID (0x0001131A)
/**< QDSP remote subsystem ID. */

/*TODO: Assign guids */
#define GPR_GLINK_DISCONNECTED            ( 0 )
  /**< GPR GLink connection is disconnected. */

#define GPR_GLINK_CONNECTED               ( 1 )
  /**< GPR GLink connection is fully connected with remote proc. */

#define GPR_GLINK_REMOTE_DISCONNECTED     ( 2 )
  /**< GPR GLink connection is disconnected by remote proc. */

#define GPR_GLINK_CONNECTING              ( 3 )
  /**< GPR GLink connection is connecting. Wainting for ack from glink. */

#define GPR_GLINK_DISCONNECTING           ( 4 )
  /**< GPR GLink connection is disconnecting. Wainting for ack from glink. */

/**< Max number of GPR GLINK ports is total number of domains */
#define GPR_GLINK_MAX_PORTS (GPR_PL_NUM_TOTAL_DOMAINS_V)

/******************************************************************************
 * Type Definitions                                                           *
 *****************************************************************************/
 /**< Client communicates with GPR GLINK using a handle of this type. */
typedef uint32_t gpr_glink_port_handle_t;


void gpr_ipc_glink_link_state_cb (glink_link_info_type *link_info, void * priv);


/**<
 * The rx callback function type definition.
 *
 * The client receives this callback when there is data available for read.
 * The rx_read_buf contains the data to be read. The rx_read_buf is valid
 * outside of this callback function as well. The client is expected to call
 * gpr_glink_rx_done() when done with using the buffer.
 *
 * The callback is processed in ISR context. Client shall perform minimal
 * amount of processing in the callback and absolutely no call blocking
 * routines in the callback.
 *
 * \param[in] cb_data The client supplied data pointer when opening a port.
 * \param[in] rx_read_buf The buffer to be read.
 * \param[in] size The size of the buffer in bytes.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
typedef uint32_t ( *gpr_glink_rx_cb_t )(void *cb_data, void *rx_read_buf, uint32_t size );


/**<
 * The tx done callback function type definition.
 *
 * The client receives this callback when GPR GLINK is done with the tx
 * buffer.
 *
 * \param[in] cb_data The client supplied data pointer when opening a port.
 * \param[in] rx_write_buf The buffer supplied during write.
 * \param[in] size The size of the buffer in bytes.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
typedef uint32_t ( *gpr_glink_tx_done_cb_t ) (void *cb_data, void *rx_write_buf, uint32_t size);


/**<
 * GPR Glink state notification callback function type definition.
 *
 * The client receives the notification when GPR GLINK connection state has
 * changed.
 *
 * \param[in] cb_data The client supplied data pointer when opening a port.
 * \param[in] event The connection state change event.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
typedef uint32_t ( *gpr_glink_state_cb_t ) (void *cb_data, uint32_t event );


/* GPR to IPC datalink callback function definition */
typedef uint32_t (*gpr_to_ipc_cb_fn_t ) (uint32_t domain_id, void* buf, uint32_t length);


/* GPR GLINK port info struct*/
typedef struct gpr_glink_port_info_t gpr_glink_port_info_t;

struct gpr_glink_port_info_t
{
   uint32_t domain_id_1;
   char_t   domain_name_1[GPR_GLINK_MAX_DOMAIN_NAME_SIZE];
   uint32_t domain_id_2;
   char_t   domain_name_2[GPR_GLINK_MAX_DOMAIN_NAME_SIZE];
   char_t   port_name[GPR_GLINK_MAX_PORT_NAME_SIZE];
};

typedef struct gpr_glink_port_data_info
{
    gpr_glink_port_info_t *glink_port_data;
    uint32_t               gpr_glink_port_pairs;
}gpr_glink_port_data_info_t;

/** Data receive notification callback type*/
typedef uint32_t (*gpr_glink_receive_cb)(void *                  ptr,        /* pointer to the received buffer */
                                         uint32_t                length);

/** Data send done notification callback type*/
typedef uint32_t (*gpr_glink_send_done_cb)(void *                  ptr,        /* pointer to the received buffer */
                                           uint32_t                length);

typedef struct gpr_to_glink_cb_fn_t
{
  gpr_glink_receive_cb   rx_cb;      /* Receive Data CB */
  gpr_glink_send_done_cb tx_done_cb; /* Send done Data CB */
} gpr_to_glink_cb_fn_t;


/******************************************************************************
 * Core Routines                                                              *
 *****************************************************************************/
/**
 * Initializes the GPR GLINK module.
 *
 * This function must be called before any other GPR GLINK functions are
 * called.
 * This function must only be called once.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_init(uint32_t src_domain_id, uint32_t dest_domain_id);

/**
 * Deinitializes the GPR GLINK module.
 *
 * After a call to this function, none of the other GPR GLINK functions (except
 * gpr_glink_init) can be called.
 * This function must only be called once.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_deinit (uint32_t domain_id);

/**
 * Opens the port name port_name with the remote processor specified by
 * remote_ssid.
 *
 * \param[in] port_name The name for the port. The max size of the name allowed
 *                      is defined by #GPR_GLINK_MAX_CHANNEL_NAME_SIZE.
 *
 * \param[in] port_name_size The size of the port name including the NULL
 *                           termination character.
 *
 * \param[in] remote_ssid The remote subsystem id identifying remote
 *                        processors to which this channel communicates.
 *                        Supported values:
 *                        #GPR_GLINK_APPS_REMOTE_SS_ID
 *                        #GPR_GLINK_MODEM_REMOTE_SS_ID
 *                        #GPR_GLINK_QDSP_REMOTE_SS_ID
 *
 * \param[in] rx_cb The rx callback function defined by #gpr_glink_rx_cb_t. The
 *                  client receives this callback when there is data available
 *                  for read.
 *
 * \param[in] rx_cb_data The client data pointer for the rx callback.
 *
 * \param[in] tx_done_cb The tx done callback function defined by
 *                       #gpr_glink_tx_done_cb_t. The client receives this
 *                       callback when GPR GLINK is done with a tx buffer.
 *
 * \param[in] tx_done_cb_data The client data pointer for tx_done callback.
 *
 * \param[in] num_intents The number of intents to be queued for this channel.
 *
 * \param[in] intent_size Size of each intent to be queued.
 *
 * \param[in] qos_latency_us QoS latency requirement specified in microseconds
 *                           for given packet size thru qos_pkt_size.
 *
 * \param[in] qos_pkt_size Packet size for which qos_latency_us is specified.
 *
 * \param[out] port_handle Positive GPR GLINK port handle on success. NULL on
 *                         failure.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_open (char_t *port_name,
                                      uint32_t port_name_size,
                                      gpr_glink_port_handle_t port_handle,
                                      char_t *remote_ss);

/**
 * Close a port specified by the port_handle.
 *
 * \param[in] port_handle The handle of the port to be closed.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_close (uint32_t domain_id );

/**
 * Send data to the specified GPR GLINK port.
 *
 * \param[in] port_handle The handle of the port to write data.
 * \param[in] buf The data to be written.
 * \param[in] size The size of the data in bytes.
 *
 * \return AR_EOK on success, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_tx (uint32_t domain_id, void *buf, uint32_t size);

/**
 * Indicate GPR GLINK that a previously given Rx buffer is done.
 *
 * \param[in] port_handle The handle of the port for which rx buffer is done.
 * \param[in] buf The rx buffer pointer for which rx is done.
 *
 * \return AR_EOK on succes, GPR error code on failure.
 */
GPR_INTERNAL uint32_t gpr_glink_rx_done (uint32_t domain_id, void *buf);

void get_glink_port_data_info(gpr_glink_port_data_info_t *port_data_info);

#endif /* _GPR_GLINK_I_H_ */

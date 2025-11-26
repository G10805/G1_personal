/*=============================================================================
Copyright (c) 2018-2021, 2023 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

/******************************************************************************
 * Includes                                                                   *
 *****************************************************************************/
#include "ar_osal_error.h"
#include "ar_osal_string.h"
#include "ar_osal_mem_op.h"
#include "gpr_glink.h"
#include "gpr_glink_i.h"
#include "gpr_api_inline.h"
#include "ar_msg.h"

#define GPR_GLINK_INTENTS_SUPPORT

#if 0
//GPR_GLINK_QOS_SUPPORT needs to be selectively enabled only for non-audioss
//platforms which are non-Integrity platforms
#if !defined(audioss_proc) && !defined(__INTEGRITY)
#define GPR_GLINK_QOS_SUPPORT
#endif

#ifdef GPR_GLINK_QOS_SUPPORT
#include "glink_qos.h"
#define GPR_IPC_QOS_LATENCY_US (550)
#define GPR_IPC_QOS_PKT_SIZE (544)
#endif
#endif
/******************************************************************************
 * Defines                                                                    *
 *****************************************************************************/
#define GPR_GLINK_PORT_HANDLE_BASE (1)
/**< Used to make sure port handle are positive ( starting from 1 ). */

#define GPR_GLINK_128_BYTES_BUFFER (128)
#define GPR_GLINK_512_BYTES_BUFFER (512)
#define GPR_GLINK_1024_BYTES_BUFFER (1024)
#define GPR_GLINK_4096_BYTES_BUFFER (4096)

#define GPR_GLINK_NUM_OF_128_BYTES_INTENTS (64)
#define GPR_GLINK_NUM_OF_512_BYTES_INTENTS (32)
#define GPR_GLINK_NUM_OF_1024_BYTES_INTENTS (16)
#define GPR_GLINK_NUM_OF_4096_BYTES_INTENTS (04)
/******************************************************************************
 * Type Definitions                                                           *
 *****************************************************************************/
typedef uint32_t gpr_glink_state_t;
typedef struct gpr_glink_port_s
{
   uint32_t          domain_id;
   gpr_glink_state_t state;
   glink_handle_type glink_handle;
   char_t            port_name[GPR_GLINK_MAX_PORT_NAME_SIZE];
} gpr_glink_port_t;

/* glink ipc datalink function table*/
static ipc_to_gpr_vtbl_t glink_vtbl = {
   gpr_glink_tx,
   gpr_glink_rx_done,
};

static bool_t first_dl_init_done = FALSE;
/******************************************************************************
 * Variables                                                                  *
 *****************************************************************************/
/* glink port pool for all domain ids, accessed based on domain id being the port index*/
gpr_glink_port_t gpr_glink_port_pool[GPR_GLINK_MAX_PORTS];

gpr_glink_port_data_info_t port_data_info;

/* Global struct to store GPR callback functions */
gpr_to_glink_cb_fn_t gpr_cb_fn_tbl;

/******************************************************************************
 * Core Routine Implementations                                               *
 *****************************************************************************/

GPR_INTERNAL uint32_t ipc_dl_glink_init(uint32_t                 src_domain_id,
                                        uint32_t                 dest_domain_id,
                                        const gpr_to_ipc_vtbl_t *p_gpr_to_ipc_vtbl,
                                        ipc_to_gpr_vtbl_t **     pp_ipc_to_gpr_vtbl)
{

   if (!first_dl_init_done)
   {
      // Save gpr cb functions in global struct
      gpr_cb_fn_tbl.rx_cb      = p_gpr_to_ipc_vtbl->receive;
      gpr_cb_fn_tbl.tx_done_cb = p_gpr_to_ipc_vtbl->send_done;

      // Init gpr_glink_port_pool state
      for (uint32_t index = 0; index < GPR_GLINK_MAX_PORTS; index++)
      {
         gpr_glink_port_pool[index].state     = GPR_GLINK_DISCONNECTED;
         gpr_glink_port_pool[index].domain_id = index;
      }
      first_dl_init_done = TRUE;
   }

   get_glink_port_data_info(&port_data_info);
   gpr_glink_init(src_domain_id, dest_domain_id);
   *pp_ipc_to_gpr_vtbl = &glink_vtbl;

   return AR_EOK;
}

GPR_INTERNAL uint32_t ipc_dl_glink_deinit(uint32_t src_domain_id, uint32_t dest_domain_id)
{
   uint32_t rc = AR_EOK;
   rc          = gpr_glink_close(dest_domain_id);
   return rc;
}

/** Data receive notification callback */
static void gpr_glink_rx_cb(glink_handle_type handle,   /* handle for the glink channel */
                            const void *      priv,     /* priv client data passed in glink_open */
                            const void *      pkt_priv, /* private client data assiciated with the
                                                           rx intent that client queued earlier */
                            const void *ptr,            /* pointer to the received buffer */
                            size_t      size,           /* size of the packet */
                            size_t      intent_used     /* size of the intent used for this packet */
                            )
{
   uint32_t          rc;
   gpr_glink_port_t *port = (gpr_glink_port_t *)priv;
   glink_err_type    glink_rc;

   if ((NULL == ptr) || (NULL == port))
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_rx_cb(): Rcvd NULL buffer ptr");
      return;
   }

#ifdef GPR_DEBUG_MSG
   gpr_packet_t *packet = (gpr_packet_t *)ptr;

   AR_MSG(DBG_HIGH_PRIO,
            "gpr_glink_rx() dst port %lx, src port %lx, opcode 0x%lx, token %lx ",
            packet->dst_port,
            packet->src_port,
            packet->opcode,
            packet->token);
#endif

   rc = gpr_cb_fn_tbl.rx_cb((void *)ptr, size);
   if (rc)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_rx_cb(): Client cb returned error %lu", rc);
      /* Failed to deliver the packet. Release the packet */
      if (GLINK_STATUS_SUCCESS != (glink_rc = glink_rx_done(port->glink_handle, ptr, TRUE)))
      {
         AR_MSG(DBG_ERROR_PRIO, "glink_rx_done() failed with rc = %d", glink_rc);
      }
   }
   return;
}

static void gpr_glink_tx_abort(glink_handle_type glink_handle, const void *priv, const void *pkt_priv)
{
   gpr_glink_port_t *port = (gpr_glink_port_t *)priv;
   gpr_packet_t *    packet;

   if ((NULL == pkt_priv) || (NULL == port))
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_tx_abort(): Rcvd NULL buffer ptr or null port");
      return;
   }

   AR_MSG(DBG_HIGH_PRIO, "Packet send failed & rcved tx_abort for : 0%lx", port->domain_id);

   packet = (gpr_packet_t *)pkt_priv;

   (void)__gpr_cmd_end_command(packet, AR_EFAILED);
   return;
}

/** Data transmit notification callback */
static void gpr_glink_tx_done_cb(glink_handle_type handle,   /* handle for the glink channel */
                                 const void *      priv,     /* priv client data passed in glink_open */
                                 const void *      pkt_priv, /* private client data assiciated with the
                                                                tx pkt that client queued earlier */
                                 const void *ptr,            /* pointer to the transmitted buffer */
                                 size_t      size            /* size of the packet */
                                 )
{
   uint32_t          rc;
   gpr_glink_port_t *port = (gpr_glink_port_t *)priv;

   if ((NULL == ptr) || (NULL == port))
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_tx_done_cb(): Rcvd NULL buffer ptr");
      return;
   }

#ifdef GPR_DEBUG_MSG
gpr_packet_t *packet = (gpr_packet_t *)ptr;

   AR_MSG(DBG_HIGH_PRIO,
            "gpr_glink_tx_done() dst port %lx, src port %lx, opcode 0x%lx, token %lx ",
            packet->dst_port,
            packet->src_port,
            packet->opcode,
            packet->token);
#endif

   rc = gpr_cb_fn_tbl.tx_done_cb((void *)ptr, size);
   if (rc)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_tx_done_cb(): Client cb returned error %lu", rc);
   }
   return;
}

GPR_INTERNAL glink_err_type gpr_glink_queue_rx_intents(uint32_t          intent_size,
                                                       uint32_t          num_of_intents,
                                                       glink_handle_type glink_handle)
{
   uint32_t       index;
   glink_err_type glink_rc;

   /* Queue rx intents */
   for (index = 0; index < num_of_intents; index++)
   {
      if (GLINK_STATUS_SUCCESS != (glink_rc = glink_queue_rx_intent(glink_handle, NULL, intent_size)))
      {
         AR_MSG(DBG_ERROR_PRIO, "glink_queue_rx_intent() failed for size = %lu @ count = %lu.", intent_size, index + 1);
         return glink_rc;
      }
   }
   return GLINK_STATUS_SUCCESS;
}

/** GLink channel state change notification callback */
static void gpr_glink_state_cb(glink_handle_type        handle, /* handle for the glink channel */
                               const void *             priv,   /* priv client data passed in glink_open */
                               glink_channel_event_type event   /* Notification event */
                               )
{
   gpr_glink_port_t *port = (gpr_glink_port_t *)priv;
   glink_err_type    glink_rc;
   glink_handle_type glink_handle;
   uint32_t          rc             = AR_EOK;
   uint32_t          dest_domain_id = 0;
   char_t *          port_name;
   bool_t            found_entry = FALSE;

   if (NULL == port)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_state_cb(): Rcvd NULL priv pointer");
      return;
   }

   dest_domain_id = port->domain_id;
   port_name      = port->port_name;

   AR_MSG(DBG_HIGH_PRIO, "gpr_glink_state_cb: dest_domain_id %lu, event = 0x%x  port_name:", dest_domain_id, event);
   uint32_t c = 0;
   while (port_name[c] != '\0')
   {
      AR_MSG(DBG_HIGH_PRIO, "%d", port_name[c]);
      c++;
   }

   glink_handle = port->glink_handle;
   switch (event)
   {
      case GLINK_CONNECTED:
      {
         port->state = GPR_GLINK_CONNECTED;
         AR_MSG(DBG_ERROR_PRIO, "GPR Glink Connected to Remote Client, ghandle: %lu", dest_domain_id);

#ifdef GPR_GLINK_INTENTS_SUPPORT
         glink_rc =
            gpr_glink_queue_rx_intents(GPR_GLINK_128_BYTES_BUFFER, GPR_GLINK_NUM_OF_128_BYTES_INTENTS, glink_handle);
         glink_rc |=
            gpr_glink_queue_rx_intents(GPR_GLINK_512_BYTES_BUFFER, GPR_GLINK_NUM_OF_512_BYTES_INTENTS, glink_handle);
         glink_rc |=
            gpr_glink_queue_rx_intents(GPR_GLINK_1024_BYTES_BUFFER, GPR_GLINK_NUM_OF_1024_BYTES_INTENTS, glink_handle);
         glink_rc |=
            gpr_glink_queue_rx_intents(GPR_GLINK_4096_BYTES_BUFFER, GPR_GLINK_NUM_OF_4096_BYTES_INTENTS, glink_handle);
         if (glink_rc != GLINK_STATUS_SUCCESS)
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "GPR Glink failed to queue Rx intents, closing the port, handle with dest domain id: %lu",
                   dest_domain_id);
            (void)glink_close(glink_handle);
            port->state = GPR_GLINK_DISCONNECTING;
            break;
         }
#endif // GPR_GLINK_INTENTS_SUPPORT

#ifdef GPR_GLINK_QOS_SUPPORT
         /* Set QoS Latency requirements */
         if (GLINK_STATUS_SUCCESS !=
             (glink_rc = glink_qos_latency(port->glink_handle, GPR_IPC_QOS_LATENCY_US, GPR_IPC_QOS_PKT_SIZE)))
         {
            AR_MSG(DBG_ERROR_PRIO,
                   "GPR Glink failed to set QOS, dest domain id: %lu, rc = %d",
                   dest_domain_id,
                   glink_rc);
         }
#endif // GPR_GLINK_QOS_SUPPORT
         break;
      }
      case GLINK_REMOTE_DISCONNECTED:
      {
         port->state = GPR_GLINK_REMOTE_DISCONNECTED;
         // When the remote has disconnected, the QDSP side is expected to call glink_close (here via
         // gpr_glink_close).
         AR_MSG(DBG_HIGH_PRIO,
                "GPR Remote Glink Client Disconnected, Calling gpr_glink_close, domain id: %lu",
                dest_domain_id);
         (void)gpr_glink_close(dest_domain_id);
         break;
      }
      case GLINK_LOCAL_DISCONNECTED:
      {
         port->state = GPR_GLINK_DISCONNECTED;
         AR_MSG(DBG_HIGH_PRIO, "GPR Local Glink Client Disconnected, dest domain id: %lu", dest_domain_id);
         // Re-open the glink channel.

         for (uint32_t i = 0; i < port_data_info.gpr_glink_port_pairs; i++)
         {
            if (ar_strcmp(port_data_info.glink_port_data[i].port_name, port_name, sizeof(port_data_info.glink_port_data[i].port_name)) == 0)
            {
               char_t *remote_ss =
                  ((dest_domain_id == port_data_info.glink_port_data[i].domain_id_1) ? port_data_info.glink_port_data[i].domain_name_1
                                                                      : port_data_info.glink_port_data[i].domain_name_2);
               // Reopen the glink channel
               AR_MSG(DBG_HIGH_PRIO, "Glink entry found with remote ss:");

               c = 0;
               while (remote_ss[c] != '\0')
               {
                  AR_MSG(DBG_HIGH_PRIO, "%d", remote_ss[c]);
                  c++;
               }

               rc = gpr_glink_open(port_data_info.glink_port_data[i].port_name,
                                   sizeof(port_data_info.glink_port_data[i].port_name),
                                   dest_domain_id,
                                   remote_ss);
               if (rc != AR_EOK)
               {
                  AR_MSG(DBG_ERROR_PRIO,
                         "Glink open failed for the domain_id:%lu with status : %lu",
                         dest_domain_id,
                         rc);
                  rc |= AR_EFAILED;
               }
               found_entry = TRUE;
               break;
            }
         }

         if (!found_entry)
         {
            AR_MSG(DBG_ERROR_PRIO, "gpr_glink_init: Cannot find port name entry %s", port_name);
            rc |= AR_EFAILED;
         }
         break;
      }
      default:
      {
         break;
      }
   }

   return;
}

void gpr_ipc_glink_link_state_cb(glink_link_info_type *link_info, void *priv)
{
   uint32_t          rc          = AR_EOK;
   gpr_glink_port_t *port        = (gpr_glink_port_t *)priv;
   bool_t            found_entry = FALSE;

   if (link_info->link_state != GLINK_LINK_STATE_UP)
   {
      AR_MSG(DBG_HIGH_PRIO, "gpr_ipc_glink_link_state_cb(): link_info->link_state != GLINK_LINK_STATE_UP ");
      return;
   }

   if (NULL == port)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "GPR Glink gpr_ipc_glink_link_state_cb: Port info ptr is NULL, cannot open glink channel for remote ss %s",
             link_info->remote_ss);
      return;
   }

   AR_MSG(DBG_HIGH_PRIO, "GPR Glink gpr_ipc_glink_link_state_cb: dest_domain_id %lu remote ss:", port->domain_id);

   uint32_t c = 0;
   while (link_info->remote_ss[c] != '\0')
   {
      AR_MSG(DBG_HIGH_PRIO, "%d", link_info->remote_ss[c]);
      c++;
   }

   AR_MSG(DBG_HIGH_PRIO, "GPR Glink gpr_ipc_glink_link_state_cb: port_name: ");

   c = 0;
   while (port->port_name[c] != '\0')
   {
      AR_MSG(DBG_HIGH_PRIO, "%d", port->port_name[c]);
      c++;
   }

   // Check to make sure entry is there
   for (uint32_t i = 0; i < port_data_info.gpr_glink_port_pairs; i++)
   {
      // Every port name will have only one entry so directly search using it
      if (ar_strcmp(port_data_info.glink_port_data[i].port_name, port->port_name, sizeof(port_data_info.glink_port_data[i].port_name)) == 0)
      {
         found_entry = TRUE;
         break;
      }
   }

   if (!found_entry)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_init: Cannot find port name entry %s", port->port_name);
      return;
   }

   rc = gpr_glink_open(port->port_name, sizeof(port->port_name), port->domain_id, (char_t *)link_info->remote_ss);

   if (rc != AR_EOK)
   {
      AR_MSG(DBG_ERROR_PRIO, "Glink open failed for the domain_id:%lu with status : %lu", port->domain_id, rc);
      (void)gpr_glink_close(port->domain_id);
   }

   return;
}

GPR_INTERNAL uint32_t gpr_glink_init(uint32_t src_domain_id, uint32_t dest_domain_id)
{
   glink_link_id_type link_id;
   glink_err_type     err;
   bool_t             found_domain_pair = FALSE;
   uint32_t           glink_port_index  = 0;

   // Register glink_link_state_notif_cb for each remote subsystem link.
   GLINK_LINK_ID_STRUCT_INIT(link_id);
   link_id.xport = "SMEM";

   for (uint32_t i = 0; i < port_data_info.gpr_glink_port_pairs; i++)
   {
      if (((port_data_info.glink_port_data[i].domain_id_1 == src_domain_id) && (port_data_info.glink_port_data[i].domain_id_2 == dest_domain_id)) ||
          ((port_data_info.glink_port_data[i].domain_id_2 == src_domain_id) && (port_data_info.glink_port_data[i].domain_id_1 == dest_domain_id)))
      {

         link_id.remote_ss = ((dest_domain_id == port_data_info.glink_port_data[i].domain_id_1) ? port_data_info.glink_port_data[i].domain_name_1
                                                                                 : port_data_info.glink_port_data[i].domain_name_2);

         AR_MSG(DBG_HIGH_PRIO, "gpr_glink_init: Glink entry found with remote ss: %s", link_id.remote_ss);
         glink_port_index  = i;
         found_domain_pair = TRUE;
         break;
      }
   }

   if (!found_domain_pair)
   {
      AR_MSG(DBG_ERROR_PRIO,
             "gpr_glink_init: Cannot find remote ss for src domain id %d and dest domain id %d pair",
             src_domain_id,
             dest_domain_id);
      return AR_EFAILED;
   }

   // Save details in local glink pool and send it as priv client info
   // domain id is the index
   gpr_glink_port_pool[dest_domain_id].domain_id = dest_domain_id;

   uint32_t size_to_copy =
      (sizeof(gpr_glink_port_pool[dest_domain_id].port_name) > sizeof(port_data_info.glink_port_data[glink_port_index].port_name))
         ? sizeof(port_data_info.glink_port_data[glink_port_index].port_name)
         : sizeof(gpr_glink_port_pool[dest_domain_id].port_name);

   // Get port name from glink port data table
   ar_strcpy(gpr_glink_port_pool[dest_domain_id].port_name,
             sizeof(gpr_glink_port_pool[dest_domain_id].port_name),
             port_data_info.glink_port_data[glink_port_index].port_name,
             size_to_copy);

   AR_MSG(DBG_HIGH_PRIO,
          "gpr_glink_init: Setting link priv info as state %d dest domain %d port name: \n",
          gpr_glink_port_pool[dest_domain_id].state,
          gpr_glink_port_pool[dest_domain_id].domain_id);

   uint32_t c = 0;
   while (gpr_glink_port_pool[dest_domain_id].port_name[c] != '\0')
   {
      AR_MSG(DBG_HIGH_PRIO, "%d", gpr_glink_port_pool[dest_domain_id].port_name[c]);
      c++;
   }

   link_id.link_notifier = (glink_link_state_notif_cb)gpr_ipc_glink_link_state_cb;
   err                   = glink_register_link_state_cb(&link_id, (void *)&gpr_glink_port_pool[dest_domain_id]);

   if (err == GLINK_STATUS_SUCCESS)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_init: Successfully register linkstate cb, domain_id: %lu", dest_domain_id);
   }
   else
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_init: Failed to register linkstate cb, domain_id: %lu", dest_domain_id);
   }
   return AR_EOK;
}

GPR_INTERNAL uint32_t gpr_glink_deinit(uint32_t domain_id)
{
   AR_MSG(DBG_ERROR_PRIO, "gpr_glink_deinit() Inside DEINIT");
   return gpr_glink_close(domain_id);
}

boolean gpr_glink_notify_rx_intent_req_cb(glink_handle_type handle,  /* handle for the glink channel */
                                          const void *      priv,    /* priv client data passed in glink_open */
                                          size_t            req_size /* Requested size */
                                          )
{
   return TRUE;
}

GPR_INTERNAL uint32_t gpr_glink_open(char_t *port_name, uint32_t port_name_size, uint32_t domain_id, char *remote_ss)
{
   gpr_glink_port_t *     port = &gpr_glink_port_pool[domain_id];
   glink_open_config_type glink_cfg;
   glink_err_type         glink_rc;

   if (NULL == port_name || port_name_size < 2 /*1 char + '\0'*/ || port_name_size > GPR_GLINK_MAX_PORT_NAME_SIZE ||
       NULL == remote_ss)
   {
      return AR_EBADPARAM;
   }

   if (GPR_GLINK_CONNECTING == port->state)
   {
      return AR_EOK;
   }

   // prepare G-link cfg
   ar_mem_set(&glink_cfg, 0, sizeof(glink_open_config_type));
   glink_cfg.name                 = port_name;
   glink_cfg.options              = GLINK_OPT_INITIAL_XPORT;
   glink_cfg.notify_rx            = gpr_glink_rx_cb;      // calls gpr_receive
   glink_cfg.notify_tx_done       = gpr_glink_tx_done_cb; // calls gpr_send_done
   glink_cfg.notify_tx_abort      = gpr_glink_tx_abort;
   glink_cfg.notify_state         = gpr_glink_state_cb;
   glink_cfg.notify_rx_intent_req = gpr_glink_notify_rx_intent_req_cb;
   glink_cfg.remote_ss            = remote_ss;
   glink_cfg.transport            = "SMEM";

   uint32_t size_to_copy = (sizeof(port->port_name) > port_name_size) ? port_name_size : sizeof(port->port_name);

   ar_strcpy(port->port_name, sizeof(port->port_name), port_name, (size_t)size_to_copy);
   glink_cfg.priv = port;
   port->state    = GPR_GLINK_CONNECTING;

   if (GLINK_STATUS_SUCCESS != (glink_rc = glink_open(&glink_cfg, &port->glink_handle)))
   {
      AR_MSG(DBG_ERROR_PRIO, "glink_open() failed with rc = %d.", glink_rc);
      port->state = GPR_GLINK_DISCONNECTED;
      return AR_EFAILED;
   }
   AR_MSG(DBG_ERROR_PRIO, "gpr_glink_open() with port handle %lu DONE.", domain_id);
   return AR_EOK;
}

GPR_INTERNAL uint32_t gpr_glink_close(uint32_t domain_id)
{
   gpr_glink_port_t *port = &gpr_glink_port_pool[domain_id];

   if (GPR_GLINK_DISCONNECTED == port->state || GPR_GLINK_DISCONNECTING == port->state)
   {
      return AR_EALREADY;
   }
   port->state = GPR_GLINK_DISCONNECTING;

   if (port->glink_handle)
   {
#ifdef GPR_GLINK_QOS_SUPPORT
      (void)glink_qos_cancel(port->glink_handle);
#endif // GPR_GLINK_QOS_SUPPORT
      (void)glink_close(port->glink_handle);
   }

   AR_MSG(DBG_HIGH_PRIO, "gpr_glink_close() called GPR_GLINK_DISCONNECTING");

   return AR_EOK;
}

GPR_INTERNAL uint32_t gpr_glink_tx(uint32_t domain_id, void *buf, uint32_t size)
{
   gpr_glink_port_t *port = &gpr_glink_port_pool[domain_id];
   glink_err_type    glink_rc;

   if (GPR_GLINK_CONNECTED != port->state)
   {
      AR_MSG(DBG_ERROR_PRIO, "gpr_glink_tx(): port not connected, state=0%lx", port->state);
      return AR_ENOTREADY;
   }

#ifdef GPR_DEBUG_MSG
 gpr_packet_t *packet = (gpr_packet_t *)buf;
   AR_MSG(DBG_HIGH_PRIO,
             "gpr_glink_tx() dst port %lx, src port %lx, opcode 0x%lx, token %lx ",
             packet->dst_port,
             packet->src_port,
             packet->opcode,
             packet->token);
#endif

   if (GLINK_STATUS_SUCCESS != (glink_rc = glink_tx(port->glink_handle, buf, buf, size, GLINK_TX_REQ_INTENT)))
   {
      AR_MSG(DBG_ERROR_PRIO, "glink_tx() failed with rc = %d.", glink_rc);
      return AR_EFAILED;
   }

   return AR_EOK;
}

GPR_INTERNAL uint32_t gpr_glink_rx_done(uint32_t domain_id, void *buf)
{
   gpr_glink_port_t *port = &gpr_glink_port_pool[domain_id];
   glink_err_type    glink_rc;

   if (GPR_GLINK_CONNECTED != port->state)
   {
      return AR_ENOTREADY;
   }

#ifdef GPR_DEBUG_MSG
   gpr_packet_t *packet = (gpr_packet_t *)buf;
   AR_MSG(DBG_HIGH_PRIO,
             "gpr_glink_rx_done() dst port %lx, src port %lx, opcode 0x%lx, token %lx ",
             packet->dst_port,
             packet->src_port,
             packet->opcode,
             packet->token);
#endif

   if (GLINK_STATUS_SUCCESS != (glink_rc = glink_rx_done(port->glink_handle, buf, TRUE)))
   {
      AR_MSG(DBG_ERROR_PRIO, "glink_rx_done() failed with rc = %d", glink_rc);
      return AR_EFAILED;
   }

   return AR_EOK;
}

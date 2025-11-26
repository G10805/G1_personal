/******************************************************************************

                N E T M G R _ X F R M _ U T I L S . h

******************************************************************************/

/******************************************************************************

  @file    netmgr_xfrm_utils.h
  @brief   Network manager xfrm utilities header file

  DESCRIPTION
  Implementation of xfrm utilities and helpers.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2021-2022 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef __NETMGR_XFRM_UTILS_H__
#define __NETMGR_XFRM_UTILS_H__

#ifndef FEATURE_DS_LINUX_AUTO_IVI
uint8_t netmgr_xfrm_utils_get_ipsec_sa_protocol_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_encapsulation_mode_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_dest_addr_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_local_addr_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_spi_rx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_spi_tx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_hash_algo_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_crypto_algo_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_traffic_selector_list_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_traffic_selector_responder_list_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_crypto_key_rx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_crypto_key_tx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_udp_encap_src_port_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_aes_mode_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_is_udp_encaps_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_hash_key_tx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_hash_key_rx_valid
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ipsec_sa_protocol_enum_v01 netmgr_xfrm_utils_get_ipsec_sa_protocol
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ipsec_sa_encapsulate_enum_v01 netmgr_xfrm_utils_get_encapsulation_mode
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ip_addr_type_v01 *netmgr_xfrm_utils_get_dest_addr
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ip_addr_type_v01 *netmgr_xfrm_utils_get_local_addr
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_traffic_selector_list_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_traffic_selector_responder_list_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_crypto_key_rx_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_crypto_key_tx_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_hash_key_tx_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_hash_key_rx_len
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_traffic_selector_type_v01 *netmgr_xfrm_utils_get_traffic_selector_list
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_traffic_selector_type_v01 *netmgr_xfrm_utils_get_traffic_selector_responder_list
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t *netmgr_xfrm_utils_get_crypto_key_rx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t *netmgr_xfrm_utils_get_crypto_key_tx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t *netmgr_xfrm_utils_get_hash_key_tx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t *netmgr_xfrm_utils_get_hash_key_rx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_spi_tx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint32_t netmgr_xfrm_utils_get_spi_rx
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint16_t netmgr_xfrm_utils_get_udp_encap_src_port
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ipsec_crypto_algo_enum_v01 netmgr_xfrm_utils_get_crypto_algo
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

uint8_t netmgr_xfrm_utils_get_is_udp_encaps
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ipsec_aes_mode_enum_v01 netmgr_xfrm_utils_get_aes_mode
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);

wds_ipsec_crypto_algo_enum_v01 netmgr_xfrm_utils_get_hash_algo
(
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,
  uint8_t frag_count
);
#else /* FEATURE_DS_LINUX_AUTO_IVI */
#define ELEMENT_ipsec_sa_protocol ipsec_sa_protocol
#define ELEMENT_encapsulation_mode encapsulation_mode
#define ELEMENT_dest_addr dest_addr
#define ELEMENT_local_addr local_addr
#define ELEMENT_spi_rx spi_rx
#define ELEMENT_spi_tx spi_tx
#define ELEMENT_hash_algo hash_algo
#define ELEMENT_crypto_algo crypto_algo
#define ELEMENT_traffic_selector_list traffic_selector_list
#define ELEMENT_traffic_selector_responder_list traffic_selector_responder_list
#define ELEMENT_udp_encap_src_port udp_encap_src_port
#define ELEMENT_crypto_key_rx crypto_key_rx
#define ELEMENT_crypto_key_tx crypto_key_tx
#define ELEMENT_is_udp_encaps is_udp_encaps
#define ELEMENT_aes_mode aes_mode
#define ELEMENT_hash_key_tx hash_key_tx
#define ELEMENT_hash_key_rx hash_key_rx

#define RETURN_TYPE_wds_ip_addr_type_v01 wds_ip_addr_type_v01
#define RETURN_TYPE_wds_ipsec_sa_protocol_enum_v01 wds_ipsec_sa_protocol_enum_v01
#define RETURN_TYPE_wds_ipsec_sa_encapsulate_enum_v01 wds_ipsec_sa_encapsulate_enum_v01
#define RETURN_TYPE_wds_traffic_selector_type_v01 wds_traffic_selector_type_v01
#define RETURN_TYPE_uint32_t uint32_t
#define RETURN_TYPE_uint16_t uint16_t
#define RETURN_TYPE_uint8_t uint8_t
#define RETURN_wds_ipsec_crypto_algo_enum_v01 wds_ipsec_crypto_algo_enum_v01
#define RETURN_wds_ipsec_aes_mode_enum_v01 wds_ipsec_aes_mode_enum_v01

#define NETMGR_XFRM_UTILS_CREATE_VALID_FUNCTION(ARGUMENT)\
 uint8_t netmgr_xfrm_utils_get_##ARGUMENT##_valid\
 (\
   wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,\
   wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,\
   uint8_t frag_count)\
 {\
   return 0;\
 }

#define NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT) NETMGR_XFRM_UTILS_CREATE_VALID_FUNCTION(ELEMENT)

NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_ipsec_sa_protocol)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_encapsulation_mode)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_dest_addr)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_local_addr)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_spi_rx)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_spi_tx)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_hash_algo)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_crypto_algo)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_traffic_selector_list)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_traffic_selector_responder_list)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_crypto_key_rx)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_crypto_key_tx)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_udp_encap_src_port)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_aes_mode)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_is_udp_encaps)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_hash_key_tx)
NETMGR_XFRM_UTILS_GET_VALID_FUNCTION(ELEMENT_hash_key_rx)


#define NETMGR_XFRM_UTILS_CREATE_GET_PTR_FUNCTION(ARGUMENT, RETURN_TYPE)\
 RETURN_TYPE *netmgr_xfrm_utils_get_##ARGUMENT\
 (\
   wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,\
   wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,\
   uint8_t frag_count)\
 {\
   return NULL;\
 }

#define NETMGR_XFRM_UTILS_GET_PTR_FUNCTION(ELEMENT, RETURN) NETMGR_XFRM_UTILS_CREATE_GET_PTR_FUNCTION(ELEMENT, RETURN)
 
NETMGR_XFRM_UTILS_GET_PTR_FUNCTION(ELEMENT_dest_addr, RETURN_TYPE_wds_ip_addr_type_v01)
NETMGR_XFRM_UTILS_GET_PTR_FUNCTION(ELEMENT_local_addr, RETURN_TYPE_wds_ip_addr_type_v01)

#define NETMGR_XFRM_UTILS_CREATE_GET_ARRAY_PTR_FUNCTION(ARGUMENT, RETURN_TYPE)\
RETURN_TYPE *netmgr_xfrm_utils_get_##ARGUMENT\
(\
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,\
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,\
  uint8_t frag_count)\
{\
  return NULL;\
}

#define NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT, RETURN) NETMGR_XFRM_UTILS_CREATE_GET_ARRAY_PTR_FUNCTION(ELEMENT, RETURN)

NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_traffic_selector_list, RETURN_TYPE_wds_traffic_selector_type_v01)
NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_traffic_selector_responder_list, RETURN_TYPE_wds_traffic_selector_type_v01)
NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_crypto_key_rx, RETURN_TYPE_uint8_t)
NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_crypto_key_tx, RETURN_TYPE_uint8_t)
NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_hash_key_tx, RETURN_TYPE_uint8_t)
NETMGR_XFRM_UTILS_GET_ARRAY_PTR_FUNCTION(ELEMENT_hash_key_rx, RETURN_TYPE_uint8_t)

#define NETMGR_XFRM_UTILS_CREATE_GET_FUNCTION(ARGUMENT, RETURN_TYPE)\
RETURN_TYPE netmgr_xfrm_utils_get_##ARGUMENT\
(\
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,\
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,\
  uint8_t frag_count)\
{\
  return 0;\
}
#define NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT, RETURN) NETMGR_XFRM_UTILS_CREATE_GET_FUNCTION(ELEMENT, RETURN)

NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_ipsec_sa_protocol, RETURN_TYPE_wds_ipsec_sa_protocol_enum_v01)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_encapsulation_mode, RETURN_TYPE_wds_ipsec_sa_encapsulate_enum_v01)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_spi_tx, RETURN_TYPE_uint32_t)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_spi_rx, RETURN_TYPE_uint32_t)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_udp_encap_src_port, RETURN_TYPE_uint16_t)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_crypto_algo, RETURN_wds_ipsec_crypto_algo_enum_v01)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_is_udp_encaps, RETURN_TYPE_uint8_t)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_aes_mode, RETURN_wds_ipsec_aes_mode_enum_v01)
NETMGR_XFRM_UTILS_GET_FUNCTION(ELEMENT_hash_algo, RETURN_wds_ipsec_crypto_algo_enum_v01)

#define NETMGR_XFRM_UTILS_CREATE_GET_LEN_FUNCTION(ARGUMENT)\
uint32_t netmgr_xfrm_utils_get_##ARGUMENT##_len\
(\
  wds_get_ipsec_static_sa_config_resp_msg_v01 *sa_config,\
  wds_ipsec_static_sa_config_ex_info_ind_msg_v01 **sa_config_ex_frag,\
  uint8_t frag_count)\
{\
  return 0;\
}

#define NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT) NETMGR_XFRM_UTILS_CREATE_GET_LEN_FUNCTION(ELEMENT)

NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_traffic_selector_list)
NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_traffic_selector_responder_list)
NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_crypto_key_rx)
NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_crypto_key_tx)
NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_hash_key_tx)
NETMGR_XFRM_UTILS_GET_LEN_FUNCTION(ELEMENT_hash_key_rx)

#endif /* FEATURE_DS_LINUX_AUTO_IVI */
#endif /* __NETMGR_XFRM_UTILS_H__ */

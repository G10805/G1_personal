//******************************************************************************************************************************
// Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  habtestmem.c
* @brief Implements uhabtest batch testing mode
********************************************************************************************************************************
*/

//#define _DEBUG

#if defined(HAB_DESKTOP) /* desktop kernel */

#include <linux/types.h>
#include "../khab_testapp/khab_drv_test/khab_drv_test.h"

#define printf HAB_LOG_INFO
#define vc_export khab_alloc_and_export
#define vc_unexport khab_unexport_and_free

#elif defined(CONFIG_KHAB_TEST) /* target kernel */

#include <linux/types.h>
#include "khab_drv_test.h"

#define printf HAB_LOG_INFO
#define vc_export khab_alloc_and_export
#define vc_unexport khab_unexport_and_free

#else /* user space */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "habtest.h"

#endif

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#define gettid() syscall(SYS_gettid)
#endif

#if defined(__INTEGRITY)
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "pmem.h"

#define gettid() ((unsigned int)(uintptr_t)pthread_self())
#endif

#if defined(__QNXNTO__)
#include <process.h>
#include "pmem.h"
#endif

#ifdef WIN32
#define gettid() pthread_getw32threadid_np(pthread_self ())
#endif

#define __ACK_RX_IMP_DONE            0x0001
#define __ACK_TX_PATTERN_READY       0x0002
#define __ACK_RX_VERIFY_DONE         0x0003
#define __ACK_RX_UNIMP_DONE          0x0004
#define __ACK_TX_UNEXP_DONE          0x0005
#define __ACK_TX_HS                  0x0006
#define __ACK_RX_HS                  0x0007

#define __ACK_QUIT                   0xFFFF

#pragma pack(push, 1)
struct tx_rx_arg {
    int32_t exp_id;
    int32_t exp_size;

    int32_t pattern;

    int32_t ack; // cmd

	// for sanity check
	int32_t vcid[2]; // indexed by tx_rx
	int32_t tid[2];
	int32_t pid[2];
};

#pragma pack(pop)

// Set to 1/0 to enable/disabled cached access when
// - mapping shared memory for export
// - mapping imported memory
#ifdef CONFIG_HGY_PLATFORM
uint8_t gCacheEnabled = 1;
#else
uint8_t gCacheEnabled = 0;
#endif

#if !defined(CONFIG_KHAB_TEST) /* target kernel */
int32_t test_mem_multiple_export(int vc_id_param, int tx_rx, int num_export, int unit_export_size, int loop) {
    int32_t ret = 0;
    int32_t flags = 0;
    int32_t vc_id;
    int32_t *exp_id=NULL;
    struct tx_rx_arg msg = {0};
    int32_t msg_size;
    int32_t **pbuf;
    int32_t **pimp;
    int i, n;

  if(vc_id_param <= 0)
  {
	int mm_id = MM_GFX;
    ret = habmm_socket_open(&vc_id, mm_id, (unsigned int)-1, flags);
    if (ret) {
      printf("hab open failed %d returned\n", ret);
      return -1;
    }
  } else {
    vc_id = vc_id_param;
  }
  // allocate export id array
#if defined(HAB_DESKTOP) || defined(CONFIG_KHAB_TEST) /* kernel */
  exp_id = (int32_t*)kmalloc(sizeof(int32_t)*num_export, GFP_KERNEL);
#else
  exp_id = (int32_t*)malloc(sizeof(int32_t)*num_export);
#endif
	if (!exp_id) {
		printf("failed to allocate expid\n");
		return -1;
	}
  memset(exp_id, 0, sizeof(int32_t)*num_export);
  // allocate pv array
#if defined(HAB_DESKTOP) || defined(CONFIG_KHAB_TEST) /* kernel */
  pbuf = (int32_t **)kmalloc(sizeof(int32_t *)*num_export, GFP_KERNEL);
#else
  pbuf = (int32_t **)malloc(sizeof(int32_t *)*num_export);
#endif  
	if (!pbuf) {
		printf("failed to allocate pbuf\n");
		return -1;
	}
  memset(pbuf, 0, sizeof(int32_t *)*num_export);
  pimp = pbuf;

  if(tx_rx == 1) {
#ifndef TEST_ION_EXPORT
	  printf("To do for ppga allocation in QNX!\n");
#else
      int j;
      // sender
      // 1. allocate and export memory
      ret = vc_export_ion_multiple(vc_id, num_export, unit_export_size, 0x12345678, exp_id, (void **)pbuf, gCacheEnabled);
	  if (ret !=0 ){
		  printf("vc_export_ion_multiple failed %d on vcid %X\n", ret, vc_id);
	  }
      // 2. send export id and size
	  for (n=0; n<num_export; n++)
	  {
		  msg.exp_id = exp_id[n];
		  msg.exp_size = unit_export_size;
		  ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
		  if (ret !=0 ) printf("send export id failed %d on vcid %X\n", ret, vc_id);
		  else printf("send export id %d, size %d on vcid %x completed, now wait for importer's response...\n", msg.exp_id, msg.exp_size, vc_id);
	  }
      // 3. wait for ack memory setup done
	  msg_size = sizeof(msg);
	  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_IMP_DONE) {
          printf("failed to receive export ack from Rx, ret %d, msg sz %d, ack %X\n", ret, msg_size, msg.ack);
          ret = -1;
          goto err_free_mem;
      }

      for (j=0; j<loop; j++) {

          // 4. set pattern
    	  for (n=0; n<num_export; n++)
    	  {
    		  for (i=0; i<unit_export_size/4; i++) {
    			  pbuf[n][i] = i+j+n;
    		  }
    	  }
          // 5. signal Rx to verify
          msg.ack = __ACK_TX_PATTERN_READY;
          msg.pattern = j;
          ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
		  if (ret !=0 ) printf("send __ACK_TX_PATTERN_READY failed %d on vcid %X\n", ret, vc_id);

          // 6. wait for verify done
		  msg_size = sizeof(msg);
		  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
          if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_VERIFY_DONE) {
              printf("failed to receive pattern verify ack from Rx, ret %d, msg sz %d, ack %X, loop %d\n", ret, msg_size, msg.ack, j);
              ret = -1;
              goto err_free_mem;
          }

      } // loop

      // 7. signal complete loop
      msg.ack = __ACK_QUIT;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("send __ACK_QUIT failed %d on vcid %X\n", ret, vc_id);

      // 8. wait for unimport done
	msg_size = sizeof(msg);
      ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_UNIMP_DONE) {
          printf("failed to receive unimport done from Rx, ret %d, msg sz %d, ack %X, loop %d\n", ret, msg_size, msg.ack, j);
		  //          return -1;
      }

      // 9. unexport
      ret = vc_unexport_ion_multiple(vc_id);
	  if (ret !=0 ) printf("vc_unexport_ion_multiple failed %d on vcid %X\n", ret, vc_id);

	msg.ack = __ACK_TX_UNEXP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("send __ACK_TX_UNEXP_DONE failed %d on vcid %X\n", ret, vc_id);

	// do we need to wait other side receives and handles this msg before close the vchan?
	// otherwise, both this msg and vchan close could arrive at the same time, and causing the msg not be received properly
	// due to vc closed remotely
#endif
  } else {
      // receiver
      uint32_t pattern_test_count = 0;
      uint32_t pattern_test_fail_count = 0;

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
	  // allocate pmem handle array
	  pmem_handle_t *handle;
	  handle = (pmem_handle_t*)malloc(sizeof(pmem_handle_t)*num_export);
	  memset(handle, 0, sizeof(pmem_handle_t)*num_export);
#endif

      // wait for export id and size
	  for (n=0; n<num_export; n++)
	  {
		  msg_size = sizeof(msg);
	      ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (unsigned int)-1, 0);
	      if (ret != 0 || msg_size != sizeof(msg)) {
	          printf("failed to receive export id from Tx, ret %d, msg sz %d\n", ret, msg_size);
	          ret = -1;
	          goto err_free_mem;
	      } else {
			printf("ready to import expid %d, size %d on vcid %x\n", msg.exp_id, msg.exp_size, vc_id);
		  }
	      exp_id[n]= msg.exp_id;
	      if (unit_export_size != msg.exp_size)
	      {
	    	  printf("unit_export_size != msg.exp_size\n");
	      }

		  // import
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
		  int sz = 0;

		  ret = habmm_import(vc_id, (void**)&handle[n], msg.exp_size, msg.exp_id,
							 HABMM_EXPIMP_FLAGS_FD | (gCacheEnabled ? HABMM_IMPORT_FLAGS_CACHED : 0));
		  if (!ret) {
			pimp[n] = pmem_map_handle(handle[n], &sz); //if GVM is ION system heap, this fails
			if (!pimp[n] || sz != msg.exp_size) {
			  printf("pmem map handle %p failed ret %p sz %d expected size %d\n", handle[n], pimp[n], sz, msg.exp_size);
			  ret = -1;
			  goto err_free_mem;
	  		}
		  } else {
			printf("failed to import export id %d to pmem handle ret %d\n", msg.exp_id, ret);
			ret = -1;
			goto err_free_mem;
		  }
#else
		ret = habmm_import(vc_id, (void**)&pimp[n], msg.exp_size, msg.exp_id,
			gCacheEnabled ? HABMM_IMPORT_FLAGS_CACHED : 0);
#endif
		if (ret != 0 || !pimp[n]) {
			printf("failed to import on vc_id %X, exp id %d, ret %d pimp %p n %d\n", vc_id, msg.exp_id, ret, pimp[n], n);
			ret = -1;
			goto err_free_mem;
		} else {
			exp_id[n]= msg.exp_id;
		}
	  }
      // ack memory setup done
      msg.ack = __ACK_RX_IMP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("send __ACK_RX_IMP_DONE failed %d on vcid %X\n", ret, vc_id);

      // wait for signal to verify
	  do {
		  msg_size = sizeof(msg);
          ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (unsigned int)-1, 0);
		  if (ret != 0 || msg_size != sizeof(msg)) {
			printf("failed to receive pattern ready from Tx! ret %d msg sz %d ack %X loop %d\n", ret, msg_size, msg.ack, msg.pattern);
			ret = -1;
			goto err_free_mem;
		  } else if (msg.ack == __ACK_QUIT) {
			  printf("Tx asks request to exit the testing after %d loop!\n", pattern_test_count);
			  break; // break the loop
		  } else if (msg.ack != __ACK_TX_PATTERN_READY) {
			  printf("unknow command received %X, ignored! test count %d\n", msg.ack, pattern_test_count);
			  continue;
		  }

		  // verify patterns
		  for (n=0; n<num_export; n++){
			  for (i=0; i<unit_export_size/4; i++) {
				  if (pimp[n][i] != i+msg.pattern+n){
					  printf("failed pattern test! expected %x, actual %x on offset %d\n", i+msg.pattern, pimp[n][i], i);
					  pattern_test_fail_count += 1;
					  break;
				  }
			  }
		  }
          pattern_test_count += 1;

		  // ack verify done
		  msg.ack = __ACK_RX_VERIFY_DONE;
		  ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
		  if (ret !=0 ) printf("send __ACK_RX_VERIFY_DONE failed %d on vcid %X\n", ret, vc_id);

		  // if signal to exit the loop
	  } while (1);

      printf("%d pattern test(s) completed with %d failures\n",
        pattern_test_count, pattern_test_fail_count);

      // unimport
	  for (n=0; n<num_export; n++)
	  {
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
		ret = pmem_unmap_handle(handle[n], pimp[n]);
		if (ret) {
		  printf("pmem_unmap_handle %p failed ret %d va %p\n", handle[n], ret, pimp[n]);
		}
#endif
		  ret = habmm_unimport(vc_id, exp_id[n], pimp[n], 0);
		  if (ret !=0 ) printf("unimport failed %d on vcid %X\n", ret, vc_id);
	  }
      // signal unimport done
      msg.ack = __ACK_RX_UNIMP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("send __ACK_RX_UNIMP_DONE failed %d on vcid %X\n", ret, vc_id);

	// make sure the remote side has completed the unexp
	msg_size = sizeof(msg);
	ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (unsigned int)-1, 0);
	if (ret != 0 || msg_size != sizeof(msg)) {
		printf("Warning! not able to receive unexport done from Tx, ret %d, msg sz %d, ack %X, loop %d\n", ret, msg_size, msg.ack, msg.pattern);
	} else if (msg.ack == __ACK_TX_UNEXP_DONE) {
		printf("Tx completes unexport! Ready to close vchan %X locally.\n", vc_id);
	} else {
		printf("Warning! not able to receive unexport fully from Tx side. Received %d as %d bytes. Ready to close vcid %X\n",
		msg.ack, msg_size, vc_id);
	}
  }

// tear down
  if(vc_id_param <= 0)
      ret = habmm_socket_close(vc_id);

err_free_mem:
#if defined(HAB_DESKTOP) || defined(CONFIG_KHAB_TEST) /* kernel */
  if (exp_id) kfree(exp_id);
  if (pbuf) kfree(pbuf);
#else
  if (exp_id) free(exp_id);
  if (pbuf) free(pbuf);
#endif

  return ret;
}
#endif

/* vc_id_param - use this pass-in vcid or not, 0 and negative is asking for hab-open() call to create new vcid, otherwise use the pass-in
 * tx_rx - 1 for sender otherwise receiver. sender doing the init ping and export, receiver is listening and importing
 */
int32_t test_mem(int vc_id_param, int tx_rx, int mm_id, int mem_size, int loop, uint32_t mem_source) {
    int32_t ret;
    int32_t flags = 0;
    int32_t vc_id;
    int32_t exp_id = -1;
    struct tx_rx_arg msg = {0};
    int32_t msg_size, imp_size;
    int32_t *pbuf;
    int32_t *pimp;
    int i, j;

  if(vc_id_param <= 0)
  {
    ret = habmm_socket_open(&vc_id, mm_id, (uint32_t)-1, flags);
    if (ret) {
      printf("tid %x: hab open failed %d returned\n", gettid(), ret);
      return -1;
    }
  } else {
	  vc_id = vc_id_param;
  }
  printf("tid %x: txrx %d, start using vcid %x...\n", gettid(), tx_rx, vc_id);

  if(tx_rx == 1) {
      // sender
	  // 0. handshake
	  msg.ack = __ACK_TX_HS;
      msg.vcid[tx_rx] = vc_id;
      msg.tid[tx_rx] = gettid();
      msg.pid[tx_rx] = getpid();
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: handshake failed %d on vcid %X\n", gettid(), ret, vc_id);
	  else printf("tid %x: handshake on vcid %x sent, now wait for importer's response...\n", gettid(), vc_id);

	  msg_size = sizeof(msg);
	  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_HS) {
          printf("tid %x: failed to receive handshake ack from Rx, ret %d, msg sz %d, ack %X\n", gettid(), ret, msg_size, msg.ack);
          return -1;
      } else {
          printf("tid %x: TxRx handshake pass! Rx vcid %x, tid %x, pid %d, Tx vcid %x, tid %x, pid %d\n", gettid(), msg.vcid[0], msg.tid[0], msg.pid[0], msg.vcid[1], msg.tid[1], msg.pid[1]);
	  }
      // 1. allocate and export memory

#ifdef TEST_ION_EXPORT
	  if (MEM_SOURCE_MMAP == mem_source)
		ret = vc_export (vc_id, mem_size, 0x12345678, &exp_id, (void**)&pbuf, gCacheEnabled);
	  else
		ret = vc_export_ion (vc_id, mem_size, 0x12345678, &exp_id, (void**)&pbuf, gCacheEnabled);
#ifdef _DEBUG
	  if (MEM_SOURCE_MMAP == mem_source) {
		printf("tid %x: export memory on vcid %x, sizse %d, mem_source is mmap\n", gettid(), vc_id, mem_size);
	  } else {
		printf("tid %x: export memory on vcid %x, sizse %d, mem_source is ion\n", gettid(), vc_id, mem_size);
	  }
#endif /*_DEBUG*/
#elif defined(TEST_DMAHEAP_EXPORT)
	  if (MEM_SOURCE_MMAP == mem_source)
		ret = vc_export (vc_id, mem_size, 0x12345678, &exp_id, (void**)&pbuf, gCacheEnabled);
	  else
		ret = vc_export_dmaheap (vc_id, mem_size, 0x12345678, gCacheEnabled, &exp_id, (void**)&pbuf);
#else
      ret = vc_export (vc_id, mem_size, 0x12345678, &exp_id, (void**)&pbuf, gCacheEnabled);
#ifdef _DEBUG
	  if (MEM_SOURCE_MMAP == mem_source) {
		printf("tid %x: export memory on vcid %x, sizse %d, mem_source is mmap\n", gettid(), vc_id, mem_size);
	  } else {
		printf("tid %x: export memory on vcid %x, sizse %d, mem_source is dmaheap\n", gettid(), vc_id, mem_size);
	  }
#endif /*_DEBUG*/
#endif /*TEST_ION_EXPORT*/
      if (ret !=0 ) printf("tid %x: export failed %d on vcid %X\n", gettid(), ret, vc_id);
      // 2. send export id and size
      msg.exp_id = exp_id;
      msg.exp_size = mem_size;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: send export id failed %d on vcid %X\n", gettid(), ret, vc_id);
	  else printf("tid %x: send export id %d, size %d on vcid %x completed, now wait for importer's response...\n", gettid(), msg.exp_id, msg.exp_size, vc_id);

      // 3. wait for ack memory setup done
	  msg_size = sizeof(msg);
	  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_IMP_DONE) {
          printf("tid %x: failed to receive export ack from Rx, ret %d, msg sz %d, ack %X\n", gettid(), ret, msg_size, msg.ack);
          return -1;
      } else {
#ifdef _DEBUG
		  printf("tid %x: received RX IMP DONE on vcid %x, msg size %d\n", gettid(), vc_id, msg_size);
#endif
	  }

      for (j=0; j<loop; j++) {

          // 4. set pattern
          for (i=0; i<mem_size/4; i++) {
              pbuf[i] = i+j;
          }

          // 5. signal Rx to verify
          msg.ack = __ACK_TX_PATTERN_READY;
          msg.pattern = j;
          ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
		  if (ret !=0 ) printf("tid %x: send __ACK_TX_PATTERN_READY failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
		  else printf("tid %x: send TX PATTERN READY on vcid %x\n", gettid(), vc_id);
#endif

          // 6. wait for verify done
		  msg_size = sizeof(msg);
		  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
          if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_VERIFY_DONE) {
              printf("tid %x: failed to receive pattern verify ack from Rx, ret %d, msg sz %d, ack %X, loop %d\n", gettid(), ret, msg_size, msg.ack, j);
              return -1;
          } else {
#ifdef _DEBUG
			  printf("tid %x: received RX VERIFY DONE on vcid %x, msg size %d\n", gettid(), vc_id, msg_size);
#endif
		  }

      } // loop
      
      // 7. signal complete loop
      msg.ack = __ACK_QUIT;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: send __ACK_QUIT failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
	  else printf("tid %x: sent QUIT on vcid %x after %d loops\n", gettid(), vc_id, loop);
#endif

      // 8. wait for unimport done
	  msg_size = sizeof(msg);
      ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_RX_UNIMP_DONE) {
          printf("tid %x: WARNING! Failed to receive unimport done from Rx, ret %d, msg sz %d, ack %X, loop %d\n", gettid(), ret, msg_size, msg.ack, j);
		  //          return -1;
      } else {
#ifdef _DEBUG
		  printf("tid %x: received RX UNIMP DONE on vcid %x, msg size %d\n", gettid(), vc_id, msg_size);
#endif
	  }

      // 9. unexport
      ret = vc_unexport(vc_id, exp_id);
	  if (ret !=0 ) printf("tid %x: unexport failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
	  else printf("tid %x: unexport done on vcid %x, exp id %d\n", gettid(), vc_id, exp_id);
#endif

	  msg.ack = __ACK_TX_UNEXP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: send __ACK_TX_UNEXP_DONE failed %d on vcid %X\n", gettid(), ret, vc_id);

  } else {
      // receiver
      uint32_t pattern_test_count = 0;
      uint32_t pattern_test_fail_count = 0;
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
	  pmem_handle_t handle = NULL;
	  int sz = 0;
#endif

	  // handshake
	  msg_size = sizeof(msg);
	  ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg) || msg.ack != __ACK_TX_HS) {
          printf("tid %x: failed to receive handshake ack from Tx, ret %d, msg sz %d, ack %X\n", gettid(), ret, msg_size, msg.ack);
          return -1;
      } else {
          printf("tid %x: TxRx handshake recevied! Rx vcid %x, tid %x, pid %d, Tx vcid %x, tid %x, pid %d\n", gettid(), msg.vcid[0], msg.tid[0], msg.pid[0], msg.vcid[1], msg.tid[1], msg.pid[1]);
	  }

	  msg.ack = __ACK_RX_HS;
      msg.vcid[tx_rx] = vc_id;
      msg.tid[tx_rx] = gettid();
      msg.pid[tx_rx] = getpid();
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: handshake Rx failed %d on vcid %X\n", gettid(), ret, vc_id);
	  else {
          printf("tid %x: TxRx handshake pass! Rx vcid %x, tid %x, pid %d, Tx vcid %x, tid %x, pid %d\n", gettid(), msg.vcid[0], msg.tid[0], msg.pid[0], msg.vcid[1], msg.tid[1], msg.pid[1]);
		  printf("tid %x: handshake on vcid %x sent, now wait for exp..\n", gettid(), vc_id);
	  }


      // wait for export id and size
	  msg_size = sizeof(msg);
      ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
      if (ret != 0 || msg_size != sizeof(msg)) {
          printf("tid %x: failed to receive export id from Tx, ret %d, msg sz %d\n", gettid(), ret, msg_size);
          return -1;
      } else {
		  printf("tid %x: ready to import expid %d, size %d on vcid %x\n", gettid(), msg.exp_id, msg.exp_size, vc_id);
	  }

      // import
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
      ret = habmm_import(vc_id, (void**)&handle, msg.exp_size, msg.exp_id,
						 HABMM_EXPIMP_FLAGS_FD |(gCacheEnabled ? HABMM_IMPORT_FLAGS_CACHED : 0));
	  if (!ret) {
		pimp = pmem_map_handle(handle, &sz); //if GVM is ION system heap, this fails
		if (!pimp || sz != msg.exp_size) {
		  printf("pmem map handle %p failed ret %p sz %d expected size %d\n", handle, pimp, sz, msg.exp_size);
		  return -1;
		}
	  } else {
		printf("failed to import export id %d to pmem handle ret %d\n", msg.exp_id, ret);
		return -1;
	  }
#else
      ret = habmm_import(vc_id, (void**)&pimp, msg.exp_size, msg.exp_id,
        gCacheEnabled ? HABMM_IMPORT_FLAGS_CACHED : 0);
#endif
      if (ret == 0) {
          imp_size = msg.exp_size;
          exp_id = msg.exp_id;
#ifdef _DEBUG
		  printf("tid %x: import done on vcid %x, exp id %d\n", gettid(), vc_id, msg.exp_id);
#endif
      } else {
          printf("tid %x: failed to import on vc_id %X, exp id %d, ret %d\n", gettid(), vc_id, msg.exp_id, ret);
       return -1;
      }

      // ack memory setup done
      msg.ack = __ACK_RX_IMP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: send __ACK_RX_IMP_DONE failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
	  else printf("tid %x: send RX IMP DONE on vcid %x\n", gettid(), vc_id);
#endif

      // wait for signal to verify
	  do {
		  msg_size = sizeof(msg);
          ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
		  if (ret != 0 || msg_size != sizeof(msg)) {
			  printf("tid %x: failed to receive pattern ready from Tx, ret %d, msg sz %d, ack %X, loop %d\n", gettid(), ret, msg_size, msg.ack, msg.pattern);
			  return -1;
		  } else if (msg.ack == __ACK_QUIT) {
			  printf("tid %x: Tx asks request to exit the testing after %d loop!\n", gettid(), pattern_test_count);
			  break; // break the loop
		  } else if (msg.ack != __ACK_TX_PATTERN_READY) {
			  printf("tid %x: unknow command received %X, ignored!\n", gettid(), msg.ack);
			  continue;
		  }

		  // verify patterns
		  for (i=0; i<imp_size/4; i++) {
			  if (pimp[i] != i+msg.pattern) {
				  printf("tid %x: failed pattern test! expected %X, actual %X on offset %d\n", gettid(), i+msg.pattern, pimp[i], i);
                  pattern_test_fail_count += 1;
			      break;
              }
		  }
          pattern_test_count += 1;

		  // ack verify done
		  msg.ack = __ACK_RX_VERIFY_DONE;
		  ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
		  if (ret !=0 ) printf("tid %x: send __ACK_RX_VERIFY_DONE failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
		  else printf("tid %x: send RX VERIFY DONE on vcid %x\n", gettid(), vc_id);
#endif

		  // if signal to exit the loop
	  } while (1);

      printf("tid %x: %d pattern test(s) completed with %d failures\n", gettid(),
        pattern_test_count, pattern_test_fail_count);

      // unimport
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
	ret = pmem_unmap_handle(handle, pimp);
	if (ret) {
		printf("pmem_unmap_handle %p failed ret %d va %p\n", handle, ret, pimp);
	}
#endif
	  ret = habmm_unimport(vc_id, exp_id, pimp, 0);
	  if (ret !=0 ) printf("tid %x: unimport failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
	  else printf("tid %x: unimport done on vcid %x\n", gettid(), vc_id);
#endif

      // signal unimport done
      msg.ack = __ACK_RX_UNIMP_DONE;
      ret = habmm_socket_send(vc_id, &msg, sizeof(msg), 0);
	  if (ret !=0 ) printf("tid %x: send __ACK_RX_UNIMP_DONE failed %d on vcid %X\n", gettid(), ret, vc_id);
#ifdef _DEBUG
	  else printf("tid %x: send RX UNIMP DONE on vcid %x\n", gettid(), vc_id);
#endif

	msg_size = sizeof(msg);
	ret = habmm_socket_recv(vc_id, &msg, (uint32_t *)&msg_size, (uint32_t)-1, 0);
	if (ret != 0 || msg_size != sizeof(msg)) {
		printf("tid %x: Warning! not able to receive unexport done from Tx, ret %d, msg sz %d, ack %X, loop %d\n", gettid(), ret, msg_size, msg.ack, msg.pattern);
	} else if (msg.ack == __ACK_TX_UNEXP_DONE) {
		printf("tid %x: Tx completes unexport! Ready to close vchan %X locally.\n", gettid(), vc_id);
	} else {
		printf("tid %x: Warning! not able to receive unexport fully from Tx side. Received %d as %d bytes. Ready to close vcid %X\n", gettid(),
		msg.ack, msg_size, vc_id);
	}

  }


// tear down
  if(vc_id_param <= 0)
      ret = habmm_socket_close(vc_id); /* close locally created vcid */

  return ret;
}

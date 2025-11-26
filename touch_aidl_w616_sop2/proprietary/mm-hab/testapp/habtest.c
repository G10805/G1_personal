//******************************************************************************************************************************
// Copyright (c) 2016-2019, 2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  habtest.c
* @brief Implements uhabtest testing functions
********************************************************************************************************************************
*/
#include <errno.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#if defined(ENABLE_VIRTIO_VHOST)
#include <sys/socket.h>
#include <sys/un.h>
#include <bsd/string.h>
#endif
#else
#include <Windows.h>
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "habtest.h"

#ifdef __QNXNTO__
#include "sys/mman.h"
#include "pmem.h"
#include <sys/neutrino.h>
#include <sys/syspage.h>

#define MAP_POPULATE MAP_PRIVATE
#define HAB_MAX_MSG_SIZEBYTES 0xFFFF
#define test_khab_grant(a,b,c)
#define test_khab_map(a,b,c)
// From ppga_wrapper.c
extern int hv_nk_init(unsigned long heap_size);
extern int free_ppga_pages(unsigned long pfn, unsigned long count);
int alloc_ppga_pages(unsigned long max_pf_count,
			 unsigned long min_pf_count,
			 unsigned long pfn_align_order,
			 unsigned long* pfn,
			 unsigned long* pf_countp);
#elif defined(__INTEGRITY)
#define CLOCK_MONOTONIC CLOCK_REALTIME // this is per GHS suggestion
#define HAB_MAX_MSG_SIZEBYTES 0x7000
#include "pmem.h"
#include "pmemext.h"
#elif defined(WIN32)
#include "uhab_win.h"
#define HAB_MAX_MSG_SIZEBYTES 0xFFFF // Todo: distinguish max msg size according to gvm is running on QNX HYP or GHS HYP
#else
#define HAB_MAX_MSG_SIZEBYTES 0xFFFF // Todo: distinguish max msg size according to gvm is running on QNX HYP or GHS HYP
//#include "../uhab/test_khab.h" // test only
#endif

#ifdef TEST_ION_EXPORT
#include <sys/ioctl.h>
#include <linux/msm_ion.h>

//#undef TARGET_ION_ABI_VERSION
typedef int ion_user_handle_t;
#define ION_ALLOC_ALIGN 0x1000

#if TARGET_ION_ABI_VERSION >= 2
#include <ion/ion.h>
#endif
int ion_fd=-1;
#endif

#ifdef TEST_DMAHEAP_EXPORT
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
int dmaheap_fd = -1;
int dmaheap_fd_cached = -1;
#endif

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
struct pmem_container {
	pmem_handle_t handle;
	void *va;
};
#endif

#ifdef WIN32
#define	CLOCK_REALTIME	3
#define CLOCK_MONOTONIC 1
#define sleep(_sec_) Sleep(_sec_*1000)
#endif

// khab test mode
// check khab_drv_test.h, this has to match
#define XVM_SCHE_MAX_LATENCY_US 1000000
#define KHAB_API_ERR_CHECK(__x__) if (0!=__x__) \
	                             printf("%s(%d): habmm call failed %d\n", __FUNCTION__, __LINE__, __x__)

#define MAKE_KHAB_TEST_CMD(_c1_, _c2_, _c3_, _c4_) ((uint32_t)((_c1_<<24)|(_c2_<<16)|(_c3_<<8)|_c4_) )
#define KHAB_TEST_CMD_SEND  MAKE_KHAB_TEST_CMD('S','E','N','D')
#define KHAB_TEST_CMD_RECV  MAKE_KHAB_TEST_CMD('R','E','C','V')
#define KHAB_TEST_CMD_EXP   MAKE_KHAB_TEST_CMD('E','X','P',' ')
#define KHAB_TEST_CMD_IMP   MAKE_KHAB_TEST_CMD('I','M','P',' ')
#define KHAB_TEST_CMD_UNEXP MAKE_KHAB_TEST_CMD('U','N','E','X')
#define KHAB_TEST_CMD_UNIMP MAKE_KHAB_TEST_CMD('U','N','I','M')
#define KHAB_TEST_CMD_PROF  MAKE_KHAB_TEST_CMD('P','R','O','F')
#define KHAB_TEST_CMD_ACK   MAKE_KHAB_TEST_CMD('A','C','K',' ')
#define KHAB_TEST_CMD_EXIT  MAKE_KHAB_TEST_CMD('E','X','I','T')
#define KHAB_TEST_CMD_TPUT  MAKE_KHAB_TEST_CMD('T','P','U','T')
#define KHAB_TEST_CMD_BURST MAKE_KHAB_TEST_CMD('B','R','S','T')
#define KHAB_TEST_CMD_SCHE  MAKE_KHAB_TEST_CMD('S','C','H','E')



#define PAGE_SIZE 4096

#if TARGET_ION_ABI_VERSION >= 2
struct allocation_data {
	__u64 len;
	__u32 heap_id_mask;
	__u32 flags;
	__u32 fd;
	__u64 align;
};
#define GET_ION_FD(__x__) ((__x__).fd)
#else
#define allocation_data ion_allocation_data
#define GET_ION_FD(__x__) ((__x__).handle)
#endif

struct export_item {
	int32_t vcid;
	void *uva;
	uint64_t alloc_index;
	uint32_t exp_id;
	uint32_t size; //bytes
	int num_pgs; // for kernel side
#ifdef TEST_ION_EXPORT
	struct allocation_data data;
#endif
#ifdef TEST_DMAHEAP_EXPORT
	struct dma_heap_allocation_data dmaheap_data;
#endif
	struct export_item *next;
} *g_export_list = NULL;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int item_cnt = 0;

#if defined(__QNXNTO__) || defined(__INTEGRITY)
int is_guest = 0; // 0 for host, 1 for guest in QNX. set during init time
#endif

#if defined(ENABLE_VIRTIO_VHOST)
int pvm_carveout_fd = -1;
size_t pvm_carveout_size = 0;
uint64_t pvm_carveout_offset = 0;
uint64_t carveout_curr = 0; // ToDo rollback and match total size of CMA!!
#endif

#if defined(__linux__) || defined(__QNXNTO__)
static int8_t habtest_stat_init(void);
static int8_t habtest_stat_dump(void);

static HABTEST_STAT habtest_stat =
{
	.init = habtest_stat_init,
	.dump = habtest_stat_dump,
};

static int8_t habtest_stat_init(void)
{
#if defined(__QNXNTO__)
	habtest_stat.CPS = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
#endif
	habtest_stat.counter = 0;
	habtest_stat.data = calloc(HABTEST_STAT_BUFFER_SIZE, sizeof(HABTEST_STAT_DATA));
	if(!habtest_stat.data)
	{
		fprintf(stderr, "[ERR] calloc failed!");
		return EXIT_FAILURE;
	}
	printf("\n[INFO] stat buffer size = %ld MB\n",
			HABTEST_STAT_BUFFER_SIZE * sizeof(HABTEST_STAT_DATA) / 1024 / 1024);
	return EXIT_SUCCESS;
}

static int8_t habtest_stat_dump(void)
{
	FILE *fp = NULL;
	time_t curr_time = 0;
	struct tm local_time = {};
	char buf[HABTEST_STAT_TIME_STRLEN] = {};

	fp = fopen(HABTEST_STAT_FILE, "a");
	if(!fp)
	{
		fprintf(stderr, "[ERR] fopen failed!");
		return EXIT_FAILURE;
	}

	// debug info
	time(&curr_time);
	localtime_r(&curr_time, &local_time);
	strftime(buf, HABTEST_STAT_TIME_STRLEN, "%T", &local_time);
	printf("\e[K\r[INFO] %s: dump stat buffer\n", buf);

	for(int cnt = 0; cnt < habtest_stat.counter; cnt++)
	{
		if(fprintf(fp, "%s\t%ld\n", habtest_stat.data[cnt].time,
					habtest_stat.data[cnt].round_us) < 0)
		{
			fprintf(stderr, "[ERR] fprintf failed!");
			return EXIT_FAILURE;
		}
	}
	habtest_stat.counter = 0;
	fclose(fp);

	return EXIT_SUCCESS;
}
#endif

void hab_test_init(void)
{
#if defined(__QNXNTO__)
  FILE *fp;
	fp = fopen("/dev/kgsl", "rb");
	if (fp)
	  is_guest = 0;
	else
	  is_guest = 1;
	if (fp) fclose(fp);

	printf("QNX host %s\n", is_guest? "No" : "Yes");

	if (0 != pmem_init()) {
		printf("could not pmem_init!");
	} else {
		printf("pmem_init success!");
	}

#elif defined (__INTEGRITY)
#ifdef HAB_FE
	is_guest = 1;
#else
	is_guest = 0;
#endif
	printf("GHS host %s\n", is_guest? "No" : "Yes");

	if (0 != pmem_init()) {
		printf("could not pmem_init!");
	} else {
		printf("pmem_init success!");
	}
#elif defined(WIN32)
	page_allocator_init();
#elif defined(ENABLE_VIRTIO_VHOST)
	pvm_carveout_fd = get_vhost_fd();
	if (pvm_carveout_fd < 0) {
		printf("failed to get pvm_carveout_fd\n");
		//exit(1);
	}
#endif
#ifdef TEST_ION_EXPORT
	if (ion_fd < 0)
	{
#if TARGET_ION_ABI_VERSION >= 2
		ion_fd = ion_open();
#else
		ion_fd = open("/dev/ion", O_RDONLY);
#endif
		if (ion_fd < 0)
		{
			printf("vc_export_ion.failed to open ion device! return ion fd=%d\n", ion_fd);
		}
		else
		{
			printf("vc_export_ion.open.passed ion_node_fd=%d\n",ion_fd);
		}
	}
#endif
#ifdef TEST_DMAHEAP_EXPORT
	if ((dmaheap_fd < 0) || (dmaheap_fd_cached < 0))
	{
        /* TODO: auto detect heap name */
		if (0 != access("/dev/dma_heap/qcom,system-uncached", R_OK|W_OK)) {
			printf("access qcom,system-uncached failed, try to open system-uncached\n");
			dmaheap_fd = open("/dev/dma_heap/system-uncached", O_RDWR);
		} else {
			dmaheap_fd = open("/dev/dma_heap/qcom,system-uncached", O_RDWR);
		}

		if (0 != access("/dev/dma_heap/qcom,system", R_OK|W_OK)) {
			printf("access qcom,system failed, try to open /dev/dma_heap/system\n");
			dmaheap_fd_cached = open("/dev/dma_heap/system", O_RDWR);
		} else {
			dmaheap_fd_cached = open("/dev/dma_heap/qcom,system", O_RDWR);
		}

		if ((dmaheap_fd < 0) || (dmaheap_fd_cached < 0)) {
			printf("failed to open dma_heap device! return %d, %d\n", dmaheap_fd, dmaheap_fd_cached);
		} else {
			printf("open dma_heap successfully fd %d, cached_fd %d\n", dmaheap_fd, dmaheap_fd_cached);
		}
	}
#endif

#if defined(__linux__) || defined(__QNXNTO__)
	if(habtest_stat.init() != EXIT_SUCCESS)
	{
		fprintf(stderr, "[ERR] habtest_stat.init failed!\n");
		return;
	}
#endif
}

void hab_test_deinit(void)
{
#if defined(__QNXNTO__)
	int ret;

	if (ret = pmem_deinit()) {
	  printf("could not pmem_deinit! %d", ret);
	} else {
		printf("pmem_init success!");
	}
#elif defined(WIN32)
	page_allocator_deinit();
#endif
#ifdef TEST_ION_EXPORT
	if (ion_fd >= 0) {
		close(ion_fd);
		ion_fd = -1;
	}
#endif
#ifdef TEST_DMAHEAP_EXPORT
	if (dmaheap_fd >= 0) {
		close(dmaheap_fd);
		dmaheap_fd = -1;
	}
	if (dmaheap_fd_cached >= 0) {
		close(dmaheap_fd_cached);
		dmaheap_fd = -1;
	}
#endif
#if defined(ENABLE_VIRTIO_VHOST)
	close(pvm_carveout_fd);
#endif
}


int32_t vc_open   (int32_t *vc_id_ret, unsigned int mmid, uint32_t timeout_ms, int32_t flags){
  int32_t ret;
  int32_t vc_id;

  ret = habmm_socket_open(&vc_id, mmid, timeout_ms, flags);
  if (ret) {
    printf("hab open failed %d returned\n", ret);
  } else {
	  if (vc_id_ret) {
		  *vc_id_ret = vc_id;
		  printf("for mmid %d, new vcid created 0x%X (%d)\n", mmid, vc_id, vc_id);
	  }
    else
      printf("wrong parameter! %X virtual channel ID cannot be returned\n", vc_id);
  }

  return ret;
}

int32_t vc_close  (int32_t vc_id){
  int32_t ret = habmm_socket_close(vc_id);
  return ret;
}

int32_t vc_send   (int32_t vc_id, int32_t size, int32_t pattern){
  int32_t ret = -1;
  char* pdata = malloc(size);
  if (pdata) {
	memset(pdata, pattern, size);
	ret = habmm_socket_send(vc_id, pdata, size, 0);
	free(pdata);
  }
  printf("send %d bytes, pattern %X, ret %d\n", size, pattern, ret);
  return ret;
}

int32_t vc_recv   (int32_t vc_id, int32_t size){
  int32_t ret;
  uint32_t recv_size = size;
  char *pdata = malloc(size);

  if (!pdata)
	  return -ENOMEM;
  memset(pdata, 0 , size);

  ret = habmm_socket_recv(vc_id, pdata, &recv_size, (uint32_t)-1, 0);

  if (recv_size > (uint32_t)size) printf("recv buffer is too small. alloc %d, actual %d\n", size, recv_size);
  else if (ret) printf("receive failed! ret %d, recv size %d\n", ret, recv_size);
  else printf("ret %d, received %d bytes successfully, alloced size %d, first %X, returned last %X, asked last %X\n", ret, recv_size, size, pdata[0], pdata[recv_size-1], pdata[size-1]);
  free(pdata);
  return ret;
}

#if defined(USE_QNX_PMEM_ALLOC) || defined(__INTEGRITY)

// Allocate and then map PMEM
static int alloc_map_pmem(int32_t vc_id, size_t size, void** p, uint8_t cached)
{
  unsigned int num_pages = 0;
  void* va = NULL;
  uint32_t flags = 0, pmem_id = PMEM_GRAPHICS_FRAMEBUFFER_ID;
  int ret = 0;

  if (is_guest)
	flags = 0;
  else
	flags = PMEM_FLAGS_TYPED_MEM;

  // allocation should be from a dedicated pool
  int vmid_remote;
  struct hab_socket_info info = {0};

  ret = habmm_socket_query(vc_id, &info, 0);

#ifdef __INTEGRITY
  if (!ret) {
	// assumption the multiGVM in GHS is LV-GVM:2, LA-GVM:3, host:0
	if (info.vmid_remote == 2)
	  flags = PMEM_FLAGS_SHM_GVM_1;
	else if (info.vmid_remote == 3)
	  flags = PMEM_FLAGS_SHM_GVM_2;
	else
	  flags = PMEM_FLAGS_SHM_GVM_1;
	printf("socket query succeeded. vmid local(%s) %d , remote(%s) %d, pmem flag %d\n",
		   info.vmname_local, info.vmid_local, info.vmname_remote, info.vmid_remote, flags);
  } else {
	flags = PMEM_FLAGS_SHM_GVM_1;
	printf("socket query failed! ret %d, default to pmem flags %d\n", ret, flags);
  }
#else
  if (ret) {
	printf("socket query failed! ret %d, default to remote vmid 2\n", ret);
	info.vmid_remote = 2;
  }
#endif

  if(cached)
    flags |= PMEM_FLAGS_CACHE_WB_WA;

  // Allocate physical memory
  num_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
  struct pmem_container *pmem = malloc(sizeof(struct pmem_container));
  if (!pmem) {
	printf("failed to allocate pmem container for %d bytes\n", sizeof(struct pmem_container));
	return -1;
  }
  pmem->va = pmem_malloc_ext_v2(num_pages*PAGE_SIZE, pmem_id, flags, PMEM_ALIGNMENT_4K,
								1 << info.vmid_remote,
								&pmem->handle,
								NULL);
  if (pmem->va)
	va = pmem;
  else {
	printf("failed to allocate pmem for %d bytes\n", num_pages*PAGE_SIZE);
	free(pmem);
	return -1;
  }
#else
  va = pmem_malloc_ext(num_pages * PAGE_SIZE, pmem_id, flags,
    PMEM_ALIGNMENT_4K);
  if(!va)
  {
    printf("%s: mem allocation failed, size %d bytes, flags %x\n", __func__, num_pages * PAGE_SIZE, flags);
    return -1;
  }
#endif

  *p = va;

  return ret;
}

// Unmap and then free PMEM
// va has different meaning based on platform setting
static int unmap_free_pmem(size_t size, void* va)
{
  int err;

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
  struct pmem_container *pmem = va;

  err = pmem_free(pmem->va);
  if(err < 0) {
    printf("%s: error when freeing pages: err %d\n", __func__, err);
    return err;
  } else {
	free(pmem);
  }

#else
  err = pmem_free(va);
  if(err < 0)
  {
    printf("%s: error when freeing pages: err %d\n", __func__, err);
    return err;
  }
#endif
  return 0;
}

#endif /* USE_QNX_PMEM_ALLOC */

//#if defined(ENABLE_VIRTIO_GUEST)
//int memfd = -1;
//#endif

char *vc_allocate_memory(int32_t vc_id, int32_t size, uint8_t cached, uint64_t *alloc_index)
{
  char *pdata = NULL; // = malloc(size);
  int32_t ret = 0;
//#if defined(ENABLE_VIRTIO_GUEST)
//  uint64_t pinfo;
//  uintptr_t vaddr, paddr;
//  uintptr_t pagesize = getpagesize();
//  off_t offset;
//  int fd, i;
//#endif
#if defined(USE_QNX_PMEM_ALLOC) || defined(__INTEGRITY)
  ret = alloc_map_pmem(vc_id, size, (void**) &pdata, cached);
#elif !defined(WIN32) && defined(ENABLE_VIRTIO_VHOST) // PVM
  if (size >= pvm_carveout_size) {
	  printf("Error requested carve-out mapping size %d is bigger than allowed %d\n",
			 size, pvm_carveout_size);
  }
  // carve-out fd is sent in from vhost-user-qcom
	pdata = mmap(0, /*256*1024*1024*/size/*0x40000000UL*/,
                         PROT_READ | PROT_WRITE, MAP_SHARED,
				 pvm_carveout_fd, /*0xC0000000UL*/pvm_carveout_offset + carveout_curr);
	carveout_curr += size; // need to roll back!!!
//#elif defined(ENABLE_VIRTIO_GUEST)
//  memfd = open("/dev/mem", O_RDWR | O_SYNC);
//  if (memfd < 0)
//          printf("failed to open /dev/mem!\n");
//  printf("memfd = %d\n", memfd);
//  pdata = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, memfd, 0x300000000);
//  //memset(pdata, 4096, 0xcf);
//  vaddr = (uintptr_t)pdata;
//  printf("vaddr = 0x%lx\n", vaddr);
//  offset = vaddr / pagesize * sizeof(pinfo);

//  fd = open("/proc/self/pagemap", O_RDONLY);
//  if (fd < 0)
//	  printf("/dev/pid/pagemap is not open\n");

  //atomic_add((uint8_t *)pdata, 0);
//  if (pread(fd, &pinfo, sizeof(pinfo), offset) != sizeof(pinfo)) {
//	  printf("cannot read pagemap\n");
//  }

//  if ((pinfo & (1ull << 63)) == 0) {
//	  printf("page not present\n");
//  }

//  paddr = ((pinfo & 0x007fffffffffffffull) * pagesize) | (vaddr & (pagesize - 1));
//  printf("paddr = 0x%lx\n", paddr);
#elif defined(WIN32)
  pdata = page_allocator_alloc(size / PAGE_SIZE);
#else  // map physical memory
  pdata = mmap(NULL, /*PAGE_SIZE*page_counts*/size, PROT_READ|PROT_WRITE,
			   MAP_ANONYMOUS|MAP_SHARED|MAP_POPULATE, -1, 0); // map general physical address
#endif
  if (ret) printf("failed to allocate memory of %d bytes, return %d\n", size, ret);
#if defined(ENABLE_VIRTIO_VHOST)
  else printf("check fd %d size %lX offset %lX p %p %X %X %X %X\n", pvm_carveout_fd, pvm_carveout_size, pvm_carveout_offset, pdata,
			  ((unsigned long*)pdata)[0], ((unsigned long*)pdata)[1], ((unsigned long*)pdata)[2], ((unsigned long*)pdata)[3]);
#endif
  return pdata;
}

int32_t vc_export (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void** pv, uint32_t cached){
  int32_t ret = 0;
  char *pdata = NULL;
  uint32_t exp_id = 0;
  struct export_item *it = NULL;
  struct export_item *item = NULL;

  if ((0 == vc_id) || (size <= 0) || (NULL == export_id)) {
	printf("invalid argument! vc_id = %d, size = %d, export_id = %p\n", vc_id, size, export_id);
	return -1;
  }

  item = malloc(sizeof(*item));
  if (item)  memset(item, 0, sizeof(*item));
  else {
      printf("failed to alloc memory of %zd bytes\n", sizeof(*item));
      return -1;
  }

  pdata = vc_allocate_memory(vc_id, size, cached, 0);
  if (!pdata) {
	  printf("%s: failed to allocate source buffer of %d bytes!\n", __func__, size);
	  ret = -1;
	  goto err_vc_alloc;
  }
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
  struct pmem_container *pmem = pdata;
  pdata = pmem->va;
#endif

  memset(pdata, pattern, size); // fill in the pattern for testing
  printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

  // memory pattern testing
  uint32_t *p32;
  int i;

  printf("access memory on short boundary...\n");
  p32 = (uint32_t*)(pdata + 2);
  for (i=0; i<100; i++) {
    p32[i] = 0x12345678;
  }
  printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

  printf("access memory on byte boundary...\n");
  p32 = (uint32_t*)(pdata + 1);
  for (i=0; i<100; i++) {
    p32[i] = 0x12345678;
  }
  printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
  ret = habmm_export(vc_id, pmem->handle, size, &exp_id, HABMM_EXPIMP_FLAGS_FD);
#else
  ret = habmm_export(vc_id, pdata, size, &exp_id, 0);
#endif

  //++
  *(unsigned int*)(&pdata[0]) = 0xBEEF;
  *(unsigned int*)(&pdata[size-4]) = 0xDEAD;

  printf("preset contents @ %p to %X, and %p to %X\n", &pdata[0], *(unsigned int*)(&pdata[0]), &pdata[size-4], *(unsigned int*)(&pdata[size-4]));
  //--

  printf("export id is %X, memory address is %p\n", exp_id, pdata);
  *export_id = exp_id;

#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
  item->uva = pmem;
#else
  item->uva = pdata;
#endif
  item->exp_id = exp_id;
  item->size = size;
  item->next = NULL;

// add to the list for book-keeping
	pthread_mutex_lock(&mutex);
  if(!g_export_list) {
      g_export_list = item;
  } else {
      it = g_export_list;
      while(it->next) it = it->next; // go to the tail
      it->next = item;
  }
  item_cnt++;
	pthread_mutex_unlock(&mutex);

  if(pv)
#if defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
	*pv = pmem->va;
#else
   *pv = pdata;
#endif

  return ret;

err_vc_alloc:
	free(item);
	return ret;
}

#define ION_MEMORY_KEY 0xabbabeef
#define ION_MEMORY_MULTIPLE_EXPORT_KEY 0xbeefbeef
#define DMABUFHEAP_MEMORY_KEY 0xabbaabba

char *g_pdata = NULL;
int32_t g_size = 0;

#ifdef TEST_ION_EXPORT

char *vc_allocate_ion_memory(int32_t size, uint8_t cached, struct allocation_data *ion_data)
{
	char *pdata = NULL;
	// allocate memory using ion method
	if (ion_fd >= 0)
	{
		// allocate memory
		int rc = 0;

		ion_data->len = (size_t)size;
		ion_data->flags = (unsigned int)cached;

#if TARGET_ION_ABI_VERSION >= 2
		rc = ion_alloc_fd(ion_fd, ion_data->len, ION_ALLOC_ALIGN, ion_data->heap_id_mask, ion_data->flags, &ion_data->fd);
#else
		rc = ioctl(ion_fd, ION_IOC_ALLOC, ion_data);
#endif

		if (rc)
		{
			printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc=%d errno %d %s ion-fd %d, len %d, id %X flag %d fd %d\n",rc, errno, strerror(errno),
			ion_fd, ion_data->len, ion_data->heap_id_mask, ion_data->flags, GET_ION_FD((*ion_data)));
		}
		else
		{
#if TARGET_ION_ABI_VERSION >= 2
			pdata = (char *)mmap(NULL,
			    size,
			    PROT_READ|PROT_WRITE, MAP_SHARED, ion_data->fd, 0);
			// store the pdata to global
			g_pdata = pdata;
			g_size = size;
#else
			struct ion_fd_data fd_data;
			fd_data.handle = ion_data->handle;
			fd_data.fd = 0;

			printf("vc_export_ion.ioctl.ION_IOC_ALLOC.passed.handle=0x%x.data.flags=%d\n", ion_data->handle, ion_data->flags);

			rc = ioctl(ion_fd, ION_IOC_MAP, &fd_data);
			if (rc)
			{
				printf("vc_export_ion.ioctl.ION_IOC_MAP handle %d failed rc=%d errno %d %s\n",fd_data.handle, rc, errno, strerror(errno));
			}
			else
			{
				printf("vc_export_ion.ioctl.ION_IOC_MAP.passed.fd=%d\n",fd_data.fd);

				pdata = (char *)mmap(NULL,
						size,
						PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
				// store the pdata to global
				g_pdata = pdata;
				g_size = size;
			}
#endif
			if (!pdata)
			{
				printf("vc_export_ion.mmap.failed ion fd %d will be closed\n", ion_fd);

				// free the memory
#ifndef TARGET_ION_ABI_VERSION
				ioctl(ion_fd, ION_IOC_FREE, ion_data->handle);
				ion_data->handle = -1;
#else
				close(ion_data->fd);
				ion_data->fd = -1;
#endif
				ion_data->len = 0;
			}
		}
	}
	return pdata;
}

int32_t vc_export_ion1 (int32_t vc_id, char *pdata, int32_t size, int32_t pattern, int32_t* export_id, int32_t is_multiple_export, struct allocation_data *ion_data)
{
	int32_t ret;
	uint32_t exp_id;
	struct export_item *it;
	struct export_item *item = malloc(sizeof(*item));

	if (!pdata) {
		printf("failed to allocate source buffer of %d bytes!\n", size);
		return -1;
	}

	if (item) memset(item, 0, sizeof(*item));
	else {
		printf("vc_export_ion.failed to alloc memory of %zd bytes\n", sizeof(*item));
		return -1;
	}

	memset(pdata, pattern, size); // fill in the pattern for testing
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	// memory pattern testing
	uint32_t *p32;
	int i;

	printf("access memory on short boundary...\n");
	p32 = (uint32_t*)(pdata + 2);
	for (i=0; i<100; i++) {
		p32[i] = 0x12345678;
	}
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	printf("access memory on byte boundary...\n");
	p32 = (uint32_t*)(pdata + 1);
	for (i=0; i<100; i++) {
		p32[i] = 0x12345678;
	}
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	ret = habmm_export(vc_id, pdata, size, &exp_id, HABMM_EXP_MEM_TYPE_DMA);
	if (ret) printf("export failed %d vcid %X size %d\n", ret, vc_id, size);

	//++
	*(unsigned int*)(&pdata[0]) = 0xBEEF;
	*(unsigned int*)(&pdata[size-4]) = 0xDEAD;

	printf("preset contents @ %p to %X, and %p to %X\n", &pdata[0], *(unsigned int*)(&pdata[0]), &pdata[size-4], *(unsigned int*)(&pdata[size-4]));
	//--

	printf("export id is %X, memory address is %p\n", exp_id, pdata);
	*export_id = exp_id;

	if (is_multiple_export)
		item->alloc_index = ION_MEMORY_MULTIPLE_EXPORT_KEY;
	else
		item->alloc_index = ION_MEMORY_KEY;
	item->uva = pdata;
	item->exp_id = exp_id;
	item->size = size;
	item->next = NULL;
	item->data = *ion_data; //store ion info the list for book keeping

	// add to the list for book-keeping
	pthread_mutex_lock(&mutex);
	if(!g_export_list) {
		g_export_list = item;
	} else {
		it = g_export_list;
		while(it->next) it = it->next; // go to the tail
		it->next = item;
	}
	item_cnt++;
	pthread_mutex_unlock(&mutex);

	return ret;
}

int32_t vc_export_ion (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void** pv, uint32_t cached) {
	int32_t ret;
	char *pdata = NULL; // = malloc(size);

	struct allocation_data data = {
      .len = 0,
      .flags = 0,
#if TARGET_ION_ABI_VERSION >= 2
      .heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID),
#else
      .align = 4096,
      .heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID),
      .handle = -1,
#endif
	};

	pdata = vc_allocate_ion_memory(size, cached, &data);
	if (!pdata) {
		printf("failed to allocate source buffer of %d bytes!\n", size);
		return -1;
	}
	printf("To test ion memory allocated size same as export ...\n");
	ret = vc_export_ion1 (vc_id, pdata, size, pattern, export_id, 0, &data); //ion data is stored inside this function for book keeping
	if(pv) *pv = pdata;

	return ret;
}

int32_t vc_export_ion_multiple(int32_t vc_id, int32_t num_export, int32_t unit_export_size, int32_t pattern, int32_t export_id[], void* pv[], uint32_t cached) {
	int32_t ret = 0;
	int32_t rc = 0;
	char *pdata = NULL; // = malloc(size);
	char *pdata_export = NULL;
	int32_t unit_export_id=0;
	int32_t i;

	struct allocation_data data = {
      .len = 0,
      .flags = 0,
#if TARGET_ION_ABI_VERSION >= 2
      .heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID),
#else
      .align = 4096,
      .heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID),
      .handle = -1,
#endif
	};

	int32_t total_size = num_export * unit_export_size;
	pdata = vc_allocate_ion_memory(total_size, cached, &data);
	if (!pdata) {
		printf("vc_export_ion_multiple.vc_allocate_ion_memory.failed to allocate source buffer of %d bytes!\n", total_size);
		return -1;
	}
	printf("vc_export_ion_multiple.allocate_ion_memory=%p.size=0x%x\n",pdata,total_size);
	for (i=0; i<num_export; i++)
	{
		pdata_export = pdata + i*unit_export_size;
		ret = vc_export_ion1 (vc_id, pdata_export, unit_export_size, pattern, &unit_export_id, 1, &data);
		ret = vc_id;
		ret = 0;
		printf("vc_export_ion_multiple.export:#=%d pdata=%p size=0x%x pattern=%d, export id=%d\n",i+1,pdata_export,unit_export_size,pattern,unit_export_id);
		if(ret != 0) rc = ret;
		if(pv) pv[i] = pdata_export;
		if(export_id) export_id[i] = unit_export_id;
		pattern += i;
	}
	return rc;
}

// assume that only one export_ion_multiple exists before calling vc_unexport_ion_multiple
int32_t vc_unexport_ion_multiple(int32_t vc_id) {
	// get the export id from the global list and unexport it
	int32_t ret = 0;
	struct export_item *it, *itpre;
	int32_t export_id;

	struct allocation_data data = {
		.len = 0,
		.flags = 0,
#if TARGET_ION_ABI_VERSION >= 2
		.heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID),
#else
		.heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID),
		.align = 4096,
		.handle = -1,
#endif
	};

	printf("%s\n", __FUNCTION__);

	pthread_mutex_lock(&mutex);
	if(!g_export_list || 0>=item_cnt) {
		printf("Error in export item list head %p, cnt %d\n", g_export_list, item_cnt);
	}
	it = itpre = g_export_list;
	while (it) {
		export_id = it->exp_id;
		pthread_mutex_unlock(&mutex);
		ret = habmm_unexport(vc_id, export_id, 0);
		pthread_mutex_lock(&mutex);
		if (ret == 0) {
			printf("%s vcid=%d export_id=%d uva = %p size=%d\n", __FUNCTION__, vc_id, export_id, it->uva, it->size);
			// detach the item
			if (g_export_list == it) {
				g_export_list = it->next;
			} else {
				itpre->next = it->next;
			}
			data = it->data; //fetch at least one ion info since this is one ion alloc but multiple exp
			free(it);
			item_cnt--;
		}

		itpre = it;
		it = it->next;
	}
	pthread_mutex_unlock(&mutex);

	// free the memory after all multiple export have been unexported
	if (ret == 0)
	{
		if (g_pdata != NULL)
		{
			ret = munmap(g_pdata, g_size);
		}
		if (ion_fd >= 0)
		{
#ifndef TARGET_ION_ABI_VERSION
			ioctl(ion_fd, ION_IOC_FREE, &data.handle);
			data.handle = -1;
#else
			close(data.fd);
			data.fd = -1;
#endif
			data.len = 0;
		}
	}
	return ret;
}
#endif

#ifdef TEST_DMAHEAP_EXPORT
static char *vc_allocate_dmaheap_memory(int32_t size, struct dma_heap_allocation_data *data, uint32_t cached)
{
	char *pdata = NULL;
	int rc = 0;
	int fd = -1;

	fd = cached ? dmaheap_fd_cached : dmaheap_fd;
	if (fd >= 0)
	{
		rc = ioctl(fd, DMA_HEAP_IOCTL_ALLOC, data);
		if (rc) {
			printf("vc_export_dmaheap.ioctl.DMA_HEAP_IOCTL_ALLOC.failed rc=%d errno %d %s dmaheap_fd %d, len %d\n",rc, errno, strerror(errno),
			fd, data->len);
		} else {
			pdata = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, data->fd, 0);
			/* store the pdata to global */
			g_pdata = pdata;
			g_size = size;
		}
	}

	return pdata;
}

static int32_t vc_export_dmaheap1(int32_t vc_id, char *pdata, int32_t size, int32_t pattern, int32_t* export_id, struct dma_heap_allocation_data *data)
{
	int32_t ret;
	uint32_t exp_id;
	struct export_item *it;
	struct export_item *item = malloc(sizeof(*item));

	if (!pdata) {
		printf("failed to allocate source buffer of %d bytes!\n", size);
		return -1;
	}

	if (item)
		memset(item, 0, sizeof(*item));
	else {
		printf("vc_export_dmaheap.failed to alloc memory of %zd bytes\n", sizeof(*item));
		return -1;
	}

	memset(pdata, pattern, size); // fill in the pattern for testing
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	// memory pattern testing
	uint32_t *p32;
	int i;

	printf("access memory on short boundary...\n");
	p32 = (uint32_t*)(pdata + 2);
	for (i=0; i<100; i++) {
		p32[i] = 0x12345678;
	}
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	printf("access memory on byte boundary...\n");
	p32 = (uint32_t*)(pdata + 1);
	for (i=0; i<100; i++) {
		p32[i] = 0x12345678;
	}
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	ret = habmm_export(vc_id, pdata, size, &exp_id, HABMM_EXP_MEM_TYPE_DMA);
	if (ret)
		printf("export failed %d vcid %X size %d\n", ret, vc_id, size);

	//++
	*(unsigned int*)(&pdata[0]) = 0xBEEF;
	*(unsigned int*)(&pdata[size-4]) = 0xDEAD;

	printf("preset contents @ %p to %X, and %p to %X\n", &pdata[0], *(unsigned int*)(&pdata[0]), &pdata[size-4], *(unsigned int*)(&pdata[size-4]));
	//--

	printf("export id is %X, memory address is %p\n", exp_id, pdata);
	*export_id = exp_id;

	item->alloc_index = DMABUFHEAP_MEMORY_KEY;
	item->uva = pdata;
	item->exp_id = exp_id;
	item->size = size;
	item->next = NULL;
	item->dmaheap_data = *data; //store dmaheap info the list for book keeping

	// add to the list for book-keeping
	pthread_mutex_lock(&mutex);
	if(!g_export_list) {
		g_export_list = item;
	} else {
		it = g_export_list;
		while(it->next) it = it->next; // go to the tail
		it->next = item;
	}
	item_cnt++;
	pthread_mutex_unlock(&mutex);

	return ret;
}

int32_t vc_export_dmaheap(int32_t vc_id, int32_t size, int32_t pattern, uint32_t cached, int32_t *export_id, void **pv)
{
	int32_t ret;
	char *pdata = NULL;
	struct dma_heap_allocation_data data = {
		.len = (size_t)size,
		.fd_flags = O_RDWR,
	};

	pdata = vc_allocate_dmaheap_memory(size, &data, cached);
	if (!pdata) {
		printf("failed to allocate source buffer of %d bytes!\n", size);
		return -1;
	}
	printf("To test dmaheap memory allocated size same as export ...\n");
	ret = vc_export_dmaheap1(vc_id, pdata, size, pattern, export_id, &data);
	if(pv) *pv = pdata;

	return ret;
}
#endif

int32_t vc_export_from_fd (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, uint32_t cached)
{
    uint32_t *p32;
    int i;
    int32_t ret;
    char *pdata = NULL;
    uint32_t exp_id;
    struct export_item *it;
    struct export_item *item = malloc(sizeof(*item));
#ifdef TEST_ION_EXPORT
	int rc = 0;
#ifndef TARGET_ION_ABI_VERSION
	struct ion_fd_data fd_data;
#endif
	struct allocation_data data = {
      .len = 0,
      .flags = 0,
#if TARGET_ION_ABI_VERSION >= 2
      .heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID),
#else
      .align = 4096,
      .heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID),
      .handle = -1,
#endif
	};
#elif defined TEST_DMAHEAP_EXPORT
	struct dma_heap_allocation_data data = {
		.len = (size_t)size,
		.fd_flags = O_RDWR,
	};
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	pmem_handle_t pmemhdl = NULL;
#endif

    if (item) {
        memset(item, 0, sizeof(*item));
    } else {
        printf("failed to alloc memory of %zd bytes\n", sizeof(*item));
        return -1;
    }

#ifdef TEST_ION_EXPORT
    if (ion_fd < 0) {
        ion_fd = open("/dev/ion", O_RDONLY);
        if (ion_fd < 0) {
            printf("vc_export_ion.failed to open ion device! return ion fd=%d\n", ion_fd);
            return -1;
        } else {
            printf("vc_export_ion.open.passed ion_node_fd=%d\n",ion_fd);
        }
    }

	data.len = (size_t)size;
	data.flags = (unsigned int)cached;
#if TARGET_ION_ABI_VERSION >= 2
	rc = ion_alloc_fd(ion_fd, data.len, ION_ALLOC_ALIGN, data.heap_id_mask, data.flags, &data.fd);
#else
	rc = ioctl(ion_fd, ION_IOC_ALLOC,&data);
#endif
	if (rc) {
		printf("vc_export_ion.ioctl.ION_IOC_ALLOC.failed rc=%d ion-fd %d size %d heap %X flag %d fd %d\n",
		rc, ion_fd, data.len, data.heap_id_mask, data.flags, GET_ION_FD(data));
		return -1;
	}

#if TARGET_ION_ABI_VERSION >= 2
	pdata = (char *)mmap(NULL,
						 size,
						 PROT_READ|PROT_WRITE, MAP_SHARED, data.fd, 0);
	g_pdata = pdata;
	g_size = size;
#else
	fd_data.handle = data.handle;
	fd_data.fd = 0;

	printf("vc_export_ion.ioctl.ION_IOC_ALLOC.passed.fd=0x%x.data.flags=%d\n",data.handle, data.flags);

	rc = ioctl(ion_fd, ION_IOC_MAP, &fd_data);
	if (rc) {
		printf("vc_export_ion.ioctl.ION_IOC_MAP.failed rc=%d\n",rc);
	} else {
		printf("vc_export_ion.ioctl.ION_IOC_MAP.passed.fd=%d\n",fd_data.fd);

		pdata = (char *)mmap(NULL,
							 size,
							 PROT_READ|PROT_WRITE, MAP_SHARED, fd_data.fd, 0);
		g_pdata = pdata;
		g_size = size;
	}
#endif

	if (!pdata) {
		printf("vc_export_ion.mmap.failed\n");

		// free the memory
#ifndef TARGET_ION_ABI_VERSION
		ioctl(ion_fd, ION_IOC_FREE, &data.handle);
		data.handle = -1;
#else
		close(data.fd);
		data.fd = -1;
#endif
		data.len = 0;
		return -1;
	}
#elif defined TEST_DMAHEAP_EXPORT
	pdata = vc_allocate_dmaheap_memory(size, &data, cached);
	if (!pdata) {
		printf("failed to allocate source buffer of %d bytes!\n", size);
		return -1;
	}
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	struct hab_socket_info info = {0};
	uint32_t flags, pmem_id = PMEM_GRAPHICS_FRAMEBUFFER_ID;

	if (is_guest)
		flags = 0;
	else
		flags = PMEM_FLAGS_TYPED_MEM;

	if(cached)
		flags |= PMEM_FLAGS_CACHE_WB_WA;

	ret = habmm_socket_query(vc_id, &info, 0);
	pdata = pmem_malloc_ext_v2(size, pmem_id, flags, PMEM_ALIGNMENT_4K,
							   1 << info.vmid_remote,
							   &pmemhdl,
							   NULL);
	if (!pdata) {
		printf("pmem alloc ext v2 failed size %d flags %X remote vmid %d\n",
			   size, flags, info.vmid_remote);
		return -1;
	}
#endif

	memset(pdata, pattern, size); // fill in the pattern for testing
	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	printf("access memory on short boundary...\n");
	p32 = (uint32_t*)(pdata + 2);
	for (i = 0; i < 100; i++)
		p32[i] = 0x12345678;

	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

	printf("access memory on byte boundary...\n");
	p32 = (uint32_t*)(pdata + 1);
	for (i=0; i<100; i++)
		p32[i] = 0x12345678;

	printf("check memory sanity %d %d %d %d\n", pdata[0],pdata[1],pdata[2],pdata[3]);

#ifdef TEST_ION_EXPORT
#if TARGET_ION_ABI_VERSION >= 2
	ret = habmm_export(vc_id, *(void**)&data.fd, size, &exp_id, HABMM_EXPIMP_FLAGS_FD);
#else
	ret = habmm_export(vc_id, *(void**)&fd_data.fd, size, &exp_id, HABMM_EXPIMP_FLAGS_FD);
#endif
	if (ret) {
		printf("exp %d bytes on vcid %X failed %d\n", size, vc_id, ret);
		return -1;
	}
#elif defined TEST_DMAHEAP_EXPORT
	ret = habmm_export(vc_id, *(void **)&data.fd, size, &exp_id, HABMM_EXPIMP_FLAGS_FD);
	if (ret) {
		printf("exp %d bytes on vcid %X failed %d\n", size, vc_id, ret);
		return -1;
	}
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	ret = habmm_export(vc_id, (void*)pmemhdl, size, &exp_id, HABMM_EXPIMP_FLAGS_FD);
	if (ret) {
	  printf("exp %d bytes of handle %p on vcid %X failed %d\n", size, pmemhdl, vc_id, ret);
		return -1;
	}
#endif

	*(unsigned int*)(&pdata[0]) = 0xBEEF;
	*(unsigned int*)(&pdata[size-4]) = 0xDEAD;
	printf("preset contents @ %p to %X, and %p to %X\n", &pdata[0], *(unsigned int*)(&pdata[0]), &pdata[size-4], *(unsigned int*)(&pdata[size-4]));

	printf("export id is %X, memory address is %p\n", exp_id, pdata);
	*export_id = exp_id;

	item->uva = pdata;
	item->exp_id = exp_id;
	item->size = size;
	item->next = NULL;
#ifdef TEST_ION_EXPORT
	item->alloc_index = ION_MEMORY_KEY;
	item->data = data;
#endif
#ifdef TEST_DMAHEAP_EXPORT
	item->alloc_index = DMABUFHEAP_MEMORY_KEY;
	item->dmaheap_data = data;
#endif
	// add to the list for book-keeping
	pthread_mutex_lock(&mutex);
	if(!g_export_list) {
		g_export_list = item;
	} else {
		it = g_export_list;
		while(it->next) it = it->next; // go to the tail
		it->next = item;
	}
	item_cnt++;
	pthread_mutex_unlock(&mutex);
    return 0;
}

int32_t vc_free_memory(int32_t size, void *uva, uint64_t alloc_index){
    int32_t ret;
#if defined(USE_QNX_PMEM_ALLOC) || defined(__INTEGRITY)
    ret = unmap_free_pmem(size, uva);
#elif defined(__linux__)
    ret = munmap(uva, size);
#elif defined(WIN32)
	ret = page_allocator_free(uva);
#endif
//#if defined(ENABLE_VIRTIO_GUEST)
//    if (memfd)
//        close(memfd);
//#endif
    return ret;
}

int32_t vc_unexport (int32_t vc_id, int32_t export_id) {
	int32_t ret;
	struct export_item *it, *itpre;

#ifdef _DEBUG
	printf("%s vcid %X, export id %d\n", __FUNCTION__, vc_id, export_id);
#endif
	ret = habmm_unexport(vc_id, export_id, 0); // unexp right away
	if(ret) {
		printf("failure in unexp vcid %x expid %d\n", vc_id, export_id);
	}

	pthread_mutex_lock(&mutex);

	if(!g_export_list || 0>=item_cnt) {
		printf("Error in export item list head %p, cnt %d\n", g_export_list, item_cnt);
		pthread_mutex_unlock(&mutex);
		return -1;
	}
	// find the item
	it = itpre = g_export_list;
	while (it->exp_id != (unsigned int)export_id) {
		itpre = it;
		it = it->next;
		if (!it) {
			printf("hit the end of the exp list failed to find exp id %d item cnt %d\n", export_id, item_cnt);
			pthread_mutex_unlock(&mutex);
			return -1;
		}
	}
	// detach the item
	if (it && it->exp_id != (unsigned int)export_id) {
		printf("failed to find export id %d in list head %p, cnt %d, last item checked %p\n", export_id, g_export_list, item_cnt, it);
			pthread_mutex_unlock(&mutex);
		return -1;
	}
#ifdef _DEBUG
	printf("find export id %d in item head %p, cnt %d, pre %p, next %p\n", export_id, it, item_cnt, itpre, it->next);
#endif
	if (g_export_list == it) {
		g_export_list = it->next;
	} else {
		itpre->next = it->next;
	}
	item_cnt--;
	pthread_mutex_unlock(&mutex);

	// free or unmap
	ret = vc_free_memory(it->size, it->uva, it->alloc_index);

#ifdef TEST_ION_EXPORT
	if (it->alloc_index == ION_MEMORY_KEY)
	{	// free ion resource
#ifndef TARGET_ION_ABI_VERSION
		int ret2 = ioctl(ion_fd, ION_IOC_FREE, &it->data.handle);
		if (ret2)
			printf("free ion fd %d len %d align %d mask %X flag %d fd %d ret %d expid %d errno %d %s\n",
				   it->data.handle, it->data.len, it->data.align, it->data.heap_id_mask, it->data.flags,
				   ion_fd, ret2, export_id, errno, strerror(errno));
		it->data.handle = -1;
#else
		close(it->data.fd);
		it->data.fd = -1;
#endif
		it->data.len = 0;
	}
#endif
#ifdef TEST_DMAHEAP_EXPORT
	if (it->alloc_index == DMABUFHEAP_MEMORY_KEY) {
		close(it->dmaheap_data.fd);
		it->dmaheap_data.fd = -1;
	}
#endif
	printf("unmap returns %d\n", ret);
	free(it);

	return ret;
}

int32_t vc_import (int32_t vc_id, int32_t export_id, int32_t size){
  int32_t ret;
  char *p = NULL;

#if defined(ENABLE_HOST_VM)
  pmem_handle_t handle = NULL;
  int sz = 0;

  ret = habmm_import(vc_id, (void**)&handle, size, export_id, HABMM_EXPIMP_FLAGS_FD);
  if (!ret) {
	p = pmem_map_handle(handle, &sz); //if GVM is ION system heap, this fails
	if (!p || sz != size) {
		printf("pmem map handle %p failed ret %p sz %d expected size %d\n", handle, p, sz, size);
		return -1;
	}
  } else {
	printf("failed to import export id %d to pmem handle ret %d\n", export_id, ret);
	return -1;
  }
#else
#ifdef CONFIG_HGY_PLATFORM
  ret = habmm_import(vc_id, (void**)&p, size, export_id, HABMM_IMPORT_FLAGS_CACHED);
#else
  ret = habmm_import(vc_id, (void**)&p, size, export_id, 0);
#endif
#endif
	if (p) {
		printf("imported %d bytes from export id %d, memory address mapped is %p\n", size, export_id, p);
		printf("contents @ %p = 0x%X, and %p = 0x%X\n", &p[0], *(unsigned int*)(&p[0]), &p[size-4], *(unsigned int*)(&p[size-4]));
	} else
		printf("failed imported %d bytes from export id %d ret %d\n", size, export_id, ret);
  return ret;
}

int32_t vc_import_to_fd(int32_t vc_id, int32_t export_id, int32_t size)
{
    int ret = 0, sz = 0;
#if defined(__linux__) || defined(WIN32)
	void *fd = NULL;
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	pmem_handle_t fd = NULL;
#endif
    char *p;

    ret = habmm_import(vc_id, (void**)&fd, size, export_id, HABMM_EXPIMP_FLAGS_FD);
    KHAB_API_ERR_CHECK(ret);

#if defined(__QNXNTO__) || defined(__INTEGRITY)
    printf("import return fd %p on vcid %X, export_id %d, size %d\n", fd, vc_id, export_id, size);
#else
    printf("import return fd %ld on vcid %X, export_id %d, size %d\n", (int64_t)fd, vc_id, export_id, size);
#endif

#ifdef __linux__
    p = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, (int)fd, 0);
    if(MAP_FAILED == p) {
        printf("import return fd %d on vcid %X, export_id %d, mmap failed\n", (int)fd, vc_id, export_id);
        return -1;
    }
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	p = pmem_map_handle(fd, &sz);
	if (!p || sz != size) {
		printf("pmem map handle %p failed ret %p sz %d expected size %d\n", fd, p, sz, size);
		return -1;
	}
#endif
    printf("contents @ %p = 0x%X, and %p = 0x%X\n", &p[0], *(unsigned int*)(&p[0]), &p[size-4], *(unsigned int*)(&p[size-4]));

#ifdef __linux__
    munmap(p, size);
#elif defined(__QNXNTO__) || defined(__INTEGRITY)
	ret = pmem_unmap_handle(fd, p);
	if (ret) {
		printf("pmem_unmap_handle %p failed ret %d va %p\n", fd, ret, p);
	}
#endif
    return 0;
}

int32_t vc_unimport(int32_t vc_id, int32_t export_id, uint64_t buff_addr) {
  int32_t ret;
  char *p = (char*)(intptr_t)buff_addr;
  ret = habmm_unimport(vc_id, export_id, p, 0);
  printf("unimport ret %d\n", ret);
  return ret;
}

int32_t vc_unimport_from_fd(int32_t vc_id, int32_t export_id, uint64_t buff_addr) {
  int32_t ret;
  ret = habmm_unimport(vc_id, export_id, (void*)((intptr_t)buff_addr), HABMM_EXPIMP_FLAGS_FD);
  printf("unimport_from_fd ret %d\n", ret);
  return ret;
}

void    vc_check    (int32_t size, int32_t tick, uint64_t buff_addr) {
	char *p = (char*)(intptr_t)buff_addr;

	printf("original   [0] 0x%X, [1]0x%X, ..., [%X] 0x%X, [%X] 0x%X\n", p[0], p[1], size-2, p[size-2], size-1, p[size-1]);

	// make a change to the first and last items
	p[0] = p[0] + tick;
	p[size-1] = p[size-1] + tick;

	printf("after tick [0] 0x%X, [1]0x%X, ..., [%X] 0x%X, [%X] 0x%X\n", p[0], p[1], size-2, p[size-2], size-1, p[size-1]);
}

static int push_expid(int32_t vcid, uint32_t expid, void *kva) {

	struct export_item *it;
	struct export_item *item = malloc(sizeof(*item));

	if (item) memset(item, 0, sizeof(*item));
	else {
		printf("vc_export_ion.failed to alloc memory of %zd bytes\n", sizeof(*item));
		return -1;
	}
	item->vcid = vcid;
	item->uva = kva;
	item->exp_id = expid;
	item->next = NULL;

	// add to the list for book-keeping
	pthread_mutex_lock(&mutex);
	if(!g_export_list) {
		g_export_list = item;
	} else {
		it = g_export_list;
		while(it->next) it = it->next; // go to the tail
		it->next = item;
	}
	item_cnt++; // test app is single threaded
	pthread_mutex_unlock(&mutex);

	return 0;
}

// based on vcid && expid to find kva
//-1: failed to find the matching one; index value: found, and pkva filled and returned
static int pop_expid(int32_t vcid, uint32_t expid, void **pkva) {
	struct export_item *it, *itpre;

	pthread_mutex_lock(&mutex);

  if(!g_export_list || 0>=item_cnt) {
      pthread_mutex_unlock(&mutex);
      printf("Error in export item list head %p, cnt %d\n", g_export_list, item_cnt);
      return -1;
  }
  // find the item
  it = itpre = g_export_list;
  while (it->exp_id != (unsigned int)expid) {
      itpre = it;
      it = it->next;
      if (!it) break;
  }
  // detach the item
  if (it && it->exp_id == (unsigned int)expid) {

#ifdef _DEBUG
      printf("find export id %d in item head %p, cnt %d, pre %p, next %p\n", expid, it, item_cnt, itpre, it->next);
#endif
      if (g_export_list == it) {
          g_export_list = it->next;
      } else {
          itpre->next = it->next;
      }
      item_cnt--;

	  *pkva = it->uva;
      free(it);
  } else {
      printf("failed to find export id %d in list head %p, cnt %d, last item checked %p\n", expid, g_export_list, item_cnt, it);
	  pthread_mutex_unlock(&mutex);
      return -1;
  }
	pthread_mutex_unlock(&mutex);

  return 0;
}

uint64_t timespec_diff_us (struct timespec *start, struct timespec *stop) {
    uint64_t us;
    if (stop->tv_nsec < start->tv_nsec) {
        us  = (stop->tv_sec - start->tv_sec - 1)*1000000 + (1000000000 - start->tv_nsec + stop->tv_nsec)/1000;
    } else {
        us  = (stop->tv_sec - start->tv_sec)*1000000 + (stop->tv_nsec - start->tv_nsec)/1000;
    }
    return us;
}

uint64_t timeval_diff_us (struct timeval *start, struct timeval *stop) {
    uint64_t us;
    if (stop->tv_usec < start->tv_usec) {
        us  = (stop->tv_sec - start->tv_sec - 1)*1000000 + (1000000 - start->tv_usec + stop->tv_usec);
    } else {
        us  = (stop->tv_sec - start->tv_sec)*1000000 + (stop->tv_usec - start->tv_usec);
    }
    return us;
}

#define MPM_TIMER_HZ 32768
uint64_t mpm_timer_to_us(unsigned long long start, unsigned long long end) {
	if (start >= end)
		return 0;
	return (end - start) * 1000000 / MPM_TIMER_HZ;
}

// define echo - ping test command structures
struct _TESTCMD_ACK {
    uint32_t cmd;
    uint64_t imp_us; // import call spent in us
    uint64_t unimp_us; // unimport call spent in us
};

// define the package number in throuput test
#define TPUT_TX_NUM                     0x8000

int32_t vc_echo(int32_t vc_id) {
	char data[HAB_MAX_MSG_SIZEBYTES] = {0};
	uint32_t size = HAB_MAX_MSG_SIZEBYTES;
	uint32_t expid_array[2] = {0};
	uint32_t expid, num_pgs;
	void *kva = NULL;
	int r, ret, i, count;
	uint64_t imp_us = 0, unimp_us = 0;
	int retry_cnt = 0;
	int tput_counter;
	unsigned long long rx_tv0;
	int loops = 0; //TPUT_TX_NUM
	uint32_t testcmd = 0, testcmd_pre = 0;
	float burst, burst_total = 0.0, burst_min = 0.0, burst_max = 0.0;

	printf("start listening at hab_socket %x...\n", vc_id);
	for (count = 0; ; count++) {
        struct timespec start_imp, stop_imp, start_unimp, stop_unimp;

		testcmd_pre = testcmd;
		testcmd = 0;

		// 1. identify the test command from FE first
		size = HAB_MAX_MSG_SIZEBYTES; //sizeof(testcmd);
		ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);

		if (0 != ret) {
			printf("failed to receive a command, return value %X\n, retry %d...", ret, retry_cnt++);
			if (-EPIPE == ret) {
				printf("vchan error! exits...\n");
				break;
			} else {
				if (retry_cnt > 10)
					break;
				else
					continue; // re-try
			}
		} else if (size != sizeof(testcmd)) {
			printf("receive a command %d, but size mismatch with intended %d\n, retry...", size, (int)sizeof(testcmd));
		} else {
#ifdef _DEBUG
			printf("Received %d bytes, cmd %c%c%c%c\n", size, data[3], data[2], data[1], data[0]);
#endif
		}

		testcmd = *(uint32_t*)data;
		if (KHAB_TEST_CMD_SEND == testcmd)  {
			// send/recv testing
			size = HAB_MAX_MSG_SIZEBYTES;
			ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
			if(ret) { // if FE quits, this will fail
				printf("habmm_socket_recv failed\n");
				break;
			} else {
                if(size >= HAB_MAX_MSG_SIZEBYTES) printf("Error! received too many bytes %d, overflow %d\n", size, HAB_MAX_MSG_SIZEBYTES);
				data[size] = 0; // add NULL terminator since FE only send over exact string
#ifdef _DEBUG
				printf("received %d bytes %s\n", size, data);
#endif
			}

			//for (i=0; i<size; i++)
			//	data[i]++; // change the pattern a little

			ret = habmm_socket_send(vc_id, data, size, 0);
			if(ret) {
				printf("habmm_socket_send failed\n");
				break;
			}
		} else if (KHAB_TEST_CMD_PROF == testcmd) {
			size = HAB_MAX_MSG_SIZEBYTES;
			ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
            KHAB_API_ERR_CHECK(ret);

			// after receiving, store the incoming leave-VM1 and enter-VM2 to the 2 slots below, so the first 2 slots are available for the return trip measurement
			//			printf("incoming profile leave vm1 %ld %ld, enter vm2 %ld %ld\n", ((uint64_t*)data)[0],
			//	   ((uint64_t*)data)[1], ((uint64_t*)data)[2], ((uint64_t*)data)[3]);

			((uint64_t*)data)[4] = ((uint64_t*)data)[0]; // leave vm1 s
			((uint64_t*)data)[5] = ((uint64_t*)data)[1]; // leave vm1 ns
			((uint64_t*)data)[6] = ((uint64_t*)data)[2]; // enter vm2 s
			((uint64_t*)data)[7] = ((uint64_t*)data)[3]; // enter vm2 ns

			ret = habmm_socket_send(vc_id, data, size, HABMM_SOCKET_SEND_FLAGS_XING_VM_STAT);
            KHAB_API_ERR_CHECK(ret);
		} else if (KHAB_TEST_CMD_IMP == testcmd) {
			// wait for export id
			size = sizeof(expid_array);
			ret = habmm_socket_recv(vc_id, expid_array, &size, (uint32_t)-1, 0);
			KHAB_API_ERR_CHECK(ret);

			// get imported memory
			expid = expid_array[0];
			num_pgs = expid_array[1];
            clock_gettime(CLOCK_MONOTONIC, &start_imp);
			ret = habmm_import(vc_id, &kva, num_pgs*PAGE_SIZE,
							   expid, 0);
			KHAB_API_ERR_CHECK(ret);
			if (!kva)
				break;
            clock_gettime(CLOCK_MONOTONIC, &stop_imp);
            imp_us = timespec_diff_us(&start_imp, &stop_imp);
#ifdef _DEBUG
			printf("import return kva %p on vcid %X, expid %d, pg cnt %d\n", kva, vc_id, expid, num_pgs );
#endif
			if (0 == ret) {
#ifdef _DEBUG
				printf("import passes! expid %d, pgcnt %d, pattern at %p is %X, %p is %X\n",
					   expid, num_pgs, kva, ((uint32_t*)kva)[0],
					   &(((uint32_t*)kva)[num_pgs*PAGE_SIZE/4-1]), ((uint32_t*)kva)[num_pgs*PAGE_SIZE/4-1]);
#endif

				// add 1 to all the imported memory
				for (i=0; i<num_pgs*PAGE_SIZE; i++) {
					((unsigned char*)kva)[i] += 1;
				}

				r = push_expid(vc_id, expid, kva);
				if (-1 == r) {
					printf("Failed to store vcid %X, expid %d, kva %p\n", vc_id, expid, kva);
				} else {
#ifdef _DEBUG
					printf("store vcid %X, expid %d, kva %p at slot %d\n", vc_id, expid, kva, r);
#endif
				}
			} else {
				printf("import failed! on expid %d, pgcnt %d, kva %p\n",
							 expid, num_pgs, kva);
			}
		} else if (KHAB_TEST_CMD_UNIMP == testcmd) {
			struct _TESTCMD_ACK testcmd_tx;

#ifdef _DEBUG
			printf("wait for unimport parameters on vcid %X\n", vc_id);
#endif
			// 1. wait for export id
			size = sizeof(expid_array);
			ret = habmm_socket_recv(vc_id, expid_array, &size, (uint32_t)-1, 0);
			KHAB_API_ERR_CHECK(ret);

			// 2. get imported memory
			expid = expid_array[0];
			num_pgs = expid_array[1];
			r = pop_expid(vc_id, expid, &kva);
			if (-1 == r) {
				printf("failed to get kva based on expid %d! try again\n", expid);
				continue;
			} else {
#ifdef _DEBUG
				printf("start unimport on expid %d, kva %p...\n", expid, kva);
#endif
			}

            clock_gettime(CLOCK_MONOTONIC, &start_unimp);
			ret = habmm_unimport(vc_id, expid, kva, 0);
            clock_gettime(CLOCK_MONOTONIC, &stop_unimp);
			KHAB_API_ERR_CHECK(ret);
            unimp_us = timespec_diff_us(&start_unimp, &stop_unimp);
#ifdef _DEBUG
			printf("unimport return %d, send ack to exporter on vcid %X, expid %d, kva %p\n", ret, vc_id, expid, kva);
#endif
			// 3. send ack to exporter
			testcmd_tx.cmd = KHAB_TEST_CMD_ACK;
            testcmd_tx.imp_us = imp_us;
            testcmd_tx.unimp_us = unimp_us;
			ret = habmm_socket_send(vc_id, &testcmd_tx, sizeof(testcmd_tx), 0); // into khab public API
			KHAB_API_ERR_CHECK(ret);
#ifdef _DEBUG
			printf("unimport done on vcid %X, expid %d\n", vc_id, expid);
#endif
		} else if (KHAB_TEST_CMD_BURST == testcmd) {
			ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
			KHAB_API_ERR_CHECK(ret);

			loops = *((int *)data);
			printf("burst test loops %d\n", loops);
			tput_counter = 0;
            clock_gettime(CLOCK_MONOTONIC, &start_imp);
			while (tput_counter < loops) {
				// send/recv testing
				size = HAB_MAX_MSG_SIZEBYTES;
				ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
				if (ret) {
					printf("habmm_socket_recv failed\n");
					break;
				} else {
					if (size > HAB_MAX_MSG_SIZEBYTES)
						printf("Error! Received too many bytes %d\n", size);
					data[size] = 0;
#ifdef _DEBUG
					printf("received %d bytes %s\n", size, data);
#endif
				}
				tput_counter++;
			}
            clock_gettime(CLOCK_MONOTONIC, &stop_imp);
            imp_us = timespec_diff_us(&start_imp, &stop_imp);
			burst = imp_us*1.0/loops;
			printf("recv burst test %d loop %d bytes avg recv %f us\n",  loops, size, burst);

			ret = habmm_socket_send(vc_id, data, size, 0);
			if (ret) {
				printf("habmm_socket_send failed\n");
				break;
			}

			burst_total += burst;
			burst_min = (count == 1 || burst < burst_min ) ? burst : burst_min;
			burst_max = (burst > burst_max) ? burst : burst_max;
		} else if (KHAB_TEST_CMD_TPUT == testcmd) {
			tput_counter = 0;
			while (tput_counter < TPUT_TX_NUM) {
				// send/recv testing
				size = HAB_MAX_MSG_SIZEBYTES;
				ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
				if (ret) {
					printf("habmm_socket_recv failed\n");
					break;
				} else {
					if (size > HAB_MAX_MSG_SIZEBYTES)
						printf("Error! Received too many bytes %d\n", size);
					data[size] = 0;
#ifdef _DEBUG
					printf("received %d bytes %s\n", size, data);
#endif
				}
				tput_counter++;
			}

			memcpy(data, &size, sizeof(size));
			ret = habmm_socket_send(vc_id, data, sizeof(size), 0);
			if (ret) {
				printf("habmm_socket_send failed\n");
				break;
			} else
				printf("send the received message, size:%d\n", size);
		} else if (KHAB_TEST_CMD_SCHE == testcmd) {
			ret = habmm_socket_send(vc_id, data, sizeof(unsigned long long), 0); // sync message
			KHAB_API_ERR_CHECK(ret);

			size = HAB_MAX_MSG_SIZEBYTES;
			ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
			KHAB_API_ERR_CHECK(ret);
			rx_tv0 = ((unsigned long long*)data)[0];

			ret = habmm_socket_send(vc_id, data, sizeof(unsigned long long), HABMM_SOCKET_XVM_SCHE_TEST_ACK); // ack message from VM2 to VM1
			KHAB_API_ERR_CHECK(ret);

			// wait for terminate to request the test results
			size = sizeof(unsigned long long)*3;
			ret = habmm_socket_recv(vc_id, data, &size, (uint32_t)-1, 0);
			KHAB_API_ERR_CHECK(ret);

			// send the test results back
			((unsigned long long*)data)[1] = rx_tv0;
			ret = habmm_socket_send(vc_id, data, size, HABMM_SOCKET_XVM_SCHE_RESULT_RSP); // response message from VM2 to VM1
			KHAB_API_ERR_CHECK(ret);
		} else if (KHAB_TEST_CMD_EXIT == testcmd) {
			printf("exit command received!\n");
			break; // exit state machine
		} else {
			printf("unknow command received %x, re-try...\n", testcmd);
		}
	}

	// print min/avg/max
	if (KHAB_TEST_CMD_BURST == testcmd_pre) {
		printf("burst recv %d times burst recv min/avg/max %f %f %f us\n", count, burst_min, burst_total/count, burst_max);
	}
	printf("%s: exit echo mode on vcid 0x%X\n", __FUNCTION__, vc_id);
	return 0;
}

// support different loopback testing mode
#define PING_TEST_TYPE_TXRX             1
#define PING_TEST_TYPE_EXPIMP           2
#define PING_TEST_TYPE_XVM_OVERHEAD     3
#define PING_TEST_TYPE_TPUT             4
#define PING_TEST_TYPE_SCHE_LATENCY     5
#define PING_TEST_TYPE_BURST            6

//#ifdef __INTEGRITY
uint32_t databuf[HAB_MAX_MSG_SIZEBYTES];
//#endif

int32_t vc_ping     (int32_t vc_id, int32_t sec, int32_t test_type, int32_t page_cnt, int32_t interval_sec, int32_t print_period_sec, int32_t loops) { // loops is for PING_TEST_TYPE_BURST
	uint32_t i, i_last = 0;
	uint32_t datasize;
	uint32_t testcmd;
	struct timespec start, round_start, stop, start_exp, stop_exp, start_unexp, stop_unexp;
	int ret;
	uint64_t ms, round_us = 0, round_us_sum = 0, round_us_sum_last = 0, round_us_avg = 0, round_us_avg_period = 0, round_us_min = 0, round_us_max = 0, lastprint = 0, exp_us = 0, unexp_us = 0, imp_us = 0, unimp_us = 0, vm1_us = 0, vm2_us = 0;
	void *pdata = NULL;
	uint64_t alloc_index = 0; // only for ppga or khab alike testing
	int32_t size = PAGE_SIZE*page_cnt;
	int32_t counter;
	unsigned long long tx_tv0, rx_tv0, tx_tv1, rx_tv1; // timestamp for scheduler latency benchmark
	unsigned long long vm1_vm2_sum_us = 0, vm2_vm1_sum_us = 0, vm1_vm2_min_us = XVM_SCHE_MAX_LATENCY_US, vm2_vm1_min_us = XVM_SCHE_MAX_LATENCY_US, vm1_vm2_max_us = 0, vm2_vm1_max_us = 0; // for scheduler latency benchmark
	unsigned long long cur_vm1_vm2, cur_vm2_vm1; // scheduler latency in each round of test
	int tput_size = HAB_MAX_MSG_SIZEBYTES;
	float burst_total = 0.0, burst_min, burst_max = 0.0;
#if defined(__linux__) || defined(__QNXNTO__)
	time_t curr_time = 0;
	struct tm local_time = {};
#endif
#if defined(__QNXNTO__)
	uint64_t qnx_round_start, qnx_round_stop;
#endif

#ifdef TEST_ION_EXPORT
	struct allocation_data data = {
      .len = 0,
      .flags = 0,
#if TARGET_ION_ABI_VERSION >= 2
      .heap_id_mask = ION_HEAP(ION_SYSTEM_HEAP_ID),
#else
      .align = 4096,
      .heap_id_mask = ION_HEAP(ION_IOMMU_HEAP_ID),
      .handle = -1,
#endif
	};
#endif
#ifdef TEST_DMAHEAP_EXPORT
	struct dma_heap_allocation_data data = {
		.len = (size_t)size,
		.fd_flags = O_RDWR,
	};
#endif

	if (!print_period_sec)
		print_period_sec = 2; /* default is 2 seconds */

	if (PING_TEST_TYPE_EXPIMP == test_type) {
#ifdef TEST_ION_EXPORT
		pdata = vc_allocate_ion_memory(size, 0 /*not cached*/, &data);
#elif defined(TEST_DMAHEAP_EXPORT)
		pdata = vc_allocate_dmaheap_memory(size, &data, DMABUF_HEAP_CACHED);
#else
        pdata = vc_allocate_memory(vc_id, size, 0 /*not cached*/, &alloc_index); // alloc index is ignore unless PPGA is enabled
#endif
        if (!pdata) {
			printf("failed to allocate memory to export for %d bytes, and exit\n",size);
			return -1;
		}
    }

	ret = clock_gettime(CLOCK_MONOTONIC, &start);
	if (ret) printf("failed to get start time!\n");

	for (i = 1; ;i++) {
#if defined(__QNXNTO__)
		qnx_round_start = ClockCycles();
#endif
		ret = clock_gettime(CLOCK_MONOTONIC, &round_start); /* start time for each round */
		if (ret) printf("failed to get start time!\n");

        if (PING_TEST_TYPE_TXRX == test_type) {
            // send cmd
			testcmd = KHAB_TEST_CMD_SEND;
            ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); // into khab public API
            KHAB_API_ERR_CHECK(ret);

            // send test pattern - timespec
            ((int64_t*)databuf)[0] = start.tv_sec;
            ((int64_t*)databuf)[1] = start.tv_nsec;

            ret = habmm_socket_send(vc_id, databuf, sizeof(int64_t)*2, 0); // into khab public API
            KHAB_API_ERR_CHECK(ret);

            // recv result
            datasize = sizeof(databuf);
            ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
            KHAB_API_ERR_CHECK(ret);

        } else if (PING_TEST_TYPE_XVM_OVERHEAD == test_type) {
			testcmd = KHAB_TEST_CMD_PROF;
            ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); // send command
            KHAB_API_ERR_CHECK(ret);

			// outbound
            ret = habmm_socket_send(vc_id, databuf, sizeof(uint64_t)*2*4, HABMM_SOCKET_SEND_FLAGS_XING_VM_STAT); // send stat-buffer from VM1 to VM2
            KHAB_API_ERR_CHECK(ret);

			// inbound
            datasize = sizeof(uint64_t)*2*4;
            ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
            KHAB_API_ERR_CHECK(ret);

			// 4 timestamps are 0:vm2 leave (inbound), 1:vm1 enter (inbound), 2:vm1 leave (outbound), 3:vm2 enter (outbound)
            vm1_us += timeval_diff_us( &((struct timeval*)databuf)[2], &((struct timeval*)databuf)[1]);
            vm2_us += timeval_diff_us( &((struct timeval*)databuf)[3], &((struct timeval*)databuf)[0]);

		} else if (PING_TEST_TYPE_EXPIMP == test_type) {
            // export/import profiling testing
            uint32_t expid_array[2], expid;
            struct _TESTCMD_ACK cmd_ack = {0};

            // 1. export
            clock_gettime(CLOCK_MONOTONIC, &start_exp);
#if defined(TEST_ION_EXPORT) || defined(TEST_DMAHEAP_EXPORT)
			ret = habmm_export(vc_id, pdata, size, &expid, HABMM_EXP_MEM_TYPE_DMA);
#elif defined(ENABLE_HOST_VM) || defined(__INTEGRITY)
			struct pmem_container *pmem = pdata;
            ret = habmm_export(vc_id, pmem->handle, size, &expid, HABMM_EXPIMP_FLAGS_FD);
#else
            ret = habmm_export(vc_id, pdata, size, &expid, 0);
#endif
            clock_gettime(CLOCK_MONOTONIC, &stop_exp);
            KHAB_API_ERR_CHECK(ret);

            // 2. send export id to echo machine and ask for import
			expid_array[0] = expid;
			expid_array[1] = page_cnt;
            testcmd = KHAB_TEST_CMD_IMP;

            ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0);
            KHAB_API_ERR_CHECK(ret);

            ret = habmm_socket_send(vc_id, &expid_array, sizeof(expid_array), 0);
            KHAB_API_ERR_CHECK(ret);

            // do we want to wait??

            // 3. send export id to echo machine and ask for unimport
            testcmd = KHAB_TEST_CMD_UNIMP;
            ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0);
            KHAB_API_ERR_CHECK(ret);

            ret = habmm_socket_send(vc_id, &expid_array, sizeof(expid_array), 0);
            KHAB_API_ERR_CHECK(ret);

            // 4. wait for ack and profiling data
            datasize = sizeof(cmd_ack);
            ret = habmm_socket_recv(vc_id, &cmd_ack, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
            KHAB_API_ERR_CHECK(ret);

            // 5. unexport
            clock_gettime(CLOCK_MONOTONIC, &start_unexp);
            ret = habmm_unexport(vc_id, expid, 0);
            clock_gettime(CLOCK_MONOTONIC, &stop_unexp);
            KHAB_API_ERR_CHECK(ret);

            // 6 calculate profiling result on both sides
            exp_us += timespec_diff_us(&start_exp, &stop_exp);
            unexp_us += timespec_diff_us(&start_unexp, &stop_unexp);
            imp_us += cmd_ack.imp_us;
            unimp_us += cmd_ack.unimp_us;
		} else if (PING_TEST_TYPE_BURST == test_type) {
			struct timespec start_time, stop_time;
			uint64_t time_us;
			float burst;

			if (!loops)
				loops = TPUT_TX_NUM;

			// send cmd to start tput test
			testcmd = KHAB_TEST_CMD_BURST;
			ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); //into khab public API
			KHAB_API_ERR_CHECK(ret);
			testcmd = loops;
			ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); // loops
			KHAB_API_ERR_CHECK(ret);

			counter = 0;
			if (page_cnt == 0)
				tput_size = 16; // 16 bytes as light payload
			else if (page_cnt != -1)
				tput_size = size;
			printf("burst test %d loops %d bytes as payload blocking...\n", loops, tput_size);
            clock_gettime(CLOCK_MONOTONIC, &start_time);
			while (counter < loops) {
				ret = habmm_socket_send(vc_id, databuf, tput_size, 0); // into khab public API
				KHAB_API_ERR_CHECK(ret);
				counter++;
			}
            clock_gettime(CLOCK_MONOTONIC, &stop_time);
            time_us = timespec_diff_us(&start_time, &stop_time);
			burst = time_us*1.0/loops;
			printf("burst test %d loops %d bytes as payload one send %f us\n", loops, tput_size, burst);

			// recv result
			datasize = sizeof(databuf);
			ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
			KHAB_API_ERR_CHECK(ret);

			burst_total += burst;
			burst_min = (i == 1 || burst < burst_min ) ? burst : burst_min;
			burst_max = (burst > burst_max) ? burst : burst_max;
			//ret = clock_gettime(CLOCK_MONOTONIC, &stop);
			//if (ret) printf("failed to get stop time!\n");
			//break;
		} else if (PING_TEST_TYPE_TPUT == test_type) {
			// send cmd to start tput test
			testcmd = KHAB_TEST_CMD_TPUT;
			ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); //into khab public API
			KHAB_API_ERR_CHECK(ret);

			if (page_cnt == 0)
				tput_size = HAB_MAX_MSG_SIZEBYTES;
			else
				tput_size = size;

			counter = 0;
			while (counter < TPUT_TX_NUM) {
				ret = habmm_socket_send(vc_id, databuf, tput_size, 0); // into khab public API
				KHAB_API_ERR_CHECK(ret);
				if (0 != ret) printf("tput_size is %d\n", tput_size);
				counter++;
			}

			// recv result
			datasize = sizeof(databuf);
			ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
			KHAB_API_ERR_CHECK(ret);
			printf("payload of message we received is %d, size of message we sent is %d\n", *databuf, tput_size);
			ret = clock_gettime(CLOCK_MONOTONIC, &stop);
			if (ret) printf("failed to get stop time!\n");
			break;
        } else if (PING_TEST_TYPE_SCHE_LATENCY == test_type) {
			testcmd = KHAB_TEST_CMD_SCHE;
			ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); // send command
			KHAB_API_ERR_CHECK(ret);

			// wait for sync from VM2
			datasize = sizeof(unsigned long long);
			ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
			KHAB_API_ERR_CHECK(ret);

			// kick off message from VM1 to VM2
			ret = habmm_socket_send(vc_id, databuf, sizeof(unsigned long long), HABMM_SOCKET_XVM_SCHE_TEST);
			KHAB_API_ERR_CHECK(ret);

			// wait for ack from VM2
			datasize = sizeof(unsigned long long);
			ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
			KHAB_API_ERR_CHECK(ret);

			// collect the mpm_timer value, VM1 -> VM2 (tx_tv0, rx_tv0), VM2 -> VM1 (tx_tv1, rx_tv1)
			rx_tv1 = ((unsigned long long*)databuf)[0];
			ret = habmm_socket_send(vc_id, databuf, sizeof(unsigned long long)*3, HABMM_SOCKET_XVM_SCHE_RESULT_REQ);
			KHAB_API_ERR_CHECK(ret);

			// wait for the feedback of the mpm_timer value, VM1 -> VM2 (tx0, rx0), VM2 -> VM1 (tx1, rx1)
			datasize = sizeof(unsigned long long)*3;
			ret = habmm_socket_recv(vc_id, databuf, &datasize, (uint32_t)-1, 0); // -1: wait until data arrives
			KHAB_API_ERR_CHECK(ret);

			// calculate cross-vm schedule latency: vm1_vm2_us is latency of schedule from vm1 to vm2, vm2_vm1_us is latency of schdule from vm2 to vm1
			tx_tv0 = ((unsigned long long *)databuf)[0];
			rx_tv0 = ((unsigned long long *)databuf)[1];
			tx_tv1 = ((unsigned long long *)databuf)[2];

			cur_vm1_vm2 = mpm_timer_to_us(tx_tv0, rx_tv0);
			cur_vm2_vm1 = mpm_timer_to_us(tx_tv1, rx_tv1);
			vm1_vm2_sum_us += cur_vm1_vm2;
			vm2_vm1_sum_us += cur_vm2_vm1;
			vm1_vm2_min_us = cur_vm1_vm2 < vm1_vm2_min_us ? cur_vm1_vm2 : vm1_vm2_min_us;
			vm2_vm1_min_us = cur_vm2_vm1 < vm2_vm1_min_us ? cur_vm2_vm1 : vm2_vm1_min_us;
			vm1_vm2_max_us = cur_vm1_vm2 > vm1_vm2_max_us ? cur_vm1_vm2 : vm1_vm2_max_us;
			vm2_vm1_max_us = cur_vm2_vm1 > vm2_vm1_max_us ? cur_vm2_vm1 : vm2_vm1_max_us;
        }
		// calculate per-round time
#if defined(__QNXNTO__)
		qnx_round_stop = ClockCycles();
#endif
		ret = clock_gettime(CLOCK_MONOTONIC, &stop);
		if (ret)
			printf("failed to get stop time!\n");
		else {
#if defined(__QNXNTO__)
			round_us = (qnx_round_stop - qnx_round_start) * 1000000 / habtest_stat.CPS;
#else
			round_us = timespec_diff_us(&round_start, &stop);
#endif
			round_us_min = (i == 1 || round_us < round_us_min ) ? round_us : round_us_min;
			round_us_max = (round_us > round_us_max) ? round_us : round_us_max;

			round_us_sum += round_us;
			round_us_avg = round_us_sum/i; /* average from beginning to now */

			if (stop.tv_nsec < start.tv_nsec) {
				ms  = (stop.tv_sec - start.tv_sec - 1)*1000 + (1000000000 - start.tv_nsec + stop.tv_nsec)/1000000;
			} else {
				ms  = (stop.tv_sec - start.tv_sec)*1000 + (stop.tv_nsec - start.tv_nsec)/1000000;
			}

#if defined(__linux__) || defined(__QNXNTO__)
			// Dump stat buffer
			if(habtest_stat.counter == HABTEST_STAT_BUFFER_SIZE)
			{
				if(habtest_stat.dump() != EXIT_SUCCESS)
				{
					fprintf(stderr, "[ERR] habtest_stat.dump failed!\n");
					return EXIT_FAILURE;
				}
			}

			// Save stat data
			time(&curr_time);
			localtime_r(&curr_time, &local_time);
			strftime(habtest_stat.data[habtest_stat.counter].time,
					HABTEST_STAT_TIME_STRLEN, "%T", &local_time);
			habtest_stat.data[habtest_stat.counter].round_us = round_us;
			habtest_stat.counter++;
#endif

			if ((stop.tv_sec - start.tv_sec) > lastprint) { /* print 1st in 1 sec */
				/* average in the recent period */
				round_us_avg_period = (round_us_sum - round_us_sum_last) / (i - i_last);

				/* save for the next report's computation */
				i_last = i;
				round_us_sum_last = round_us_sum;

                if(PING_TEST_TYPE_TXRX == test_type) {
					printf("[%lds] Speed: %.2f ping/ms %ld us/ping Min/Avg/Max: %ld/%ld/%ldus - %d Rounds\r",
							ms / 1000, 1000.0f / round_us_avg_period, round_us_avg_period, 
							round_us_min, round_us_avg, round_us_max, i); fflush(stdout);
                } else if (PING_TEST_TYPE_EXPIMP == test_type) {
                    printf("exp speed is %ld us, unexp speed is %ld us, imp speed is %ld us, unimp speed is %ld us\n",
                           exp_us/i, unexp_us/i, imp_us/i, unimp_us/i);
                } else if (PING_TEST_TYPE_XVM_OVERHEAD == test_type) {
                    printf("round trip cross-vm cost is %f us, total vm1 side time %ld us, total vm2 side time %ld us for %d loops\n",
                           (vm1_us - vm2_us)*1.0/i, vm1_us, vm2_us, i);
				}

				lastprint += print_period_sec;// always ahead
			}
			// check if it is time to finish/break
			if (ms/1000 >= sec)
				break;
			else
				/* wait interval_sec seconds before start next ping */
				interval_sec ? sleep(interval_sec) : 0;
		}
	}

#if defined(__linux__) || defined(__QNXNTO__)
	// Dump stat buffer
	if(habtest_stat.dump() != EXIT_SUCCESS)
	{
		fprintf(stderr, "[ERR] habtest_stat.dump failed!\n");
		return EXIT_FAILURE;
	}
#endif

	// ask remote to exit echo mode
	testcmd = KHAB_TEST_CMD_EXIT ;
	ret = habmm_socket_send(vc_id, &testcmd, sizeof(testcmd), 0); // into khab public API
	KHAB_API_ERR_CHECK(ret);

    if(PING_TEST_TYPE_TXRX == test_type) {
		printf("vcid 0x%X ping %d times spent %ld sec, interval is %ld sec, print_period_sec is %ld sec, speed is %f ping/ms, each ping min/avg/max=%ld/%ld/%ld us\n",
			   vc_id, i, stop.tv_sec - start.tv_sec, interval_sec, print_period_sec, 1000.0f/round_us_avg, round_us_min, round_us_avg, round_us_max);

    } else if (PING_TEST_TYPE_EXPIMP == test_type) {
		printf("vcid 0x%X for %d loops, exp speed is %ld us, unexp speed is %ld us, imp speed is %ld us, unimp speed is %ld us\n",
			   vc_id, i, exp_us/i, unexp_us/i, imp_us/i, unimp_us/i);
    } else if (PING_TEST_TYPE_XVM_OVERHEAD == test_type) {
		printf("vcid 0x%X for %d round trips, cross-vm cost is %f us\n",
			   vc_id, i, (vm1_us - vm2_us)*1.0/i);
	} else if (PING_TEST_TYPE_BURST == test_type) {
		printf("vcid 0x%X burst send %d times burst send min/avg/max %f %f %f us\n", vc_id, i, burst_min, burst_total/i, burst_max);
	} else if (PING_TEST_TYPE_TPUT == test_type) {
		printf("TPUT test sends %ld MB, speed is %f MB/s\n",
			   			tput_size*TPUT_TX_NUM/1024/1024,
			   			tput_size*TPUT_TX_NUM*1.0f /((stop.tv_sec - start.tv_sec)*1000000 + (stop.tv_nsec - start.tv_nsec)/1000));
	} else if (PING_TEST_TYPE_SCHE_LATENCY == test_type) {
		printf("vcid 0x%X for %d round trips, vm1 schedule to vm2 costs (min: %llu us, max: %llu, avg: %.1f us), vm2 schedule to vm1 costs (min: %llu us, max: %llu us, avg: %.1f us)\n",
			   vc_id, i, vm1_vm2_min_us, vm1_vm2_max_us, vm1_vm2_sum_us*1.0/i, vm2_vm1_min_us, vm2_vm1_max_us, vm2_vm1_sum_us*1.0/i);
	}

    if (pdata) {
#ifdef TEST_ION_EXPORT
        ret = munmap(pdata, PAGE_SIZE*page_cnt);
        // free the memory
#ifndef TARGET_ION_ABI_VERSION
        ioctl(ion_fd, ION_IOC_FREE, &data.handle);
	data.handle = -1;
#else
	close(data.fd);
	data.fd = -1;
#endif
	data.len = 0;
#else
        ret = vc_free_memory(PAGE_SIZE*page_cnt, pdata, alloc_index);
#endif
        KHAB_API_ERR_CHECK(ret);
    }
    return ret;
}

int32_t vc_query    (int32_t vc_id)
{
  int32_t ret;
  struct hab_socket_info info = {0};

  ret = habmm_socket_query(vc_id, &info, 0);
  printf("ret %d, skt %X, vmid local %d remote %d, vmname local %s remote %s\n", ret,
		 vc_id, info.vmid_local, info.vmid_remote, info.vmname_local, info.vmname_remote);

  return ret;
}

void do_ping() {
	int32_t ret, h, cnt = 0, cnt2 = 0, size;

	ret = habmm_socket_open(&h, MM_MISC, -1, 0);
	if (ret)
		printf("%s: failed to open ret %d\n", __func__, ret);

	while (1) {
		ret = habmm_socket_send(h, &cnt, sizeof(cnt), 0);
		if (ret)
			printf("%s: failed to send value %d to remote ret %d\n", __func__, cnt, ret);

		size = sizeof(cnt2);
		ret = habmm_socket_recv(h, &cnt2, &size, -1, 0);
		if (cnt2 != cnt + 1 || ret) {
			printf("%s: failed to receive value from remote %d expect %d\n", __func__, cnt2, cnt + 1, size, ret);
		}
		cnt = cnt2 + 1;
		if (cnt % 10000 == 0) printf("%s: loop %d times\n", __func__, cnt);
	}
}

void do_pong() {
	int32_t ret, h, cnt = 0, cnt2 = 0, size;

	ret = habmm_socket_open(&h, MM_MISC, -1, 0);
	if (ret)
		printf("%s: failed to open ret %d\n", __func__, ret);

	while (1) {
		size = sizeof(cnt2);
		ret = habmm_socket_recv(h, &cnt2, &size, -1, 0);
		if (cnt2 != cnt || ret) {
			printf("%s: failed to receive value from remote %d expect %d\n", __func__, cnt2, cnt, size, ret);
		}

		cnt = cnt2 + 1;
		ret = habmm_socket_send(h, &cnt, sizeof(cnt), 0);
		if (ret)
			printf("%s: failed to send value %d to remote ret %d\n", __func__, cnt, ret);
		cnt++;
		if (cnt % 10000 == 0) printf("%s: loop %d times\n", __func__, cnt);
	}
}

void do_echo_auto(void) {
	int32_t ret, h;

	ret = habmm_socket_open(&h, MM_MISC, -1, 0);
	if (ret) {
		printf("%s: failed to open ret %d\n", __func__, ret);
		return;
	}

	vc_echo(h);
}


void do_ping_auto(const char *line) {
	const unsigned int num_words = 4;
	char words[num_words][64];
	int32_t a = 0, b = 0, c = 0, d = 0;
	int32_t ret, h;

	int scan_result = 0;
	memset(words, 0, sizeof(words));
	scan_result = sscanf(line, "%s %s %s %s", words[0], words[1], words[2], words[3]);

	if(scan_result == EOF)
		return;

	a = strtol(words[0], NULL, 0); // sec
	b = strtol(words[1], NULL, 0); // test_type
	c = strtol(words[2], NULL, 0); // page_cnt
	d = strtol(words[3], NULL, 0); // loops

	ret = habmm_socket_open(&h, MM_MISC, -1, 0);
	if (ret) {
		printf("%s: failed to open ret %d\n", __func__, ret);
		return;
	}
	printf("executing sec:type:pg-cnt:loops %d %d %d %d\n", a, b, c, d);
	vc_ping(h, a, b, c, 0, 0, d);
}

#if defined(ENABLE_VIRTIO_VHOST)
#define HAB_NAME_SOCKET "/tmp/socket-hab"

static int receive_fd(int socket, size_t *size, uint64_t *offset)  // receive fd from AF_UNIX socket
{
    int fd = 0;
    struct msghdr msg = {0};
    char m_buffer[1];
    struct iovec io = { .iov_base = m_buffer, .iov_len = sizeof(m_buffer) };
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;

    char c_buffer[256];
    msg.msg_control = c_buffer;
    msg.msg_controllen = sizeof(c_buffer);

    printf("ready for receiving msg\n");
    if (recvmsg(socket, &msg, 0) < 0)
        printf("Failed to receive message\n");

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);

    printf("About to extract fd\n");

    memmove(&fd, CMSG_DATA(cmsg), sizeof(fd));
    printf("Extracted fd %d\n", fd);

	if(read(socket, size, sizeof(*size)) != sizeof(*size))
		printf("failed to read size %zd bytes from fd %d %s\n", sizeof(*size), socket, strerror(errno));
	else
		printf("buff size %zd\n", *size);

	if(read(socket, offset, sizeof(*offset)) != sizeof(*offset))
		printf("failed to read offset %zd bytes from fd %d %s\n", sizeof(*offset), socket, strerror(errno));
	else
		printf("buff offset %lX\n", *offset);

    return fd;
}

int get_vhost_fd() {
    int af_unix_sock;
    struct sockaddr_un af_unix_addr;
    int ret_fd;

    // setup socket to send fd to uhabtest
    /* Create AF_UNIX socket from which to read fd */
    af_unix_sock = socket(AF_UNIX, /*SOCK_DGRAM*/SOCK_STREAM, 0);
    if (af_unix_sock < 0) {
	printf("error opening datagram socket %s %s\n", strerror(errno));
	return -1;
    }

    /* Create name. */
    af_unix_addr.sun_family = AF_UNIX;
    strlcpy(af_unix_addr.sun_path, HAB_NAME_SOCKET, sizeof(af_unix_addr.sun_path));

    /* Bind the UNIX domain address to the created socket */
    if (connect(af_unix_sock, (const struct sockaddr*)&af_unix_addr, sizeof(af_unix_addr)) == -1) {
        printf("connect to socket %s error, %s\n", HAB_NAME_SOCKET, strerror(errno));
	return -1;
    }
    printf("AF_UNIX socket %d -->%s\n", af_unix_sock, HAB_NAME_SOCKET);

    ret_fd = receive_fd(af_unix_sock, &pvm_carveout_size, &pvm_carveout_offset);

    close(af_unix_sock);
    //remove(HAB_NAME_SOCKET);

    return ret_fd;
}
#endif


#if 0
void    test_grant  (int32_t domid) {
  char *pdata; // = malloc(size);
  uint32_t grantid;
  int size = 4096; // only one page is tested

  pdata = mmap(NULL, 4096, PROT_READ|PROT_WRITE,
			MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE, -1, 0);

  *(unsigned long*)(&pdata[0]) = 0xBEEF;
  *(unsigned long*)(&pdata[size-5]) = 0xDEAD;

  test_khab_grant(domid, pdata, &grantid);
  printf("%s: returned grant id %d\n", __func__, grantid);
}

void    test_map    (int32_t domid, int32_t gref) {
  char *p = NULL;
  int size = 4096; // only one page is tested

  test_khab_map(domid, gref, (void**)&p);
  printf("%s: contents @ %p to %lX, and %p to %lX\n", __func__, &p[0], *(unsigned long*)(&p[0]), &p[size-5], *(unsigned long*)(&p[size-5]));

}
#endif

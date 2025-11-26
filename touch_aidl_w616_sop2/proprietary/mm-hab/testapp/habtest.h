//******************************************************************************************************************************
// Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  habtest.h
* @brief uhabtest header declarations
********************************************************************************************************************************
*/
#include "habmm.h"

#if defined(__linux__) || defined(__QNXNTO__)
#define HABTEST_STAT_BUFFER_SIZE 18000000                // Store up to half an hour data
#define HABTEST_STAT_TIME_STRLEN 9                       // Format: 12:34:56
#endif
#if defined(__ANDROID__)
#define HABTEST_STAT_FILE "/data/local/tmp/habtest.stat" // The file to save the data
#elif defined(__linux__) || defined(__QNXNTO__)
#define HABTEST_STAT_FILE "/tmp/habtest.stat"            // The file to save the data
#endif

struct hab_mem_test_params {
	int tx_rx;	/* 0 for rx, 1 for tx */
	int mm_id;	/* mmid to open hab device */
	int mem_size;	/* memory size for export test */
	int loop_cnt;	/* times of verification test */
	int usermode;   /* user mode or kernel mode test */
	int intermode;  /* inter mode or commandline mode test */
	uint32_t mem_source;
	uint32_t thread_number; /* num of threads to share the same vchan opened over the above mm_id */
#ifdef HAB_PING_TEST
	uint32_t vc_id;
	uint32_t sec;
	int test_type;
	int pg_cnt;
#endif
};

#define DMABUF_HEAP_CACHED 1U

#if defined(__linux__) || defined(__QNXNTO__)
typedef struct habtest_stat_data
{
	char     time[HABTEST_STAT_TIME_STRLEN];
	uint64_t round_us;
} HABTEST_STAT_DATA;

typedef struct habtest_stat
{
#if defined(__QNXNTO__)
	uint64_t          CPS; // clock cycles per second (currently 19200000)
#endif
	uint64_t          counter;
	HABTEST_STAT_DATA *data;
	int8_t            (*init)(void);
	int8_t            (*dump)(void);
} HABTEST_STAT;
#endif

enum hab_mem_source {
	MEM_SOURCE_DEFAULT = 0,
	MEM_SOURCE_MMAP,
};

void hab_test_init(void);
void hab_test_deinit(void);

int32_t vc_open     (int32_t *vc_id_ret, unsigned int mmid, uint32_t timeout_ms, int32_t flags);
int32_t vc_close    (int32_t vc_id);
int32_t vc_send     (int32_t vc_id, int32_t size, int32_t pattern);
int32_t vc_recv     (int32_t vc_id, int32_t size);
int32_t vc_export   (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void **pv, uint32_t cached);
int32_t vc_export_from_fd (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, uint32_t cached);
int32_t vc_export_ion   (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void **pv, uint32_t cached);
int32_t vc_export_ion_multiple(int32_t vc_id, int32_t num_export, int32_t unit_export_size, int32_t pattern, int32_t export_id[], void* pv[], uint32_t cached);
int32_t vc_export_dmaheap (int32_t vc_id, int32_t size, int32_t pattern, uint32_t cached, int32_t *export_id, void **pv);
int32_t vc_unexport_ion_multiple(int32_t vc_id);
int32_t vc_import   (int32_t vc_id, int32_t export_id, int32_t size);
int32_t	vc_import_to_fd (int32_t vc_id, int32_t export_id, int32_t size);
int32_t vc_unexport (int32_t vc_id, int32_t export_id);
int32_t vc_unimport (int32_t vc_id, int32_t export_id, uint64_t buff_addr);
int32_t vc_unimport_from_fd (int32_t vc_id, int32_t export_id, uint64_t buff_addr);
void    vc_check    (int32_t size, int32_t tick, uint64_t buff_addr);
int32_t vc_echo     (int32_t vc_id);
int32_t vc_ping     (int32_t vc_id, int32_t seconds, int32_t test_type, int32_t page_cnt, int32_t interval_sec, int32_t print_period_sec, int32_t loops);
int32_t vc_query    (int32_t vc_id);


void    test_grant  (int32_t domid);
void    test_map    (int32_t domid, int32_t gref);

void do_ping(void);
void do_pong(void);
void do_echo_auto(void);
void do_ping_auto(const char *line);

/* New VC is opened if vc_id parameter is <= 0 */
int32_t test_mem(int vc_id, int tx_rx, int mm_id, int mem_size, int loop, uint32_t mem_source);
int32_t test_mem_multiple_export(int vc_id_param, int tx_rx, int num_export, int unit_export_size, int loop);

int32_t khabtest_open     (int32_t *vc_id_ret, unsigned int mmid, uint32_t timeout_ms, int32_t flags);
int32_t khabtest_close    (int32_t vc_id);
int32_t khabtest_send     (int32_t vc_id, int32_t size, int32_t pattern);
int32_t khabtest_recv     (int32_t vc_id, int32_t size);
int32_t khabtest_export   (int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void **pv, uint8_t cached);
int32_t khabtest_unexport (int32_t vc_id, int32_t expid);
int32_t khabtest_test_mem (int vc_id, int tx_rx, int mm_id, int mem_size, int loop);
int32_t khabtest_echo     (int32_t vc_id);
int32_t khabtest_query    (int32_t vc_id);

struct hab_operations {
	int32_t	(*open)				(int32_t *vc_id_ret, unsigned int mmid, uint32_t timeout_ms, int32_t flags);
	int32_t	(*close)			(int32_t vc_id);
	int32_t	(*send)				(int32_t vc_id, int32_t size, int32_t pattern);
	int32_t	(*recv)				(int32_t vc_id, int32_t size);
	int32_t	(*export)			(int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void **pv, uint8_t cached);
	int32_t	(*export_from_fd)	(int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, uint8_t cached);
	int32_t	(*export_ion)		(int32_t vc_id, int32_t size, int32_t pattern, int32_t* export_id, void **pv, uint8_t cached);
	int32_t	(*export_ion_multiple)		(int32_t vc_id, int32_t num_export, int32_t unit_export_size, int32_t pattern, int32_t export_id[], void* pv[], uint8_t cached);
	int32_t (*export_dmaheap)	(int32_t vc_id, int32_t size, int32_t pattern, int32_t *export_id, void **pv);
	int32_t	(*unexport_ion_multiple)	(int32_t vc_id);
	int32_t	(*import)			(int32_t vc_id, int32_t export_id, int32_t size);
	int32_t	(*import_to_fd)		(int32_t vc_id, int32_t export_id, int32_t size);
	int32_t	(*unexport)			(int32_t vc_id, int32_t export_id);
	int32_t	(*unimport)			(int32_t vc_id, int32_t export_id, uint64_t buff_addr);
	int32_t (*echo)             (int32_t vc_id);
    int32_t (*ping)             (int32_t vc_id, int32_t seconds, int32_t test_type, int32_t page_cnt, int32_t interval_sec, int32_t print_period_sec, int32_t loops);
	void	(*check)			(int32_t size, int32_t tick, uint64_t buff_addr);

	void    (*test_grant)		(int32_t domid);
	void    (*test_map)			(int32_t domid, int32_t gref);

	/* New VC is opened if vc_id parameter is <= 0 */
	int32_t	(*test_mem)			(int vc_id, int tx_rx, int mm_id, int mem_size, int loop, uint32_t mem_source);
	int32_t	(*test_mem_multiple_export)	(int vc_id_param, int tx_rx, int num_export, int unit_export_size, int loop);
};

//******************************************************************************************************************************
// Copyright (c) 2016-2019, 2022 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//******************************************************************************************************************************

/**
********************************************************************************************************************************
* @file  habmenu.c
* @brief Implements uhabtest menu handling
********************************************************************************************************************************
*/
#include <errno.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <Windows.h>
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "habtest.h"
#define PAGE_SIZE 4096

#if defined(__INTEGRITY)
#include <util/getopt.h>
#endif

#ifdef WIN32
#define usleep(_us_) Sleep(_us_/1000)
#endif

static char habtestapp_ver[]="$Date: 2020/03/02 $ $Change: 22790002 $ $Revision: #35 $";

static struct hab_operations uhabops = {
	.open				= vc_open,
	.close				= vc_close,
	.send				= vc_send,
	.recv				= vc_recv,
	.export				= vc_export,
	.export_from_fd			= vc_export_from_fd,
#ifdef TEST_ION_EXPORT
	.export_ion			= vc_export_ion,
	.export_ion_multiple		= vc_export_ion_multiple,
	.unexport_ion_multiple		= vc_unexport_ion_multiple,
#endif
#ifdef TEST_DMAHEAP_EXPORT
	.export_dmaheap			= vc_export_dmaheap,
#endif
	.import				= vc_import,
	.import_to_fd			= vc_import_to_fd,
	.unexport			= vc_unexport,
	.unimport			= vc_unimport,
	.check				= vc_check,
#if 0
	.test_grant			= test_grant,
	.test_map			= test_map,
#endif
	.test_mem			= test_mem,
	.test_mem_multiple_export	= test_mem_multiple_export,
    .echo               = vc_echo,
    .ping               = vc_ping,
};

#if !defined(__QNXNTO__) && !defined(__INTEGRITY) && !defined(WIN32)
static struct hab_operations khabops = {
	.open				= khabtest_open,
	.close				= khabtest_close,
	.send				= khabtest_send,
	.recv				= khabtest_recv,
	.export				= khabtest_export,
	.export_ion			= NULL,
	.export_ion_multiple		= NULL,
	.unexport_ion_multiple		= NULL,
	.import				= NULL,
	.unexport			= khabtest_unexport,
	.unimport			= NULL,
	.check				= NULL,
	.test_grant			= NULL,
	.test_map			= NULL,

	.test_mem			= khabtest_test_mem,
	.test_mem_multiple_export	= NULL,
    .echo               = NULL,
    .ping               = NULL
};
#else
static struct hab_operations khabops = {0};
extern char    *optarg;                /* argument associated with option */ // for getopt
#endif

static struct hab_operations habops;

enum multi_action {
	WAIT,
	SEND,
	RECV,
	CLOSE,
};

struct open_thread_param {
	int32_t vcid;
	int32_t  mmid;
	uint32_t timeout_ms;
	uint32_t flags;
	pthread_mutex_t mut;
	pthread_cond_t cond;
	enum multi_action ac;
} op;

struct multi_open_thread_info {
	struct open_thread_param op;
	pthread_t open_thread_id;
};

/*
 * Mode interactive supports up to 8 parameters, and the first parameter is occupied by the command name.
 * Then we can have up to 7 parameters to fill in mmids. Therefore, we only need to support up to 7 mmids.
 */
#define MAX_MMID_OPEN_SUPPORT 7

struct multi_open_thread_info mop[MAX_MMID_OPEN_SUPPORT] = {0};

void show(void) {
  printf(
	 "open <mm_id> <timeout in ms> <&>        return virtual channel # (vc_id) in HEX\n"
	 "close <virtual channel # in hex>        return status\n"
	 "send <vc_id> <bytes> <pattern>          return status\n"
	 "recv <vc_id> <bytes> <&>                return status\n"
	 "multi-open <mm_id> ... <mm_id>          Supports up to 7 mmids\n"
	 "multi-send                              For each vchan gotten from multi-open, the above \"send <vc_id> 4096 2\" will be executed in each thread w/ each vchan\n"
	 "multi-recv                              For each vchan gotten from multi-open, the above \"recv <vc_id> 4096\" will be executed in each thread w/ each vchan\n"
	 "multi-close                             will close all vchans gotten from multi-open\n"
#ifdef TEST_ION_EXPORT
	 "export_ion <vc_id> <bytes> <pattern>    return export_id and ion memory address\n"
	 "export_ion_m <vc_id> <num_export> <export_size> return export_id and memory address\n"
	 "unexport_ion_m <vcid>                   return status\n"
#endif
#ifdef TEST_DMAHEAP_EXPORT
	 "export_dmaheap <vc_id> <bytes> <pattern> <cached> return export_id and dma-buf heap memory address\n"
#endif
	 "export <vc_id> <bytes> <pattern> <cached> return export_id and memory address\n"
	 "export_from_fd <vc_id> <bytes> <pattern> <cached> return export_id and memory address\n"
	 "unexport <vc_id> <export_id>            return status\n"
	 "import <vc_id> <export_id> <bytes>      return status\n"
	 "import_to_fd <vc_id> <export_id> <bytes>  return status\n"
	 "unimport <vc_id> <export_id> <mem addr> return status\n"
	 "unimport_from_fd <vc_id> <export_id> <fd> return status\n"
	 "query <vc_id>                           return status\n"
	 "check <size> <tick> <mem addr>          print memory contents before and after tick is added\n"
	 "grant <domid>                           Test-only: grant one page to the remote dom, return refId and mem address in print\n"
	 "map <domid> <gRef>                      Test-only: map remotely granted one page and return memory address through print\n"
	 "test_mem <tx_rx> <mm_id> <mem_size> <loop>\n"
	 "fork                                    no rv\n"
	 "echo                                    no rv, put existing session to echo mode and service remote ping\n"
	 "ping <vcid> <sec> <test_type> <pg_cnt> <interval_sec> <print_period_sec> <burst_loops> no rv, print ping round trip speed as type 1, measure export/import performance as type 2 and tested memory size as page count, type 3 as cross-VM profiling, type 4 as cross-VM throughput test, type 5 as cross-VM scheduler profiling, type 6 as burst profiling and it is only for virtio vhost hab, <burst_loops> is only for test type 6, default value of <burst_loops> is TPUT_TX_NUM if not set it\n"
	 "sleep <milliseconds>                    no rv\n"
	 "exit | q                                no rv\n"
	 "[ use %%vc and %%ei for substitution of last vc_id and export_id, respectively ]\n"
	 );
}

static void keyword_subst(
    char words[][64],
    unsigned int num_words,
    int32_t vcid,
    int32_t exportid)
{
    unsigned int i;
    for(i = 0; i < num_words; i++)
    {
        if(!strcmp(words[i], "%vc"))
        {
            snprintf(words[i], 64, "%u", vcid);
        }
        else if(!strcmp(words[i], "%ei"))
        {
            snprintf(words[i], 64, "%u", exportid);
        }
    }
}

void fork_prepare() {
	printf("%s: pid %d\n", __FUNCTION__, getpid());
}

void fork_parent() {
	printf("%s: pid %d\n", __FUNCTION__, getpid());

}

void fork_child() {
	printf("%s: pid %d\n", __FUNCTION__, getpid());

}

void vc_fork(void) {
	pid_t pid = 0;
#if !defined(__INTEGRITY) && !defined(WIN32)
	pid = fork();
#else
	printf("fork is NOT supported!\n");
#endif
	printf("fork return %d\n", pid);
	if (0 == pid) {
		printf("this is child\n");
	} else {
		printf("this is parent\n");
	}
}

pthread_t open_thread_id = {0};
int bopen_thread = 0;
void *spawn_open_thread(void *p) {
	struct open_thread_param *op = (struct open_thread_param *)p;

	printf("open thread vcid %p mmid %d timeout %d flag %d\n", &(op->vcid), op->mmid, op->timeout_ms, op->flags);
	habops.open(&(op->vcid), op->mmid, op->timeout_ms, op->flags);
	printf("open thread exits\n");
	return NULL;
}

void *spawn_multi_pchan_test_thread(void *p) {
	struct open_thread_param *op = (struct open_thread_param *)p;
	int ret;

	pthread_detach(pthread_self());
	printf("thread 0x%lx open mmid %d timeout %d flag %d\n", pthread_self(), op->vcid, op->mmid, op->timeout_ms, op->flags);
	ret = habops.open(&(op->vcid), op->mmid, op->timeout_ms, op->flags);
	printf("thread 0x%lx open mmid %d success\n", pthread_self(), op->mmid);

	while (!ret) {
		pthread_mutex_lock(&op->mut);
		while (op->ac == WAIT)
		{
			pthread_cond_wait(&op->cond, &op->mut);
		}
		pthread_mutex_unlock(&op->mut);

		switch (op->ac)
		{
		case SEND:
			habops.send(op->vcid, 4096, 2);
			if (op->ac == SEND)
				op->ac = WAIT;
			break;
		case RECV:
			habops.recv(op->vcid, 4096);
			if (op->ac == RECV)
				op->ac = WAIT;
			break;
		case CLOSE:
			habops.close(op->vcid);
			pthread_exit(NULL);

		default:
			printf("unsupport test cmd\n");
			break;
		}
	}

	printf("thread 0x%lx exit\n", pthread_self());
	return NULL;
}

static void signal_multi_open_thread(enum multi_action ac)
{
	int i = 0;

	for (i = 0; i < MAX_MMID_OPEN_SUPPORT; i++) {
		if (!mop[i].open_thread_id)
			break;

		pthread_mutex_lock(&mop[i].op.mut);
		mop[i].op.ac = ac;
		pthread_cond_signal(&mop[i].op.cond);
		pthread_mutex_unlock(&mop[i].op.mut);
	}
}

static int handle_command(const char* line)
{
    int32_t vc_id = 0;
    int32_t export_id = 0;
#ifndef WIN32
    const unsigned int num_words = 8;
#else
#define num_words 8
#endif
	char words[num_words][64]; // words max at 64 bytes
	int32_t a = 0, b, d, e = 0, f = 0, g = 0;
    int64_t c;
    int scan_result = 0;
    int rc = 0;
	int ret;
	int i, j;

	memset(words, 0, sizeof(words));
    scan_result =
		sscanf(line, "%s %s %s %s %s %s %s %s", words[0], words[1], words[2], words[3],
			   words[4], words[5], words[6], words[7]);

    if(scan_result == EOF)
        return 0;
    keyword_subst(words, num_words, vc_id, export_id);
    a = strtol(words[1], NULL, 0); // support 0x as hex
    b = strtol(words[2], NULL, 0);
    c = strtol(words[3], NULL, 0); // 64b int
    d = strtol(words[4], NULL, 0);
    e = strtol(words[5], NULL, 0);
    f = strtol(words[6], NULL, 0);
    g = strtol(words[7], NULL, 0);

    if (!strcmp(words[0], "open")) {
		uint32_t timeout_ms = (uint32_t)-1;
		uint32_t flags = HABMM_SOCKET_OPEN_FLAGS_SINGLE_BE_SINGLE_FE;

        if (scan_result < 2) {
            a = MM_MISC; // lazy testing, no actual mm id lookup from the user
#if defined(__QNXNTO__) || defined(__INTEGRITY)
            habops = uhabops; // user mode only
#else
            habops = khabops;
            printf("scan_result %d, khabtest starts...\n", scan_result);
#endif
        } else {
            habops = uhabops;
        }
        if (scan_result == 3) {
            timeout_ms = b;
		} else if (scan_result >= 4 && words[3][0] == '&') {
			// dual threaded
			op.vcid = 0;
			op.mmid = a;
			op.timeout_ms = b;
			op.flags = flags;

			bopen_thread = 1;
			ret = pthread_create(&open_thread_id, NULL, spawn_open_thread, &op);
			if (ret) {
				printf("pthread_create failed: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
			printf("Create thread successfully, thread id:0x%lx\n", open_thread_id);

		} else {
			// single threaded
			habops.open(&vc_id, a, timeout_ms, flags);
		}
    } else if (!strcmp(words[0], "close")) {
        habops.close(a);
		if (bopen_thread == 1) {
			printf("Try to join thread id:0x%lx......\n", open_thread_id);
			ret = pthread_join(open_thread_id, NULL);
			if (ret) {
				printf("pthread_join failed: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
			printf("Join thread successfully, thread id:0x%lx\n", open_thread_id);
			bopen_thread = 0;
#ifndef WIN32
			open_thread_id = 0;
#else
			pthread_t tmp_clean = { 0 };
			open_thread_id = tmp_clean;
#endif
		}
    } else if (!strcmp(words[0], "send")) {
        habops.send(a, b, c);
    } else if (!strcmp(words[0], "recv")) {
        habops.recv(a, b);
#ifdef TEST_ION_EXPORT
    } else if (!strcmp(words[0], "export_ion")) {
        vc_export_ion(a, b, c, &export_id, NULL, (uint32_t)d /* non-cached */);
    } else if (!strcmp(words[0], "export_ion_m")) {
        vc_export_ion_multiple(a, b, c, 3, NULL, NULL, (uint32_t)d /* non-cached */);
    } else if (!strcmp(words[0], "unexport_ion_m")){
    	vc_unexport_ion_multiple(a);
#endif
#ifdef TEST_DMAHEAP_EXPORT
    } else if (!strcmp(words[0], "export_dmaheap")) {
        vc_export_dmaheap(a, b, c, (uint32_t)d, &export_id, NULL);
#endif
    } else if (!strcmp(words[0], "export")) {
        habops.export(a, b, c, &export_id, NULL, (uint32_t)d /* non-cached */);
    } else if (!strcmp(words[0], "export_from_fd")) {
        habops.export_from_fd(a, b, c, &export_id, (uint32_t)d /* non-cached */);
    } else if (!strcmp(words[0], "unexport")) {
        habops.unexport(a, b);
    } else if (!strcmp(words[0], "import")) {
		vc_import(a, b, c);
    } else if (!strcmp(words[0], "import_to_fd")) {
		vc_import_to_fd(a, b, c);
    } else if (!strcmp(words[0], "unimport")) {
      vc_unimport(a, b, c);
    } else if (!strcmp(words[0], "unimport_from_fd")) {
      vc_unimport_from_fd(a, b, c);
    } else if (!strcmp(words[0], "test_mem")) {
	  int tx_rx = a;
	  int mm_id = b;
	  int mem_size = c;
	  int loop = d;
	  printf("tx_rx %d, mmid %d, mem_size %d, loop %d\n",
        tx_rx, mm_id, mem_size, loop);
	habops.test_mem(-1, tx_rx, mm_id, mem_size, loop, 0);
    } else if (!strcmp(words[0], "check")) {
      vc_check(a, b, c);
    } else if (!strcmp(words[0], "query")) {
      vc_query(a);
    } else if (!strcmp(words[0], "fork")) {
      vc_fork();
#if 0
// test only
    } else if (!strcmp(words[0], "grant")) {
      test_grant(a);
    } else if (!strcmp(words[0], "map")) {
      test_map(a, b);
// test end
#endif
	} else if (!strcmp(words[0], "sleep")) {
      usleep(a * 1000);
	} else if (words[0][0] == '#') {
      // comment
	} else if (!strcmp(words[0], "ping")) {
		habops.ping(a, b, c, d, e, f, g);
	} else if (!strcmp(words[0], "echo")) {
		habops.echo(a);
	} else if (!strcmp(words[0], "exit") || !strcmp(words[0], "q")) {
		for (i = 0; i < 7; i++) {
			if (!mop[i].open_thread_id)
				break;

			pthread_kill(mop[i].open_thread_id, SIGKILL);
			printf("kill thread 0x%lx\n", mop[i].open_thread_id);
		}
		memset(mop, 0, sizeof(struct multi_open_thread_info) * 7);
		printf("bye\n");
		rc = 1;
	} else if (!strcmp(words[0], "multi-open")) {
		uint32_t timeout_ms = (uint32_t)-1;
		uint32_t flags = HABMM_SOCKET_OPEN_FLAGS_SINGLE_BE_SINGLE_FE;

		if (scan_result < 2)
			show();
		else {
			if (mop[0].open_thread_id) {
				printf("Pls close the previous multi-open with multi-close\n");
				return 0;
			}

			for (i = 0; i < (scan_result - 1); i++) {
				mop[i].op.vcid = 0;
				mop[i].op.mmid = strtol(words[i + 1], NULL, 0);
				mop[i].op.timeout_ms = timeout_ms;
				mop[i].op.flags = flags;

				if (pthread_mutex_init(&mop[i].op.mut, NULL) != 0)
				{
					printf("mutex init error\n");
					exit(EXIT_FAILURE);

				}
				if (pthread_cond_init(&mop[i].op.cond, NULL) != 0)
				{
					printf("cond init error\n");
					exit(EXIT_FAILURE);
				}
				mop[i].op.ac = WAIT;
				ret = pthread_create(&mop[i].open_thread_id, NULL, spawn_multi_pchan_test_thread, &mop[i].op);
				if (ret) {
					printf("pthread_create failed: %s\n", strerror(ret));
					goto err_thread;
				}
				printf("Create thread successfully, thread id:0x%lx\n", mop[i].open_thread_id);
			}
		}
	} else if (!strcmp(words[0], "multi-send")) {
		signal_multi_open_thread(SEND);
	} else if (!strcmp(words[0], "multi-recv")) {
		signal_multi_open_thread(RECV);
	} else if (!strcmp(words[0], "multi-close")) {
		signal_multi_open_thread(CLOSE);
		memset(mop, 0, sizeof(struct multi_open_thread_info) * 7);
	} else {
		show();
	}

	return rc;

err_thread:
	for (j = 0; j < i; j++) {
		pthread_kill(mop[i].open_thread_id, SIGKILL);
		printf("kill thread 0x%lx\n", mop[i].open_thread_id);
	}
	memset(mop, 0, sizeof(struct multi_open_thread_info) * 7);
	exit(EXIT_FAILURE);
}

static void do_interactive_mode(void)
{
  char line[80];

  show();

  while (1) {
    printf("\n> ");
    fflush(stdout);
    fgets(line, 80, stdin);
    if(handle_command(line))
        break;
  }

}

void print_usage(void)
{
	printf("Usage:\n"
		"\thabtest [-t|-r -d mm_id [-d mm_id ... -d mm_id] -m mem_size -l loop_cnt [-p thread_num_per_mmid] [-f mem_source] [-k]] | [-i]\n"
		"\t\t-t|-r               : Tx end or Rx end\n"
		"\t\t-d mm_id [-d mm_id ... -d mm_id]  : Multimedia channel id. Multiple -d parameters (up to 20) could be used for multiple MMIDs. The total num of MMIDs * thread_number input w/ the below \"-p\"\n"
		"\t\t-m mem_size         : memory size to export(Tx) or import(Rx)\n"
		"\t\t-l loop_cnt         : times of memory pattern verification\n"
		"\t\t-p thread_num_per_mmid  : optional, specify the num of threads created for one MMID, default as 1\n"
		"\t\t-f \"mem_source\"   : optional, only make sense when -p is set, specify how to allocate memory when do export, only have one available parameter \"mmap\" currently\n"
		"\t\t-k                  : optional, specify it's khab test\n"
		"\t\t-c \"commandline\"  : commandline mode\n"
		"\t\t-i                  : interactive mode\n"
		"\t\t-s                  : auto-echo mode w/o vchan number\n"
		"\t\t-v \"<sec> <test_type> <pg_cnt>\"         : auto-ping mode w/o vchan number\n");
	return;
}

void *mt_fn(void *arg)
{
	struct hab_mem_test_params *mt_params = (struct hab_mem_test_params *)arg;
#ifndef HAB_PING_TEST
	habops.test_mem(-1, mt_params->tx_rx, mt_params->mm_id, mt_params->mem_size, mt_params->loop_cnt, mt_params->mem_source);
#else
	if (mt_params->vc_id != -1) {
		printf("%s vcid is %x good calling ping\n", __func__, mt_params->vc_id);
		habops.ping(mt_params->vc_id, 60, 1, 10, 1, 2);
	}
	else {
		printf("%s vcid is -1, not testing ping\n", __func__);
	}
#endif
	return NULL;
}

void set_test_mode(struct hab_mem_test_params mt_params)
{
	if (mt_params.usermode == 0) {
		printf("Through kernel habtest interface\n");
		habops = khabops;
		/* TODO: incompatible interface between khabtest and uhabtest */
		habops.open(NULL, mt_params.mm_id, (uint32_t)-1, 0);
	} else {
		printf("Through hab device interface\n");
		habops = uhabops;
	}
}

static int multi_thread_test(void *p)
{
	int i;
	int thread_number = ((struct hab_mem_test_params *)p)->thread_number;
#ifndef WIN32
	pthread_t thread_ids[thread_number];
#else
	pthread_t *thread_ids = malloc(sizeof(pthread_t)*thread_number);
#endif
	uint32_t vc_dummy;
	struct hab_mem_test_params mt_params;

	memcpy(&mt_params, p, sizeof(struct hab_mem_test_params));

	printf("Multi-threads mode: %d threads for one vchan over mm_id %d\n", thread_number, mt_params.mm_id);

	// dummy open to hold the hab fd
	i = habops.open(&vc_dummy, mt_params.mm_id, (uint32_t)-1, 0);
	if (i != 0) {
		printf("failed to open dummy vcid %d ret %d!\n", mt_params.mm_id, i);
		return -1;
	} else {
		printf("open dummy vcid %x on %d OK ret %d\n", vc_dummy, mt_params.mm_id, i);
	}

#if defined (__INTEGRITY) && defined (HAB_PING_TEST)
	mt_params.vc_id = vc_dummy;
	/* Add halt task so that AGL can issue echo command, and then integrity will
	start ping test */
	HaltTask(CurrentTask());
#endif

	for (i = 0; i < thread_number; i++) {
		int ret;
		ret = pthread_create(&thread_ids[i], NULL, mt_fn, &mt_params);
		if (ret) {
			printf("pthread_create failed: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
		printf("Create thread successfully, thread id:0x%lx\n", thread_ids[i]);
	}
	for (i = 0; i < thread_number; i++) {
		int ret;
		ret = pthread_join(thread_ids[i], NULL);
		if (ret) {
			printf("pthread_join failed: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
		printf("Join thread successfully, thread id:0x%lx\n", thread_ids[i]);
	}
	// dummy close to release the hab fd
	i = habops.close(vc_dummy);
	if (i != 0 ) {
		printf("failed to close dummy vcid %d ret %d!\n", vc_dummy, i);
		return -1;
	} else {
		printf("close dummy vcid on %d OK ret %d\n", mt_params.mm_id, i);
	}
#if defined (__INTEGRITY) && defined (HAB_PING_TEST)
	mt_params.vc_id = -1;
#endif
#ifdef WIN32
	free(thread_ids);
#endif

	return 0;
}

#define MAX_MMID_NUM 20
#ifndef WIN32
int main(int argc, char **argv)
#else
int submain(int argc, char **argv)
#endif
{
	int c;
	struct hab_mem_test_params mt_params = {
#ifdef HAB_PING_TEST
			.vc_id = -1,
#endif
			.tx_rx = -1,
			.mm_id = -1,
			.mem_size = -1,
			.loop_cnt = -1,
			.usermode = 1,
			.intermode = 0,
			.mem_source = MEM_SOURCE_DEFAULT,
	};

	int thread_num_per_mmid = 1;
	char *str, *str_loops;
	int len, scan_ret;
	char mem_type[64];
	int ret = 0;
	int mmid_arr[MAX_MMID_NUM] = {0};
	int mmid_num = 0;

	printf("testapp version %s\n", habtestapp_ver);
	hab_test_init();

#if defined(__INTEGRITY)
	getopt_init(argv[0]);
#endif

	while ((c = getopt(argc, argv, "abtrikc:d:m:l:p:sv:f:")) != -1) {
		switch (c) {
		case 'i':
			printf("Enter interactive mode\n");
			set_test_mode(mt_params);
			do_interactive_mode();
			exit(EXIT_SUCCESS);
			break;
		case 'c':
			printf("Commandline mode: %s\n", optarg);
			set_test_mode(mt_params);
			handle_command(optarg);
			exit(EXIT_SUCCESS);
                        break;
		case 't':
			if (mt_params.tx_rx != -1) {
				print_usage();
				exit(EXIT_FAILURE);
			}
			mt_params.tx_rx = 1;
			break;
		case 'r':
			if (mt_params.tx_rx != -1) {
				print_usage();
				exit(EXIT_FAILURE);
			}
			mt_params.tx_rx = 0;
			break;
		case 'k':
			mt_params.usermode = 0;
			break;
		case 'd':
			mmid_arr[mmid_num] = atoi(optarg);
			if (mmid_arr[mmid_num] <= 100 || mmid_arr[mmid_num] >= MM_ID_MAX) {
				printf("invaild mmid %d\n", mmid_arr[mmid_num]);
				exit(EXIT_FAILURE);
			}

			mmid_num += 1;
			if (mmid_num >= MAX_MMID_NUM) {
				printf("The maximum number of mmid supported is %d\n", MAX_MMID_NUM);
				exit(EXIT_FAILURE);
			}
			break;
		case 'm':
			mt_params.mem_size = atoi(optarg);
			break;
		case 'l':
			mt_params.loop_cnt = atoi(optarg);
			break;
		case 'f':
			//mt_params.mem_soruce = atoi(optarg);
			scan_ret = sscanf(optarg, "%s", mem_type);
			if (scan_ret == EOF)
				break;

			if (!strcmp(mem_type, "mmap")) {
				mt_params.mem_source = MEM_SOURCE_MMAP;
				printf("Memory source is %s\n", mem_type);
			} else {
				printf("Input invalid parameter for -f option\n");
				print_usage();
				exit(EXIT_FAILURE);
			}

			break;
		case 'p':
			thread_num_per_mmid = atoi(optarg);
			break;
		case 'a':
			do_ping();
			break;
		case 'b':
			do_pong();
			break;
		case 's':
			do_echo_auto();
			exit(0);
			break;
		case 'v':
			if (argc < 5) {
				printf("invalid number of arguments %d\n", argc);
				print_usage();
				exit(EXIT_FAILURE);
			} else if (argc == 5)
				str_loops = "0";
			else
				str_loops = argv[5];

			len = strlen(argv[2]) + 2 + strlen(argv[3]) + 2 + strlen(argv[4]) + 2 + strlen(str_loops) + 2;
			str = malloc(len);
			if (str) {
				snprintf(str, len, "%s %s %s %s", argv[2], argv[3], argv[4], str_loops);
				do_ping_auto(str);
				free(str);
				exit(0);
			} else {
				printf("%d memory allocation failed\n", len);
				exit(EXIT_FAILURE);
			}
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	if ((mt_params.tx_rx < 0 || mt_params.tx_rx > 1) || (mt_params.mem_size <= 0) || (mt_params.loop_cnt <= 0) || (mmid_num == 0)) {
		printf("Error! batch mode testing parameters are not set properly, tx_rx = %d, mmid_num = %d, mem_size = %d, loop_cnt = %d\n",
			mt_params.tx_rx, mmid_num, mt_params.mem_size, mt_params.loop_cnt);
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (thread_num_per_mmid == 1  && mmid_num == 1) {
		printf("Single thread mode: mmid %d\n", mmid_arr[0]);
		mt_params.mm_id = mmid_arr[0];
		habops.test_mem(-1, mt_params.tx_rx, mt_params.mm_id, mt_params.mem_size, mt_params.loop_cnt, mt_params.mem_source);
	} else {
#ifndef WIN32
		pthread_t thread_ids[mmid_num];
#else
		pthread_t *thread_ids = malloc(sizeof(pthread_t) * mmid_num);
#endif
		uint32_t vc_dummy[mmid_num];
		int mmid_index;
		struct hab_mem_test_params test_para[mmid_num];
		int ret = 0;

		for (mmid_index = 0; mmid_index < mmid_num; mmid_index++) {
			memcpy(&(test_para[mmid_index]), &mt_params, sizeof(struct hab_mem_test_params));
			struct hab_mem_test_params *tmp_mt_params = &(test_para[mmid_index]);
			tmp_mt_params->mm_id = mmid_arr[mmid_index];
			tmp_mt_params->thread_number = thread_num_per_mmid;

			printf("Options: tx_rx = %d, mm_id = %d, mem_size = %d, loop_cnt = %d\n",
				tmp_mt_params->tx_rx, tmp_mt_params->mm_id, tmp_mt_params->mem_size, tmp_mt_params->loop_cnt);
			set_test_mode(*tmp_mt_params);

			ret = pthread_create(&thread_ids[mmid_index], NULL, multi_thread_test, tmp_mt_params);
			if (ret) {
				printf("pthread_create failed: %s\n", strerror(ret));
				exit(EXIT_FAILURE);
			}
		}

		for (mmid_index = 0; mmid_index < mmid_num; mmid_index++) {
			ret = pthread_join(thread_ids[mmid_index], NULL);
			if (ret) {
				printf("pthread_join failed: %s\n", strerror(ret));
			}
			printf("Join thread successfully, thread id:0x%lx\n", thread_ids[mmid_index]);
		}

	}
	hab_test_deinit();
	return ret;
}

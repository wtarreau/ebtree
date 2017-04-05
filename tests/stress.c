/*
 * ebtree benchmark on random accesses - Willy Tarreau - 2017/04/05
 *
 * The principle is the following : we create N nodes that we store in an
 * indexed array, and assign each of them a unique value between 0 and N-1, and
 * we have an empty tree. We loop for a given time doing this :
 *
 *    - pick X = random number between 0 and N-1
 *    - lookup value X in the tree => NODE
 *    - if NODE exists, take NNODE = next(NODE), and PNODE=prev(NODE), then
 *      delete NODE from the tree and put the node back into the array
 *    - if NODE doesn't exist, pick it from the array and insert it in the
 *      tree at position X ; take NNODE = next(NODE), PNODE = prev(NODE)
 *    - if NNODE, validate that NNODE->key > NODE->key
 *    - if PNODE, validate that PNODE->key < NODE->key
 *
 * This causes random lookup/insertion/removal and also uses next() and prev().
 * Threads are not used for now.
 *
 * Compilation :
 *  $ gcc -Ieb -pthread -DUSE_ALIGN -O3 -o stress \
 *        eb/eba32tree.c eb/ebatree.c tests/stress.c
 *
 * Execution :
 *  $ ./stress -j 1000 -r 4
 */

#include <sys/time.h>
#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ebtree.h"
#include "eb32tree.h"

#define MAXTHREADS	1

static struct eb_root tree;
static struct eb32_node **nodes;

static volatile unsigned int step;
static struct timeval start, stop;
static volatile unsigned int actthreads;

/* per-thread stats */
static struct {
	unsigned int lookup;
	unsigned int insert;
	unsigned int remove;
	unsigned int pad[13];
} stats[MAXTHREADS] __attribute((aligned(64)));

pthread_t thr[MAXTHREADS];
unsigned int nbthreads;
unsigned int arg_nodes = 1000;
unsigned int arg_run = 1;

/* display the message and exit with the code */
__attribute__((noreturn)) void die(int code, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(code);
}

#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

static inline struct timeval *tv_now(struct timeval *tv)
{
        gettimeofday(tv, NULL);
        return tv;
}

static inline unsigned long tv_ms_elapsed(const struct timeval *tv1, const struct timeval *tv2)
{
        unsigned long ret;

        ret  = ((signed long)(tv2->tv_sec  - tv1->tv_sec))  * 1000;
        ret += ((signed long)(tv2->tv_usec - tv1->tv_usec)) / 1000;
        return ret;
}

unsigned int rnd32()
{
        static unsigned int y = 1;

        y ^= y << 13;
        y ^= y >> 17;
        y ^= y << 5;
        return y;
}

unsigned int rnd32range(unsigned int range)
{
        unsigned long long res = rnd32();

        res *= (range + 1);
        return res >> 32;
}

void run(void *arg)
{
	int tid = (long)arg;
	struct eb32_node *node, *pnode, *nnode;
	volatile unsigned int loop;
	unsigned int rnd;

	/* step 0: create all threads */
	while (step == 0) {
		/* don't disturb pthread_create() */
		usleep(10000);
	}

	/* step 1 : wait for signal to start */
	__sync_fetch_and_add(&actthreads, 1);
	while (step == 1);

	/* step 2 : run */
	while (step == 2) {
		rnd = rnd32range(arg_nodes - 1);

		stats[tid].lookup++;
		node = eb32_lookup(&tree, rnd);

		if (node) {
			pnode = eb32_prev(node);
			nnode = eb32_next(node);
			eb32_delete(node);
			stats[tid].remove++;
		} else {
			node = eb32_insert(&tree, nodes[rnd]);
			pnode = eb32_prev(node);
			nnode = eb32_next(node);
			stats[tid].insert++;
		}

		if (pnode && pnode->key >= rnd)
			die(2, "pnode.key=%u rnd=%u\n", pnode->key, rnd);
		if (nnode && nnode->key <= rnd)
			die(2, "nnode.key=%u rnd=%u\n", nnode->key, rnd);
	}

	fprintf(stderr, "thread %d quitting\n", tid);

	/* step 3 : stop */
	__sync_fetch_and_sub(&actthreads, 1);
	pthread_exit(0);
}

/* stops all threads upon SIG_ALRM */
void alarm_handler(int sig)
{
	fprintf(stderr, "received signal %d\n", sig);
	step = 3;
}

void create_nodes(int num)
{
	struct eb32_node *node;

	nodes = memalign(64, sizeof(node) * num);
	if (!nodes)
		die(1, "Out of memory\n");

	while (num--) {
#ifdef USE_ALIGN
		node = memalign(64, sizeof(*node));
#else
		node = malloc(sizeof(*node));
#endif
		if (!node)
			die(1, "Out of memory\n");
		memset(node, 0, sizeof(*node));
		nodes[num] = node;
		node->key = num;
	}
}

void usage(int ret)
{
	die(ret, "usage: sched [-h] [-n nodes] [-r run_time]\n");
}

int main(int argc, char **argv)
{
	int i, err;
	unsigned int u;
	unsigned int lookup, insert, remove;

	nbthreads = 1;

	argc--; argv++;
	while (argc > 0) {
		if (!strcmp(*argv, "-n")) {
			if (--argc < 0)
				usage(1);
			arg_nodes = atol(*++argv);
		}
		else if (!strcmp(*argv, "-r")) {
			if (--argc < 0)
				usage(1);
			arg_run = atol(*++argv);
		}
		else if (!strcmp(*argv, "-h"))
			usage(0);
		else
			usage(1);
		argc--; argv++;
	}

	if (nbthreads >= MAXTHREADS)
		nbthreads = MAXTHREADS;

	actthreads = 0;	step = 0;

	if (arg_nodes <= 0)
		arg_nodes = 1;

	if (arg_run <= 0)
		arg_run = 1;

	create_nodes(arg_nodes);

	printf("Starting with %d nodes on %d thread%c\n", arg_nodes, nbthreads, (nbthreads > 1)?'s':' ');

	for (u = 0; u < nbthreads; u++) {
		err = pthread_create(&thr[u], NULL, (void *)&run, (void *)(long)u);
		if (err)
			die(1, "pthread_create(): %s\n", strerror(err));
	}

	__sync_fetch_and_add(&step, 1);  /* let the threads warm up and get ready to start */

	while (actthreads != nbthreads);

	signal(SIGALRM, alarm_handler);
	alarm(arg_run);

	gettimeofday(&start, NULL);
	__sync_fetch_and_add(&step, 1); /* fire ! */

	/* and wait for all threads to die */

	lookup = insert = remove = 0;
	for (u = 0; u < nbthreads; u++) {
		pthread_join(thr[u], NULL);
		lookup += stats[u].lookup;
		insert += stats[u].insert;
		remove += stats[u].remove;
	}

	gettimeofday(&stop, NULL);

	i = (stop.tv_usec - start.tv_usec);
	while (i < 0) {
		i += 1000000;
		start.tv_sec++;
	}
	i = i / 1000 + (int)(stop.tv_sec - start.tv_sec) * 1000;
	printf("threads: %d lookup: %u insert: %u remove: %u time(ms): %u rate(lps): %Ld\n",
	       nbthreads, lookup, insert, remove, i, lookup * 1000ULL / (unsigned)i);

	/* All the work has ended */

	exit(0);
}

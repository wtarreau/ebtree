/*
 * cebtree stress testing tool
 *
 * It runs several parallel threads each with their own test.
 * The test consists in organizing a large area in nodes, picking random
 * indexes and either deleting the node if it's already there, or inserting
 * it if it's not.
 *
 * The types are defined using the macros below.
 */

/* set defaults to cebul */
#ifndef INCLUDE_FILE
# define INCLUDE_FILE "cebul_tree.h"
# define DATA_TYPE    unsigned long
# define NODE_TYPE    struct ceb_node
# define ROOT_TYPE    struct ceb_root*
# define NODE_INS(r,k)     cebul_insert(r,k)
# define NODE_DEL(r,k)     cebul_delete(r,k)
# define NODE_FND(r,k)     cebul_lookup(r,k)
# define NODE_INTREE(n)    ceb_intree(n)
# define STORAGE_STRING 0  // greater for string size
#endif

#if !defined(STORAGE_STRING)
# define STORAGE_STRING 0
#endif

#include <sys/time.h>

#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include INCLUDE_FILE

/* settings for the test */

#define MAXTHREADS      256


/* Some utility functions */
#if defined(__TINYC__)
#include <stdatomic.h>
#define __atomic_load_n(addr, order) __atomic_load(addr, order)
#define __atomic_store_n(addr, value, order) __atomic_store(addr, value, order)
#define __builtin_trap() abort()
#define __thread
#endif

#ifndef container_of
#define container_of(ptr, type, name) ((type *)(((char *)(ptr)) - ((long)&((type *)0)->name)))
#endif

#define RND32SEED 2463534242U
static __thread uint32_t rnd32seed = RND32SEED;
static inline uint32_t rnd32()
{
	rnd32seed ^= rnd32seed << 13;
	rnd32seed ^= rnd32seed >> 17;
	rnd32seed ^= rnd32seed << 5;
	return rnd32seed;
}

#define RND64SEED 0x9876543210abcdefull
static __thread uint64_t rnd64seed = RND64SEED;
static inline uint64_t rnd64()
{
	rnd64seed ^= rnd64seed << 13;
	rnd64seed ^= rnd64seed >>  7;
	rnd64seed ^= rnd64seed << 17;
	return rnd64seed;
}

static inline unsigned long rndl()
{
	return (sizeof(long) < sizeof(uint64_t)) ? rnd32() : rnd64();
}

/* produce a random between 0 and range+1 */
static inline unsigned int rnd32range(unsigned int range)
{
        unsigned long long res = rnd32();

        res *= (range + 1);
        return res >> 32;
}

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

/* Returns the pointer to the '\0' or NULL if not enough space in dst */
char *ulltoa(unsigned long long n, char *dst, ssize_t size)
{
	ssize_t i = 0;
	char *res;

	switch(n) {
	case 10000000000000000000ULL ... 18446744073709551615ULL: i++; /* fall through */
	case 1000000000000000000ULL ... 9999999999999999999ULL: i++; /* fall through */
	case 100000000000000000ULL ... 999999999999999999ULL: i++; /* fall through */
	case 10000000000000000ULL ... 99999999999999999ULL: i++; /* fall through */
	case 1000000000000000ULL ... 9999999999999999ULL: i++; /* fall through */
	case 100000000000000ULL ... 999999999999999ULL: i++; /* fall through */
	case 10000000000000ULL ... 99999999999999ULL: i++; /* fall through */
	case 1000000000000ULL ... 9999999999999ULL: i++; /* fall through */
	case 100000000000ULL ... 999999999999ULL: i++; /* fall through */
	case 10000000000ULL ... 99999999999ULL: i++; /* fall through */
	case 1000000000ULL ... 9999999999ULL: i++; /* fall through */
	case 100000000ULL ... 999999999ULL: i++; /* fall through */
	case 10000000ULL ... 99999999ULL: i++; /* fall through */
	case 1000000ULL ... 9999999ULL: i++; /* fall through */
	case 100000ULL ... 999999ULL: i++; /* fall through */
	case 10000ULL ... 99999ULL: i++; /* fall through */
	case 1000ULL ... 9999ULL: i++; /* fall through */
	case 100ULL ... 999ULL: i++; /* fall through */
	case 10ULL ... 99ULL: i++; /* fall through */
	default: break; /* single char, nothing to add */
	}

	if (i + 2 > size)
		return NULL;

	res = dst + i + 1;
	*res = '\0';
	for (; i >= 0; i--) {
		dst[i] = n % 10ULL + '0';
		n /= 10ULL;
	}
	return res;
}

/* display the message and exit with the code */
__attribute__((noreturn)) void die(int code, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(code);
}

#ifndef NODEBUG
#define BUG_ON(x) do {							\
		if (x) {						\
			fprintf(stderr, "BUG at %s:%d after %lu loops: %s\n", \
				__func__, __LINE__, ctx->loops, #x);	\
			__builtin_trap();				\
		}							\
	} while (0)
#else
#define BUG_ON(x) do { } while (0)
#endif

/* flags for item->flags */
#define IN_TREE         0x00000001

/* one item */
struct item {
	NODE_TYPE node;
#if STORAGE_STRING > 0
	char key[STORAGE_STRING];
#else
	DATA_TYPE key;
#endif
	unsigned long flags;
};

/* thread context */
struct ctx {
	struct item *table;
	ROOT_TYPE root;
	unsigned long min, max;
	pthread_t thr;
	unsigned long loops;
	unsigned long ins;
} __attribute__((aligned(64)));


struct ctx th_ctx[MAXTHREADS] = { };
static volatile unsigned int actthreads;
static volatile unsigned int step, meas;
static struct timeval start, prev, now;
unsigned int nbthreads = 1;
unsigned int nbelem = 32768;
unsigned int arg_run = 1;
unsigned int arg_lkups = 0; // distance between two writes

/* run the test for a thread */
void run(void *arg)
{
	int tid = (long)arg;
	struct ctx *ctx = &th_ctx[tid];
	unsigned int idx;
	struct item *itm;
	NODE_TYPE *node1;
	int ign_write = 0;
	DATA_TYPE v;

	rnd32seed += tid + 1;
	rnd64seed += tid + 1;

	/* step 0: create all threads */
	while (__atomic_load_n(&step, __ATOMIC_ACQUIRE) == 0) {
		/* don't disturb pthread_create() */
		usleep(10000);
	}

	/* step 1 : wait for signal to start */
	__atomic_fetch_add(&actthreads, 1, __ATOMIC_SEQ_CST);

	while (__atomic_load_n(&step, __ATOMIC_ACQUIRE) == 1)
		;

	//cebl_default_dump(0, "test", 0, 0); // prologue

	/* step 2 : run */
	for (; __atomic_load_n(&step, __ATOMIC_ACQUIRE) == 2; ctx->loops++) {
		/* pick a random index number */
		idx = rnd32range(nbelem - 1);
		BUG_ON(idx >= nbelem);
		itm = &ctx->table[idx];
		if (itm->flags & IN_TREE) {
			//fprintf(stderr, "idx=%5u itm=%p key=%llu flg=%lu intr=%d\n", idx, itm, (unsigned long long)itm->key, itm->flags, NODE_INTREE(&itm->node));
			/* the item is expected to already be in the tree, so
			 * let's verify a few things.
			 */
			BUG_ON(!NODE_INTREE(&itm->node));

			if (ign_write) {
				/* only perform a lookup */
				node1 = NODE_FND(&ctx->root, itm->key);
				BUG_ON(!node1);
#if STORAGE_STRING > 0
				BUG_ON(strcmp(container_of(node1, struct item, node)->key, itm->key) != 0);
#else
				BUG_ON(container_of(node1, struct item, node)->key != itm->key);
#endif
				ign_write--;
			} else {
				/* If the new picked value matches the existing
				 * one, it is just removed. If it does not match,
				 * it is removed and we try to enter the new one.
				 */
				node1 = NODE_DEL(&ctx->root, &itm->node);
				ign_write = arg_lkups;

				//cebl_default_dump(&ctx->root, "del", &itm->node, ctx->loops);

				if (node1 != &itm->node) {
#if STORAGE_STRING > 0
					fprintf(stderr, "BUG@%d: node1=%p('%s',%d,%lu) itm->node=%p('%s',%d,%lu)\n", __LINE__,
						node1, container_of(node1, struct item, node)->key,
						NODE_INTREE(node1), container_of(node1, struct item, node)->flags,
						&itm->node, itm->key, NODE_INTREE(&itm->node), itm->flags);
#else
					fprintf(stderr, "BUG@%d: node1=%p(%llu,%d,%lu) itm->node=%p(%llu,%d,%lu)\n", __LINE__,
						node1, (unsigned long long)container_of(node1, struct item, node)->key,
						NODE_INTREE(node1), container_of(node1, struct item, node)->flags,
						&itm->node, (unsigned long long)itm->key, NODE_INTREE(&itm->node), itm->flags);
#endif
					BUG_ON(node1 != &itm->node);
				}
				BUG_ON(NODE_INTREE(node1));
				itm->flags &= ~IN_TREE;
			}
		} else {
			/* this item is not in the tree, let's invent a new
			 * value.
			 */
			BUG_ON(NODE_INTREE(&itm->node));

			v = rnd64();
			v >>= (v & 63);

#if STORAGE_STRING > 0
			ulltoa(v, itm->key, sizeof(itm->key));
#else
			itm->key = v;
#endif

#ifdef SET_KEY
			SET_KEY(&itm->node, itm->key);
#endif
			//fprintf(stderr, "idx=%5u itm=%p key=%llu flg=%lu intr=%d\n", idx, itm, (unsigned long long)itm->key, itm->flags, NODE_INTREE(&itm->node));
			node1 = NODE_INS(&ctx->root, &itm->node);

			//cebl_default_dump(&ctx->root, "ins", &itm->node, ctx->loops);

			BUG_ON(!NODE_INTREE(node1));
			/* note: we support both dups and unique entries */
			if (node1 == &itm->node)
				itm->flags |= IN_TREE;
			ctx->ins++;
		}
	}

	//cebl_default_dump(0, 0, 0, 0); // epilogue

	/* step 3 : stop */
	__atomic_fetch_sub(&actthreads, 1, __ATOMIC_SEQ_CST);
	pthread_exit(0);
}

/* wakes up on SIG_ALRM to report stats or to stop */
void alarm_handler(int sig)
{
	static unsigned long prev_loops, prev_ins;
	unsigned long loops = 0;
	unsigned long ins = 0;
	int i;

	for (i = 0; i < (int)nbthreads; i++) {
		loops += th_ctx[i].loops;
		ins += th_ctx[i].ins;
	}

	gettimeofday(&now, NULL);

	i = (now.tv_usec - prev.tv_usec);
	while (i < 0) {
		i += 1000000;
		prev.tv_sec++;
	}
	i = i / 1000 + (int)(now.tv_sec - prev.tv_sec) * 1000;

	printf("meas: %d threads: %d loops: %lu (%lu ins) time(ms): %u rate(lps): %llu (%llu ins)\n",
	       meas, nbthreads, loops - prev_loops, ins - prev_ins, i,
	       (loops - prev_loops) * 1000ULL / (unsigned)i, (ins - prev_ins) * 1000ULL / (unsigned)i);

	prev_loops = loops;
	prev_ins = ins;
	prev = now;
	meas++;

	/* re-schedule or stop */
	if (--arg_run)
		alarm(1);
	else
		__atomic_store_n(&step, 3, __ATOMIC_RELEASE);
}

void usage(const char *name, int ret)
{
	die(ret, "usage: %s [-h] [-d*] [-n nbelem] [-t threads] [-r run_secs] [-s seed] [-l lkups]\n", name);
}

int main(int argc, char **argv)
{
	unsigned long seed = 0;
	char *argv0 = *argv;
	unsigned int u;
	int debug = 0;
	int err;

	setlinebuf(stdout);

	argv++; argc--;

	while (argc && **argv == '-') {
		if (strcmp(*argv, "-d") == 0) {
			debug++;
		}
		else if (!strcmp(*argv, "-n")) {
			if (--argc < 0)
				usage(argv0, 1);
			nbelem = atol(*++argv);
		}
		else if (!strcmp(*argv, "-t")) {
			if (--argc < 0)
				usage(argv0, 1);
			nbthreads = atol(*++argv);
		}
		else if (!strcmp(*argv, "-s")) {
			if (--argc < 0)
				usage(argv0, 1);
			seed = atol(*++argv);
		}
		else if (!strcmp(*argv, "-r")) {
			if (--argc < 0)
				usage(argv0, 1);
			arg_run = atol(*++argv);
		}
		else if (!strcmp(*argv, "-l")) {
			if (--argc < 0)
				usage(argv0, 1);
			arg_lkups = atol(*++argv);
		}
		else if (strcmp(*argv, "-h") == 0)
			usage(argv0, 0);
		else
			usage(argv0, 1);
		argc--; argv++;
	}

	if (nbthreads >= MAXTHREADS)
		nbthreads = MAXTHREADS;

	rnd32seed += seed;
	rnd64seed += seed;

	actthreads = 0;	step = 0;

	printf("Starting %d thread%c for %d elems each\n", nbthreads, (nbthreads > 1)?'s':' ', nbelem);

	for (u = 0; u < nbthreads; u++) {
		th_ctx[u].table = calloc(nbelem, sizeof(*th_ctx[u].table));
		if (!th_ctx[u].table)
			die(1, "not enough memory for calloc(nbelem*threads)\n");

		err = pthread_create(&th_ctx[u].thr, NULL, (void *)&run, (void *)(long)u);
		if (err)
			die(1, "pthread_create(): %s\n", strerror(err));
	}

	/* prepare the threads to start */
	__atomic_fetch_add(&step, 1, __ATOMIC_SEQ_CST);

	/* wait for them all to be ready */
	while (__atomic_load_n(&actthreads, __ATOMIC_ACQUIRE) != nbthreads)
		;

	signal(SIGALRM, alarm_handler);
	alarm(1);

	gettimeofday(&start, NULL);
	prev = start;

	/* Go! */
	__atomic_fetch_add(&step, 1, __ATOMIC_SEQ_CST);

	/* Threads are now running until the alarm rings. Wait for them all to
	 * die. alarm will display the stats and finish.
	 */
	for (u = 0; u < nbthreads; u++)
		pthread_join(th_ctx[u].thr, NULL);
	return 0;
}

/*
 * ebtree performance evaluation for schedulers - Willy Tarreau - 2017/04/04
 *
 * This test consists in dequeuing a task and requeuing it at a later position
 * after an arbitrary amount of work. At the end the total and average amount
 * of work performed is reported. Each thread does this :
 *
 *  - lookup of the best task to pick (eb_lookup_ge)
 *  - extraction of the task (eb_delete)
 *  - fake work (for loop on <work_time> cycles)
 *  - requeuing of the task
 *
 * The run queue insertion index may be shared (default) or thread-local
 * (-DUSE_TLS). The latter option causes more variations to occur during the
 * tests. Tasks may be allocated memory-aligned (-DUSE_ALIGN) to avoid false
 * sharing of cache lines. The run queue lock may be implemented using
 * spinlocks (default) or mutexes (-DUSE_MUTEX). The latter is slower but
 * degrades less under contention. The total number of tasks in the run queue
 * is specified with -j. The total run time is specified with -r (seconds).
 *
 * Compilation :
 *  $ gcc -Ieb -pthread -DUSE_MUTEX -DUSE_TLS -DUSE_ALIGN -O3 -o sched \
 *        eb/eba32tree.c eb/ebatree.c tests/sched.c
 *
 * Execution :
 *  $ taskset -c 0,1,2,3 ./sched -j 1000 -t 4 -r 4
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

#define MAXTHREADS	64

struct task {
	struct task *next;
	struct eb32_node tree;
};

/* a high initial rq_idx ensures that eb_lookup_ge is involved */
#ifdef USE_TLS
/* avg is slightly higher with TLS but the variance is around 30%! */
static __thread unsigned int rq_idx = 0x20000000;
#else
static unsigned int rq_idx = 0x20000000;
#endif

static unsigned int rq_idx_global = 0x20000000;

#if USE_MUTEX
static pthread_mutex_t rq_lock = PTHREAD_MUTEX_INITIALIZER;
#elif USE_SPINLOCK
static pthread_spinlock_t rq_lock;
#endif

static struct eb_root run_queue;
static struct task *tasks;

static volatile unsigned int step;
static struct timeval start, stop;
static volatile unsigned int actthreads;

/* per-thread stats */
static struct {
	unsigned int done;
	unsigned int fail;
	unsigned int pad[14];
} stats[MAXTHREADS] __attribute((aligned(64)));

pthread_t thr[MAXTHREADS];
unsigned int nbthreads, arg_wait;
unsigned int arg_jobs = 1000;
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

/* adds a task to the next position in the RQ */
static inline void task_queue(struct task *task)
{
	int new_idx;

	new_idx = 0;
#ifdef USE_TLS
	new_idx = rq_idx++;
#else
	new_idx = __sync_fetch_and_add(&rq_idx, 1);
#endif
	task->tree.key = new_idx;

#ifdef USE_MUTEX
	pthread_mutex_lock(&rq_lock);
#elif USE_SPINLOCK
	pthread_spin_lock(&rq_lock);
#endif
	eb32_insert(&run_queue, &task->tree);
#ifdef USE_MUTEX
	pthread_mutex_unlock(&rq_lock);
#elif USE_SPINLOCK
	pthread_spin_unlock(&rq_lock);
#endif
}


/* picks the oldest task from the RQ */
static inline struct task *task_pick_next()
{
	struct task *task;
	struct eb32_node *node;

#ifdef USE_MUTEX
	pthread_mutex_lock(&rq_lock);
#elif USE_SPINLOCK
	pthread_spin_lock(&rq_lock);
#endif
	node = eb32_lookup_ge(&run_queue, rq_idx - (1U << 31));
	if (!node)
		//node = eb32_first(&run_queue);
		node = eb32_lookup_ge(&run_queue, 0);
	if (node)
		eba32_delete(&run_queue, node);
#ifdef USE_MUTEX
	pthread_mutex_unlock(&rq_lock);
#elif USE_SPINLOCK
	pthread_spin_unlock(&rq_lock);
#endif
	if (node)
		task = container_of(node, struct task, tree);
	else
		task = NULL;
	return task;
}


void run(void *arg)
{
	int tid = (long)arg;
	struct task *task;
	volatile unsigned int loop;

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
		int rounds;

#ifdef USE_TLS
		rq_idx = rq_idx_global;
#endif
		for (rounds = 100; step == 2 && rounds; rounds--) {
			task = task_pick_next();
			if (!task) {
				stats[tid].fail++;
				continue;
			}
			/* "work" */
			for (loop = 0; loop < arg_wait; loop++);
			task_queue(task);
			stats[tid].done++;
		}
#ifdef USE_TLS
		__sync_fetch_and_add(&rq_idx_global, 100);
#endif
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

void start_tasks()
{
	struct task *task;

	for (task = tasks; task; task = task->next) {
		task_queue(task);
	}
}

void create_tasks(int num)
{
	struct task *task;

	while (num--) {
#ifdef USE_ALIGN
		task = memalign(64, sizeof(*task));
#else
		task = malloc(sizeof(*task));
#endif
		if (!task)
			die(1, "Out of memory\n");
		memset(task, 0, sizeof(*task));
		task->next = tasks;
		tasks = task;
	}
}

void usage(int ret)
{
	die(ret, "usage: sched [-h] [-w work_time] [-j jobs] [-t threads] [-r run_time]\n");
}

int main(int argc, char **argv)
{
	int i, err;
	unsigned int u;
	unsigned int done, fail;

	nbthreads = 1;
	arg_wait = 1;

	argc--; argv++;
	while (argc > 0) {
		if (!strcmp(*argv, "-t")) {
			if (--argc < 0)
				usage(1);
			nbthreads = atol(*++argv);
		}
		else if (!strcmp(*argv, "-w")) {
			if (--argc < 0)
				usage(1);
			arg_wait = atol(*++argv);
		}
		else if (!strcmp(*argv, "-j")) {
			if (--argc < 0)
				usage(1);
			arg_jobs = atol(*++argv);
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

	if (arg_jobs <= 0)
		arg_jobs = 1;

	if (arg_run <= 0)
		arg_run = 1;

#ifdef USE_SPINLOCK
	pthread_spin_init(&rq_lock, PTHREAD_PROCESS_PRIVATE);
#endif
	create_tasks(arg_jobs);
	start_tasks();

	printf("Starting %d jobs on %d thread%c\n", arg_jobs, nbthreads, (nbthreads > 1)?'s':' ');

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

	/* and wait for them to die */

	done = fail = 0;
	for (u = 0; u < nbthreads; u++) {
		pthread_join(thr[u], NULL);
		done += stats[u].done;
		fail += stats[u].fail;
	}

	gettimeofday(&stop, NULL);

	i = (stop.tv_usec - start.tv_usec);
	while (i < 0) {
		i += 1000000;
		start.tv_sec++;
	}
	i = i / 1000 + (int)(stop.tv_sec - start.tv_sec) * 1000;
	printf("threads: %d done: %u fail: %u time(ms): %u rate(lps): %Ld\n",
	       nbthreads, done, fail, i, done * 1000ULL / (unsigned)i);

	/* All the work has ended */

	exit(0);
}

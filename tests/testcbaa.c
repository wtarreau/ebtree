#include <sys/time.h>

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cbaatree.h"

struct cbaa_node *cbaa_root;
static struct timeval last_tv;
static volatile long free_ops;
static volatile long last_free_ops;
static volatile long alloc_ops;
static volatile long last_alloc_ops;
static volatile long alloc_bytes;
static volatile long last_alloc_bytes;
static volatile unsigned long used;
static volatile unsigned long objects;

struct wd_large_entry {
	size_t size;
	struct cbaa_node by_addr;
};

__attribute__((noreturn)) void die(int code, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(code);
}

void handle_alarm(int sig)
{
	struct timeval tv;
	double sec;
	long us;

	gettimeofday(&tv, NULL);
	us = (tv.tv_sec - last_tv.tv_sec) * 1000000 + tv.tv_usec - last_tv.tv_usec;
	sec = us / 1000000.0;
	last_tv = tv;

	alarm(1); // avoid delaying by the printf() time.

	printf("obj: %ld, B: %ld, Mops: %ld, Mfree/s: %3.1f, Malloc/s: %3.1f, MBalloc/s: %3.1f\n",
	       objects,
	       used,
	       (free_ops + alloc_ops) >> 20,
	       (free_ops - last_free_ops) / sec / 1000000,
	       (alloc_ops - last_alloc_ops) / sec / 1000000,
	       (alloc_bytes - last_alloc_bytes) / sec / 1048576);

	last_free_ops = free_ops;
	last_alloc_ops = alloc_ops;
	last_alloc_bytes = alloc_bytes;
}

/* Xorshift RNGs from http://www.jstatsoft.org/v08/i14/paper */
#define uint32_t unsigned int
#define uint64_t unsigned long long

uint32_t rnd32()
{
	static uint32_t y = 2463534242U;

	y ^= y << 13;
	y ^= y >> 17;
	y ^= y << 5;
	return y;
}

uint32_t rnd32range(unsigned int range)
{
	uint64_t res = rnd32();

	res *= (range + 1);
	return res >> 32;
}

/* size-aware functions */
static inline struct wd_large_entry *wd_large_lookup_le(struct cbaa_node **root, struct wd_large_entry *data)
{
	return container_of_safe(cbaa_lookup_le(root, &data->by_addr), struct wd_large_entry, by_addr);
}

static inline struct wd_large_entry *wd_large_lookup_ge(struct cbaa_node **root, struct wd_large_entry *data)
{
	return container_of_safe(cbaa_lookup_ge(root, &data->by_addr), struct wd_large_entry, by_addr);
}

static inline struct wd_large_entry *wd_large_insert(struct cbaa_node **root, struct wd_large_entry *data, unsigned long size)
{
	if (size < sizeof(*data))
		return NULL;

	data->size = size;
	return container_of_safe(cbaa_insert(root, &data->by_addr), struct wd_large_entry, by_addr);
}

static void dump_node(struct cbaa_node *node, int level)
{
	printf("visiting node %p at level %d\n", node, level);
}

static void dump_leaf(struct cbaa_node *node, int level)
{
	printf("visiting leaf %p at level %d\n", node, level);
}

void leave(int sig)
{
	//visit_node(cbaa_root, 0, NULL);
	exit(0);
}

int main(int argc, char **argv)
{
	unsigned long max;
	unsigned long max_block;
	unsigned long size;

	void *base, *addr/*, *data*/;
	struct wd_large_entry *prev, *next/*, *next_prev, *prev_next*/;

	if (argc <= 1)
		die(1, "Missing argument: maxsize(bytes) [max_block(bytes)]\n");

	max = atol(argv[1]);

	max_block = 1;
	while (max_block * max_block < max / 16)
		max_block *= 2;

	if (argc > 2)
		max_block = atol(argv[2]);

	base = calloc(max, 1);
	if (!base)
		die(1, "out of memory\n");

	setlinebuf(stdout);
	printf("Max total=%ld, max block=%ld\n", max, max_block);

	signal(SIGALRM, handle_alarm);
	signal(SIGINT, leave);

	gettimeofday(&last_tv, NULL);
	alarm(1);

	//cbaa_insert(&cbaa_root, (void *)0x602040, 0x20);
	//cbaa_insert(&cbaa_root, (void *)0x602018, 0x20);
	//cbaa_insert(&cbaa_root, (void *)0x602070, 0x20);
	//cbaa_insert(&cbaa_root, (void *)0x6020A0, 0x20);

	//printf("prev1=%p\n", cbaa_lookup_ge(&cbaa_root, (void *)0x602080));
	//exit(0);

	// //cbaa_insert(&cbaa_root, (void *)0x6023d8, 0x20);
	// cbaa_insert(&cbaa_root, (void *)0x602088, 0x20);
	// cbaa_insert(&cbaa_root, (void *)0x602110, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602340, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x6020d0, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602208, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602158, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x6022d8, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602408, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x6023a0, 0x20);
	// cbaa_insert(&cbaa_root, (void *)0x602170, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602278, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602378, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602020, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602248, 0x20);
	//
	// //cbaa_insert(&cbaa_root, (void *)0x602170, 0x20);
	// //cbaa_insert(&cbaa_root, (void *)0x602278, 0x20);
	// //printf("prev1=%p\n", cbaa_lookup_le(&cbaa_root, (void *)0x602170));
	// printf("prev2=%p\n", cbaa_lookup_le(&cbaa_root, (void *)0x60218f));
	// cbaa_insert(&cbaa_root, (void *)0x602170, 0x20);
	// exit(0);

	while (1) {
		/* Ensure most objects are small */
		size = rnd32range(max_block);
		size = size * size * size * size / (max_block * max_block * max_block);
		size = (size + 64) & ~63;

		while (1) {
			addr = base + (rnd32range(max - 1) & -64);

			alloc_ops++;
			next = wd_large_lookup_ge(&cbaa_root, addr);
			if (next) {
				if ((void *)next < addr) {
					cbaa_dump_tree(cbaa_root, 0, NULL, 0, dump_node, dump_leaf);
					die(4, "1: addr=%p addr+size=%p next=%p!\n", addr, addr+size, next);
				}

				if ((addr + size) > (void *)next) {
					//printf("addr=%p-%p collides with next=%p-%p\n", addr, addr + size - 1, next, ((void *)next + next->size) - 1);
					continue;
				}
			}

			free_ops++;
			prev = wd_large_lookup_le(&cbaa_root, addr - 1);
			if (prev) {
				if ((void *)prev > addr) {
					cbaa_dump_tree(cbaa_root, 0, NULL, 0, dump_node, dump_leaf);
					die(6, "2: addr=%p addr+size=%p prev=%p!\n", addr, addr+size, prev);
				}

				if (((void *)prev + prev->size) > addr) {
					//printf("addr=%p-%p collides with prev=%p-%p\n", addr, addr + size - 1, prev, ((void *)prev + prev->size) - 1);
					continue;
				}
			}

			//printf("prev=%p-%p\n", prev, prev ? ((void *)prev + prev->size) - 1 : NULL);
			//if (prev && ((void *)prev + prev->size) > addr) {
			//	//printf("addr=%p-%p collides with prev=%p-%p\n", addr, addr + size - 1, prev, ((void *)prev + prev->size) - 1);
			//	//alloc_ops++;
			//	if (alloc_ops >= 1000000)
			//		goto end;
			//	continue;
			//}

			//if (prev && (void *)prev >= (void *)addr && ((void *)addr + size) > (void *)prev) {
			//	//printf("addr=%p-%p collides with prev=%p\n", addr, ((void *)addr + size) - 1, prev);
			//	//alloc_ops++;
			//	if (alloc_ops >= 1000000)
			//		goto end;
			//	continue;
			//}

			break;
		}

		//printf("#### inserting %p-%p\n", addr, ((void *)addr + size) - 1);
		wd_large_insert(&cbaa_root, addr, size);
		objects++;
		used += size;

		//next_prev = next;
		//next = cbaa_lookup_ge(&cbaa_root, addr + size);
		//
		//if (next != next_prev) {
		//	visit_node(cbaa_root, 0, NULL);
		//	die(4, "3: addr=%p addr+size=%p next=%p was %p before insert!\n", addr, addr+size, next, next_prev);
		//}
		//
		//if (next) {
		//	if ((void *)next < (addr + size)) {
		//		visit_node(cbaa_root, 0, NULL);
		//		die(4, "3: addr=%p addr+size=%p next=%p!\n", addr, addr+size, next);
		//	}
		//
		//	next_prev = cbaa_lookup_le(&cbaa_root, (void *)next - 1);
		//	if (next_prev && (void *)next_prev != addr) {
		//		visit_node(cbaa_root, 0, NULL);
		//		die(5, "4: addr=%p addr+size=%p next=%p le(%p)=%p!\n", addr, addr+size, next, (void *)next - 1, next_prev);
		//	}
		//}
		//
		//prev_next = prev;
		//prev = cbaa_lookup_le(&cbaa_root, addr - 1);
		//
		//if (prev != prev_next) {
		//	visit_node(cbaa_root, 0, NULL);
		//	die(4, "5: addr=%p addr+size=%p prev=%p was %p before insert!\n", addr, addr+size, prev, prev_next);
		//}
		//
		//if (prev) {
		//	if ((void *)prev > (addr + size)) {
		//		visit_node(cbaa_root, 0, NULL);
		//		die(6, "5: addr=%p addr+size=%p prev=%p!\n", addr, addr+size, prev);
		//	}
		//
		//	prev_next = cbaa_lookup_ge(&cbaa_root, (void *)prev + 1);
		//	if (prev_next && (void *)prev_next != addr) {
		//		visit_node(cbaa_root, 0, NULL);
		//		die(7, "6: addr=%p addr+size=%p next=%p prev=%p ge(%p)=%p!\n", addr, addr+size, next, prev, (void *)prev + 1, prev_next);
		//	}
		//}

		//while (used + size > max) {
		//force_free:
		//	next = tail->next;
		//	used -= tail->size;
		//	free(tail);
		//	tail = next;
		//	free_ops++;
		//	objects--;
		//}

		//next = malloc(size);
		//if (!next) {
		//	if (!used)
		//		die(3, "Out of memory after %ld allocs and %d bytes\n", alloc_ops, alloc_bytes);
		//	goto force_free;
		//}

		//next->next = NULL;
		//next->size = size;
		//if (head)
		//	head->next = next;
		//head = next;
		//if (!tail)
		//	tail = next;
		//used += size;
		//alloc_bytes += size;
		//alloc_ops++;
		//objects++;
	}

	//visit_node(cbaa_root, 0, NULL);
	return 0;
}

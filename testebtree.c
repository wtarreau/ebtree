#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "../../../public/haproxy-1.3/include/common/time.h"

#include "ebtree.h"

//#include "../../../public/haproxy-1.3/include/common/defaults.h"
//#include "../../../public/haproxy-1.3/include/common/config.h"
//#include "../../../public/haproxy-1.3/include/common/standard.h"
//#include "../../../public/haproxy-1.3/include/common/time.h"

EB32_TREE_HEAD(root);

struct eb32_node *eb32_insert(struct eb32_hnode *root, struct eb32_node *new) {
    return __eb32_insert(root, new);
}

struct eb32_hnode *eb32_delete(struct eb32_node *node) {
    return __eb32_delete(node);
}

struct eb32_node *eb32_lookup(struct eb32_hnode *root, unsigned long x) {
    return __eb32_lookup(root, x);
}


unsigned long total_jumps = 0;

static unsigned long rev32(unsigned long x) {
    unsigned long y;

    x = ((x & 0xFFFF0000) >> 16) | ((x & 0x0000FFFF) << 16);
    x = ((x & 0xFF00FF00) >>  8) | ((x & 0x00FF00FF) <<  8);
    x = ((x & 0xF0F0F0F0) >>  4) | ((x & 0x0F0F0F0F) <<  4);
    x = ((x & 0xCCCCCCCC) >>  2) | ((x & 0x33333333) <<  2);
    x = ((x & 0xAAAAAAAA) >>  1) | ((x & 0x55555555) <<  1);
    return x;
}


int main(int argc, char **argv) {
    char buffer[1024];
    int total = 0, i;
    unsigned long links_used = 0;
    unsigned long neighbours = 0;
    unsigned long long x;
    struct eb32_node *node, *lastnode;
    struct timeval t_start, t_random, t_insert, t_lookup, t_walk, t_delete;

    if (argc < 2) {
	tv_now(&t_start);
	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
	    char *ret = strchr(buffer, '\n');
	    if (ret)
		*ret = 0;
	    //printf("read=%lld\n", x);
	    x = atoll(buffer);
	    total++;
	    node = (struct eb32_node *)malloc(sizeof(*node));
	    node->leaf.val = x;

	    eb32_insert(&root, node);
	}
	tv_now(&t_random);
	tv_now(&t_insert);
    }
    else {
	total = atol(argv[1]);

	/* preallocation */
	tv_now(&t_start);

	printf("Timing %d random()+malloc...\n", total);
	lastnode = NULL;
	for (i = 0; i < total; i++) {
	    ////unsigned long long x = (i << 16) + ((random() & 63ULL) << 32) + (random() % 1000);
	    //unsigned long l = (random() % 1000)*1000; // to emulate tv_usec based on milliseconds
	    //unsigned long h = ((random() & 63ULL)<<8) + (i%100); // to emulate lots of close seconds
	    ////unsigned long long x = ((unsigned long long)h << 32) + l;
	    //unsigned long long x = ((unsigned long long)h << 16) + l;
	    unsigned long x = random();     // triggers worst case cache patterns

	    node = (struct eb32_node *)calloc(1,sizeof(*node));
	    node->leaf.val = (x>>10)&65535;//i&65535;//(x>>8)&65535;//rev32(i);//i&32767;//x;//i ^ (long)lastnode;
	    node->leaf.parent = (void *)lastnode;
	    lastnode = node;
	}

	tv_now(&t_random);

	printf("Timing %d insert...\n", total);
	for (i = 0; i < total; i++) {
	    node = lastnode;
	    lastnode = (void *)node->leaf.parent;
	    eb32_insert(&root, node);
	    if (!node->leaf.parent)
		neighbours++;
	    else if (node->link.bit)
		links_used++;
	}

	tv_now(&t_insert);
    }

    printf("Looking up %d entries...\n", total);
    for (i = 0; i < total; i++) {
	unsigned long long x = (i << 16) + ((random() & 63ULL) << 32) + (random() % 1000);
	node = eb32_lookup(&root, x);
	if (node && (node->leaf.val != (int)x)) {
	    printf("node = %p, wanted = %d, returned = %d\n", node, (int)x, node->leaf.val);
	}
    }

    tv_now(&t_lookup);
#if 1
    printf("Walking forwards %d entries...\n", total);
    node = eb32_first(&root);
    while (node) {
	//printf("node = %p, node->val = 0x%08x, parent=%p, parent->bit=%d\n",
	//       node, node->leaf.val, node->leaf.parent, node->leaf.parent?node->leaf.parent->bit:-1);
	node = eb_next(node);
    }
#endif

    printf("Walking backwards %d entries...\n", total);
    node = eb32_last(&root);
    while (node) {
	//printf("node = %p, node->val = 0x%08x, parent=%p, parent->bit=%d\n",
	//       node, node->leaf.val, node->leaf.parent, node->leaf.parent?node->leaf.parent->bit:-1);
	node = eb_prev(node);
    }
    tv_now(&t_walk);

    printf("Deleting %d entries...\n", total);
    node = eb32_first(&root);

#if 1
    /* slightly faster version, but harder to understand */
    while (node) {
	struct eb32_hnode *next;

	//printf("deleting %p: node->val = 0x%08x, parent=%p\n", node, node->leaf.val, node->leaf.parent);

	next = eb32_delete(node);
	if (!next)
	    break;
	//node = eb32_walk_down_left(next);
	node = eb_walk_down_left(next, typeof(*node));
    }
#else

    while (node) {
	struct eb32_node *node2;

	node2 = eb32_next(node);
	//printf("deleting %p: node->val = 0x%08x, parent=%p\n", node, node->leaf.val, node->leaf.parent);

	eb32_delete(node);
	node = node2;
    }

#endif

    node = eb32_first(&root);


    tv_now(&t_delete);


    printf("total=%lu, links=%lu, neighbours=%lu entries, total_jumps=%lu\n", total, links_used, neighbours, total_jumps);
    printf("random+malloc =%d ms\n", __tv_ms_elapsed(&t_start, &t_random));
    printf("insert        =%d ms\n", __tv_ms_elapsed(&t_random, &t_insert));
    printf("lookup        =%d ms\n", __tv_ms_elapsed(&t_insert, &t_lookup));
    printf("walk          =%d ms\n", __tv_ms_elapsed(&t_lookup, &t_walk));
    printf("delete        =%d ms\n", __tv_ms_elapsed(&t_walk, &t_delete));

#if 0
    tv_now(&t_walk_start);
#define MAXLEN 64
    {
	void *stack[MAXLEN];
	int slen;
	struct leaf *leaf;
	void *list;
	tree64_foreach/*_destructive*/(&root, list, stack, slen) {
	    node_count++;
	    foreach_dlist_item_cst(leaf, list, struct leaf*, list) {
		  val_count++;
		  //printf("%016llx\n", leaf->val);
		  //printf("%08x\n", (unsigned)leaf->val);
		  //pool_free(leaf, leaf);
	    }
	    //((struct ultree*)stack[slen])->data = NULL;
	}
    }
    
    tv_now(&t_walk_stop);

    printf("time: alloc:%ld ms, random:%ld ms, insert:%ld-%ld=%ld ms (%ld k.insert/s), lookup: %ld-%ld=%ld ms, walk=%ld ms\n",
	   tv_delta_ms(&t_start, &t_alloc),
	   tv_delta_ms(&t_alloc, &t_random),
	   tv_delta_ms(&t_random, &t_insert), tv_delta_ms(&t_alloc, &t_random), 
	   tv_delta_ms(&t_random, &t_insert) - tv_delta_ms(&t_alloc, &t_random),
	   total / (tv_delta_ms(&t_random, &t_insert) - tv_delta_ms(&t_alloc, &t_random)),
	   tv_delta_ms(&t_insert, &t_lookup), tv_delta_ms(&t_alloc, &t_random),
	   tv_delta_ms(&t_insert, &t_lookup) - tv_delta_ms(&t_alloc, &t_random),
	   tv_delta_ms(&t_walk_start, &t_walk_stop));
    
    printf("total=%d, node_count=%d, val_count=%d, node_lookup=%d, node_right_lookup=%d (left=%d)\n", total, node_count, val_count,
	   node_lookup, node_right_lookup, node_lookup - node_right_lookup);
    return 0;
#endif

}

OBJS = ebtree.o eb32tree.o eb64tree.o ebmbtree.o ebsttree.o ebimtree.o ebistree.o
CFLAGS = -O3 -W -Wall -Wextra -Wundef -Wdeclaration-after-statement -Wno-address-of-packed-member
EXAMPLES = $(basename $(wildcard examples/*.c))
TEST_DIR = tests
TEST_BIN = $(addprefix $(TEST_DIR)/,test32 test64 testst bencheb32 bencheb64 benchebmb benchebst)

all: libebtree.a

examples: ${EXAMPLES}

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

examples/%: examples/%.c libebtree.a
	$(CC) $(CFLAGS) -I. -DSTANDALONE -o $@ $< -L. -lebtree

test: $(TEST_BIN)

tests/test%: test%.c libebtree.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lebtree

tests/bencheb32: tests/bench.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree -pthread -DINCLUDE_FILE='"eb32tree.h"' -DDATA_TYPE='unsigned int' -DNODE_TYPE='struct eb32_node' -D'SET_KEY(n,k)=(n)->key=k' -DROOT_TYPE='struct eb_root' -D'NODE_INS(r,n)=eb32_insert(r,n)' -D'NODE_DEL(r,n)=({ eb32_delete((n)); (n); })' -D'NODE_FND(r,k)=eb32_lookup(r,k)' -D'NODE_INTREE(n)=(!!(n)->node.leaf_p)'

tests/bencheb64: tests/bench.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree -pthread -DINCLUDE_FILE='"eb64tree.h"' -DDATA_TYPE='unsigned long long' -DNODE_TYPE='struct eb64_node' -D'SET_KEY(n,k)=(n)->key=k' -DROOT_TYPE='struct eb_root' -D'NODE_INS(r,n)=eb64_insert(r,n)' -D'NODE_DEL(r,n)=({ eb64_delete((n)); (n); })' -D'NODE_FND(r,k)=eb64_lookup(r,k)' -D'NODE_INTREE(n)=(!!(n)->node.leaf_p)'

tests/benchebmb: tests/bench.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree -pthread -DINCLUDE_FILE='"ebmbtree.h"' -DDATA_TYPE='unsigned long long' -DNODE_TYPE='struct ebmb_node' -DROOT_TYPE='struct eb_root' -D'NODE_INS(r,n)=ebmb_insert(r,n,sizeof(long long))' -D'NODE_DEL(r,n)=({ ebmb_delete((n)); (n); })' -D'NODE_FND(r,k)=ebmb_lookup(r,&k,sizeof(long long))' -D'NODE_INTREE(n)=(!!(n)->node.leaf_p)'

tests/benchebst: tests/bench.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree -pthread -DINCLUDE_FILE='"ebsttree.h"' -DDATA_TYPE='unsigned long long' -DNODE_TYPE='struct ebmb_node' -DROOT_TYPE='struct eb_root' -D'NODE_INS(r,n)=ebst_insert(r,n)' -D'NODE_DEL(r,n)=({ ebmb_delete((n)); (n); })' -D'NODE_FND(r,k)=ebst_lookup(r,k)' -D'NODE_INTREE(n)=(!!(n)->node.leaf_p)' -DSTORAGE_STRING=24

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core ${EXAMPLES} ${TEST_BIN}

ifeq ($(wildcard .git),.git)
VERSION := $(shell [ -d .git/. ] && ref=`(git describe --tags --match 'v*') 2>/dev/null` && ref=$${ref%-g*} && echo "$${ref\#v}")
SUBVERS := $(shell comms=`git log --no-merges v$(VERSION).. 2>/dev/null |grep -c ^commit `; [ $$comms -gt 0 ] && echo "-$$comms" )
endif

git-tar: .git
	git archive --format=tar --prefix="ebtree-$(VERSION)/" HEAD | gzip -9 > ebtree-$(VERSION)$(SUBVERS).tar.gz

.PHONY: examples tests

OBJS = ebtree.o eb32tree.o eb64tree.o ebmbtree.o ebsttree.o ebimtree.o ebistree.o
CFLAGS = -O3 -W -Wall -Wdeclaration-after-statement

all: libebtree.a

examples: examples/reduce

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

examples/reduce: examples/reduce.c libebtree.a
	$(CC) $(CFLAGS) -I. -o $@ $< -L. -lebtree

test: test32 test64 testst

test%: test%.c libebtree.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lebtree

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core test32 test64 testst examples/reduce

.PHONY: examples tests

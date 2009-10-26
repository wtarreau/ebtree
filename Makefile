OBJS = ebtree.o eb32tree.o eb64tree.o ebmbtree.o

all: libebtree.a

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

test: test32 test64

test%: test%.c libebtree.a
	$(CC) $(CFLAGS) -o $@ $< -L. -lebtree

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core test32 test64

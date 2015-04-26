CFLAGS = -O3 -W -Wall -Wdeclaration-after-statement -Wno-unused-parameter
EXAMPLES = $(basename $(wildcard examples/*.c))

COMMON_DIR = common

CB_DIR = cb
CB_SRC = $(wildcard $(CB_DIR)/cba*.c)
CB_OBJ = $(CB_SRC:%.c=%.o)

EB_DIR = eb
EB_SRC = $(wildcard $(EB_DIR)/eba*.c $(EB_DIR)/ebl*.c $(EB_DIR)/ebm*.c $(EB_DIR)/ebs*.c)
EB_OBJ = $(EB_SRC:%.c=%.o)

OBJS = $(CB_OBJ) $(EB_OBJ)

TEST_DIR = tests
TEST_BIN = $(addprefix $(TEST_DIR)/,test32 test64 testst testcbaa)

all: libebtree.a

examples: ${EXAMPLES}

libebtree.a: $(OBJS)
	$(AR) rv $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -I$(COMMON_DIR) -o $@ -c $^

examples/%: examples/%.c libebtree.a
	$(CC) $(CFLAGS) -I$(COMMON_DIR) -I$(CB_DIR) -I$(EB_DIR) -o $@ $< -L. -lebtree

test: $(TEST_BIN)

test%: test%.c libebtree.a
	$(CC) $(CFLAGS) -I$(COMMON_DIR) -I$(CB_DIR) -I$(EB_DIR) -o $@ $< -L. -lebtree

clean:
	-rm -fv libebtree.a $(OBJS) *~ *.rej core $(TEST_BIN) ${EXAMPLES}
	-rm -fv $(addprefix $(CB_DIR)/,*~ *.rej core)
	-rm -fv $(addprefix $(EB_DIR)/,*~ *.rej core)

ifeq ($(wildcard .git),.git)
VERSION := $(shell [ -d .git/. ] && ref=`(git describe --tags --match 'v*') 2>/dev/null` && ref=$${ref%-g*} && echo "$${ref\#v}")
SUBVERS := $(shell comms=`git log --no-merges v$(VERSION).. 2>/dev/null |grep -c ^commit `; [ $$comms -gt 0 ] && echo "-$$comms" )
endif

git-tar: .git
	git archive --format=tar --prefix="ebtree-$(VERSION)/" HEAD | gzip -9 > ebtree-$(VERSION)$(SUBVERS).tar.gz

.PHONY: examples tests

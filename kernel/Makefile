# source files
CSRCS = $(shell find . -type f -name '*.c')
HSRCS = $(shell find . -type f -name '*.h')

# options
SHRK_DEBUG = 1

# kernel config is shit for out of tree modules
EXTRA_CFLAGS += -DSHRK_DEBUG=$(SHRK_DEBUG)

# kernel vars cant define with patsubst for some fucking reason
obj-m := shrk.o 
shrk-objs := main.o        \
        util.o             \
        hook.o             \
        systable.o         \
        hooks/newfstatat.o \
        hooks/getdents.o   \
        hooks/statx.o      \
        hooks/chdir.o      \
        hooks/write.o      \
        hooks/read.o       \
        hooks/open.o       \
        hooks/kill.o

all: $(CSRCS) $(HSRCS)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	rm -f *.order *.symvers *.mod *.mod.c *.o

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

format:
	clang-format -i -style=file $(CSRCS) $(HSRCS)

load:
	@dmesg -C
	@insmod shrk.ko
	@dmesg

unload:
	@dmesg -C
	@rmmod shrk.ko
	@dmesg

.PHONY: clean format load unload

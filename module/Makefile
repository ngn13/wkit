obj-m := wkit.o
wkit-objs := main.o lib/util.o lib/calls.o lib/hooks.o lib/procs.o 

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	rm *.order *.symvers *.mod *.mod.c *.o

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

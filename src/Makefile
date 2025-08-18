obj-m := tasker.o
tasker-objs := tasker_core.o tasker_cdev.o tasker_queue.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

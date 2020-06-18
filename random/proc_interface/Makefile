obj-m += rw_proc.o
KVERSION = $(shell uname -r)

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
test:
	make all
	insmod rw_proc.ko
	cat /proc/p_valor
	rmmod rw_proc.ko

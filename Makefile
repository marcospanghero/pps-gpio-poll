PWD := $(shell pwd)
obj-m := pps-gpio-poll.o

ARCH:=arm

all:
	make ARCH=$(ARCH) -C $(KER) SUBDIRS=$(PWD) modules
clean:
	make -C $(KER) SUBDIRS=$(PWD) clean

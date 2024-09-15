KDIR ?= /lib/modules/$(shell uname -r)/build

root := lab1mod
obj-m := $(root).o

all:
	$(MAKE) -C $(KDIR) M=$$PWD

clean: 
	@rm *.o *.ko *.symvers *.order *.mod *.mod.c *.dwo 2> /dev/null || true

CURRENT = $(shell uname -r)
#HPATH= /usr/src/linux/include
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/misc
VPATH = /drivers
DR = drivers/
OBJS = dspa.o misparw.o $(DR)sbkfp7.o $(DR)fds16r.o $(DR)vas84r.o $(DR)vds32r.o $(DR)vchs2.o

TARGET = dsp
obj-m := $(TARGET).o
$(TARGET)-objs := $(OBJS)

all: clean default copy

default:
	$(MAKE) -C/usr/src/linux SUBDIRS=$(PWD) modules
$(TARGET).o: $(OBJS)
	$(LD) -r -o $@ $^
	
insmod:
	@insmod dsp.ko

copy:
	@-mkdir /lib/modules/4.9.76-gentoo-r1/kernel/dsp
	@cp dsp.ko /lib/modules/4.9.76-gentoo-r1/kernel/dsp
	@modprobe dsp
	
setup:
	@depmod
	@echo "modules=\"dsp\"" >> /etc/conf.d/modules  
	

clean:
	@rm -f *.o *.mod.o *.mod.c
	@rm -f *.cmd *.*.cmd .*.*.cmd
	@rm -f *.ko *.order *.symvers
	@rm -f drivers/.*.*.cmd drivers/*.o
	@-rmmod dsp
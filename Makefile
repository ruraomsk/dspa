CURRENT = $(shell uname -r)
#HPATH= /usr/src/linux/include
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
DEST = /lib/modules/$(CURRENT)/misc
TARGET = dspa
obj-m := $(TARGET).o 
default:
#    @echo "Starting ...." $(MAKE) -C -I$(HPATH) $(KDIR) M=$(PWD) modules
#    $(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
#    @rm -f *.o *.cmd *.flags *.mod.c *.order
#    @rm -f *.*.cmd *.symvers TODO.*
#    @rm -fR tmp*
#    @rm -rf tmp_versions
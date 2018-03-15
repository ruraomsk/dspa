/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include "dspa.h"
#include "misparw.h"
#define DEVNAME "dspa"
#define DEVICE_FIRST 0
#define DEVICE_COUNT 1
#define MODNAME "spa_device"
#define EOK 0
#define SHARED_IRQ 6
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yury Rusinov <ruraomsk@list.ru>");
MODULE_VERSION("1.0");

static int irq = SHARED_IRQ;
static int spa_dev_id;
static int major = 0;
module_param(irq, int, S_IRUGO);
static int device_open = 0;
static char name_dev[] = "spa_device_ports";

static union {
    loff_t ppos;
    def_dev df;
} loff;

// Обрабатываем прерывание на отсутствие модуля по МИСПА

static irqreturn_t no_port_irq(int irq, void *dev_id) {
    unsigned char in;
    if ((in = ReadPort(0x120))&1 == 1) {
        irq_count++;
        printk(KERN_INFO "Not ready counter=%d\n", irq_count);
        WritePort(0x120, in & 0x2);
        return IRQ_HANDLED;
    }
    return IRQ_NONE;

}

void clearMemory(void) {
    if (drv_count <= 0) return;
    int i;
    for (i = 0; i < drv_count; i++) {
        kfree(table_drvs[i].inimod);
        kfree(table_drvs[i].data);
    }
    drv_count = 0;
}

static int dev_open(struct inode *n, struct file *f) {
    if (device_open) return -EBUSY;
    clearMemory();
    device_open++;
    return EOK;
}

static int dev_release(struct inode *n, struct file *f) {
    clearMemory();
    device_open--;
    return EOK;
}

int make_init(int driver_no) {
    int type = table_drvs[driver_no].tdrv.codedrv;
    int i = 0;
    while (tab_t[i].type > 0) {
        if (tab_t[i].type == type) {
            drv_len_data[driver_no].init = tab_t[i].init;
            drv_len_data[driver_no].step1 = tab_t[i].step1;
            drv_len_data[driver_no].step2 = tab_t[i].step2;
            return 0;
        }
        i++;
    }
    printk(KERN_ERR "=== Can not spa-ps device driver code:%d\n", driver_no);
    return 1;

}

static ssize_t dev_read(struct file * file, char * buf,
        size_t count, loff_t *ppos) {
    int i;
    //    short errors[128];
    //    if(*ppos==0){
    //        //вернуть коды ошибок
    //        int i;
    //        for (i = 0; i < drv_count; i++) {
    //            errors[i]=table_drvs[i].error;
    //        }
    //            if (copy_to_user(buf, errors, drv_count*sizeof(short))) {
    //                printk(KERN_ERR "=== Can not move data spa-ps device region\n");
    //                return -EINVAL;
    //            }
    //    }

    for (i = 0; i < drv_count; i++) {
        // reading data from user area
        int count = drv_len_data[i].lenght;
        //        printk(KERN_INFO "run step section %d\n",i);
        void (*ptr)(table_drv *) = NULL;
        if (*ppos == 1) ptr = drv_len_data[i].step1;
        if (*ppos == 2) ptr = drv_len_data[i].step2;

        if (ptr != NULL) {
            if (copy_from_user(table_drvs[i].data, drv_len_data[i].data, count)) {
                printk(KERN_ERR "=== Can not move data spa-ps device region\n");
                return -EINVAL;
            }
            ptr(&table_drvs[i]);

            //    table_drv *td = (table_drv *)drv_len_data[i].td ;
            if (copy_to_user(drv_len_data[i].data, table_drvs[i].data, count)) {
                printk(KERN_ERR "=== Can not move data spa-ps device region\n");
                return -EINVAL;
            }
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            void *pt_error = drv_len_data[i].td + sizeof (table_drv) - sizeof (short); //+2;
            void *pt_time = pt_error - sizeof (long long int);
            //            printk(KERN_INFO "move data buf: %x %x %x driver:%d\n",drv_len_data[i].td,pt_time,pt_error,table_drvs[i].tdrv.typedev);
            copy_to_user(pt_error, &table_drvs[i].error, sizeof (short));
            copy_to_user(pt_time, &table_drvs[i].time, sizeof (long long int));
        }
        //        printk(KERN_INFO "end step section %d\n",i);
    }
    return EOK;

}

static ssize_t dev_write(struct file * file, const char * buf,
        size_t count, loff_t *ppos) {
    loff.ppos = *ppos;
    unsigned char *in_buf_ptr;
    unsigned char *init_buf_ptr;
    table_drv *td = (table_drv *) buf;
    if (*ppos == 0) {
        printk(KERN_INFO "=== end init block : %s\n", DEVNAME);
        return -EBUSY;
    }
    //    printk(KERN_INFO "init table sizeof loff:%d %ld\n", sizeof (loff), loff.ppos);
    //    printk(KERN_INFO "init table len: %d driver:%d adr:%d lenData:%d\n", count, loff.df.code_driver, loff.df.address, loff.df.len_buffer);
    init_buf_ptr = kmalloc(count, GFP_KERNEL);
    if (NULL == init_buf_ptr) {
        printk(KERN_ERR "=== memory allocation error spa-ps device region\n");
        return -EINVAL;
    }
    in_buf_ptr = kmalloc(loff.df.len_buffer, GFP_KERNEL);
    if (NULL == in_buf_ptr) {
        printk(KERN_ERR "=== memory allocation error spa-ps device region\n");
        return -EINVAL;
    }
    if (copy_from_user(init_buf_ptr, td->inimod, count)) {
        printk(KERN_ERR "=== Can not move data spa-ps device region\n");
        return -EINVAL;
    }
    if (copy_from_user(in_buf_ptr, td->data, loff.df.len_buffer)) {
        printk(KERN_ERR "=== Can not move data spa-ps device region\n");
        return -EINVAL;
    }
    drv_len_data[drv_count].lenght = loff.df.len_buffer;
    drv_len_data[drv_count].data = td->data;
    drv_len_data[drv_count].inimod = td->inimod;
    drv_len_data[drv_count].td = buf;
    //    printk(KERN_INFO "init table buf: %x %x driver:%d adr:%d \n",drv_len_data[drv_count].td,buf,loff.df.code_driver, loff.df.address);



    table_drvs[drv_count].tdrv.codedrv = loff.df.code_driver;
    table_drvs[drv_count].address.i = loff.df.address;
    table_drvs[drv_count].inimod = init_buf_ptr;
    table_drvs[drv_count].data = in_buf_ptr;
    table_drvs[drv_count].error = 0;
    table_drvs[drv_count].time = 0;
    if (make_init(drv_count) == 0) {
        //        printk(KERN_INFO "run init section\n");
        void (*ptr)(table_drv *) = NULL;
        ptr = drv_len_data[drv_count].init;
        ptr(&table_drvs[drv_count]);
        //        printk(KERN_INFO "end init section\n");
    };
    drv_count++;
    if (copy_to_user(td->data, in_buf_ptr, loff.df.len_buffer)) {
        printk(KERN_ERR "=== Can not move data spa-ps device region\n");
        return -EINVAL;
    }
    return count;
}

static const struct file_operations dev_fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};


/*
 Разделы инициализации и выгрузки из ядра
 */


static struct cdev hcdev;
static struct class *devclass;

static int __init dev_init(void) {
    int ret;
    dev_t dev;

    //    printk("file property = %s {%d}\n", name_file, strlen(name_file));

    if (major != 0) {
        dev = MKDEV(major, DEVICE_FIRST);
        ret = register_chrdev_region(dev, DEVICE_COUNT, MODNAME);
    } else {
        ret = alloc_chrdev_region(&dev, DEVICE_FIRST, DEVICE_COUNT, MODNAME);
        major = MAJOR(dev); // не забыть зафиксировать!
    }
    if (ret < 0) {
        printk(KERN_ERR "=== Can not register spa-ps device region\n");
        goto err;
    }
    cdev_init(&hcdev, &dev_fops);
    hcdev.owner = THIS_MODULE;
    ret = cdev_add(&hcdev, dev, DEVICE_COUNT);
    if (ret < 0) {
        unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
        printk(KERN_ERR "=== Can not add spa-ps device\n");
        goto err;
    }
    devclass = class_create(THIS_MODULE, "dyn_class");
    int i;
    for (i = 0; i < DEVICE_COUNT; i++) {

        dev = MKDEV(major, DEVICE_FIRST + i);

        /* struct device *device_create( struct class *cls, struct device *parent,
        dev_t devt, void *drvdata, const char *fmt, ...); */
        device_create(devclass, NULL, dev, NULL, "%s_%d", DEVNAME, i);
    }
    printk(KERN_INFO "=========== module spa-ps installed %d:%d ==============\n",
            MAJOR(dev), MINOR(dev));
    if (ret == 0) drv_count = 0;
    if (irq == 0) irq = SHARED_IRQ;
    if (request_irq(irq, no_port_irq, IRQF_SHARED, name_dev, &spa_dev_id)) return -1;
    if (init_memory() != 0) return -1;
    printk(KERN_INFO "==== Successfully setup on IRQ %d id:%d =========\n", irq, spa_dev_id);
    irq_count = 0;
err:
    return ret;
}

static void __exit dev_exit(void) {
    //чисти память за собой
    clearMemory();
    // Осбождаем устройство
    dev_t dev;
    int i;
    for (i = 0; i < DEVICE_COUNT; i++) {
        dev = MKDEV(major, DEVICE_FIRST + i);
        device_destroy(devclass, dev);
    }
    class_destroy(devclass);
    cdev_del(&hcdev);
    unregister_chrdev_region(MKDEV(major, DEVICE_FIRST), DEVICE_COUNT);
    synchronize_irq(irq);
    free_irq(irq, &spa_dev_id);
    free_memory();
    printk(KERN_INFO "=============== module spa-ps removed ==================\n");
}

module_init(dev_init);
module_exit(dev_exit);

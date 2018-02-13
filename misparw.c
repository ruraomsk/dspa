/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <linux/io.h>
#include <linux/delay.h>

#include "dspadef.h"
#include "misparw.h"


static int Box_len;
static void __iomem *rambase = NULL;

int init_memory() {
    if (rambase == NULL) {
        if ((rambase = ioremap(ADR_BOX, SIZE_BOX)) == NULL) {
            return -1;
        }
    }
    return 0;
}
void free_memory(){
    rambase=NULL;
}

unsigned char ReadPort(int port) {
    irq_count = 0;
    drv_flag = 1;
    //    printk(KERN_INFO "read port %x start\n",port);
    char c = inb(port);
    //    printk(KERN_INFO "read port %x end\n",port);
    drv_flag = 0;
    return c;
}

void WritePort(int port, unsigned char byte) {
    irq_count = 0;
    drv_flag = 1;
    outb(byte, port);
    drv_flag = 0;
    return;
}

unsigned char ReadMem(int adr) {
    irq_count = 0;
    drv_flag = 1;
    printk(KERN_INFO "read mem %x start\n", adr);
    char c = ioread8((u8 *) rambase + adr);
    printk(KERN_INFO "read mem %x end %hhx\n", adr, c);
    drv_flag = 0;
    return c;
}

void SetBoxLen(int lenBox) {
    Box_len = lenBox;
}
int WriteBox(unsigned char ptr,unsigned char value){
    iowrite8(value,rambase+ptr);
    if (ERR_MEM) return BUSY_BOX;
    value=!value;
    iowrite8(value,rambase+(Box_len-ptr));
    if (ERR_MEM) return BUSY_BOX;
    return 0;
}
int ReadBox(unsigned char ptr,unsigned char *value){
    unsigned char val=ioread8(rambase+ptr);
    if (ERR_MEM) return BUSY_BOX;
    unsigned char lav=ioread8(rambase+(Box_len-ptr));
    if (ERR_MEM) return BUSY_BOX;
    if(!lav <> val) return NEGC_BOX;
    *value=val;
    return 0;
}

int ReadSinglBox(unsigned char ptr,unsigned char *value){
    unsigned char val=ioread8(rambase+ptr);
    if (ERR_MEM) return BUSY_BOX;
    *value=val;
    return 0;
}
int ReadBx3w(unsigned char ptr,unsigned char *value){
    char x1,x2,x3;
    if(ReadSinglBox(ptr,&x1)) return BUSY_BOX;
    delaymcs(5);
    if(ReadSinglBox(ptr,&x2)) return BUSY_BOX;
    delaymcs(5);
    if(ReadSinglBox(ptr,&x3)) return BUSY_BOX;
    *value=(x1&x2)|(x1&x3)|(x2&x3);
    return 0;
}

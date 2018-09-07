/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <linux/io.h>
#include <linux/delay.h>

#include "dspadef.h"
#include "misparw.h"
#include "asm/io.h"
#include "linux/kernel.h"

static int Box_len = 0xff;
static void __iomem *rambase = NULL;
extern volatile unsigned int irq_count;

int init_memory() {
    if (rambase == NULL) {
        if ((rambase = ioremap(ADR_BOX, SIZE_BOX)) == NULL) {
            return -1;
        }
    }
    return 0;
}

void free_memory() {
    iounmap(rambase);
    rambase = NULL;
}

unsigned char ReadPort(int port) {
    //    return 0;
    unsigned char c;
    c = inb(port);
    //    printk(KERN_INFO "read port %x = %hhx \n", port,c);
    return c;
}

void WritePort(int port, unsigned char byte) {
    //    return;
    //    printk(KERN_INFO "write port %x = %hhx \n", port,byte);

    outb(byte, port);
    //    if(port==0x118) msleep_interruptible(1);
    return;
}

void SetBoxLen(int lenBox) {

    //    Box_len = lenBox;
}

unsigned char WriteBox(unsigned char ptr, unsigned char value) {
    CLEAR_MEM
    iowrite8(value, (unsigned char *) rambase + ptr);
    if (ERR_MEM)
        return BUSY_BOX;
    // delaymcs(20);
    CLEAR_MEM
    iowrite8(~value, (unsigned char *) rambase + (Box_len - ptr));
    if (ERR_MEM)
        return BUSY_BOX;
    return SPAPS_OK;
}

unsigned char ReadBox(unsigned char ptr, unsigned char *value) {
    //    sprintf(logstr, "ReadBox_1 %x ", ptr);
    //    log_debug();
    unsigned char val;
    unsigned char lav;
    CLEAR_MEM
    val = ioread8((unsigned char *) rambase + ptr);
    if (ERR_MEM)
        return BUSY_BOX;
    //    sprintf(logstr, "ReadBox_2 %x ", ( (Box_len - ptr)));
    //    log_debug();
    // delaymcs(20);
    CLEAR_MEM
    lav = ioread8((unsigned char *) rambase + (Box_len - ptr));
    if (ERR_MEM)
        return BUSY_BOX;
    //    printk(KERN_INFO "read adr %x ptr %d = %x !=%x\n", inb(0x118),ptr,val,lav);
        if( value )
     *value = val;
    if ((lav | val) != 0xff)
        return NEGC_BOX;
    return SPAPS_OK;
}

unsigned char ReadSinglBox(unsigned char ptr, unsigned char *value) {
    unsigned char val;
    CLEAR_MEM
    val = ioread8((unsigned char *) rambase + ptr);
        if( value )
     *value = val;
    if (ERR_MEM)
        return BUSY_BOX;
    return 0;
}

unsigned char WriteSinglBox(unsigned char ptr, unsigned char value) {
    CLEAR_MEM
    iowrite8(value, (unsigned char *) rambase + ptr);
    if (ERR_MEM)
        return BUSY_BOX;
    return 0;
}

unsigned char ReadBx3w(unsigned char ptr, unsigned char *value) {
    char x1, x2, x3;
    if (ReadSinglBox(ptr, &x1))
        return BUSY_BOX;
    // delaymcs(5);
    if (ReadSinglBox(ptr, &x2))
        return BUSY_BOX;
    // delaymcs(5);
    if (ReadSinglBox(ptr, &x3))
        return BUSY_BOX;
    if( value )
    *value = (x1 & x2) | (x1 & x3) | (x2 & x3);
    return 0;
}

unsigned char ReadBox3(unsigned char ptr, unsigned char *value) {
unsigned char RH, i = 0;
    while (i < 3) {
        RH = ReadBox(ptr, value);
        if( RH == BUSY_BOX || RH == SPAPS_OK )
            break;
        i++;
    }
    // for (i = 0; i < 3; i++) {
    //     //        sprintf(logstr, "ReadBox3 %hhx", ptr);
    //     //        log_debug();
    //     RH = ReadBox(ptr, value);
    //     if (RH != 0xC0)
    //         break;
    // }
    return RH;
}

unsigned char CatchBox(void) {
    char ret;
    int count = 0;
    while (count < 3) {
        WriteSinglBox(SV, 1);
        ReadSinglBox(SV, &ret);
        if (ret == 1)
            return 0;
        count++;
    }
    return BUSY_BOX;

    //    int RH = WriteSinglBox(SV, 1);
    //    if (RH) return BUSY_BOX;
    //    RH = ReadSinglBox(SV, &ret);
    //    if (RH) return BUSY_BOX;
    //    if (ret == 1) return 0;
    //    if (ret == 0) {
    //        RH = WriteSinglBox(SV, 1);
    //        if (RH) return BUSY_BOX;
    //        return 0;
    //    }
    //    return BUSY_BOX;
}

unsigned char FreeBox(void) {
    char ret;
    int count = 0;
    while (count < 3) {
        WriteSinglBox(SV, 0);
        ReadSinglBox(SV, &ret);
        if (ret == 0)
            return 0;
        count++;
    }
    return BUSY_BOX;

    //    int RH = WriteSinglBox(SV, 0);
    //    if (RH) return BUSY_BOX;
    //    RH = ReadSinglBox(SV, &ret);
    //    if (RH) return BUSY_BOX;
    //    if (ret == 0) return 0;
    //    if (ret == 1) {
    //        RH = WriteSinglBox(SV, 0);
    //        if (RH) return BUSY_BOX;
    //        return 0;
    //    }
    //    return BUSY_BOX;
}

unsigned long decodegray(unsigned long k) {
    unsigned char i;
    for (i = 1; i < 24; i <<= 1)
        k ^= (k >> i);
    return k;
}

void delaymcs(int x) {
    ndelay(x);
}

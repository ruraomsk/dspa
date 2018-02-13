#include "../dspadef.h"
#define SIZE_BUF1 17

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
void testdrv1_ini(table_drv* tdrv) {
    log_init(tdrv);
    //    sprintf(str,"data: %d %d %d %d %d",tdrv->data[0],tdrv->data[1],tdrv->data[2],tdrv->data[3],tdrv->data[4]);
    //    log(str);
    //    tdrv->time=read_timer();
    tdrv->error = 0;
}

void testdrv1_step(table_drv* tdrv) {
    //    char str[120];
    //    sprintf(str,"data: %d %d %d %d %d",tdrv->data[0],tdrv->data[1],tdrv->data[2],tdrv->data[3],tdrv->data[4]);
    //    log(str);
    int i;
    for (i = 0; i < SIZE_BUF1; i++) {
        tdrv->data[i] = tdrv->data[i] + 1;
    }
//    WritePort(MISPA_COMMON_CTRL_PORT,0x0d);
//    mdelay(5);
//    char in=ReadPort(MISPA_CTRL1_PORT);
//    printk(KERN_INFO "port %x=%hhx\n",MISPA_CTRL1_PORT,in);
    
    for (i = 0; i < MISPA_PORTS; i++) {
        int port = table_ports[i];
//        char in = ReadPort(port);
//        if (ERR_PORT) break;
//        printk(KERN_INFO "port %x=%hhx\n",port,in);

    }
//    ReadMem(0);

    //        WritePort(0x108,0);
    //        if(ERR_PORT) break;

    //    tdrv->time=read_timer();
    tdrv->error = 0;
    if (ERR_PORT) {
        tdrv->error = ERR_PORT;
    }
    log_step(tdrv);
}
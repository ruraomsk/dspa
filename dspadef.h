/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dspadef.h
 * Author: rusin
 *
 * Created on 2 февраля 2018 г., 14:51
 */

#ifndef DSPADEF_H
#define DSPADEF_H


#define MAX_DRIVERS 100
#define SPA_DEBUG 1
//static int drv_count = 0;
//static int drv_flag = 0;
//static unsigned int irq_count;

//#pragma pack(push, 1)

typedef struct __attribute__((packed)) {
    unsigned char code_driver;
    unsigned char address;
    unsigned short len_buffer;
}
def_dev;
//#pragma pop

//#pragma pack(push, 1)
typedef struct __attribute__((packed)) {

    union {
        unsigned char typedev[2];
        unsigned short codedrv;
    } tdrv;

    unsigned short address;

    unsigned char *inimod;
    unsigned char *data;
    unsigned long int time;
    short error;
} table_drv;
//#pragma pop

//#define DRIVER_TEST1 0x1c
//void testdrv1_ini(table_drv* tdrv);
//void testdrv1_step(table_drv* tdrv);
//
//#define DRIVER_TEST2 0x2d
//void testdrv2_ini(table_drv* tdrv);
//void testdrv2_step(table_drv* tdrv);
//
//#define DRIVER_TEST3 0x03
//void testdrv3_ini(table_drv* tdrv);
//void testdrv3_step(table_drv* tdrv);


//void log_init(table_drv* tdrv) {
//    printk(KERN_INFO "Driver adr %x init \n", tdrv->address);
//}
//void log_step(table_drv* tdrv) {
//    printk(KERN_INFO "Driver adr %x step \n", tdrv->address);
//}
//#define MISPA_PORTS 10
//#define MISPA_SIGNAL_PORT 0x100
//#define MISPA_COMMON_CTRL_PORT 0x108
//#define MISPA_CTRL1_PORT 0x110
//#define MISPA_CTRL2_PORT 0x112
//#define MISPA_CTRL3_PORT 0x114
//#define MISPA_MOD_PORT 0x118
//
//#define MISPA_CTRLIRQ_PORT 0x120
//#define MISPA_RESET_PORT 0x128
//#define MISPA_POSTBOX_PORT 0x130
//#define MISPA_REG_PORT 0x138
//
//static int table_ports[MISPA_PORTS] = {
//    MISPA_SIGNAL_PORT,
//    MISPA_COMMON_CTRL_PORT,
//    MISPA_CTRL1_PORT,
//    MISPA_CTRL2_PORT,
//    MISPA_CTRL3_PORT,
//    MISPA_MOD_PORT,
//    MISPA_CTRLIRQ_PORT,
//    MISPA_RESET_PORT,
//    MISPA_POSTBOX_PORT,
//    MISPA_REG_PORT,
//};

//void perm_ports(void){
//    for (int i = 0; i < MISPA_PORS; i++) {
//    }
//
//}

/*
 * Команды чтения и записи в порт МИСПА
 * нужно заметить что наличе ошибок нужно проверять макросом ERR_PORT
 */

#define ERR_MEM irq_count 
#define CLEAR_MEM irq_count=0;
//static unsigned long int drv_timer=0L;
//unsigned long int read_timer(void){
////    timezone tz;
////    gettimeofday(tv,&tz);
//    
//    return clock();
//}



#endif /* DSPADEF_H */


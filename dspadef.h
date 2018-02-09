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
//#include <time.h>

typedef struct __attribute__((__packed__)) {
    unsigned char code_driver;
    unsigned char address;
    unsigned short len_buffer;
}def_dev ;

typedef struct {

    union {
        unsigned char typedev[2];
        unsigned short codedrv;
    } tdrv;

    union {
        unsigned short i;
        unsigned char c[2];
    } address;

    unsigned char *inimod;
    unsigned char *data;
    unsigned long int time;
    short error;
} table_drv;

void log_init (table_drv* tdrv){
    char str[120];
    sprintf(str,"Driver %ld adr %ld init ",tdrv->tdrv.codedrv,tdrv->address.i);
//    printk(KERN_INFO "log:%s\n",str);
}
void log_step (table_drv* tdrv){
    char str[120];
    sprintf(str,"Driver %ld adr %ld step ",tdrv->tdrv.codedrv,tdrv->address.i);
//    printk(KERN_INFO "log:%s\n",str);
}
/*
 * Команды чтения и записи в порт МИСПА
 * нужно заметить что наличе ошибок нужно проверять макросом ERR_PORT
 */
inline unsigned char ReadPort(int port){
    irq_count=0;
    // make read from port
    return 0;
}
inline void WritePort(int port,unsigned char byte){
    irq_count=0;
    // make write to port
    return;
}
#define ERR_PORT irq_count 
//static unsigned long int drv_timer=0L;
//unsigned long int read_timer(void){
////    timezone tz;
////    gettimeofday(tv,&tz);
//    
//    return clock();
//}
#define DRIVER_TEST1 0x1c
void testdrv1_ini(table_drv* tdrv);
void testdrv1_step(table_drv* tdrv);

#define DRIVER_TEST2 0x2d
void testdrv2_ini(table_drv* tdrv);
void testdrv2_step(table_drv* tdrv);

#define DRIVER_TEST3 0x03
void testdrv3_ini(table_drv* tdrv);
void testdrv3_step(table_drv* tdrv);




#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* DSPADEF_H */


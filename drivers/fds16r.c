#include "../dspadef.h"
#include "../misparw.h"
#include "fds16r.h"
#include "linux/printk.h"

#define inipar  ((fds16r_inipar*)(tdrv->inimod)) 
#define fdsDate ((fds16r_data*)(tdrv->data)) 

extern volatile unsigned int irq_count;


#define AdrType          0x04  // тип модуля 
#define AdrOut18         0x01  // регистр вывода сигналов каналов 1-8  
#define AdrOut916        0x02  // регистр вывода сигналов каналов 9-16  
#define AdrISP18         0x03  // регистр исправности каналов 1-8  
#define AdrISP916        0x0F  // регистр исправности каналов 9-16  
//Регистры состояния выходов
#define AdrSost14        0x05 // регистр состояния каналов 1-4   
#define AdrSost58        0x06 // регистр состояния каналов 5-8   
#define AdrSost912       0x07 // регистр состояния каналов 9-12   
#define AdrSost1316      0x08 // регистр состояния каналов 13-16   

void fds16r_ini(table_drv* tdrv) {
    int ADR_MISPA;
    ADR_MISPA = 0x118;

    SetBoxLen(inipar->BoxLen);
    CLEAR_MEM
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //адрес модуля на миспа
    if (ERR_MEM) {
        fdsDate->Diagn = 0x80;
        tdrv->error = 0x80;
        return;
    }
    fdsDate->Diagn = 0;
    tdrv->error = 0;
    // unsigned char RL;    
    // ReadBox3(AdrType, &RL);
    // printk("rl- %hhx",RL);
    // printk("type - %hhx",inipar->type);
    // if (RL != inipar->type) {
    //     tdrv->error = 0x80;
    //     return;
    // } //ошибка типа модуля

};

void fds16r_dw(table_drv* tdrv) {
    unsigned char RH = 0, temp;
    int ADR_MISPA = 0x118, i, j;

    if (tdrv->error & 0x80) // что-то с модулем не работаем
        return;
    
    SetBoxLen(inipar->BoxLen);
    CLEAR_MEM
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        fdsDate->Diagn = 0x80;
        tdrv->error = 0x80;
        return;
    }

    fdsDate->Diagn = 0;
    tdrv->error = 0;

    RH |= WriteBox(AdrOut18, 0);
    RH |= WriteBox(AdrOut916, 0);
    if (RH == 0x80) { // нет устройства
        fdsDate->Diagn = 0x80;
        tdrv->error = 0x80;
        return;
    } else if (RH == 0xC0) { // NEGC_BOX
        fdsDate->Diagn = 0xC0;
        tdrv->error = 0xC0;
        return;
    }

    for (i = 0; i < 2; i++) {
        temp = 0;
        for (j = 8; j >= 0; j--) {
            temp <<= 1;
            temp |= (fdsDate->SIGN[j + i * 8].c & 1);
        }
        WriteBox(i + 1, temp);
    }

    ReadBx3w(AdrISP18, &temp);
    fdsDate->ISP[0].i = temp;
    ReadBx3w(AdrISP916, &temp);
    fdsDate->ISP[1].i = temp;
    if (fdsDate->ISP[0].i || fdsDate->ISP[1].i) {
        fdsDate->Diagn = 0xE0;
    }
};


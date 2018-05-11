/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/*
===========================================================
Типы диагностических сообщений модуля

структура байта достоверности модуля 
 
  Бит         Значение

   0   -   неинверсия исправности каналов 1-8
   1   -   неинверсия исправности каналов 9-16
   2   -   неинверсия статуса каналов 1 - 4
   3   -   неинверсия статуса каналов 5 - 8  
   4   -   неинверсия статуса каналов 9 - 12
   5   -   неинверсия статуса каналов 13 - 16
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

   исправность: бит исправности == 1 - канал неисправен

   статус: биты 0-2
               команда 0 (выключить) 000 - нет напряжения в коммутируемой сети и выходной ключ закрыт
                                     010 - есть напряжение в коммутируемой сети и выходной ключ закрыт
                                     011 - резерв
               команда 1 (включить)  101 - нет неисправности и выходной ключ открыт 
                                     110 - короткое замыкание или перегрузка и выходной ключ открыт 
                             001,100,111 - неисправность


===========================================================
 */
#include "../dspadef.h"
#include "../misparw.h"
#include "fds16r.h"
#include "linux/printk.h"
#define inipar  ((fds16r_inipar*)(tdrv->inimod)) 
#define fdsDate ((fds16r_data*)(tdrv->data)) 
#define LastIn  ((char*)(&tdrv->time))



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


extern unsigned int irq_count;

void fds16r_ini(table_drv* tdrv) {
    unsigned char RH, RL, aRH;
    int ADR_MISPA;
    ADR_MISPA = 0x118;

    SetBoxLen(inipar->BoxLen);
    CLEAR_MEM
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //адрес модуля на миспа
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    tdrv->error = 0;

    // ReadBox3(AdrType, &RL);
    // printk("rl- %hhx",RL);
    // printk("type - %hhx",inipar->type);
    // if (RL != inipar->type) {
    //     tdrv->error = 0x80;
    //     return;
    // } //ошибка типа модуля

};

void fds16r_dw(table_drv* tdrv) {
    unsigned char RH, temp;
    int ADR_MISPA = 0x118, i, j;

    if (tdrv->error == 0x80)
        return;
    SetBoxLen(inipar->BoxLen);
    CLEAR_MEM
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }
    tdrv->error = 0;

    RH = WriteBox(AdrOut18, 0);
    if (RH == 0x80) { // нет устройства
        tdrv->error = 0x80;
        return;
    } else if (RH == 0xC0) { // NEGC_BOX
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



    //     for (i = 7, j = 0x80, ch18 = 0, msk = inipar->UsMask18; i >= 0; i--) { // упаковать используемые каналы 1 - 8
    //         ch18 <<= 1;
    //         if (msk & j)
    //             k = (fdsDate->SIGN[i].c & 1);
    //         else
    //             k = 0;
    //         ch18 |= k;
    //         j >>= 1;
    //     }
    //     inipar->ChMask18 = LastIn[0] ^ ch18; // маска изменившихся каналов
    //     tdrv->tdrv.typedev[0] = ch18;

    //     for (i = 15, j = 0x80, ch916 = 0, msk = inipar->UsMask916; i > 7; i--) { // упаковать используемые каналы 9 - 16
    //         ch916 <<= 1;
    //         if (msk & j)
    //             k = (fdsDate->SIGN[i].c & 1);
    //         else
    //             k = 0;
    //         ch916 |= k;
    //         j >>= 1;
    //     }
    //     inipar->ChMask916 = LastIn[1] ^ ch916; // маска изменившихся каналов
    //     tdrv->tdrv.typedev[1] = ch916;

    //     // вывод сигналов
    //     RH = WriteBox(AdrOut18, ch18);
    //     if (RH) {
    //         tdrv->error = 0x80;
    //         return;
    //     } //ошибка миспа

    //     RH = WriteBox(AdrOut916, ch916);
    //     if (RH) {
    //         tdrv->error = 0x80;
    //         return;
    //     } //ошибка миспа

    //     // контроль состояния 


    //     delaymcs(150);

    //     RH = ReadBx3w(AdrSost14, &st14);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x4;
    //         st14 = 0xFF;
    //     } // неинверсия статуса 

    //     RL = inipar->s11;
    //     inipar->s11 = inipar->s12;
    //     inipar->s12 = st14;
    //     st14 = (inipar->s12 & inipar->s11) | (inipar->s12 & RL) | (inipar->s11 & RL);

    //     //   WDEBUG_PRINT_HEX(21,"st14=",&st14);

    //     RH = ReadBx3w(AdrSost58, &st58);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x8;
    //         st58 = 0xFF;
    //     } // неинверсия статуса модуля

    //     RL = inipar->s51;
    //     inipar->s51 = inipar->s52;
    //     inipar->s52 = st58;
    //     st58 = (inipar->s52 & inipar->s51) | (inipar->s52 & RL) | (inipar->s51 & RL);

    //     //WDEBUG_PRINT_HEX(22,"st58=",&st58);

    //     RH = ReadBx3w(AdrSost912, &st912);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x10;
    //         st912 = 0xFF;
    //     } // неинверсия статуса модуля

    //     RL = inipar->s91;
    //     inipar->s91 = inipar->s92;
    //     inipar->s92 = st912;
    //     st912 = (inipar->s92 & inipar->s91) | (inipar->s92 & RL) | (inipar->s91 & RL);

    //     //WDEBUG_PRINT_HEX(23,"st912=",&st912);

    //     RH = ReadBx3w(AdrSost1316, &st1316);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x20;
    //         st1316 = 0xFF;
    //     } // неинверсия статуса модуля

    //     RL = inipar->s91;
    //     inipar->s131 = inipar->s132;
    //     inipar->s132 = st1316;
    //     st1316 = (inipar->s132 & inipar->s131) | (inipar->s132 & RL) | (inipar->s131 & RL);

    //     //WDEBUG_PRINT_HEX(24,"st1316=",&st1316);

    //     RH = ReadBx3w(AdrISP18, &is18);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x01;
    //         is18 = 0xFF;
    //     } // неинверсия исправности модуля

    //     RL = inipar->is11;
    //     inipar->is11 = inipar->is12;
    //     inipar->is12 = is18;
    //     is18 = (inipar->is12 & inipar->is11) | (inipar->is12 & RL) | (inipar->is11 & RL);

    //     RH = ReadBx3w(AdrISP916, &is916);

    //     if (RH == 0x80) {
    //         tdrv->error = RH;
    //         return;
    //     }// ошибка миспа
    //     else
    //         if (RH) {
    //         tdrv->error |= 0x02;
    //         is916 = 0xFF;
    //     } // неинверсия исправности модуля

    //     RL = inipar->is91;
    //     inipar->is91 = inipar->is92;
    //     inipar->is92 = is916;
    //     is916 = (inipar->is92 & inipar->is91) | (inipar->is92 & RL) | (inipar->is91 & RL);


    //     // есть исправные каналы?


    //     if (((tdrv->error & 1) || (tdrv->error & 0xC) == 0xC) &&
    //             ((tdrv->error & 2) || (tdrv->error & 0x30) == 0x30))
    //         tdrv->error |= 0x80; //нет исправных каналов

    //     for (i = 0, j = 1, msk = inipar->UsMask18; i < 4; i++) { // определить состояние каналов 1 - 4

    //         if (st14 == 0xFF) {
    //             k = 0x87;
    //         } else {
    //             if (msk & j) {
    //                 k = (st14 & 3);
    //                 if (ch18 & j)
    //                     k |= 4;

    //                 if ((k != 0 && k != 2 && k != 5) || is18 & j)
    //                     k |= 0x80;
    //             } else
    //                 k = 3;

    //             st14 >>= 2;
    //             j <<= 1;
    //         }
    //         fdsDate->SIGN[i].error = k;
    //     }

    //     for (i = 4; i < 8; i++) { // определить состояние каналов 5 - 8

    //         if (st58 == 0xFF) {
    //             k = 0x87;
    //         } else {
    //             if (msk & j) {
    //                 k = (st58 & 3);
    //                 if (ch18 & j)
    //                     k |= 4;
    //                 if ((k != 0 && k != 2 && k != 5) || is18 & j)
    //                     k |= 0x80;
    //             } else
    //                 k = 3;
    //             st58 >>= 2;
    //             j <<= 1;
    //         }
    //         fdsDate->SIGN[i].error = k;

    //     }

    //     for (i = 8, j = 1, msk = inipar->UsMask916; i < 12; i++) { // определить состояние каналов 9 - 12

    //         if (st912 == 0xFF) {
    //             k = 0x87;
    //         } else {
    //             if (msk & j) {
    //                 k = (st912 & 3);
    //                 if (ch916 & j)
    //                     k |= 4;
    //                 if ((k != 0 && k != 2 && k != 5) || is916 & j)
    //                     k |= 0x80;
    //             } else
    //                 k = 3;
    //             st912 >>= 2;
    //             j <<= 1;
    //         }
    //         fdsDate->SIGN[i].error = k;

    //     }

    //     for (i = 12; i < 16; i++) { // определить состояние каналов 13 - 16

    //         if (st1316 == 0xFF) {
    //             k = 0x87;
    //         } else {
    //             if (msk & j) {
    //                 k = (st1316 & 3);
    //                 if (ch916 & j)
    //                     k |= 4;
    //                 if ((k != 0 && k != 2 && k != 5) || is916 & j)
    //                     k |= 0x80;
    //             } else
    //                 k = 3;
    //             st1316 >>= 2;
    //             j <<= 1;
    //         }
    //         fdsDate->SIGN[i].error = k;

    //     }

};


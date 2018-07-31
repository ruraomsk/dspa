/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "../dspadef.h"
#include "../misparw.h"
#include "vds32r.h"
#include "linux/printk.h"

#define inipar ((vds32r_inipar *)(tdrv->inimod))
#define vdsDate ((vds32r_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))

#define AdrType 0x04 // тип модуля
#define AdrRQ 0x5    // регистр запроса обслуживания
//Регистры состояния контактов датчиков
#define AdrSostContact0 0x10 // каналы 1-8
#define AdrSostContact1 0x11 // каналы 9-16
#define AdrSostContact2 0x40 // каналы 17-24
#define AdrSostContact3 0x41 // каналы 25-32
//Регистры обрывов линий связи с датчиками
#define AdrOpnCircuit0 0x14 // каналы 1-8
#define AdrOpnCircuit1 0x15 // каналы 9-16
#define AdrOpnCircuit2 0x16 // каналы 17-24
#define AdrOpnCircuit3 0x17 // каналы 25-32
//Регистры коротких замыканий линий связи с датчиками
#define AdrShortCircuit0 0x18 // каналы 1-8
#define AdrShortCircuit1 0x19 // каналы 9-16
#define AdrShortCircuit2 0x1A // каналы 17-24
#define AdrShortCircuit3 0x1B // каналы 25-32
//Регистры настройки времени антидребезга
#define AdrAntiTrembl0 0x20 // каналы 1-16
#define AdrAntiTrembl1 0x23 // каналы 17-32
//Регистры маски каналов
#define AdrChanlsMask0 0x21 // каналы 1-16
#define AdrChanlsMask1 0x24 // каналы 17-32
//Регистрыы статуса
#define AdrStatus0 0x22 // каналы 1-16
#define AdrStatus1 0x25 // каналы 17-32



extern unsigned int irq_count;

/*
===========================================================
Типы диагностических сообщений модуля

структура байта достоверности модуля 
 
  Бит         Значение

   0   -   ошибки каналов 1-8
   1   -   ошибка каналов 9-16
   2   -   ошибка каналов 17 - 24
   3   -   ошибка каналов 25 - 32  
   4   -   ошибка статуса  ()
   5   -   ошибка конфигурирования
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

===========================================================

 */


void vds32r_ini(table_drv *tdrv) {
    unsigned char RQ, RH = 0, RL;
    int ADR_MISPA = 0x118;


    SetBoxLen(inipar->BoxLen);

    RQ = (unsigned char) (tdrv->address & 0xff);
    CLEAR_MEM

    WritePort(ADR_MISPA, RQ);

    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }
    tdrv->error = 0;

    RH |= WriteSinglBox(6, 0);
    RH |= WriteSinglBox(4, 0);
    //    проверка типа модуля
    RH |= ReadBx3w(AdrType, &RL);
    if (RL != inipar->type) {
        RH |= 0x80;
        return;
    } //ошибка типа модуля
    RH |= WriteBox(AdrAntiTrembl0, inipar->tadr116); // каналы 1-16   0x20
    // RH = WriteBox(AdrChanlsMask0, inipar->Dmask116); // каналы 1-16   0x21
    RH |= WriteBox(AdrChanlsMask0, 0x0); // каналы 1-16   0x21
    RH |= WriteBox(AdrAntiTrembl1, inipar->tadr1732); // каналы 17-32  0x23
    // RH = WriteBox(AdrChanlsMask1, inipar->Dmask1732); // каналы 17-32  0x24
    RH |= WriteBox(AdrChanlsMask1, 0x0); // каналы 1-16   0x21
    RH |= ReadBx3w(AdrStatus0, &RL);
    RH |= ReadBx3w(AdrStatus1, &RL);
    RH |= ReadBx3w(AdrRQ, &RL);
    if (RH == 0x80) { // нет устройства
        tdrv->error = 0x80;
        return;
    }
    // else if (RH == 0xC0) { // NEGC_BOX
    //     tdrv->error = 0xC0;
    //     return;
    // }
}

// static unsigned char RQl = 0;

void vds32r_rd(table_drv *tdrv) {
    vds32r_str vdsValue;
    unsigned char i, j, z;
    unsigned char RH = 0, SErr = 0, RQ = 0; // RL;
    int k = 0;
    int ADR_MISPA;
    SetBoxLen(0xFF);

    if (tdrv->error) {
        return; // пока повременить
    }

    ADR_MISPA = 0x118;
    CLEAR_MEM
    WritePort(ADR_MISPA, (unsigned char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    for (k = 0; k < 32; k++)
        vdsDate->SIGN[k].error = 0xff;
    
    
//    ReadBx3w(AdrRQ, &RQ);
//    if (RQl != RQ){ 
//        printk("RQ1=%hhx", RQ);
//        printk("addres = %d",tdrv->address);
//    }
//    RQl = RQ;


    // проверка инверсии в статусе
    RH |= ReadBox(AdrStatus0, &vdsValue.stat[0]);
    RH |= ReadBox(AdrStatus1, &vdsValue.stat[1]);

    if (RH == 0x80) { // нет устройства
        tdrv->error = 0x80;
        return;
    } else if (RH == 0xC0) { // NEGC_BOX
        tdrv->error = 0xC0;
        return;
    }
    RH = 0;

    RH |= ReadBx3w(AdrSostContact0, &vdsValue.sost[0]);
    RH |= ReadBx3w(AdrSostContact1, &vdsValue.sost[1]);
    RH |= ReadBx3w(AdrSostContact2, &vdsValue.sost[2]);
    RH |= ReadBx3w(AdrSostContact3, &vdsValue.sost[3]);
    RH |= ReadBx3w(AdrOpnCircuit0, &vdsValue.obr[0]);
    RH |= ReadBx3w(AdrOpnCircuit1, &vdsValue.obr[1]);
    RH |= ReadBx3w(AdrOpnCircuit2, &vdsValue.obr[2]);
    RH |= ReadBx3w(AdrOpnCircuit3, &vdsValue.obr[3]);
    RH |= ReadBx3w(AdrShortCircuit0, &vdsValue.kz[0]);
    RH |= ReadBx3w(AdrShortCircuit1, &vdsValue.kz[1]);
    RH |= ReadBx3w(AdrShortCircuit2, &vdsValue.kz[2]);
    RH |= ReadBx3w(AdrShortCircuit3, &vdsValue.kz[3]);

//    ReadBx3w(AdrRQ, &RQ);
//    if (RQl != RQ) 
//        printk("RQ2=%hhx", RQ);
//    RQl = RQ;


    if (RH == 0x80) { // нет устройства
        tdrv->error = 0x80;
        return;
    }
    // else if (RH == 0xC0) { // NEGC_BOX
    //     tdrv->error = 0xC0;
    //     return;
    // }

    if (!inipar->inv) {
        for (k = 0; k < 4; k++)
            vdsValue.sost[k] = ~vdsValue.sost[k];
    }

    for (i = 0, k = 0; i < 4; i++) {
        j = 1;
        for (z = 0; z < 8; z++) {
            SErr = 0;
            if (vdsValue.obr[i] & j)
                SErr |= 0x0c;
            if (!(vdsValue.kz[i] & j))
                SErr |= 0x30;
            vdsDate->SIGN[k].error = SErr;

            if (vdsValue.sost[i] & j)
                vdsDate->SIGN[k].b = 1;
            else
                vdsDate->SIGN[k].b = 0;
            j <<= 1;
            k++;
        }
    }


}

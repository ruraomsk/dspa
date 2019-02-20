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

//extern unsigned int irq_count;

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

// static unsigned char test[4][2];

void vds32r_ini(table_drv *tdrv)
{
    unsigned char RQ, RH = 0, RL;
    int ADR_MISPA = 0x118;
    tdrv->error = SPAPS_OK;
    vdsDate->Diagn = SPAPS_OK;
    
    RQ = (unsigned char)(tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM)
    {
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    RH |= WriteSinglBox(6, 0);
    RH |= WriteSinglBox(4, 0);
    //    проверка типа модуля
    RH |= ReadBx3w(AdrType, &RL);
    if (RL != inipar->type)
    {
        vdsDate->Diagn = WRONG_DEV;
        tdrv->error = BUSY_BOX;
        return;
    }                                                //ошибка типа модуля
    RH |= WriteBox(AdrAntiTrembl0, inipar->tadr116); // каналы 1-16   0x20
    // RH = WriteBox(AdrChanlsMask0, inipar->Dmask116); // каналы 1-16   0x21
    RH |= WriteBox(AdrChanlsMask0, 0x0);              // каналы 1-16   0x21
    RH |= WriteBox(AdrAntiTrembl1, inipar->tadr1732); // каналы 17-32  0x23
    // RH = WriteBox(AdrChanlsMask1, inipar->Dmask1732); // каналы 17-32  0x24
    RH |= WriteBox(AdrChanlsMask1, 0x0); // каналы 1-16   0x21
    RH |= ReadBx3w(AdrStatus0, &RL);
    RH |= ReadBx3w(AdrStatus1, &RL);
    RH |= ReadBx3w(AdrRQ, &RL);
    if (RH == BUSY_BOX)
    { // нет устройства
        vdsDate->Diagn = SOST_ERR;
        tdrv->error = SOST_ERR;
        return;
    }
    // else if (RH == 0xC0) { // NEGC_BOX
    //     tdrv->error = 0xC0;
    //     return;
    // }

    // int i, j;
    // for (i = 0; i < 4; i++)
    //     for (j = 0; j < 2; j++)
    //         test[i][j] = 0;
}

void vds32r_rd(table_drv *tdrv)
{
    vds32r_str vdsValue;
    unsigned char RH = 0, SErr = 0, RQ = 0, i, j, z; 
    int k = 0, ADR_MISPA = 0x118;

    if (tdrv->error)
        return; // пока повременить

    vdsDate->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK;

    CLEAR_MEM
    WritePort(ADR_MISPA, (unsigned char)(tdrv->address & 0xff));
    if (ERR_MEM){
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }
    

    for (k = 0; k < 32; k++)
        vdsDate->SIGN[k].error = 0xff;

    // попытка потушить неисправность (красная лампочка)
    // -----------------
    RH = 0;
    RH |= ReadBox(AdrAntiTrembl0, &RQ);
    RH |= ReadBox(AdrAntiTrembl1, &RQ);
    RH |= ReadBox(AdrChanlsMask0, &RQ);
    RH |= ReadBox(AdrChanlsMask1, &RQ);
    RH |= ReadBox(AdrStatus0, &vdsValue.stat[0]);
    RH |= ReadBox(AdrStatus1, &vdsValue.stat[1]);
    if (RH == BUSY_BOX)
    { // нет устройства
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }
    if (RH == NEGC_BOX)
    {
        RH = 0;
        RH |= WriteBox(AdrAntiTrembl0, inipar->tadr116);  // каналы 1-16   0x20
        RH |= WriteBox(AdrChanlsMask0, 0x0);              // каналы 1-16   0x21
        RH |= WriteBox(AdrAntiTrembl1, inipar->tadr1732); // каналы 17-32  0x23
        RH |= WriteBox(AdrChanlsMask1, 0x0);              // каналы 17-32  0x24
        if (RH == BUSY_BOX)
        { // нет устройства
            vdsDate->Diagn = BUSY_BOX;
            tdrv->error = BUSY_BOX;
            return;
        }
    }
    // -----------------
    
    RH = 0;
    ReadBox(AdrRQ, &RH);
    if ((RH & 0x01) || (RH & 0x10))
    {
        RH = 0;
        // проверка инверсии в статусе
        RH |= ReadBox3(AdrStatus0, &vdsValue.stat[0]);
        RH |= ReadBox3(AdrStatus1, &vdsValue.stat[1]);

        if (RH == BUSY_BOX)
        { // нет устройства
            vdsDate->Diagn = BUSY_BOX;
            tdrv->error = BUSY_BOX;
            return;
        }
        else if (RH == 0xC0)
        { // NEGC_BOX
            vdsDate->Diagn = NEGC_BOX;
            tdrv->error = NEGC_BOX;
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

        if (RH == BUSY_BOX)
        { // нет устройства
            vdsDate->Diagn = BUSY_BOX;
            tdrv->error = BUSY_BOX;
            return;
        }
        // else if (RH == 0xC0) { // NEGC_BOX
        //     tdrv->error = 0xC0;
        //     return;
        // }

        if (!inipar->inv)
        {
            for (k = 0; k < 4; k++)
                vdsValue.sost[k] = ~vdsValue.sost[k];
        }

        for (i = 0, k = 0; i < 4; i++)
        {
            j = 1;
            for (z = 0; z < 8; z++)
            {
                SErr = 0;
                if (vdsValue.obr[i] & j){
                    vdsDate->Diagn |= CHAN_ERR;    
                    SErr |= 0x0c;
                }
                if (!(vdsValue.kz[i] & j)){
                    vdsDate->Diagn |= CHAN_ERR;
                    SErr |= 0x30;
                }
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
}


    // для отладки
    // if (tdrv->address == 0x03) {
    //     // printk("----------------------------");
    //     // 21 de
    //     ReadSinglBox(0x21, &RH);
    //     if (RH != test[0][0]) {
    //         temp = 1;
    //         printk("0x21 izmenilsia -  old = %hhx  , new = %hhx", test[0][0], RH);
    //         test[0][0] = RH;
    //     }

    //     ReadSinglBox(0xde, &RH);
    //     if (RH != test[0][1]) {
    //         temp = 1;
    //         printk("0xde izmenilsia -  old = %hhx  , new = %hhx", test[0][1], RH);
    //         test[0][1] = RH;
    //     }
    //     if (temp == 1) {
    //         printk("tekyhee 21(de) %hhx - %hhx", test[0][0], test[0][1]);
    //         temp = 0;
    //     }

    //     // 24 db
    //     ReadSinglBox(0x24, &RH);
    //     if (RH != test[1][0]) {
    //         temp = 1;
    //         printk("0x24 izmenilsia -  old = %hhx  , new = %hhx", test[1][0], RH);
    //         test[1][0] = RH;
    //     }

    //     ReadSinglBox(0xdb, &RH);
    //     if (RH != test[1][1]) {
    //         temp = 1;
    //         printk("0xdb izmenilsia -  old = %hhx  , new = %hhx", test[1][1], RH);
    //         test[1][1] = RH;
    //     }
    //     if (temp == 1) {
    //         printk("tekyhee 24(db) %hhx - %hhx", test[1][0], test[1][1]);
    //         temp = 0;
    //     }

    //     // 20 df
    //     ReadSinglBox(0x20, &RH);
    //     if (RH != test[2][0]) {
    //         temp = 1;
    //         printk("0x20 izmenilsia -  old = %hhx  , new = %hhx", test[2][0], RH);
    //         test[2][0] = RH;
    //     }

    //     ReadSinglBox(0xdf, &RH);
    //     if (RH != test[2][1]) {
    //         temp = 1;
    //         printk("0xdf izmenilsia -  old = %hhx  , new = %hhx", test[2][1], RH);
    //         test[2][1] = RH;
    //     }
    //     if (temp == 1) {
    //         printk("tekyhee 20(df) %hhx - %hhx", test[2][0], test[2][1]);
    //         temp = 0;
    //     }

    //     // 23 dc
    //     ReadSinglBox(0x23, &RH);
    //     if (RH != test[3][0]) {
    //         temp = 1;
    //         printk("0x23 izmenilsia -  old = %hhx  , new = %hhx", test[3][0], RH);
    //         test[3][0] = RH;
    //     }

    //     ReadSinglBox(0xdc, &RH);
    //     if (RH != test[3][1]) {
    //         temp = 1;
    //         printk("0xdc izmenilsia -  old = %hhx  , new = %hhx", test[3][1], RH);
    //         test[3][1] = RH;
    //     }
    //     if (temp == 1) {
    //         printk("tekyhee 23(dc) %hhx - %hhx", test[3][0], test[3][1]);
    //         temp = 0;
    //     }

    //     // printk("============================");
    // }
    // убириаем красный глаз?
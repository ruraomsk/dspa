/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "../dspadef.h"
#include "../misparw.h"
#include "vas84r.h"
#include "linux/printk.h"

#define inipar  ((vas84r_inipar*)(tdrv->inimod)) 

#define VasData ((vas84r_data*)(tdrv->data))
//extern unsigned int irq_count;  ??
#define AdrType       0x4   // тип модуля   

#define AdrRQ         0x5   // запрос на обслуживание  

#define AdrSV         0x6   // флаг захвата ПЯ со стороны ФП                 

#define AdrSVE        0x7   // флаг завершения обслуживания ПЯ 

#define AdrMASTER     0x8   // флаг задатчика времени                        

#define AdrNewTime    0x9   // флаг нового времени                           

#define AdrNOTINV     0xa   // слово - счетчик количества неинверсий ПЯ      
// обнаруженных модулем                          
#define AdrNINVFP     0xc   // слово - счетчик количества неинверсий ПЯ      
// обнаруженных ФП                               
#define AdrCT_GLOB    0xe   // счетчик переворотов ПЯ                        

#define AdrSTAT       0xF   // байт состояния модуля

#define AdrData       0x10  // начало таблицы данных                 

#define AdrSOST       0x43  // расширенный байт состояния  

/*
===========================================================
Типы диагностических сообщений модуля

структура байта достоверности модуля 
 
  Бит         Значение

   0   -   ошибки канала 1,5
   1   -   ошибки канала 2,6
   2   -   ошибки канала 3,7
   3   -   ошибки канала 4,8  
   4   -   ошибка статуса ( неинверсия по чтению и пр)   
   5   -   ошибка функционирования(неинверсии, неактивен...)
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

;коды ошибки:
;080h-модуль недоступен(NMI)
;0c0h-неинверсия по чтению CT_GLOB
;088h-неинверсия по чтению RQ
;0c4h-неинверсия по чтению STAT
;084h-неисправность по всем каналам
;004h-поканальная ошибка


==========================================================

 */

void vas84r_ini(table_drv* tdrv) {
    int ADR_MISPA = 0x118;
    unsigned char RQ = (unsigned char) (tdrv->address & 0xff);
    tdrv->error = 0;
    //log_init(tdrv);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    RQ = CatchBox();
        if (RQ) {
            tdrv->error = RQ;
            return;
        } 
    RQ = FreeBox(); // освободить ПЯ
    if (RQ) {
        tdrv->error = RQ; // ошибка миспа
        return;
    }
    
}

void vas84r_rd(table_drv* tdrv) {
    unsigned char RQ, RH, RL;
    short temp;
    int i, ADR_MISPA = 0x118;
    //    log_step(tdrv);

    SetBoxLen(inipar->BoxLen);
    if (tdrv->error == 0x80) return;
    // установить адрес модуля на МИСПА

    RQ = (char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    for (i = 0; i < 8; i++)
        VasData->SIGN[i].error = 0xff;

    //     // захват ПЯ модуля 
    while (1) {
        RH = CatchBox();
        if (RH) {
            tdrv->error = RH;
            break;
        } // не могу захватить ПЯ


        RH |= ReadBx3w(AdrSOST, &VasData->widesos.c);
        RH |= ReadBx3w(AdrSTAT, &RQ);
        if (RQ & 0x80) {
            tdrv->error = RQ;
            break;
        }
        if (RH == 0x80) { // нет устройства
            tdrv->error = RH;
            break;
        }
        ReadBx3w(AdrRQ, &RQ);
        if (RQ == 0)
            break;
        RH = 0;
        for (i = 0; i < 8; i++) {
            RH |= ReadBx3w(AdrData + (i * 3), &RL);
            RH |= ReadBx3w(AdrData + (i * 3) + 1, &RQ);
            if (RH == 0x80) {
                tdrv->error = RH;
                break;
            }
            temp = ((RL << 8) | RQ);
            VasData->SIGN[i].i = temp;
            VasData->SIGN[i].error = 0;
        }
        break;
    }
    WriteBox(AdrRQ, 0);
    WriteBox(AdrSVE, 1);

    RH = FreeBox(); // освободить ПЯ
    if (RH) {
        tdrv->error = RH; // ошибка миспа
    }
}


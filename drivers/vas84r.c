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
    unsigned char RQ;
    
    VasData->NumK = 0;
    tdrv->error = SPAPS_OK;
    VasData->Diagn = SPAPS_OK;

    CLEAR_MEM
    WritePort(ADR_MISPA, (unsigned char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        VasData->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    // ReadBox3(AdrType, &RQ);
    //  if (RQ != inipar->type) {
    //      tdrv->error = BUSY_BOX;
    //      VasData->Diagn = WRONG_DEV;
    //      return;
    //  } //ошибка типа модуля


    RQ = CatchBox();
        if (RQ) {
            VasData->Diagn = SOST_ERR;
            tdrv->error = RQ;
            return;
        } 

    RQ = FreeBox(); // освободить ПЯ
    if (RQ) {
        VasData->Diagn = SOST_ERR;
        tdrv->error = RQ; // ошибка миспа
        return;
    }
    
}

void vas84r_rd(table_drv* tdrv) {
    unsigned char RQ, RH, RL;
    short temp;
    int i, ADR_MISPA = 0x118;

    if (tdrv->error) 
        return;

    // установить адрес модуля на МИСПА
    RQ = (char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        VasData->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    if(VasData->NumK == 0)
        for (i = 0; i < 8; i++)
            VasData->SIGN[i].error = 0xff;

    //     // захват ПЯ модуля 
    while (1) {
        RH = CatchBox();
        if (RH) {
            VasData->Diagn = BUSY_BOX;
            tdrv->error = RH;
            break;
        } // не могу захватить ПЯ


        RH |= ReadBx3w(AdrSOST, &VasData->widesos.c);
        RH |= ReadBx3w(AdrSTAT, &RQ);
        if (RQ & BUSY_BOX) {
            VasData->Diagn = SOST_ERR;
            tdrv->error = RQ;
            break;
        }
        // if (RH == BUSY_BOX) { // нет устройства
        //     VasData->Diagn = BUSY_BOX;
        //     tdrv->error = RH;
        //     break;
        // }
        ReadBx3w(AdrRQ, &RQ);
        if (RQ == 0)
            break;

        RH = 0;
        // for (i = 0; i < 8; i++) {
        RH |= ReadBx3w(AdrData + (VasData->NumK * 3), &RL);
        RH |= ReadBx3w(AdrData + (VasData->NumK * 3) + 1, &RQ);
        if (RH == BUSY_BOX) {
            // VasData->Diagn = BUSY_BOX;
            // tdrv->error = RH;
            break;
        }
        temp = ((RL << 8) | RQ);
        VasData->SIGN[VasData->NumK].i = temp;
        VasData->SIGN[VasData->NumK].error = 0;
        // }
        break;
    }

    WriteBox(AdrRQ, 0);
    WriteBox(AdrSVE, 1);

    RH = FreeBox(); // освободить ПЯ
    if (RH) {
        VasData->Diagn = BUSY_BOX;
        tdrv->error = RH; // ошибка миспа
    }

    if(VasData->NumK == 7)
        VasData->NumK = 0;
    else
        VasData->NumK++;
}


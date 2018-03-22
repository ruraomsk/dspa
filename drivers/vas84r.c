/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "../dspadef.h"
#include "../misparw.h"
#include "vas84r.h"
/*
typedef struct
{
  unsigned char type;    // default = 0xC6; // тип модуля 
  unsigned int  BoxLen;  // default = 0xFF; // длина ПЯ, уменьшенная на 1 
  unsigned char vip;     // default = 0;    // флаг критически важного для системы модуля 
  unsigned char NumCh;   // default = 8;    // количество каналов 
  unsigned char UsMask;  // default = 0xFF; // маска использования каналов
  unsigned char ChMask;  // default = 0x0;  // флаги изменения каналов 
  unsigned char Aprt;    // default = 0x17;  // апертура 
} vas84r_inipar;

typedef struct
{
  psint SIGN[8];   // Результат счета каналов 1-8   
  pschar widesos;  // расширенный байт состояния
} vas84r_data;

 */

#define inipar  ((vas84r_inipar*)(tdrv->inimod)) 

#define CT_GLOB   tdrv->tdrv.typedev[0]    // последний счетчик переворотов ПЯ
#define CounGL    tdrv->tdrv.typedev[1]    // счетчик непереворотов ПЯ

#define ModData ((vas84r_data*)(tdrv->data))

#define AdrType       0x0   // тип модуля   

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
typedef struct
{
  unsigned char type;    // тип модуля 
  unsigned int  BoxLen;  // длина ПЯ, уменьшенная на 1 
  unsigned char vip;     // флаг критически важного для системы модуля 
} proc_inipar;

 */
//===========================================================
//	Инициализация процессорного модуля
//===========================================================
//
// процедура осуществляет проверку исправности процессорного модуля
//

void vas84r_ini(table_drv* tdrv) {
    //?????????????????????????????????????????????????????????????????????????
    int ADR_MISPA = 0x118;
    unsigned char RQ = (unsigned char) (tdrv->address & 0xff);
    log_init(tdrv);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }
    tdrv->error = 0;
}

void vas84r_dw(table_drv* tdrv) {
    unsigned char SOST, RL;
    unsigned char STAT, RQ;
    int RH, i, k;
    int ADR_MISPA = 0x118;
    log_step(tdrv);
    ssint rr = {0, 0};
    sschar rc = {0, 0};
    SetBoxLen(inipar->BoxLen);

    if(tdrv->error == 0x80) return;
    // установить адрес модуля на МИСПА

    RQ = (char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    tdrv->error = 0;


    while (1) {

        //инициализация процессорного модуля

        if (tdrv->error & 0x80)
            break;

        // захват ПЯ модуля 

        RH = CatchBox();
        if (RH) {
            tdrv->error = RH;
            break;
        } // не могу захватить ПЯ
        RH = ReadBox3(AdrCT_GLOB, &RL);
        rc.error = RH;
        rc.c = RL;
        //     WDEBUG_PRINT_HEX(3,"CT_GLOB=",&rc);

        if (RH) {
            if (RH == 0x80) {
                tdrv->error = RH;
            } else
                // неинверсия на счётчик переворотов
                tdrv->error = 0xA0; // ящик не крутится
            break;
        }

        if ((RL == tdrv->tdrv.typedev[0]) || (tdrv->error == 0xA0)) { // ящик не крутился
            if (CounGL < 120)
                ++CounGL;
            else {
                tdrv->error |= 0x20;
                break;
            } // ??  в прототипе - нет

        } else {
            CounGL = 0;
            tdrv->tdrv.typedev[0] = RL;
        }

        // загрузить слово состояния 

        RH = ReadBox3(AdrSTAT, &STAT);
        rc.c = STAT;
        rc.error = RH;

        //     WDEBUG_PRINT_HEX(4,"STAT=",&rc);

        if (RH) { //ошибка статуса модуля
            if (RH == 0x80)
                tdrv->error = RH;
            else
                tdrv->error = 0xC0;
            break;
        } else
            if ((tdrv->error = STAT) & 0x80)
            break; //ошибка статуса модуля

        // есть данные ?

        RH = ReadBox3(AdrRQ, &RQ);

        if (RH) { // ошибка готовности модуля
            if (RH == 0x80)
                tdrv->error = RH;
            else
                tdrv->error = 0x90;
            break;
        }

        if (RQ) { // есть новые данные

            //  unsigned char NumCh;   // default = 8;    // количество каналов 
            //  unsigned char UsMask;  // default = 0xFF; // маска использования каналов
            //  unsigned char ChMask;  // default = 0x0;  // флаги изменения каналов 
            //  unsigned char Aprt;    // default = 0x17;  // апертура 

            RH = ReadBox3(AdrSOST, &SOST);
            rc.c = SOST;
            rc.error = RH;

            //       WDEBUG_PRINT_HEX(5,"Wsost=",&rc);

            if (RH) { //ошибка статуса модуля
                if (RH == 0x80)
                    tdrv->error = RH;
                else
                    tdrv->error = 0xC0; // 
                break;
            }

            ModData->widesos.c = SOST;
            ModData->widesos.error = 0;
            inipar->ChMask = 0;
            for (i = 0, k = 1, inipar->ChMask = 0; i < 8; i++) {
                if ((inipar->UsMask & k) && !(SOST & k)) { // канал используется и исправен

                    //  psint SIGN[8];   // Результат счета каналов 1-8   
                    //  pschar widesos;  // расширенный байт состояния

                    RH = ReadBox3(AdrData + (i * 3), &RL);

                    if (RH) {
                        if (RH == 0x80) {
                            tdrv->error = RH;
                            break;
                        } else
                            ModData->SIGN[i].error |= 0x20; // 

                    } else
                        ModData->SIGN[i].error = 0x0; // 

                    RH = ReadBox3(AdrData + (i * 3) + 1, &RQ);

                    if (RH) {
                        if (RH == 0x80) {
                            tdrv->error = RH;
                            break;
                        } else
                            ModData->SIGN[i].error |= 0x20; // 

                    }
                    if (!ModData->SIGN[i].error)

                        rr.i = (int) RL * 256 + RQ;
                    if (abs(rr.i - ModData->SIGN[i].i) > inipar->Aprt) {
                        inipar->ChMask |= k;
                    }
                    ModData->SIGN[i].i = rr.i;

                    //            WDEBUG_PRINT_INT(10+i,"VAS84 =",&ModData->SIGN[i]);

                }
                k <<= 1;
            }
            rc.c = inipar->ChMask;
            rc.error = 0;
            //        WDEBUG_PRINT_HEX(20,"ChMask=",&rc);

        }
        break;
    }
    //ящик обслужен
    RH = WriteBox(AdrSVE, 1);

    if (RH) {
        tdrv->error = RH; // ошибка миспа
    }

    RH = FreeBox(); // освободить ПЯ
    if (RH) {
        tdrv->error = RH; // ошибка миспа
    }

    if (tdrv->error & 0x80) {
        for (i = 0, k = 1, inipar->ChMask = 0; i < 8; i++) {
            ModData->SIGN[i].error = tdrv->error;
        }
        ModData->widesos.error = tdrv->error;
    }

}



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
#define NewDate ((vds32r_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))

#define AdrType 0x04 // тип модуля
#define AdrRQ 0x5    // регистр запроса обслуживания
//Регистры состояния контактов датчиков
#define AdrSostContact0 0x10 // каналы 1-8
#define AdrSostContact1 0x11 // каналы 9-16
#define AdrSostContact2 0x12 // каналы 17-24
#define AdrSostContact3 0x13 // каналы 25-32
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
//Регистры маски каналов
#define AdrChanlsMask0 0x21 // каналы 1-16
//Регистрыы статуса
#define AdrStatus0 0x22 // каналы 1-16
//Регистры настройки времени антидребезга
#define AdrAntiTrembl1 0x23 // каналы 17-32
//Регистры маски каналов
#define AdrChanlsMask1 0x24 // каналы 17-32
//Регистрыы статуса
#define AdrStatus1 0x25 // каналы 17-32

/*
extern unsigned char flag_ini;
 */
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
typedef struct
{
  unsigned char type;       // default = 0xC2;  тип модуля 
  unsigned int  BoxLen;     // default = 0xFF;  длина ПЯ, уменьшенная на 1 
//  unsigned char vip;        // default = 0;     флаг критически важного для системы модуля 
  unsigned char inv;        // флаг инверсии 0 - прямой 1 - инверстный 
  unsigned char NumCh;      // default = 8;     количество каналов 
  unsigned char tadr116;    // default = 0xFF;  Время антидребезга каналов 1-16  
  unsigned char tadr1732;   // default = 0xFF;  Время антидребезга каналов 17-32 
  unsigned char Dmask116;   // default = 0xFF;  маска диагностики каналов 1-16   
  unsigned char Dmask1732;  // default = 0xFF;  маска диагностики каналов 17-32 
  unsigned char UsMask18;   // default = 0xFF;  маска использования каналов 1-8  
  unsigned char UsMask916;  // default = 0xFF;  маска использования каналов 9-16 
  unsigned char UsMask1724; // default = 0xFF;  маска использования каналов 17-24
  unsigned char UsMask2532; // default = 0xFF;  маска использования каналов 25-32
  unsigned char ChMask18;   // default = 0x0;   маска изменения состояния каналов 1-8  
  unsigned char ChMask916;  // default = 0x0;   маска изменения состояния каналов 9-16 
  unsigned char ChMask1724; // default = 0x0;   маска изменения состояния каналов 17-24
  unsigned char ChMask2532; // default = 0x0;   маска изменения состояния каналов 25-32
} vds32r_inipar;

typedef struct
{
  ssbool SIGN[32]; // Результат счета каналов 1-8   

} vds32r_data;

 */

//===========================================================
//  Инициализация модуля ВДС-32Р
//===========================================================

void vds32r_ini(table_drv *tdrv) {
    unsigned char RQ, RH, RL;
    int ADR_MISPA;
    ADR_MISPA = 0x118;

    SetBoxLen(inipar->BoxLen);

    RQ = (unsigned char) (tdrv->address & 0xff);
    CLEAR_MEM

    WritePort(ADR_MISPA, RQ);

    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }
    tdrv->error = 0;

    //сброс регистров модуля
    // WriteSinglBox(0x22, 0);
    // WriteSinglBox(0x25, 0);
    RH = WriteSinglBox(4, 0);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    RH = WriteSinglBox(6, 0);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    // проверка типа модуля

    RH = ReadBox3(AdrType, &RL);
    RQ = inipar->type;
    if (RH) {
        if (RH == 0x80) {
            tdrv->error = RH;
            return;
        } else {
            tdrv->error = 0xC0;
            return;
        }
    }

    if (RL != RQ) {
        tdrv->error = 0xC0;
        return;
    } //ошибка типа модуля

    //настроить регистры конфигурирования каналов
    //Регистры настройки времени антидребезга

    RQ = inipar->tadr116; // каналы 1-16   0x20
    RH = WriteBox(AdrAntiTrembl0, RQ);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }
    //Регистры маски каналов

    RQ = inipar->Dmask116; // каналы 1-16   0x21
    // RQ = 0;
    RH = WriteBox(AdrChanlsMask0, RQ);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистры настройки времени антидребезга

    RQ = inipar->tadr1732; // каналы 17-32  0x23
    RH = WriteBox(AdrAntiTrembl1, RQ);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистры маски каналов

    RQ = inipar->Dmask1732; // каналы 17-32  0x24
    // RQ = 0;
    RH = WriteBox(AdrChanlsMask1, RQ);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистрыы статуса

    RH = ReadBox3(AdrStatus0, &RL);

    if (RH == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else if (RH) {
        tdrv->error = 0x90;
        return;
    } // неинверсия статуса модуля

    if (RL & 0xC0) {
        tdrv->error = 0xA0;
        return;
    } // ошибка конфигурирования модуля

    RH = ReadBox3(AdrStatus1, &RL);

    if (RH == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else if (RH) {
        tdrv->error = 0x90;
        return;
    } // неинверсия статуса модуля

    if (RL & 0xC0) {
        tdrv->error = 0xA0;
        return;
    } // ошибка инициализации модуля

    //Еще раз проверить, возникла ли неинверсия

    // в настроенных регистрах

    //  проверка запроса обслуживания

    RH = ReadBox3(AdrRQ, &RL);
    printk("RH= $hhx", RH);
    printk("RL= $hhx", RL);
    if ((tdrv->error = RH) == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else if (RH) {
        tdrv->error = 0xA0;
        return;
    } // неинверсия запроса обслуживания

    if (RL & 0xCC) {
        tdrv->error = 0xA0;
        return;
    } // ошибка инициализации модуля
}

//===========================================================/
//  Прием данных из модуля ВДС-32Р
//===========================================================

/*
===========================================================
структура байта достоверности канала 
 
  Бит         Значение

   0   -   обрыв линии связи
   1   -   короткое замыкание линии связи
   2   -   неинверсия чтения диагностики обрыва
   3   -   неинверсия чтения диагностики короткого замыкания линии связи  
   4   -   неинверсия чтения данных  
   5   -   ошибка конфигурирования
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

===========================================================
 */
void vds32r_dw(table_drv *tdrv) {
    vds32r_str vdsValue;
    char stat0, stat1, chn1, chn2, chn3, chn4, chn1er, chn2er, chn3er, chn4er;
    unsigned char ercn = 0, erdn = 0, obr = 0, kz = 0, i, j, z;
    sschar rc, ro; // rz;
    unsigned char msk = 0;
    unsigned char RH, aRH = 0, SErr = 0; // RL;
    //    unsigned char RQ;
    //    log_step(tdrv);
    int k = 0; // указатель заполняемого данного
    int ADR_MISPA;

    SetBoxLen(0xFF);

    if (tdrv->error & 0x80) {
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
        NewDate->SIGN[k].error = 0xff;

    RH = ReadBox3(AdrStatus0, &vdsValue.stat[0]);
    aRH |= RH;
    printk("vdsValue.stat[0] = %hhx", vdsValue.stat[0]);
    RH = ReadBox3(AdrStatus1, &vdsValue.stat[1]);
    aRH |= RH;
    printk("vdsValue.stat[1] = %hhx", vdsValue.stat[1]);
    RH = ReadBox3(AdrSostContact0, &vdsValue.sost[0]);
    aRH |= RH;
    printk("vdsValue.sost[0] = %hhx", vdsValue.sost[0]);
    RH = ReadBox3(AdrSostContact1, &vdsValue.sost[1]);
    aRH |= RH;
    printk("vdsValue.sost[1] = %hhx", vdsValue.sost[1]);
    RH = ReadBox3(AdrSostContact2, &vdsValue.sost[2]);
    aRH |= RH;
    printk("vdsValue.sost[2] = %hhx", vdsValue.sost[2]);
    RH = ReadBox3(AdrSostContact3, &vdsValue.sost[3]);
    aRH |= RH;
    printk("vdsValue.sost[3] = %hhx", vdsValue.sost[3]);
    RH = ReadBox3(AdrOpnCircuit0, &vdsValue.obr[0]);
    aRH |= RH;
    printk("vdsValue.obr[0] = %hhx", vdsValue.obr[0]);
    RH = ReadBox3(AdrOpnCircuit1, &vdsValue.obr[1]);
    aRH |= RH;
    printk("vdsValue.obr[1] = %hhx", vdsValue.obr[1]);
    RH = ReadBox3(AdrOpnCircuit2, &vdsValue.obr[2]);
    aRH |= RH;
    printk("vdsValue.obr[2] = %hhx", vdsValue.obr[2]);
    RH = ReadBox3(AdrOpnCircuit3, &vdsValue.obr[3]);
    aRH |= RH;
    printk("vdsValue.obr[3] = %hhx", vdsValue.obr[3]);
    RH = ReadBox3(AdrShortCircuit0, &vdsValue.kz[0]);
    aRH |= RH;
    printk("vdsValue.kz[0] = %hhx", vdsValue.kz[0]);
    RH = ReadBox3(AdrShortCircuit1, &vdsValue.kz[1]);
    aRH |= RH;
    printk("vdsValue.kz[1] = %hhx", vdsValue.kz[1]);
    RH = ReadBox3(AdrShortCircuit2, &vdsValue.kz[2]);
    aRH |= RH;
    printk("vdsValue.kz[2] = %hhx", vdsValue.kz[2]);
    RH = ReadBox3(AdrShortCircuit3, &vdsValue.kz[3]);
    aRH |= RH;
    printk("vdsValue.kz[3] = %hhx", vdsValue.kz[3]);

    if (aRH == 0x80) {
        tdrv->error = 0x80;
        return;
    }
    // } else if (aRH == 0xC0) {
    //     tdrv->error = 0xC0;
    //     return;
    // }

    for (i = 0, k = 0; i < 4; i++) {
        j = 1;
        for (z = 0; z < 8; z++) {
            SErr = 0;
            if (vdsValue.obr[i] & j)
                SErr |= 0x0c;
            if (!(vdsValue.kz[i] & j))
                SErr |= 0x30;
            NewDate->SIGN[k].error = SErr;

            if (vdsValue.sost[i] & j)
                NewDate->SIGN[k].b = 1;
            else
                NewDate->SIGN[k].b = 0;
            j <<= 1;
            k++;
        }
    }


    if (!inipar->inv) {
        for (k = 0; k < 32; k++)
            NewDate->SIGN[k].b = !NewDate->SIGN[k].b;
    }



    //   RH = ReadBox3(AdrStatus0, &stat0);
    //    RH = ReadBox3(AdrStatus0, &vdsValue.stat[0]);

    // if (RH == 0x80) {
    //     tdrv->error = 0x80;
    //     return;
    // }// ошибка миспа
    // else if (RH) {
    //     tdrv->error = 0x90;
    //     return;
    // } // неинверсия статуса модуля

    // if (vdsValue.stat[0] & 0xC0)
    //     ercn = 0xA3; // ошибка инициализации модуля
    // else
    //     ercn = 0;

    //  RH = ReadBox3(AdrStatus1, &stat1);
//    RH = ReadBox3(AdrStatus1, &vdsValue.stat[1]);

    // if (RH == 0x80) {
    //     tdrv->error = 0x80;
    //     return;
    // }// ошибка миспа
    // else if (RH) {
    //     tdrv->error = 0x90;
    //     return;
    // } // неинверсия статуса модуля

    // if (vdsValue.stat[1] & 0xC0)
    //     ercn |= 0xAC; // ошибка инициализации модуля

//    tdrv->error &= 0xfff0;
//
//    chn1er = chn2er = chn3er = chn4er = ercn;
//    msk = inipar->UsMask18;

    // используются каналы  1-8

    //   0   -   обрыв линии связи
    //   1   -   короткое замыкание линии связи
    //   2   -   неинверсия чтения диагностики обрыва
    //   3   -   неинверсия чтения диагностики короткого замыкания линии связи
    //   4   -   неинверсия чтения данных
    //   5   -   ошибка конфигурирования
    //   6   -   резерв
    //   7   -   критическая ошибка или нет доступа к ПЯ

    //Регистры состояния контактов датчиков
    // printk("stat0 = %hhx", stat0);
//    while (msk) {
        // if (stat0 & 1)
        // {
        //     RH = ReadBox3(AdrSostContact0, &chn1);
        //     printk("chn1 = %hhx", chn1);
        //     chn1 = !chn1;
        //     if (RH == 0x80)
        //     {
        //         tdrv->error = 0x80;
        //         return;
        //     } // ошибка миспа
        //     else if (RH)
        //     {
        //         chn1er |= 0x10; //неинверсия данных канала 1-8
        //         break;
        //     }
        //     else
        //     {
        //         inipar->ChMask18 = LastIn[0] ^ chn1;
        //         LastIn[0] = chn1;
        //     }
        // }
        // else
        // {
        //     chn1 = LastIn[0];
        //     inipar->ChMask18 = 0;
        // }
        // rc.c = chn1;
        // rc.error = chn1er;

        //--------------------------------------------
        //      delay(100);
        //--------------------------------------------
        //Регистры обрывов линий связи с датчиками

        // if (stat0 & 4) {
        //     //  RH = ReadBox3(AdrOpnCircuit0, &obr);
        //     RH = ReadBox3(AdrOpnCircuit0, &vdsValue.obr[0]);
        //     printk("obr = %hhx", vdsValue.obr[0]);

        //     if (RH == 0x80) {
        //         tdrv->error = 0x80;
        //         return;
        //     }// ошибка миспа
        //     else if (RH) {
        //         chn1er |= 4; //неинверсия обрыва
        //         break;
        //     }
        // } else
        //     vdsValue.obr[0] = 0;

        // ro.c = vdsValue.obr[0];
        // ro.error = chn1er;

        //Регистры коротких замыканий линий связи с датчиками

        // if (stat0 & 0x10) {
        //                 // RH = ReadBox3(AdrShortCircuit0, &kz);
        //     RH = ReadBox3(AdrShortCircuit0, &vdsValue.kz[0]);
        //     printk("kz = %hhx", vdsValue.kz[0]);

        //     vdsValue.kz[0] = !vdsValue.kz[0];

        //     if (RH == 0x80) {
        //         tdrv->error = 0x80;
        //         return;
        //     }// ошибка миспа
        //     else if (RH) {
        //         chn1er |= 8; //неинверсия кз
        //         break;
        //     }
        // } else
        //     vdsValue.kz[0] = 0;

//        printk("msk = %hhx", msk);
//        // ReadBox3(AdrSostContact0, &chn1);
//        ReadBox3(AdrSostContact0, &vdsValue.sost[0]);
//        char asd;
//        ReadBox3(0x21, &asd);
//        printk("21 = %hhx", asd);
//        ReadBox3(0xde, &asd);
//        printk("de = %hhx", asd);
//        ReadBox3(0x24, &asd);
//        printk("24 = %hhx", asd);
//        ReadBox3(0xdb, &asd);
//        printk("db = %hhx", asd);
//
//        ReadBox3(0x10, &asd);
//        printk("10 = %hhx", asd);
//        ReadBox3(0xef, &asd);
//        printk("ef = %hhx", asd);
//        ReadBox3(0x11, &asd);
//        printk("11 = %hhx", asd);
//        ReadBox3(0xee, &asd);
//        printk("ee = %hhx", asd);
//
//        ReadBox3(0x12, &asd);
//        printk("12 = %hhx", asd);
//        ReadBox3(0xed, &asd);
//        printk("ed = %hhx", asd);
//        ReadBox3(0x13, &asd);
//        printk("13 = %hhx", asd);
//        ReadBox3(0xec, &asd);
//        printk("ec = %hhx", asd);
//
//        ReadBox3(0x14, &asd);
//        printk("14 = %hhx", asd);
//        ReadBox3(0xeb, &asd);
//        printk("eb = %hhx", asd);
//        ReadBox3(0x15, &asd);
//        printk("15 = %hhx", asd);
//        ReadBox3(0xea, &asd);
//        printk("ea = %hhx", asd);
//
//        ReadBox3(0x16, &asd);
//        printk("16 = %hhx", asd);
//        ReadBox3(0xe9, &asd);
//        printk("e9 = %hhx", asd);
//        ReadBox3(0x17, &asd);
//        printk("19 = %hhx", asd);
//        ReadBox3(0xe8, &asd);
//        printk("e8 = %hhx", asd);
//
//        ReadBox3(0x18, &asd);
//        printk("18 = %hhx", asd);
//        ReadBox3(0xe7, &asd);
//        printk("e7 = %hhx", asd);
//        ReadBox3(0x19, &asd);
//        printk("19 = %hhx", asd);
//        ReadBox3(0xe6, &asd);
//        printk("e6 = %hhx", asd);
//
//        ReadBox3(0x1a, &asd);
//        printk("1a = %hhx", asd);
//        ReadBox3(0xe5, &asd);
//        printk("e5 = %hhx", asd);
//        ReadBox3(0x1b, &asd);
//        printk("1b = %hhx", asd);
//        ReadBox3(0xe4, &asd);
//        printk("e4 = %hhx", asd);
//
//        ReadBox3(0x25, &asd);
//        printk("25 = %hhx", asd);
//        ReadBox3(0xda, &asd);
//        printk("da = %hhx", asd);
//        ReadBox3(0x22, &asd);
//        printk("22 = %hhx", asd);
//        ReadBox3(0xdd, &asd);
//        printk("dd = %hhx", asd);
//
//
//        printk("vdsValue.sost[0] = %hhx", vdsValue.sost[0]);
        //         RH = ReadBox3(AdrSostContact0, &chn1);
        // printk("chn1 = %hhx", chn1);

        // уложить прочитанное в ящик
//        for (i = 1, j = 1; i < 9; i++) {
//            printk("k1:= %hhx", k);
//            printk("j1:= %hhx", j);
//            if (msk & j) {
//                //используется
//                if (obr & j || chn1er & 5) // обрыв или его неинверсия
//                    erdn = 1;
//                else
//                    erdn = 0;
//                if (kz & j || chn1er & 0xA) // KZ или его неинверсия
//                    erdn |= 2;
//
//                if (erdn || chn1er & 0x10) //обрыв или кз или неинверсия данных
//                    ercn = 1;
//                else {
//                    if (vdsValue.stat[0] & 1) {
//                        if (vdsValue.sost[0] & j)
//                            NewDate->SIGN[k].b = 1;
//                            // tdrv->data[k] = 1;
//                        else
//                            NewDate->SIGN[k].b = 0;
//                        // tdrv->data[k] = 0;
//                        printk("BIT:= %d", NewDate->SIGN[k].b);
//                    }
//                }
//                ++k;
//                NewDate->SIGN[k].error = erdn | chn1er;
//                //tdrv->data[k] = erdn | chn1er;
//                j <<= 1;
//                printk("j2:= %hhx", j);
//            } else
//                ++k;
//        }
//        NewDate->SIGN[k].error = ercn;
//        // tdrv->error |= ercn;
//        break;
//    }

    // msk = inipar->UsMask916;
    // while (msk)
    // {

    //     // используются каналы  9-16

    //     //Регистры состояния контактов датчиков

    //     if (stat0 & 2)
    //     {
    //         RH = ReadBox3(AdrSostContact1, &chn2);
    //         chn2 = !chn2;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn2er |= 0x10; //неинверсия данных канала 1-8
    //             break;
    //         }
    //         else
    //         {
    //             inipar->ChMask916 = LastIn[1] ^ chn2;
    //             LastIn[1] = chn2;
    //         }
    //         rc.c = chn2;
    //         rc.error = chn2er;
    //     }
    //     else
    //         chn2 = LastIn[1];
    //     //--------------------------------------------
    //     //      delay(100);
    //     //--------------------------------------------
    //     //Регистры обрывов линий связи с датчиками

    //     if (stat0 & 0x8)
    //     {
    //         RH = ReadBox3(AdrOpnCircuit1, &obr);

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn2er |= 4; //неинверсия обрыва
    //             break;
    //         }
    //     }
    //     else
    //         obr = 0;
    //     ro.c = obr;
    //     ro.error = chn2er;

    //     //Регистры коротких замыканий линий связи с датчиками

    //     if (stat0 & 0x20)
    //     {
    //         RH = ReadBox3(AdrShortCircuit1, &kz);
    //         kz = !kz;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn2er |= 8; //неинверсия кз
    //             break;
    //         }
    //     }
    //     else
    //         kz = 0;

    //     // уложить прочитанное в ящик

    //     for (i = 1, j = 1; i < 9; i++)
    //     {
    //         if (msk & j)
    //         {
    //             //используется
    //             if (obr & j || chn2er & 5) // обрыв или его неинверсия
    //                 erdn = 1;
    //             else
    //                 erdn = 0;
    //             if (kz & j || chn2er & 0xA) // KZ или его неинверсия
    //                 erdn |= 2;

    //             if (erdn || chn2er & 0x10)
    //                 ercn = 2;
    //             else if (stat0 & 0x2)
    //             {
    //                 if (chn2 & j)
    //                     tdrv->data[k] = 1;
    //                 else
    //                     tdrv->data[k] = 0;
    //             }
    //             ++k;
    //             tdrv->data[k] = erdn | chn2er;
    //             ++k;
    //             j <<= 1;
    //         }
    //         else
    //             k += 2;
    //     }

    //     tdrv->error |= ercn;
    //     break;
    // }

    // msk = inipar->UsMask1724;
    // while (msk)
    // {

    //     // используются каналы  17-24
    //     // Регистры состояния контактов датчиков

    //     if (stat1 & 0x1)
    //     {
    //         RH = ReadBox3(AdrSostContact2, &chn3);
    //         chn3 = !chn3;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn3er |= 0x10; //неинверсия данных канала 1-8
    //             break;
    //         }
    //         else
    //         {
    //             inipar->ChMask1724 = LastIn[2] ^ chn3;
    //             LastIn[2] = chn3;
    //         }
    //     }
    //     else
    //         chn3 = LastIn[2];

    //     rc.c = chn3;
    //     rc.error = chn3er;

    //     //--------------------------------------------
    //     //      delay(100);
    //     //--------------------------------------------
    //     //Регистры обрывов линий связи с датчиками

    //     if (stat1 & 0x4)
    //     {
    //         RH = ReadBox3(AdrOpnCircuit2, &obr);

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn3er |= 4; //неинверсия обрыва
    //             break;
    //         }
    //     }
    //     else
    //         obr = 0;

    //     ro.c = obr;
    //     ro.error = chn3er;

    //     //Регистры коротких замыканий линий связи с датчиками

    //     if (stat1 & 0x10)
    //     {
    //         RH = ReadBox3(AdrShortCircuit2, &kz);
    //         kz = !kz;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn3er |= 8; //неинверсия кз
    //             break;
    //         }
    //     }
    //     else
    //         kz = 0;

    //     // уложить прочитанное в ящик

    //     for (i = 1, j = 1; i < 9; i++)
    //     {
    //         if (msk & j)
    //         {
    //             //используется
    //             if (obr & j || chn3er & 5) // обрыв или его неинверсия
    //                 erdn = 1;
    //             else
    //                 erdn = 0;
    //             if (kz & j || chn3er & 0xA) // KZ или его неинверсия
    //                 erdn |= 2;

    //             if (erdn || chn3er & 0x10)
    //                 ercn = 4;
    //             else if (stat1 & 0x1)
    //             {
    //                 if (chn3 & j)
    //                     tdrv->data[k] = 1;
    //                 else
    //                     tdrv->data[k] = 0;
    //             }

    //             ++k;

    //             tdrv->data[k] = erdn | chn3er;

    //             ++k;

    //             j <<= 1;
    //         }
    //         else
    //             k += 2;
    //     }

    //     tdrv->error |= ercn;
    //     break;
    // }
    // msk = inipar->UsMask2532;
    // while (msk)
    // {

    //     // используются каналы 25-32
    //     //Регистры состояния контактов датчиков

    //     if (stat1 & 0x2)
    //     {

    //         RH = ReadBox3(AdrSostContact3, &chn4);
    //         chn4 = !chn4;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn4er |= 0x10; //неинверсия данных канала 1-8
    //             break;
    //         }
    //         else
    //         {
    //             inipar->ChMask2532 = LastIn[3] ^ chn4;
    //             LastIn[3] = chn4;
    //         }
    //     }
    //     else
    //         chn4 = LastIn[3];
    //     rc.c = chn4;
    //     rc.error = chn4er;

    //     //--------------------------------------------
    //     //      delay(100);
    //     //--------------------------------------------
    //     //Регистры обрывов линий связи с датчиками

    //     if (stat1 & 0x8)
    //     {
    //         RH = ReadBox3(AdrOpnCircuit3, &obr);

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn4er |= 4; //неинверсия обрыва
    //             break;
    //         }
    //     }
    //     else
    //         obr = 0;

    //     ro.c = obr;
    //     ro.error = chn4er;

    //     //Регистры коротких замыканий линий связи с датчиками

    //     if (stat1 & 0x20)
    //     {
    //         RH = ReadBox3(AdrShortCircuit3, &kz);
    //         kz = !kz;

    //         if (RH == 0x80)
    //         {
    //             tdrv->error = 0x80;
    //             return;
    //         } // ошибка миспа
    //         else if (RH)
    //         {
    //             chn4er |= 8; //неинверсия кз
    //             break;
    //         }
    //     }
    //     else
    //         kz = 0;

    //     // уложить прочитанное в ящик

    //     for (i = 1, j = 1; i < 9; i++)
    //     {
    //         if (msk & j)
    //         {
    //             //используется
    //             if (obr & j || chn4er & 5) // обрыв или его неинверсия
    //                 erdn = 1;
    //             else
    //                 erdn = 0;
    //             if (kz & j || chn4er & 0xA) // KZ или его неинверсия
    //                 erdn |= 2;

    //             if (erdn || chn4er & 0x10)
    //                 ercn = 8;
    //             else if (stat1 & 0x2)
    //             {
    //                 if (chn4 & j)
    //                     tdrv->data[k] = 1;
    //                 else
    //                     tdrv->data[k] = 0;
    //             }
    //             ++k;
    //             tdrv->data[k] = erdn | chn4er;
    //             ++k;
    //             j <<= 1;
    //         }
    //         else
    //             k += 2;
    //     }

    //     tdrv->error |= ercn;
    //     break;
    // }
}

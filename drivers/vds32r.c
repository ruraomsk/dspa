/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include "../dspadef.h"
#include "../misparw.h"
#include "vds32r.h"

#define inipar  ((vds32r_inipar*)(tdrv->inimod)) 
#define NewDate ((vds32r_inipar*)(tdrv->data)) 
#define LastIn  ((char*)(&tdrv->time))

#define AdrType          0x04 // тип модуля 
#define AdrRQ            0x5  // регистр запроса обслуживания  
//Регистры состояния контактов датчиков
#define AdrSostContact0  0x10 // каналы 1-8   
#define AdrSostContact1  0x11 // каналы 9-16  
#define AdrSostContact2  0x12 // каналы 17-24 
#define AdrSostContact3  0x13 // каналы 25-32 
//Регистры обрывов линий связи с датчиками
#define AdrOpnCircuit0   0x14 // каналы 1-8   
#define AdrOpnCircuit1   0x15 // каналы 9-16  
#define AdrOpnCircuit2   0x16 // каналы 17-24 
#define AdrOpnCircuit3   0x17 // каналы 25-32 
//Регистры коротких замыканий линий связи с датчиками
#define AdrShortCircuit0 0x18 // каналы 1-8   
#define AdrShortCircuit1 0x19 // каналы 9-16  
#define AdrShortCircuit2 0x1A // каналы 17-24 
#define AdrShortCircuit3 0x1B // каналы 25-32 
//Регистры настройки времени антидребезга
#define AdrAntiTrembl0   0x20 // каналы 1-16   
//Регистры маски каналов
#define AdrChanlsMask0   0x21 // каналы 1-16   
//Регистрыы статуса
#define AdrStatus0       0x22 // каналы 1-16   
//Регистры настройки времени антидребезга
#define AdrAntiTrembl1   0x23 // каналы 17-32  
//Регистры маски каналов
#define AdrChanlsMask1   0x24 // каналы 17-32  
//Регистрыы статуса
#define AdrStatus1       0x25 // каналы 17-32  

/*
extern unsigned char flag_ini;
 */


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
  unsigned char vip;        // default = 0;     флаг критически важного для системы модуля 
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

void vds32r_ini(table_drv* tdrv) {
    unsigned char RQ;
    unsigned char RH, RL;
    log_init(tdrv);
    int ADR_MISPA = 0x118;

    SetBoxLen(inipar->BoxLen);

    RQ = (unsigned char)(tdrv->address&0xff);
    WritePort(ADR_MISPA, RQ);
    tdrv->error = 0;

    //сброс регистров модуля

    RH =WriteSinglBox(4,0);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    RH=WriteSinglBox(6,0);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }


    // проверка типа модуля 


    RH =ReadBox3(AdrType,&RL);
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
    RH =WriteBox(AdrAntiTrembl0,RQ);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }
    //Регистры маски каналов

    RQ = inipar->Dmask116; // каналы 1-16   0x21
    RH =WriteBox(AdrChanlsMask0,RQ);
    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистры настройки времени антидребезга

    RQ = inipar->tadr1732; // каналы 17-32  0x23
    RH =WriteBox(AdrAntiTrembl1,RQ);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистры маски каналов

    RQ = inipar->Dmask1732; // каналы 17-32  0x24
    RH = WriteBox(AdrChanlsMask1,RQ);

    if (RH) {
        tdrv->error = 0xA0;
        return;
    }

    //Регистрыы статуса

    RH = ReadBox3(AdrStatus0,&RL);

    if (RH == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else
        if (RH) {
        tdrv->error = 0x90;
        return;
    } // неинверсия статуса модуля

    if (RL & 0xC0) {
        tdrv->error = 0xA0;
        return;
    } // ошибка конфигурирования модуля

    RH = ReadBox3(AdrStatus1,&RL);

    if (RH == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else
        if (RH) {
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

    RH = ReadBox3(AdrRQ,&RL);


    if ((tdrv->error = RH) == 0x80) {
        tdrv->error = RH;
        return;
    }// ошибка миспа
    else
        if (RH) {
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
void vds32r_dw(table_drv* tdrv) {
    char stat0, stat1, chn1, chn2, chn3, chn4, chn1er, chn2er, chn3er, chn4er;
    unsigned char ercn = 0, erdn, obr, kz, i, j;
    sschar rc, ro; // rz;
    unsigned char msk = 0;
    unsigned char RH;   // RL;
//    unsigned char RQ;
    log_step(tdrv);
    int k = 0; // указатель заполняемого данного
    int ADR_MISPA;
    
    SetBoxLen(0xFF);


    if (tdrv->error & 0x80) {
        //    return;          // пока повременить
    }

    ADR_MISPA = 0x118;
    WritePort(ADR_MISPA, (unsigned char)(tdrv->address&0xff));

    RH = ReadBox3(AdrStatus0,&stat0);

    if (RH == 0x80) {
        tdrv->error = 0x80;
        return;
    }// ошибка миспа
    else
        if (RH) {
        tdrv->error = 0x90;
        return;
    } // неинверсия статуса модуля

    if (stat0 & 0xC0)
        ercn = 0xA3; // ошибка инициализации модуля
    else
        ercn = 0;

    RH = ReadBox3(AdrStatus1,&stat1);

    if (RH == 0x80) {
        tdrv->error = 0x80;
        return;
    }// ошибка миспа
    else
        if (RH) {
        tdrv->error = 0x90;
        return;
    } // неинверсия статуса модуля

    if (stat1 & 0xC0)
        ercn |= 0xAC; // ошибка инициализации модуля

    tdrv->error &= 0xfff0;

    chn1er = chn2er = chn3er = chn4er = ercn;
    msk = inipar->UsMask18;
    while (msk) {

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

        if (stat0 & 1) {
            RH = ReadBox3(AdrSostContact0,&chn1);
            chn1=!chn1;
            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn1er |= 0x10; //неинверсия данных канала 1-8
                break;
            } else {
                inipar->ChMask18 = LastIn[0] ^ chn1;
                LastIn[0] = chn1;
            }
        } else {
            chn1 = LastIn[0];
            inipar->ChMask18 = 0;
        }

        rc.c = chn1;
        rc.error = chn1er;
        //--------------------------------------------
        //      delay(100);
        //--------------------------------------------
        //Регистры обрывов линий связи с датчиками

        if (stat0 & 4) {
            RH = ReadBox3(AdrOpnCircuit0,&obr);

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn1er |= 4; //неинверсия обрыва
                break;
            }
        } else
            obr = 0;

        ro.c = obr;
        ro.error = chn1er;


        //Регистры коротких замыканий линий связи с датчиками

        if (stat0 & 0x10) {
            RH = ReadBox3(AdrShortCircuit0,&kz);
            kz=!kz;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn1er |= 8; //неинверсия кз
                break;
            }
        } else
            kz = 0;

        // уложить прочитанное в ящик

        for (i = 1, j = 1; i < 9; i++) {
            if (msk & j) {
                //используется
                if (obr & j || chn1er & 5) // обрыв или его неинверсия
                    erdn = 1;
                else
                    erdn = 0;
                if (kz & j || chn1er & 0xA) // KZ или его неинверсия
                    erdn |= 2;

                if (erdn || chn1er & 0x10) //обрыв или кз или неинверсия данных
                    ercn = 1;
                else
                    if (stat0 & 1)
                    if (chn1 & j)
                        tdrv->data[k] = 1;
                    else
                        tdrv->data[k] = 0;
                ++k;
                tdrv->data[k] = erdn | chn1er;
                ++k;
                j <<= 1;

            } else
                k += 2;

        }

        tdrv->error |= ercn;
        break;
    }

    msk = inipar->UsMask916;
    while (msk) {

        // используются каналы  9-16 

        //Регистры состояния контактов датчиков

        if (stat0 & 2) {
            RH = ReadBox3(AdrSostContact1,&chn2);
            chn2=!chn2;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn2er |= 0x10; //неинверсия данных канала 1-8
                break;
            } else {
                inipar->ChMask916 = LastIn[1] ^ chn2;
                LastIn[1] = chn2;
            }
            rc.c = chn2;
            rc.error = chn2er;
        } else
            chn2 = LastIn[1];
        //--------------------------------------------
        //      delay(100);
        //--------------------------------------------
        //Регистры обрывов линий связи с датчиками

        if (stat0 & 0x8) {
            RH = ReadBox3(AdrOpnCircuit1,&obr);

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn2er |= 4; //неинверсия обрыва
                break;
            }
        } else obr = 0;
        ro.c = obr;
        ro.error = chn2er;

        //Регистры коротких замыканий линий связи с датчиками

        if (stat0 & 0x20) {
            RH = ReadBox3(AdrShortCircuit1,&kz);
            kz=!kz;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn2er |= 8; //неинверсия кз
                break;
            }
        } else kz = 0;

        // уложить прочитанное в ящик

        for (i = 1, j = 1; i < 9; i++) {
            if (msk & j) {
                //используется
                if (obr & j || chn2er & 5) // обрыв или его неинверсия
                    erdn = 1;
                else
                    erdn = 0;
                if (kz & j || chn2er & 0xA) // KZ или его неинверсия
                    erdn |= 2;

                if (erdn || chn2er & 0x10)
                    ercn = 2;
                else
                    if (stat0 & 0x2)
                    if (chn2 & j)
                        tdrv->data[k] = 1;
                    else
                        tdrv->data[k] = 0;
                ++k;
                tdrv->data[k] = erdn | chn2er;
                ++k;
                j <<= 1;
            } else
                k += 2;
        }

        tdrv->error |= ercn;
        break;
    }

    msk = inipar->UsMask1724;
    while (msk) {

        // используются каналы  17-24 
        // Регистры состояния контактов датчиков

        if (stat1 & 0x1) {
            RH = ReadBox3(AdrSostContact2,&chn3);
            chn3=!chn3;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn3er |= 0x10; //неинверсия данных канала 1-8
                break;
            } else {
                inipar->ChMask1724 = LastIn[2] ^ chn3;
                LastIn[2] = chn3;
            }
        } else
            chn3 = LastIn[2];

        rc.c = chn3;
        rc.error = chn3er;


        //--------------------------------------------
        //      delay(100);
        //--------------------------------------------
        //Регистры обрывов линий связи с датчиками

        if (stat1 & 0x4) {
            RH = ReadBox3(AdrOpnCircuit2,&obr);

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn3er |= 4; //неинверсия обрыва
                break;
            }
        } else obr = 0;

        ro.c = obr;
        ro.error = chn3er;

        //Регистры коротких замыканий линий связи с датчиками

        if (stat1 & 0x10) {
            RH = ReadBox3(AdrShortCircuit2,&kz);
            kz=!kz;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn3er |= 8; //неинверсия кз
                break;
            }
        } else kz = 0;

        // уложить прочитанное в ящик

        for (i = 1, j = 1; i < 9; i++) {
            if (msk & j) {
                //используется
                if (obr & j || chn3er & 5) // обрыв или его неинверсия
                    erdn = 1;
                else
                    erdn = 0;
                if (kz & j || chn3er & 0xA) // KZ или его неинверсия
                    erdn |= 2;

                if (erdn || chn3er & 0x10)
                    ercn = 4;
                else
                    if (stat1 & 0x1)
                    if (chn3 & j)
                        tdrv->data[k] = 1;
                    else
                        tdrv->data[k] = 0;

                ++k;

                tdrv->data[k] = erdn | chn3er;

                ++k;

                j <<= 1;

            } else
                k += 2;
        }

        tdrv->error |= ercn;
        break;
    }
    msk = inipar->UsMask2532;
    while (msk) {

        // используются каналы 25-32 
        //Регистры состояния контактов датчиков

        if (stat1 & 0x2) {

            RH = ReadBox3(AdrSostContact3,&chn4);
            chn4=!chn4;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn4er |= 0x10; //неинверсия данных канала 1-8
                break;
            } else {
                inipar->ChMask2532 = LastIn[3] ^ chn4;
                LastIn[3] = chn4;
            }
        } else
            chn4 = LastIn[3];
        rc.c = chn4;
        rc.error = chn4er;


        //--------------------------------------------
        //      delay(100);
        //--------------------------------------------
        //Регистры обрывов линий связи с датчиками

        if (stat1 & 0x8) {
            RH = ReadBox3(AdrOpnCircuit3,&obr);

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn4er |= 4; //неинверсия обрыва
                break;
            }
        } else
            obr = 0;

        ro.c = obr;
        ro.error = chn4er;

        //Регистры коротких замыканий линий связи с датчиками

        if (stat1 & 0x20) {
            RH=ReadBox3(AdrShortCircuit3,&kz);
            kz=!kz;

            if (RH == 0x80) {
                tdrv->error = 0x80;
                return;
            }// ошибка миспа
            else
                if (RH) {
                chn4er |= 8; //неинверсия кз
                break;
            }
        } else
            kz = 0;

        // уложить прочитанное в ящик

        for (i = 1, j = 1; i < 9; i++) {
            if (msk & j) {
                //используется
                if (obr & j || chn4er & 5) // обрыв или его неинверсия
                    erdn = 1;
                else
                    erdn = 0;
                if (kz & j || chn4er & 0xA) // KZ или его неинверсия
                    erdn |= 2;

                if (erdn || chn4er & 0x10)
                    ercn = 8;
                else
                    if (stat1 & 0x2)
                    if (chn4 & j)
                        tdrv->data[k] = 1;
                    else
                        tdrv->data[k] = 0;
                ++k;
                tdrv->data[k] = erdn | chn4er;
                ++k;
                j <<= 1;

            } else
                k += 2;
        }

        tdrv->error |= ercn;
        break;

    }
}



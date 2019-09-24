#include "../dspadef.h"
#include "../misparw.h"
#include "sbkfp7.h"

#define inipar ((sbk_inipar *)(drv->inimod))
#define sbkDate ((sbk_data *)(drv->data))

static int UpRead = 0; //переменная отсчета цикла

//===========================================================
//  Инициализация СБК
//===========================================================

void sbkfp7_ini(table_drv *drv) {
    drv->error = 0;
}

//===========================================================
//  Запись данных СБК
//===========================================================

void sbkfp7_dw(table_drv *drv) {
    ssbool temp; //буфферная переменная
    unsigned char ti; //переменная сохраняющая в себе значение порта 

    // состояние шкафа
    if (UpRead == 0) {
        ti = ReadPort(0x108); //читаем из  порта 0x108 
        ti &= 0xf9;
        ti |= 0xfd;
        WritePort(0x108, ti); //записываем измененное значение в порт 0x108
    }

    if (UpRead == 1) {
        temp.b = ReadPort(0x110);
        sbkDate->SbkSIGN[0].b = (temp.b >> 0) & 1; //расшифровываем состояние сети 1  
        sbkDate->SbkSIGN[1].b = (temp.b >> 1) & 1; //расшифровываем состояние сети 2
        sbkDate->SbkSIGN[2].b = (temp.b >> 3) & 1; //расшифровываем состояние дверей 
        sbkDate->SbkSIGN[3].b = (temp.b >> 4) & 1; //расшифровываем состояние температуры < 43 
        sbkDate->SbkSIGN[4].b = (temp.b >> 5) & 1; //расшифровываем состояние температуры > 53
    }

    // состояние БП
    if (UpRead == 2) {
        ti = ReadPort(0x108);
        ti &= 0xf9;
        ti |= 0xfb;
        WritePort(0x108, ti);
    }

    if (UpRead == 3) {
        temp.b = ReadPort(0x110);
        sbkDate->SbkSIGN[5].b = (temp.b >> 0) & 1; //расшифровываем состояние источника MP15-3.1 - 1
        sbkDate->SbkSIGN[6].b = (temp.b >> 1) & 1; //расшифровываем состояние источника MP15-3.1 - 2
        sbkDate->SbkSIGN[7].b = (temp.b >> 2) & 1; //расшифровываем состояние источника MP15-3 
        sbkDate->SbkSIGN[8].b = (temp.b >> 3) & 1; //расшифровываем состояние источника MP24-2 
    }

    temp.b = ReadPort(0x114);
    sbkDate->SbkSIGN[9].b = (temp.b >> 0) & 1; //расшифровываем состояние источника PB5/24 (1)
    sbkDate->SbkSIGN[10].b = (temp.b >> 1) & 1; //расшифровываем состояние источника PB5/24 (1)
    sbkDate->SbkSIGN[11].b = (temp.b >> 6) & 1; //расшифровываем состояние источника PB5/24 (2)
    sbkDate->SbkSIGN[12].b = (temp.b >> 7) & 1; //расшифровываем состояние источника PB5/24 (2)

    //работа по циклу из 4 значений
    if (UpRead != 3)
        UpRead++;
    else
        UpRead = 0;
}

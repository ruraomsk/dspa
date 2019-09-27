#include "../dspadef.h"
#include "../misparw.h"
#include "vds32r.h"

#define inipar ((vds32r_inipar *)(tdrv->inimod))
#define vdsDate ((vds32r_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))
#define AdrType 0x04 // тип модуля
#define AdrRQ 0x5 // регистр запроса обслуживания
//Регистры состояния контактов датчиков
#define AdrSostContact0 0x10 // каналы 1-8
#define AdrSostContact1 0x11 // каналы 9-16
#define AdrSostContact2 0x40 // каналы 17-24
#define AdrSostContact3 0x41 // каналы 25-32
//Регистры настройки времени антидребезга
#define AdrAntiTrembl0 0x20 // каналы 1-16
#define AdrAntiTrembl1 0x23 // каналы 17-32
//Регистры маски каналов
#define AdrChanlsMask0 0x21 // каналы 1-16
#define AdrChanlsMask1 0x24 // каналы 17-32
//Регистрыы статуса
#define AdrStatus0 0x22 // каналы 1-16
#define AdrStatus1 0x25 // каналы 17-32

//===========================================================
//  Инициализация модуля ВДС
//===========================================================

void vds32r_ini(table_drv *tdrv) {
    unsigned char RH = 0, RL; //переменные для хранения возвращаемых значений
    int ADR_MISPA = 0x118; //переменная хранящаая адрес миспы
    tdrv->error = SPAPS_OK;
    vdsDate->Diagn = SPAPS_OK; //считаем что модуль исправен

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    // проверка типа модуля
    RH |= ReadBx3w(AdrType, &RL);
    if (RL != inipar->type) { //проверяем верный ли модуль установлен на данном месте
        vdsDate->Diagn = WRONG_DEV;
        tdrv->error = BUSY_BOX; //запоминаем значение - ошибка типа модуля
        return; //прекращаем работу до исправления ошибки
    }

    // настройка модуля ВДС
    RH |= WriteBox(AdrAntiTrembl0, inipar->tadr116); // каналы 1-16   0x20
    RH |= WriteBox(AdrChanlsMask0, 0x0); // каналы 1-16   0x21
    RH |= WriteBox(AdrAntiTrembl1, inipar->tadr1732); // каналы 17-32  0x23
    RH |= WriteBox(AdrChanlsMask1, 0x0); // каналы 1-16   0x24
    // проверям статус после найстройки
    RH |= ReadBx3w(AdrStatus0, &RL);
    RH |= ReadBx3w(AdrStatus1, &RL);
    RH |= ReadBx3w(AdrRQ, &RL);

    if (RH == BUSY_BOX) { //проверяем наличие ошибки
        vdsDate->Diagn = SOST_ERR;
        tdrv->error = SOST_ERR;
        return;
    }
}

//===========================================================
//  Прием данных из модуля ВДС
//===========================================================

void vds32r_rd(table_drv *tdrv) {
    //переменные для хранения возвращаемых значений
    vds32r_str vdsValue;
    unsigned char RH = 0, SErr = 0, i, j, z;
    int k = 0, ADR_MISPA = 0x118;

    //если были ошибки их нужно оставить
    for (k = 0; k < 32; k++) {
        vdsDate->SIGN[k].error = vdsDate->PastValue[k].error;
    }

    //если есть ошибка ожидаем пока она не решится
    if (tdrv->error)
        return;

    //ошибок в модуле нет можно работать
    vdsDate->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK;

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    //если это начало работы, считаем что все каналы не исправны
    for (k = 0; k < 32; k++)
        vdsDate->SIGN[k].error = 0xff;

    //взбадриваем регистр запроса обслуживания
    ReadBox(AdrRQ, &RH);
    RH = 0;
    //читаем состоняние групп каналов
    RH |= ReadBx3w(AdrSostContact0, &vdsValue.sost[0]);
    RH |= ReadBx3w(AdrSostContact1, &vdsValue.sost[1]);
    RH |= ReadBx3w(AdrSostContact2, &vdsValue.sost[2]);
    RH |= ReadBx3w(AdrSostContact3, &vdsValue.sost[3]);
    //проверяем наличие доступа к данным
    if (RH == BUSY_BOX) {
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }
    //если стоит признак инверсии данных, их нужно перевернуть
    if (!inipar->inv) {
        for (k = 0; k < 4; k++)
            vdsValue.sost[k] = ~vdsValue.sost[k];
    }

    //цикл расшифровки данных полученных из ВДС
    for (i = 0, k = 0; i < 4; i++) {
        j = 1;
        for (z = 0; z < 8; z++) {
            SErr = 0;
            vdsDate->SIGN[k].error = vdsDate->PastValue[k].error = SErr;
            if (vdsValue.sost[i] & j) {
                vdsDate->SIGN[k].b = vdsDate->PastValue[k].b = 1;
            } else {
                vdsDate->SIGN[k].b = vdsDate->PastValue[k].b = 0;
            }
            j <<= 1;
            k++;
        }
    }
}
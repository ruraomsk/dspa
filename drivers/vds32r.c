#include "../dspadef.h"
#include "../misparw.h"
#include "vds32r.h"

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

void vds32r_ini(table_drv *tdrv) {
    unsigned char RQ, RH = 0, RL;
    int ADR_MISPA = 0x118;
    tdrv->error = SPAPS_OK;
    vdsDate->Diagn = SPAPS_OK;

    RQ = (unsigned char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    //    проверка типа модуля
    RH |= ReadBx3w(AdrType, &RL);
    if (RL != inipar->type) {
        vdsDate->Diagn = WRONG_DEV;
        tdrv->error = BUSY_BOX;
        return;
    } //ошибка типа модуля
    RH |= WriteBox(AdrAntiTrembl0, inipar->tadr116); // каналы 1-16   0x20
    RH |= WriteBox(AdrChanlsMask0, 0x0); // каналы 1-16   0x21
    RH |= WriteBox(AdrAntiTrembl1, inipar->tadr1732); // каналы 17-32  0x23
    RH |= WriteBox(AdrChanlsMask1, 0x0); // каналы 1-16   0x21
    RH |= ReadBx3w(AdrStatus0, &RL);
    RH |= ReadBx3w(AdrStatus1, &RL);
    RH |= ReadBx3w(AdrRQ, &RL);
    if (RH == BUSY_BOX) { // нет устройства
        vdsDate->Diagn = SOST_ERR;
        tdrv->error = SOST_ERR;
        return;
    }
}

void vds32r_rd(table_drv *tdrv) {
    vds32r_str vdsValue;
    unsigned char RH = 0, SErr = 0, RQ = 0, i, j, z;
    int k = 0, ADR_MISPA = 0x118;

    for (k = 0; k < 32; k++) {
        vdsDate->SIGN[k].error = vdsDate->PastValue[k].error;
        vdsDate->SIGN[k].error = vdsDate->PastValue[k].error;
    }

    if (tdrv->error)
        return; // пока повременить

    vdsDate->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK;

    CLEAR_MEM
    WritePort(ADR_MISPA, (unsigned char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    for (k = 0; k < 32; k++)
        vdsDate->SIGN[k].error = 0xff;

    RH = 0;
    ReadBox(AdrRQ, &RH);
    RH = 0;
    RH |= ReadBx3w(AdrSostContact0, &vdsValue.sost[0]);
    RH |= ReadBx3w(AdrSostContact1, &vdsValue.sost[1]);
    RH |= ReadBx3w(AdrSostContact2, &vdsValue.sost[2]);
    RH |= ReadBx3w(AdrSostContact3, &vdsValue.sost[3]);
    if (RH == BUSY_BOX) { // нет устройства
        vdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }
    if (!inipar->inv) {
        for (k = 0; k < 4; k++)
            vdsValue.sost[k] = ~vdsValue.sost[k];
    }
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
    // }
}
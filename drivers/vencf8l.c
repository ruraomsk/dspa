#include "../misparw.h"
#include "../dspadef.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "vencf8l.h"
#include "linux/printk.h"

#define inipar ((vencf8_inipar *)(tdrv->inimod))
#define devdata ((vencf8_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))


enc_value_t TempEnc[3];


inline void ConnMod(unsigned char addr) {
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, addr);
}

unsigned char ChoseDev(int num) {
    switch (num) {
        case 0: return 0x02; // 0x01;
        case 1: return 0x04; // 0x02;
        case 2: return 0x08; // 0x04;
        case 3: return 0x10; // 0x08;
        case 4: return 0x20; // 0x10;
        case 5: return 0x40; // 0x20;
        case 6: return 0x80; // 0x40;
        case 7: return 0x01; // 0x80;
        default: return 0;
    }
}



unsigned char vds32init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char temp = 0, RH = 0, Err = 0;
    ConnMod(AdrMod);
    if (ERR_MEM) {
        return BUSY_BOX;
    }

    RH |= WriteSinglBox(6, 0);
    RH |= WriteSinglBox(AdrType, 0);
    //    проверка типа модуля
    // RH |= ReadBx3w(AdrType, &temp);
    // if (temp != TypeMod) {
    //     return 0x80;
//    } //ошибка типа модуля
    RH |= WriteBox(AdrVdsAntiTrembl0, 0xff);  //  антидребезг каналы 1-16   0x20  AdrAntiTrembl0
    RH |= WriteBox(AdrVdsChanlsMask0, 0x0);   // маска каналы 1-16   0x21          AdrChanlsMask0
    RH |= WriteBox(AdrVdsAntiTrembl1, 0xff);  // антидребезг каналы 17-32  0x23   AdrAntiTrembl1
    RH |= WriteBox(AdrVdsChanlsMask1, 0x0);   // маска каналы 1-16   0x21          AdrChanlsMask1
    RH |= ReadBx3w(AdrVdsStatus0, &temp); // статус0                   AdrStatus0
    RH |= ReadBx3w(AdrVdsStatus1, &temp); // статус1                   AdrStatus1
    RH |= ReadBx3w(AdrRQ, &temp);  // RQ                        AdrRQ
    if (RH)
        Err = RH;
    return Err;
}

unsigned char fds16init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char RH = 0, temp = 0, Err = 0, isp = 0;
    ConnMod(AdrMod);
    if (ERR_MEM) {
        return BUSY_BOX;
    }
    // RH |= ReadBx3w(AdrType, &temp);
    // if (temp != TypeMod)
    //     Err = 0x80;
    RH |= WriteBox(AdrFdsOut18, 0);
    RH |= WriteBox(AdrFdsOut916, 0);
    RH |= ReadBx3w(AdrFdsISP18, &temp);
    isp |= temp;
    RH |= ReadBx3w(AdrFdsISP916, &temp);
    isp |= temp;
    if (RH || isp)
        Err = RH;
    return Err;
}

//===========================================================
//  Инициализация модуля VENCF8
//===========================================================

void vencf8_ini(table_drv *tdrv) {
    unsigned char RH = 0;
    devdata->numE = 0;
    tdrv->error = 0; // clear DRV error
    SetBoxLen(inipar->BoxLen);
    RH |= vds32init(tdrv->address, inipar->typeVds); // Вдс на 1 месте
    RH |= fds16init(inipar->AdrFds, inipar->typeFds); // Фдс на 3 месте
    if (RH)
        tdrv->error = RH;
}

//===========================================================
// Чтение модуля VENCF8
//===========================================================

void vencf8_dr(table_drv *tdrv) {
    unsigned char RH = 0, j, tempEnc;
    sslong ReciveEnc = {0,0};
    int k1, k2; // для считывания координат
    sschar er = {0, 0};

    SetBoxLen(inipar->BoxLen);

    // читаем данные с ВДС
    ConnMod(tdrv->address);
    if (ERR_MEM) {
        tdrv->error = BUSY_BOX;
        return;
    }
    // считываем пока не считаем 3 раза одинаковое значение
    while (1) {
        ReadBox(AdrRQ, &RH);
        if (RH & 0x01) {
            for (k1 = 0; k1 < 2; k1++) {
                for (k2 = 0; k2 < 3; k2++) {
                    er.error = ReadBx3w(AdrSostContact0 + k1, &er.c);
                    TempEnc[k2].c[k1] = ~er.c;
                    devdata->venc[k1].error |= er.error;
                }

            }
        }
        if (RH & 0x10) {
            for (k1 = 2; k1 < 4; k1++) {
                for (k2 = 0; k2 < 3; k2++) {
                    er.error = ReadBx3w(AdrSostContact2 + (k1 - 2), &er.c);
                    TempEnc[k2].c[k1] = ~er.c;
                    devdata->venc[k1].error |= er.error;
                }
            }
            if (er.error == BUSY_BOX) {
                tdrv->error = er.error;
                return;
            }
        }
        if ((TempEnc[0].l == TempEnc[1].l && TempEnc[0].l == TempEnc[2].l)) {
            break;
        }
    }

    // преобразуем и декодируем
    ReciveEnc.l = (TempEnc[0].l & TempEnc[1].l) | (TempEnc[1].l & TempEnc[2].l) | (TempEnc[0].l & TempEnc[2].l);
    devdata->gray[devdata->numE].l = (unsigned long) ReciveEnc.l & 0x00fffffful;
    // printk("do dekoda = %d", devdata->gray[j].l);
    devdata->gray[devdata->numE].error = 0;
    devdata->venc[devdata->numE].l = decodegray(devdata->gray[devdata->numE].l);
    // printk("posle dekod = %d", devdata->venc[j].l);

    tempEnc = ChoseDev(devdata->numE); // для выставления BUS
//    printk("i = %d, fds = %hhx",devdata->numE,tempEnc);
    // ставим новый BUS
    ConnMod(inipar->AdrFds);
    if (ERR_MEM) {
        tdrv->error = BUSY_BOX;
        return;
    }
    RH = WriteBox(AdrFdsOut18, tempEnc); 
    if (RH) {
        tdrv->error = RH;
        return;
    }

    if (devdata->numE < 7)
        devdata->numE++;
    else
        devdata->numE = 0;


}
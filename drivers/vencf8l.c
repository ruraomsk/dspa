#include "../misparw.h"
#include "../dspadef.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "vencf8l.h"

#define inipar ((vencf8_inipar *)(tdrv->inimod))
#define devdata ((vencf8_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))

enc_value_t TempEnc[3];

// Соединение с модулем

inline void ConnMod(unsigned char addr) {
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, addr);
}

// Выбор для выставления BUS на ФДС
// Сдвиг необходим для того, чтобы нормально уложить данные 
// считанные на ВДС, выставленные на ФДС в прошлом цикле.
// В итоге все 8 координат считываются за 8 циклов (160мс) 

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


// Инициализация модуля ВДС

unsigned char vds32init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char temp = 0, RH = 0, Err = 0;
    ConnMod(AdrMod);
    if (ERR_MEM) {
        return BUSY_BOX;
    }
    RH |= WriteSinglBox(6, 0);
    RH |= WriteSinglBox(AdrType, 0);
    RH |= WriteBox(AdrVdsAntiTrembl0, 0xff); // антидребезг каналы 1-16  
    RH |= WriteBox(AdrVdsChanlsMask0, 0x0); // маска каналы 1-16         
    RH |= WriteBox(AdrVdsAntiTrembl1, 0xff); // антидребезг каналы 17-32 
    RH |= WriteBox(AdrVdsChanlsMask1, 0x0); // маска каналы 1-16         
    RH |= ReadBx3w(AdrVdsStatus0, &temp); // статус0                     
    RH |= ReadBx3w(AdrVdsStatus1, &temp); // статус1                     
    RH |= ReadBx3w(AdrRQ, &temp); // регистр запроса обслуживания     
    if (RH)
        Err = RH;
    return Err;
}

// Инициализация модуля ФДС

unsigned char fds16init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char RH = 0, temp = 0, Err = 0, isp = 0;
    ConnMod(AdrMod);
    if (ERR_MEM) {
        return BUSY_BOX;
    }
    RH |= WriteBox(AdrFdsOut18, 0); // регистр вывода сигналов каналов 1-8  
    RH |= WriteBox(AdrFdsOut916, 0); // регистр вывода сигналов каналов 9-16  
    RH |= ReadBx3w(AdrFdsISP18, &temp); // регистр исправности каналов 1-8  
    isp |= temp;
    RH |= ReadBx3w(AdrFdsISP916, &temp); // регистр исправности каналов 9-16  
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
    devdata->DiagnFDS = 0;
    devdata->DiagnVDS = 0;
    tdrv->error = 0;
    SetBoxLen(inipar->BoxLen);
    RH = vds32init(tdrv->address, inipar->typeVds); // инициализация модуля Вдс на 1 месте
    devdata->DiagnVDS = RH;
    RH = fds16init(inipar->AdrFds, inipar->typeFds); // инициализация модуля Фдс на 3 месте
    devdata->DiagnFDS = RH;
    if (devdata->DiagnFDS || devdata->DiagnVDS) {
        tdrv->error = devdata->DiagnFDS | devdata->DiagnVDS;
        return;
    }
}

//===========================================================
// Чтение модуля VENCF8
//===========================================================

void vencf8_dr(table_drv *tdrv) {
    unsigned char RH = 0, BusEnc;
    sslong ReciveEnc = {0, 0};
    int k1, k2, PermCykl = 0; // k1,k2 - для считывания координат, PermCykl = счетчик по кол-ву циклов считывания 
    sschar er = {0, 0};
    devdata->DiagnFDS = 0;
    devdata->DiagnVDS = 0;
    ConnMod(tdrv->address); // подключаемся к ВДС на 1 месте
    if (ERR_MEM) {
        devdata->DiagnVDS = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    // считываем пока не считаем 3 раза одинаковое значение
    // всего читаем 4 байта
    while (1) {
        ReadBox(AdrRQ, &RH);
        for (k1 = 0; k1 < 2; k1++) { // при k1=0 считываем с 0х10(каналы 1-8 ВДС), при k1=1 с 0х11(каналы 9-16 ВДС)
            for (k2 = 0; k2 < 3; k2++) { // читаем по 3 раза в разные переменные массива
                er.error = ReadBx3w(AdrSostContact0 + k1, &er.c);
                TempEnc[k2].c[k1] = ~er.c;
                devdata->venc[k1].error |= er.error;
            }

        }
        for (k1 = 2; k1 < 4; k1++) { // при k1=2 считываем с 0х40(каналы 17-24 ВДС), при k2=3 с 0х41(каналы 25-32 ВДС)     (0х40 0х41 для вдс с новой прошивкой)
            for (k2 = 0; k2 < 3; k2++) { // читаем по 3 раза в разные переменные массива
                er.error = ReadBx3w(AdrSostContact2 + (k1 - 2), &er.c);
                TempEnc[k2].c[k1] = ~er.c;
                devdata->venc[k1].error |= er.error;
            }
        }
        //При ошибке во время чтения завершаем работу
        if (er.error == BUSY_BOX) {
            devdata->DiagnVDS = BUSY_BOX;
            tdrv->error = er.error;
            return;
        }
        //Если считанные значения равны, выходим
        if ((TempEnc[0].l == TempEnc[1].l && TempEnc[0].l == TempEnc[2].l)) {
            break;
        }
        if (PermCykl < 5)
            PermCykl++; // пытаемся считать 5 раз
        else {
            PermCykl = 13; // если не получилось считать, записываем 13
            break;
        }
    }
    if (PermCykl != 13) { // в обычном состоянии расчитываем координату, если не получилось считать - оставляем старое значение
        // преобразуем и декодируем
        ReciveEnc.l = (TempEnc[0].l & TempEnc[1].l) | (TempEnc[1].l & TempEnc[2].l) | (TempEnc[0].l & TempEnc[2].l); // выбор 2 из 3
        devdata->gray[devdata->numE].l = (unsigned long) ReciveEnc.l & 0x00fffffful; // убираем старший байт
        devdata->gray[devdata->numE].error = 0;
        devdata->venc[devdata->numE].l = decodegray(devdata->gray[devdata->numE].l); // декодируем
    }

    BusEnc = ChoseDev(devdata->numE); // выбираем какой BUS выставить

    // ставим новый BUS
    ConnMod(inipar->AdrFds); // подключаемся к ФДС на 3 месте
    if (ERR_MEM) {
        devdata->DiagnFDS = BUSY_BOX;
        tdrv->error = BUSY_BOX;
        return;
    }

    RH = WriteBox(AdrFdsOut18, BusEnc); // записывает нужный BUS
    if (RH) {
        devdata->DiagnFDS = RH;
        tdrv->error = RH;
        return;
    }

    // работаем по циклу из 8
    if (devdata->numE < 7)
        devdata->numE++;
    else
        devdata->numE = 0;
}
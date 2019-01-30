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
        case 0: return 0x01; // 0x01;
        case 1: return 0x02; // 0x02;
        case 2: return 0x04; // 0x04;
        case 3: return 0x08; // 0x08;
        case 4: return 0x10; // 0x10;
        case 5: return 0x20; // 0x20;
        case 6: return 0x40; // 0x40;
        case 7: return 0x80; // 0x80;

            // case 0: return 0x02; // 0x01;
            // case 1: return 0x04; // 0x02;
            // case 2: return 0x08; // 0x04;
            // case 3: return 0x10; // 0x08;
            // case 4: return 0x20; // 0x10;
            // case 5: return 0x40; // 0x20;
            // case 6: return 0x80; // 0x40;
            // case 7: return 0x01; // 0x80;
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
    //    проверка типа модуля
    // RH |= ReadBx3w(AdrType, &temp);
    // if (temp != TypeMod) {
    //     return 0x80;
    // } //ошибка типа модуля
    RH |= WriteBox(AdrVdsAntiTrembl0, 0x0); // антидребезг каналы 1-16   0x20   AdrAntiTrembl0
    RH |= WriteBox(AdrVdsChanlsMask0, 0x0); // маска каналы 1-16         0x21   AdrChanlsMask0
    RH |= WriteBox(AdrVdsAntiTrembl1, 0x0); // антидребезг каналы 17-32  0x23   AdrAntiTrembl1
    RH |= WriteBox(AdrVdsChanlsMask1, 0x0); // маска каналы 1-16         0x21   AdrChanlsMask1
    RH |= ReadBx3w(AdrVdsStatus0, &temp); // статус0                          AdrStatus0
    RH |= ReadBx3w(AdrVdsStatus1, &temp); // статус1                          AdrStatus1
    RH |= ReadBx3w(AdrRQ, &temp); // регистр запроса обслуживания     AdrRQ
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
    // RH |= ReadBx3w(AdrType, &temp);
    // if (temp != TypeMod)
    //     Err = 0x80;
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
    tdrv->error = 0; // clear DRV error
    SetBoxLen(inipar->BoxLen);
    RH |= vds32init(tdrv->address, inipar->typeVds); // инит Вдс на 1 месте
    RH |= fds16init(inipar->AdrFds, inipar->typeFds); // инит Фдс на 3 месте
    if (RH)
        tdrv->error = RH;
}

//===========================================================
// Чтение модуля VENCF8
//===========================================================


// Работаем без выставления Latch 

// Если нужно добавить выставление 
// RH = WriteBox(AdrFdsOut916, 1);  
// if (RH) {
//     tdrv->error = RH;
//     return;
// }

void vencf8_dr(table_drv *tdrv) {
    unsigned char RH = 0, BusEnc;
    sslong ReciveEnc = {0, 0};
    int vr, vw, vk = 0, vi = 0, k1, k2, PermCykl = 0; // k1,k2 - для считывания координат, PermCykl = счетчик по кол-ву циклов считывания 
    sschar er = {0, 0};

    SetBoxLen(inipar->BoxLen);

    // if(devdata->numE % 2 != 0)
    //     continue;

    devdata->numE = 0;
    vi = 0;
    vk = 0;
    vr = 1;
    vw = 0;
    // for (vi = 0; vi < 8; vi++) 
    {
        vk=0;
        // printf("i - %d k - %d r - %d w - %d",vi,vk,vr,vw);
        while (vk < 10) {
            if ((vw == 1) && (vk == 8)) {
                ConnMod(tdrv->address); // подключаемся к ВДС на 1 месте
                if (ERR_MEM) {
                    tdrv->error = BUSY_BOX;
                    return;
                }

                // считываем пока не считаем 3 раза одинаковое значение
                // всего читаем 4 байта
                while (PermCykl < 2) {
                // while (1) {                    
                    ReadBox(AdrRQ, &RH);
                    if (RH & 0x01) { //изменения состояния данных в каналах 1-16
                        for (k1 = 0; k1 < 2; k1++) { // при k1=0 считываем с 0х10(каналы 1-8 ВДС), при k1=1 с 0х11(каналы 9-16 ВДС)
                            for (k2 = 0; k2 < 3; k2++) { // читаем по 3 раза в разные переменные массива
                                er.error = ReadBx3w(AdrSostContact0 + k1, &er.c);
                                TempEnc[k2].c[k1] = ~er.c;
                                devdata->venc[k1].error |= er.error;
                            }

                        }
                    }
                    if (RH & 0x10) { //изменения состояния данных в каналах 17-32
                        for (k1 = 2; k1 < 4; k1++) { // при k1=2 считываем с 0х40(каналы 17-24 ВДС), при k2=3 с 0х41(каналы 25-32 ВДС)     (0х40 0х41 для вдс с новой прошивкой)
                            for (k2 = 0; k2 < 3; k2++) { // читаем по 3 раза в разные переменные массива
                                er.error = ReadBx3w(AdrSostContact2 + (k1 - 2), &er.c);
                                TempEnc[k2].c[k1] = ~er.c;
                                devdata->venc[k1].error |= er.error;
                            }
                        }
                        if (er.error == BUSY_BOX) { // При ошибке во время чтения вылетаем
                            tdrv->error = er.error;
                            return;
                        }
                    }
                    if ((TempEnc[0].l == TempEnc[1].l && TempEnc[0].l == TempEnc[2].l)) { // Если считанные значения равны, выходим
                        break;
                    }
                    PermCykl++; // пытаемся считать 3 раза
                }
                if (PermCykl != 2) { // в обычном состоянии расчитываем координату, если не получилось считать - оставляем старое значение
                    // преобразуем и декодируем
                    ReciveEnc.l = (TempEnc[0].l & TempEnc[1].l) | (TempEnc[1].l & TempEnc[2].l) | (TempEnc[0].l & TempEnc[2].l); // выбор 2 из 3
                    devdata->gray[devdata->numE].l = (unsigned long) ReciveEnc.l & 0x00fffffful; // убираем старший байт
                    devdata->gray[devdata->numE].error = 0;
                    devdata->venc[devdata->numE].l = decodegray(devdata->gray[devdata->numE].l); // декодируем
                }
                printk("num gray - %d", devdata->numE);
                devdata->numE++;
                printk("vk - %d", vk);

                vw=0;
                vr=1;
                // ConnMod(inipar->AdrFds); // подключаемся к ФДС на 3 месте
                // if (ERR_MEM) {
                //     tdrv->error = BUSY_BOX;
                //     return;
                // }

                // RH = WriteBox(AdrFdsOut916, 0); // записывает нужный latch
                // if (RH) {
                //     tdrv->error = RH;
                //     return;
                // }


            }
            if ((vr==1) && (vk==0)) {

                BusEnc = ChoseDev(devdata->numE); // выбираем какой BUS выставить
                printk("i - %d", vi);
                printk("num - %d", devdata->numE);
                printk("bus - %hhx", BusEnc);
                printk("vk - %d", vk);
                // ставим новый BUS

                ConnMod(inipar->AdrFds); // подключаемся к ФДС на 3 месте
                if (ERR_MEM) {
                    tdrv->error = BUSY_BOX;
                    return;
                }

                // RH = WriteBox(AdrFdsOut18, BusEnc); // записывает нужный BUS
                RH = WriteBox(AdrFdsOut18, 0x80); // записывает нужный BUS
                if (RH) {
                    tdrv->error = RH;
                    return;
                }


                // RH = WriteBox(AdrFdsOut916, 1); // записывает нужный latch
                // if (RH) {
                //     tdrv->error = RH;
                //     return;
                // }

                vr = 0;
                vw = 1;
            }
            vk++;
        }
    }


    // ConnMod(inipar->AdrFds); // подключаемся к ФДС на 3 месте
    // if (ERR_MEM) {
    //     tdrv->error = BUSY_BOX;
    //     return;
    // }

    // RH = WriteBox(AdrFdsOut18, 0); // записывает нужный BUS
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }
    // RH = WriteBox(AdrFdsOut916, 0); // записывает нужный BUS
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }
    printk("------------------");
    // // работаем по циклу от 0 до 7
    // if (devdata->numE < 7)
    //     devdata->numE++;
    // else
    //     devdata->numE = 0;


}
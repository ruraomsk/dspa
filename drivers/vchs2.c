

#include "../dspadef.h"
#include "../misparw.h"
#include "vchs2.h"

#include "linux/printk.h"

#define inipar ((vchs_inipar *)(tdrv->inimod))
#define VchDate ((vchs_data *)(tdrv->data))

#define AdrType 0x4 // тип модуля   0xC6
#define AdrRQ 0x5   // inv 0xFA регистр запроса обслуживания
#define AdrSV 0x6
#define AdrSVE 0x7
#define AdrSTAT 0xF       // inv 0xF0 – состояние модуля
#define IntrvCh1Low 0x10  // Регистр результата счета младший байт 1 канала
#define IntrvCh1High 0x11 // Регистр результата счета старший байт 1 канала
#define IntrvCh2Low 0x30  // Регистр результата счета младший байт 2 канала
#define IntrvCh2High 0x31 // Регистр результата счета старший байт 2 канала
#define RzCh1 0x12        // Регистр контроля диапазона 1 канал
#define RzCh2 0x32        // Регистр контроля диапазона 2 канал
#define RgSost1 0x13      // Регистр состояния канала 1 канал
#define RgSost2 0x33      // Регистр состояния канала 2 канал
#define CurCh1Low 0x15    // Регистр текущего значения младший байт 1 канала
#define CurCh1High 0x16   // Регистр текущего значения старший байт 1 канала
#define CurCh2Low 0x35    // Регистр текущего значения младший байт 2 канала
#define CurCh2High 0x36   // Регистр текущего значения старший байт 2 канала
#define DiCont1 0x1C      // Регистр управления диапазоном 1 канал
#define DiCont2 0x3C      // Регистр управления диапазоном 2 канал
#define CommCh1 0x1D      // Регистр управления коммутатором каналов 1 канал
#define CommCh2 0x3D      // Регистр управления коммутатором каналов 2 канал
#define KodCh1Low 0x1E    // Регистр кода генератора младший байт 1 канала
#define KodCh1High 0x1F   // Регистр кода генератора старший байт 1 канала
#define KodCh2Low 0x3E    // Регистр кода генератора младший байт 2 канала
#define KodCh2High 0x3F   // Регистр кода генератора старший байт 2 канала
#define CountCh1Low 0x19  // Регистр счетчика импульсов младший байт 1 канала
#define CountCh1High 0x1A // Регистр счетчика импульсов старший байт 1 канала
#define CountCh2Low 0x39  // Регистр счетчика импульсов младший байт 2 канала
#define CountCh2High 0x3A // Регистр счетчика импульсов старший байт 2 канала

//extern unsigned int irq_count;
extern float takt;
/*
===========================================================
Типы диагностических сообщений модуля

структура байта достоверности модуля 
 
  Бит         Значение

   0   -   ошибки импульсного канала 1
   1   -   ошибки интервального канала 1
   2   -   ошибки импульсного канала 2
   3   -   ошибки интервального канала 2  
   4   -   ошибка статуса  
   5   -   ошибка конфигурирования
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

===========================================================
 */
//===========================================================
//  Инициализация модуля ВЧС
//===========================================================

void vchs_ini(table_drv *tdrv) {
    unsigned char RQ, RH = 0;
    int ADR_MISPA = 0x118, i;
    if (tdrv->address == 0x01) 
    printk("Init");
    tdrv->error = 0;
    SetBoxLen(inipar->BoxLen);
    RQ = (unsigned char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);

    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    RH = CatchBox();
    if (RH) {
        tdrv->error = RH;
        return;
    }
    // Натсройка диапазонов
    RQ = 0x40 | inipar->chn1d;
    RH |= WriteBox(DiCont1, RQ);
    RQ = 0x40 | inipar->chn2d;
    RH |= WriteBox(DiCont2, RQ);

    // Управление коммутатором каналов
    RH |= WriteBox(CommCh1, 0x1);
    RH |= WriteBox(CommCh2, 0x1);

    // взбодрим интервальный регистр
    RH |= ReadBx3w(IntrvCh1Low, &RQ);
    RH |= ReadBx3w(IntrvCh2Low, &RQ);

    // очистка регистров состояния канала
    RH |= WriteBox(RgSost1, 0xff);
    RH |= WriteBox(RgSost2, 0xff);

    // прямое и инверсное значение 0x1F4, что  соответствует 10 мкс
    RH |= WriteBox(KodCh1Low, 0xf4);
    RH |= WriteBox(KodCh1High, 0x1);
    RH |= WriteBox(KodCh2Low, 0xf4);
    RH |= WriteBox(KodCh2High, 0x1);

    // очистка регистр RQ
    RH |= WriteBox(AdrRQ, 0xff);

    RH |= ReadBx3w(AdrSTAT, &RQ);

    if (RQ != 0) {
        tdrv->error = 0x90;
        return;
    } // ошибка состояния модуля

    // освободить ПЯ

    RH |= FreeBox();
    if (RH) {
        tdrv->error = RH; // ошибка миспа
        return;
    }
}

//===========================================================/
//  Прием данных из модуля ВЧС
//===========================================================

/*
===========================================================
структура байта достоверности модуля 
 
  Бит         Значение

   0   -   ошибки импульсного канала 1
   1   -   ошибки интервального канала 1
   2   -   ошибки импульсного канала 2
   3   -   ошибки интервального канала 2  
   4   -   ошибка статуса  
   5   -   ошибка конфигурирования
   6   -   ошибка типа модуля
   7   -   критическая ошибка или нет доступа к ПЯ

структура байта достоверности канала 
 
  Бит         Значение

   0   - ошибки импульсного канала (Fper)
   1   - ошибки интервального канала (MIN,MAX)
   2   - ошибка входного сигнала (Alw1R,Alw1Т)
   3   – несоответствие кода диапазона частоты канала 
         (выбрано больше одного диапазона или ни одного)
   4   – неинверсия в регистре диапазона канала
   5   – неинверсия в регистре кода коммутатора канала
   6   – неинверсия в регистре значения генератора канала
   7   - критическая ошибка или нет доступа к ПЯ

// Alw1R - входной сигнал рабочего ввода всегда в 1
// Alw1Т - входной сигнал тестового ввода всегда в 1
// F0 – признак отсутствия счета – меньше 2 единиц
// MIN – значение регистра интервального счетчика меньше 50 единиц
// МАХ – значение регистра интервального счетчика больше 50000 единиц
// Fper  - флаг переполнения
// Dup – запрос увеличения частоты диапазона измерения (окончание счета при значении меньше 50 единиц)
// Ddown - запрос уменьшения частоты диапазона измерения (окончание счета при значении больше 50000 единиц)

===========================================================

 */

void vchs_dr(table_drv *tdrv) {
    unsigned char RH = 0, RQ = 0, RQt = 0;
    int ADR_MISPA = 0x118, i;
    unsigned char CountChLow[2] = {0, 0}, CountChHigh[2] = {0, 0}, cerr[2] = {0, 0};
    SetBoxLen(inipar->BoxLen);
    if (tdrv->error == 0x80) {
        if (tdrv->address == 0x01) 
        printk("123");
        vchs_ini(tdrv);
        if (tdrv->error == 0)
            tdrv->error = 0x81;
        return;
    }
    CLEAR_MEM
    WritePort(ADR_MISPA, (unsigned char) (tdrv->address & 0xff));
    if (ERR_MEM) {
        tdrv->error = 0x80;
        if (tdrv->address == 0x01) 
        printk("obrahenie k modylu");
        return;
    }

    RH |= ReadBx3w(AdrRQ, &RQ);

    // читаем статус модуля
    RH |= ReadBx3w(AdrSTAT, &RQ);

    if (RH) {
        tdrv->error = RH;
        if (tdrv->address == 0x01) 
        printk("1");
        return;
    }

    if ((inipar->UsMask & 1) == 0)
        cerr[0] |= 0x3;
    else {
        RQt = RQ & 0x1;
        if (RQt) {
            tdrv->error |= 0x83;
            if (tdrv->address == 0x01) 
            printk("2");
            cerr[0] |= 0x80;
            VchDate->SVCHS[0] = 0;
        }
    }

    if ((inipar->UsMask & 2) == 0)
        cerr[1] |= 0xc0;
    else {
        RQt = RQ & 0x10;
        if (RQt) {
            tdrv->error |= 0x8c;
            if (tdrv->address == 0x01) 
            printk("3");
            cerr[1] |= 0x80;
            VchDate->SVCHS[1] = 0;
        }
    }

    for (i = 0; i < 2; i++) {
        if ((!cerr[0] && (VchDate->perm[0] <= 0)) || (!cerr[1] && (VchDate->perm[1] <= 0))) {
            if (VchDate->perm[i] <= 0) {
                if (!cerr[i]) {
                    ReadBx3w(0x13 + (0x20 * i), &RQ); // Состояния адрес 0x13 и 0x33
                    RQ &= 0x2b;
                    if (RQ & 0x20) {
                        if (tdrv->address == 0x01) 
                        printk("kanal slomalsia %hhx na modyle %hhx", 0x13 + (0x20 * i), tdrv->address);
                        cerr[i] |= 1;
                        // Сброс регистров 3 команды  //?
                        WriteSinglBox(AdrSV, 1);
                        ReadBx3w(0x19 + (0x20 * i), &RQ); // Младший регистр адрес 0x19 и 0x39
                        ReadBx3w(0x1a + (0x20 * i), &RQ); // Старший регистр адрес 0x1a и 0x3a
                        // СБРОС
                        VchDate->takt[i] = 0;
                        VchDate->cykl[i] = 0.01;
                        VchDate->SVCHS[i] = 0;
                        // tdrv->error = 0x80;
                        return;

                        // break;
                        // WriteBox(0x13 + (0x20 * i), 0xff); // Сброс состояния адрес 0x13 и 0x33
                    } else { // ?
                        ReadBx3w(0x19 + (0x20 * i), &CountChLow[i]);
                        ReadBx3w(0x1a + (0x20 * i), &CountChHigh[i]);
                        // Сброс регистров 3 команды
                        WriteSinglBox(AdrSV, 1);
                        ReadBx3w(0x19 + (0x20 * i), &RQ); // Младший регистр адрес 0x19 и 0x39
                        ReadBx3w(0x1a + (0x20 * i), &RQ); // Старший регистр адрес 0x1a и 0x3a
                        // СБРОС
                    }
                    
                    VchDate->tempI[i] = (unsigned int) ((unsigned int) (CountChHigh[i] * 256) + CountChLow[i]);
                    if ((i == 1) && (tdrv->address == 0x01)) 
                        printk("%d      %hhx %hhx - %d", i, CountChHigh[i], CountChLow[i], VchDate->tempI[i]);
                    
                    VchDate->SVCHS[i] = 1;
                } else {
                    if (tdrv->address == 0x01) 
                    printk("ohibka = %hhx", cerr[i]);
                    cerr[i] = 0xff;
                    VchDate->SVCHS[i] = 0;
                }
            }
        } else { // читать-то неча из-за статуса
            if (cerr[i] && (VchDate->perm[i] <= 0)) {
                if (tdrv->address == 0x01) 
                printk("ошибка статуса модуля  канал");
                // Сброс регистров 3 команды
                WriteSinglBox(AdrSV, 1);
                ReadBx3w(0x19 + (0x20 * i), &RQ); // Младший регистр адрес 0x19 и 0x39
                ReadBx3w(0x1a + (0x20 * i), &RQ); // Старший регистр адрес 0x1a и 0x3a
                // СБРОС
                VchDate->takt[i] = 0.0;
                VchDate->cykl[i] = 0.01;
            }
        }
    }
    //  значение полученной частоты хранится в массиве fvch полученное после вычислений, 
    //  присвоение делаем для коректного отображения (если не присваивать то в переменных будет 0)
    VchDate->K01VCHS.f = VchDate->fvch[0];
    VchDate->K02VCHS.f = VchDate->fvch[1];
    // --------
    VchDate->K01VCHS.error = cerr[0];
    VchDate->K02VCHS.error = cerr[1];
    RH |= FreeBox();
    RH |= WriteBox(AdrRQ, 0xff);
    if (RH) {
        tdrv->error = RH; // ошибка миспа
        if (tdrv->address == 0x01) 
        printk("4");
        return;
    }
} // мой общий!

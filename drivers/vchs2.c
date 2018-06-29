

#include "../dspadef.h"
#include "../misparw.h"
#include "vchs2.h"

#include "linux/printk.h"

#define inipar ((vchs_inipar *)(tdrv->inimod))
#define VchDate ((vchs_data *)(tdrv->data))

extern unsigned int irq_count;

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

extern unsigned int irq_count;
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
    int ADR_MISPA = 0x118;

    //  VchDate->Cyklen.c = 10;
     inipar->stDate.pMFast1 =  3; //inipar->stDate.pMFast2 = 3; // указатель текущей позиции массива импульсов за цикл от счетного канала
     inipar->stDate.pMSlow1 =  19; //inipar->stDate.pMSlow2 = 19; // указатель текущей позиции массива накопленных импульсов от счетного канала

    SetBoxLen(inipar->BoxLen);

    RQ = (unsigned char) (tdrv->address & 0xff);
    CLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }
    tdrv->error = 0;

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
    float fslow = 0, ffast = 0;
    unsigned char RH = 0, RQ = 0, RQt = 0, takt = 0.005;    
    int ADR_MISPA = 0x118;
    unsigned long tempI;

    SetBoxLen(inipar->BoxLen);
    if (tdrv->error == 0x80)
        return;

    RQ = (char) (tdrv->address & 0xff);
    СLEAR_MEM
    WritePort(ADR_MISPA, RQ);
    if (ERR_MEM) {
        tdrv->error = 0x80;
        return;
    }

    // читаем статус модуля
    RH |= ReadBx3w(AdrSTAT, &RQ);

    if (RH) {
        tdrv->error = RH;
        return;
    }

    RQt = (RQ >> 1) & 1;
    if (RQt) {
        tdrv->error |= 0x83;
        VchDate->K01VCHS.error = 0x80;
    }
    RQt = (RQ >> 5) & 1;
    if (RQt) {
        tdrv->error |= 0x83;
        VchDate->K02VCHS.error = 0x80;
    }


    if (!VchDate->K01VCHS.error || !VchDate->K02VCHS.error) {
        RH = CatchBox();
        if (RH) {
            tdrv->error = RH;
            return;
        }
        ReadBx3w(AdrSVE, &RQ);
        if (!RQ) {
            RH = CatchBox();
            if (RH) {
                tdrv->error = RH;
                return;
            }
        }

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! takt = gettimeofdate!!! посмотреть
        // первый канал
        if ((inipar->UsMask & 1) && !VchDate->K01VCHS.error) {
            ReadBx3w(RgSost1, &RQ);
            RQ &= 0x2b;

            if (RQ & 0x20) {
                VchDate->K01VCHS.error = 1;
                ReadBx3w(CountCh1Low, &RQ); // ЗАЧЕМ?? (сброс при чтении?)
                WriteBox(RgSost1, 0xff);
            } else {
                ReadBx3w(CountCh1Low, & inipar->stDate.CountChLow[0]);
                ReadBx3w(CountCh1High, & inipar->stDate.CountChHigh[0]);
            }

            if (!VchDate->K01VCHS.error) {
                tempI =  inipar->stDate.CountChHigh[0];
                tempI = tempI * 256 +  inipar->stDate.CountChLow[0];

                if (tempI > 64000) {
                    VchDate->K01VCHS.error = 1;
                    tdrv->error |= 1;
                } else {
                    if ( inipar->stDate.pMFast1 >= 3) {
                         inipar->stDate.pMFast1 = 0;
                        if ( inipar->stDate.pMSlow1 >= 19)
                             inipar->stDate.pMSlow1 = 0;
                        else
                             inipar->stDate.pMSlow1++;

                         inipar->stDate.lSmS1 =  inipar->stDate.lSmS1 -  inipar->stDate.lMSlow1[ inipar->stDate.pMSlow1] +  inipar->stDate.lSmF1;
                         inipar->stDate.fTimS1 =  inipar->stDate.fTimS1 -  inipar->stDate.fMStim1[ inipar->stDate.pMSlow1] +  inipar->stDate.fTimF1;
                         inipar->stDate.fMStim1[ inipar->stDate.pMSlow1] =  inipar->stDate.fTimF1;
                         inipar->stDate.lMSlow1[ inipar->stDate.pMSlow1] =  inipar->stDate.lSmF1;
                    } else
                         inipar->stDate.pMFast1++;

                     inipar->stDate.lSmF1 =  inipar->stDate.lSmF1 -  inipar->stDate.iMFast1[ inipar->stDate.pMFast1] + tempI;
                     inipar->stDate.fTimF1 =  inipar->stDate.fTimF1 -  inipar->stDate.fMFtim1[ inipar->stDate.pMFast1] + takt;
                     inipar->stDate.iMFast1[ inipar->stDate.pMFast1] = tempI;
                     inipar->stDate.fMFtim1[ inipar->stDate.pMFast1] = takt;

                    fslow = VchDate->K01VCHS.f;
                    VchDate->K01VCHS.f = tempI;

                    // незнаю зачем?
                    // if (fabs(fslow - VchDate->K01VCHS.f) > 0.002)
                    // {
                    //   inipar->ChMask = ((inipar->ChMask & 2) + 1);
                    // }
                    ffast =  inipar->stDate.lSmF1;

                    if ( inipar->stDate.fTimF1 < 0.000001)
                        ffast = 0;
                    else
                        ffast = ffast /  inipar->stDate.fTimF1;

                    if (ffast < 20) { //  посчитать частоту  по медленному усреднению
                        fslow = ffast;
                        ffast =  inipar->stDate.lSmS1;
                        if ( inipar->stDate.fTimS1 < 0.00001)
                            ffast = 0;
                        else
                            ffast = ffast /  inipar->stDate.fTimS1;
                        if (ffast > 5)
                            ffast = (fslow + ffast) / 2;
                    } else if (ffast > 1000) { //  посчитать частоту  по мгновенному значению
                        ffast =  inipar->stDate.iMFast1[ inipar->stDate.pMFast1];
                        ffast = ffast / takt;
                    }

                    if (inipar->Gmin1 > 0.1 && ffast < inipar->Gmin1 || ffast > inipar->Gmax1) { //  частота  выходит за допустимый дианазон
                        VchDate->K01VCHS.error = 2;
                    }

                    VchDate->K01VCHS.f = ffast;
                }
            }

        } else {
            VchDate->K01VCHS.error = 0xff;
            VchDate->K01VCHS.f = 0;
        }

        // тут второй

    }//
    else { // читать-то неча из-за статуса
        ReadBx3w(CountCh1Low, &RQ);
        ReadBx3w(CountCh2Low, &RQ);
    }
    RH |= FreeBox();
    RH |= WriteBox(AdrRQ, 0xff);
    if (RH) {
        tdrv->error = RH; // ошибка миспа
        return;
    }
} // мой общий!

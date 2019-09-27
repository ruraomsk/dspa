#include "../dspadef.h"
#include "../misparw.h"
#include "vchs2.h"

#define inipar ((vchs_inipar *)(tdrv->inimod))
#define VchDate ((vchs_data *)(tdrv->data))
#define AdrType 0x4 // тип модуля   0xC6
#define AdrRQ 0x5   // регистр запроса обслуживания
#define AdrSV 0x6
#define AdrSVE 0x7
#define AdrSTAT 0xF       // Регистр состояние модуля
#define IntrvCh1Low 0x10  // Регистр результата счета младший байт 1 канала
#define IntrvCh1High 0x11 // Регистр результата счета старший байт 1 канала
#define IntrvCh2Low 0x30  // Регистр результата счета младший байт 2 канала
#define IntrvCh2High 0x31 // Регистр результата счета старший байт 2 канала
#define RgSost1 0x13      // Регистр состояния канала 1 канал
#define RgSost2 0x33      // Регистр состояния канала 2 канал
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

extern float takt;

//===========================================================
//  Инициализация модуля ВЧС
//===========================================================

void vchs_ini(table_drv *tdrv) {
    unsigned char RQ, RH = 0; //переменные для хранения возвращаемых значений
    int ADR_MISPA = 0x118; //переменная хранящаая адрес миспы
    tdrv->error = SPAPS_OK;
    VchDate->Diagn = SPAPS_OK; //считаем что модуль исправен

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        VchDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    // проверка типа модуля
    ReadBox3(AdrType, &RQ);
    if (RQ != inipar->type) { //проверяем верный ли модуль установлен на данном месте
        tdrv->error = BUSY_BOX;
        VchDate->Diagn = WRONG_DEV; //запоминаем значение - ошибка типа модуля
        return; //прекращаем работу до исправления ошибки
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
    // проверка статуса модуля
    RH |= ReadBx3w(AdrSTAT, &RQ);
    if (RQ != 0) { // ошибка состояния модуля
        VchDate->Diagn = SOST_ERR;
        tdrv->error = SOST_ERR;
        return;
    }
}

//===========================================================
//  Прием данных из модуля ВЧС
//===========================================================

void vchs_dr(table_drv *tdrv) {
    unsigned char RH = 0, RQ = 0, RQt = 0; //переменные для хранения возвращаемых значений
    int ADR_MISPA = 0x118, i; //переменная хранящаая адрес миспы
    unsigned char CountChLow[2] = {0, 0}, CountChHigh[2] = {0, 0}, cerr[2] = {0, 0}; //переменные для хранения считанных значений

    if (tdrv->error == BUSY_BOX) { //проверка не произошла ли ошибка на прошлом цикле
        vchs_ini(tdrv); // если ошибка была, сделать быструю инициализацию
        if (tdrv->error == 0) {
            tdrv->error = 0x81;
        }
        VchDate->Diagn = BUSY_BOX;
        return; //быстро устранить не получилось, ожидаем...
    }

    tdrv->error = 0;
    VchDate->Diagn = 0; //ошибоки с прошлого раза нет все впорядке

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        VchDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    RH |= ReadBx3w(AdrRQ, &RQ);
    RH |= ReadBx3w(AdrSTAT, &RQ); // читаем статус модуля 
    if (RH) {
        VchDate->Diagn = BUSY_BOX;
        tdrv->error = RH;
        return; //прекращаем работу до исправления ошибки
    }

    //достаем ошибку из статуса для 1 канала 
    if ((inipar->UsMask & 1) == 0)
        cerr[0] |= 0x3;
    else {
        RQt = RQ & 0x1;
        if (RQt) {
            VchDate->Diagn = BUSY_BOX;
            tdrv->error |= 0x83;
            cerr[0] |= 0x83;
        }
    }

    //достаем ошибку из статуса для 2 канала 
    if ((inipar->UsMask & 2) == 0)
        cerr[1] |= 0xc0;
    else {
        RQt = RQ & 0x10;
        if (RQt) {
            VchDate->Diagn = BUSY_BOX;
            tdrv->error |= 0x8c;
            cerr[1] |= 0x8c;
        }
    }

    //обработка 2х каналов ВЧС
    for (i = 0; i < 2; i++) {
        if (!cerr[i] && VchDate->perm[i] <= 0) { //проверяем если ли ошибка по каналу и разрешение на чтение получено
            //проверяем переполнение канала
            ReadBx3w(0x13 + (0x20 * i), &RQ); //читаем состояния адрес 0x13 и 0x33
            RQ &= 0x2b;
            if (RQ & 0x20) { //проверка на переполнение канала
                VchDate->SVCHS[i] = 1; // 1 - есть переполнение 0 - нет
                // Сброс регистров   
                ReadBx3w(0x19 + (0x20 * i), &CountChLow[i]);
                ReadBx3w(0x1a + (0x20 * i), &CountChHigh[i]);
                //преобразуем значение по каналу, для дальнейшего решения ситуации  переполнением
                VchDate->tempI[i] = (unsigned int) ((unsigned int) (CountChHigh[i] * 256) + CountChLow[i]);
                //сбрасываем счетчик по текущему каналу
                WriteSinglBox(AdrSV, 1);
                ReadBx3w(0x19 + (0x20 * i), &RQ); // Младший регистр адрес 0x19 и 0x39
                ReadBx3w(0x1a + (0x20 * i), &RQ); // Старший регистр адрес 0x1a и 0x3a
                WriteSinglBox(AdrSV, 0);
                WriteBox(0x13 + (0x20 * i), 0xff);
                continue;
            } else {
                //переполнение не произошло значению можно доверять
                ReadBx3w(0x19 + (0x20 * i), &CountChLow[i]);
                ReadBx3w(0x1a + (0x20 * i), &CountChHigh[i]);
                //сбрасываем счетчик текущего канала для следующего интервала подсчета
                WriteSinglBox(AdrSV, 1);
                ReadBx3w(0x19 + (0x20 * i), &RQ); // Младший регистр адрес 0x19 и 0x39
                ReadBx3w(0x1a + (0x20 * i), &RQ); // Старший регистр адрес 0x1a и 0x3a
                //преобразование значения по каналу
                VchDate->tempI[i] = (unsigned int) ((unsigned int) (CountChHigh[i] * 256) + CountChLow[i]);
                WriteSinglBox(AdrSV, 0);
                VchDate->SVCHS[i] = 0; // все нормально, переполнения не было
            }

        }
    }

    //запись значений 
    VchDate->K01VCHS.error = cerr[0];
    VchDate->K02VCHS.error = cerr[1];

    //если появились ошибки по каналам, ставим частоту 3М
    if (!cerr[0])
        VchDate->K01VCHS.f = VchDate->fvch[0];
    else
        VchDate->K01VCHS.f = 3000000.0;

    if (!cerr[1])
        VchDate->K02VCHS.f = VchDate->fvch[1];
    else
        VchDate->K02VCHS.f = 3000000.0;

    RH |= WriteBox(AdrRQ, 0xff); //сообщаем модулю что бы завершили обслуживание
    if (RH) {
        VchDate->Diagn = BUSY_BOX;
        tdrv->error = RH; // ошибка миспа
        return;
    }
}

#include "../dspadef.h"
#include "../misparw.h"
#include "vas84r.h"

#define inipar  ((vas84r_inipar*)(tdrv->inimod)) 
#define VasData ((vas84r_data*)(tdrv->data))
#define AdrType       0x4   // тип модуля   
#define AdrRQ         0x5   // запрос на обслуживание  
#define AdrSV         0x6   // флаг захвата ПЯ со стороны ФП                 
#define AdrSVE        0x7   // флаг завершения обслуживания ПЯ 
#define AdrSTAT       0xF   // байт состояния модуля
#define AdrData       0x10  // начало таблицы данных                 
#define AdrSOST       0x43  // расширенный байт состояния  

//===========================================================
//  Инициализация модуля VAS84r
//===========================================================

void vas84r_ini(table_drv* tdrv) {
    int ADR_MISPA = 0x118; //переменная хранящаая адрес миспы
    unsigned char temp; //буфферная переменная

    VasData->NumK = 0; //счетчик каналов - работа начинается с 0
    VasData->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK; //считаем что модуль исправен

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (unsigned char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        VasData->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    ReadBox3(AdrType, &temp); //читаем из памяти тип установленного модуля
    if (temp != inipar->type) { //проверяем верный ли модуль установлен на данном месте
        tdrv->error = BUSY_BOX;
        VasData->Diagn = WRONG_DEV; //запоминаем значение - ошибка типа модуля
        return; //прекращаем работу до исправления ошибки
    }


    temp = CatchBox(); //захват ПЯ 
    if (temp) { //проверяем захват ПЯ
        VasData->Diagn = SOST_ERR;
        tdrv->error = temp; //запоминаем значение вернувшееся после попытки захвата ПЯ
        return; //прекращаем работу до исправления ошибки
    }

    temp = FreeBox(); // освободить ПЯ
    if (temp) {
        VasData->Diagn = SOST_ERR;
        tdrv->error = temp; // запоминаем значение вернувшееся после попытки освобождения ПЯ
        return; //прекращаем работу до исправления ошибки
    }

}

void vas84r_rd(table_drv* tdrv) {
    unsigned char RQ, RH, RL; //переменные для хранения возвращаемых значений
    short temp; //буфферная переменная
    int i, ADR_MISPA = 0x118; //переменная хранящаая адрес миспы

    if (tdrv->error) //если есть ошибка ожидаем пока она не решится
        return;

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { // проверка на наличие ошибки
        VasData->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    if (VasData->NumK == 0) //если это начало работы, считаем что все каналы не подключены
        for (i = 0; i < 8; i++)
            VasData->SIGN[i].error = 0xff;


    while (1) {
        RH = CatchBox(); //захват ПЯ 
        if (RH) { //проверяем захват ПЯ
            VasData->Diagn = BUSY_BOX;
            tdrv->error = RH; //запоминаем значение вернувшееся после попытки захвата ПЯ
            break; //выходим из основного цикла работы до исправления ошибки
        }


        RH |= ReadBx3w(AdrSOST, &VasData->widesos.c); //читаем расширеный байт состояния 
        RH |= ReadBx3w(AdrSTAT, &RQ); //читаем байт состояния модуля
        if (RQ & BUSY_BOX) { //проверяем состояние модуля
            VasData->Diagn = SOST_ERR;
            tdrv->error = RQ; //запоминаем значение
            break; //выходим из основного цикла работы до исправления ошибки
        }
        if (RH == BUSY_BOX) { //проверяем наличие модуля на миспе
            VasData->Diagn = BUSY_BOX;
            tdrv->error = RH; //запоминаем значение
            break; //выходим из основного цикла работы до исправления ошибки
        }

        ReadBx3w(AdrRQ, &RQ); //проверяем наличие разрешения на обслуживаение
        if (RQ == 0)
            break;

        RH = 0;
        RH |= ReadBx3w(AdrData + (VasData->NumK * 3), &RL); //читаем старшее значение байтай
        RH |= ReadBx3w(AdrData + (VasData->NumK * 3) + 1, &RQ); //читаем младее значение байта
        if (RH == BUSY_BOX) {
            break;
        }
        temp = ((RL << 8) | RQ); //преобразуем в байт
        VasData->SIGN[VasData->NumK].i = temp; //запоминаем расшифрованное значение для текущего канала
        VasData->SIGN[VasData->NumK].error = 0; //считаем канал исправным
        break;
    }

    WriteBox(AdrRQ, 0); //сообщаем модулю о завершении обслуживания
    WriteBox(AdrSVE, 1);

    RH = FreeBox(); // освободить ПЯ
    if (RH) {
        VasData->Diagn = BUSY_BOX;
        tdrv->error = RH; // запоминаем значение вернувшееся после попытки освобождения ПЯ
    }

    if (VasData->NumK == 7) //цикл по 8 каналам модуля VAS
        VasData->NumK = 0;
    else
        VasData->NumK++;
}


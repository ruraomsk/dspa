#include "../dspadef.h"
#include "../misparw.h"
#include "fds16r.h"

#define inipar  ((fds16r_inipar*)(tdrv->inimod)) 
#define fdsDate ((fds16r_data*)(tdrv->data)) 
#define AdrType          0x04  // тип модуля 
#define AdrOut18         0x01  // регистр вывода сигналов каналов 1-8  
#define AdrOut916        0x02  // регистр вывода сигналов каналов 9-16  
#define AdrISP18         0x03  // регистр исправности каналов 1-8  
#define AdrISP916        0x0F  // регистр исправности каналов 9-16  
#define AdrSost14        0x05 // регистр состояния каналов 1-4   
#define AdrSost58        0x06 // регистр состояния каналов 5-8   
#define AdrSost912       0x07 // регистр состояния каналов 9-12   
#define AdrSost1316      0x08 // регистр состояния каналов 13-16   

//===========================================================
//  Инициализация модуля ФДС
//===========================================================

void fds16r_ini(table_drv* tdrv) {
    int ADR_MISPA = 0x118; //переменная хранящаая адрес миспы
    unsigned char temp; //буфферная переменная
    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { //проверка на наличие ошибки
        fdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    ReadBox3(AdrType, &temp); //читаем из памяти тип установленного модуля
    if (temp != inipar->type) { //проверяем верный ли модуль установлен на данном месте
        tdrv->error = BUSY_BOX;
        fdsDate->Diagn = WRONG_DEV; //запоминаем значение - ошибка типа модуля
        return; //прекращаем работу до исправления ошибки
    }

    fdsDate->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK; //инициализация модуля прошла успешно
};

//===========================================================
//  Запись данных в модуль ФДС
//===========================================================

void fds16r_dw(table_drv* tdrv) {
    unsigned char value = 0, temp = 0; //переменная value - значение для записи в модуль, temp - буфферная переменная
    int ADR_MISPA = 0x118, i, j; //переменная ADR_MISPA хранящаая адрем миспа, i,j - переменный для работы циклов

    if (tdrv->error) //если есть ошибка ожидаем пока она не решится
        return;

    fdsDate->Diagn = SPAPS_OK;
    tdrv->error = SPAPS_OK; //ошибок в модуле нет можно работать

    CLEAR_MEM //подготовка памяти
    WritePort(ADR_MISPA, (char) (tdrv->address & 0xff)); //сообщаем миспе какой модуль к ней обращается
    if (ERR_MEM) { // проверка на наличие ошибки
        fdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    }

    temp |= WriteBox(AdrOut18, 0); //обнуляем значение в памяти для каналов 1-8
    temp |= WriteBox(AdrOut916, 0); //обнуляем значение в памяти для каналов 9-16
    if (temp == BUSY_BOX) { //проверка на наличие ошибки
        fdsDate->Diagn = BUSY_BOX;
        tdrv->error = BUSY_BOX; //запоминаем значение - Нет ответа от модуля
        return; //прекращаем работу до исправления ошибки
    } else if (temp == NEGC_BOX) { //Неинверсия в ПЯ - значение не записалась в инверсную часть памяти
        fdsDate->Diagn = NEGC_BOX;
        tdrv->error = NEGC_BOX; //запоминаем значение - Неинверсия в ПЯ
        return; //прекращаем работу до исправления ошибки
    }

    for (i = 0; i < 2; i++) { //цикл для первой группы каналов
        value = 0;
        for (j = 8; j >= 0; j--) {//цикл по каналам
            value <<= 1;
            value |= (fdsDate->SIGN[j + i * 8].c & 1); //преобразуем значения
        }
        WriteBox(i + 1, value); //записываем группу в память
    }

    ReadBx3w(AdrISP18, &temp); //читаем память исправности 1-8 каналов
    fdsDate->ISP[0].i = temp;
    ReadBx3w(AdrISP916, &temp); //читаем память исправности 9-16 каналов
    fdsDate->ISP[1].i = temp;
    if (fdsDate->ISP[0].i || fdsDate->ISP[1].i) { //проверяем если ли - неисправность каналов
        fdsDate->Diagn = CHAN_ERR;
    }
};
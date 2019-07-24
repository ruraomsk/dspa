#ifndef VENCF8L_H
#define VENCF8L_H


#define AdrRQ 0x5    // регистр запроса обслуживания
#define AdrType 0x04 // тип модуля
// Vds
//Регистры состояния контактов датчиков
#define AdrSostContact0 0x10 // каналы 1-8
#define AdrSostContact1 0x11 // каналы 9-16
#define AdrSostContact2 0x40 // каналы 17-24
#define AdrSostContact3 0x41 // каналы 25-32
//Регистры настройки времени антидребезга
#define AdrVdsAntiTrembl0 0x20 // каналы 1-16
#define AdrVdsAntiTrembl1 0x23 // каналы 17-32
//Регистры маски каналов
#define AdrVdsChanlsMask0 0x21 // каналы 1-16
#define AdrVdsChanlsMask1 0x24 // каналы 17-32
//Регистрыы статуса
#define AdrVdsStatus0 0x22 // каналы 1-16
#define AdrVdsStatus1 0x25 // каналы 17-32

//Fds
#define AdrFdsOut18         0x01  // регистр вывода сигналов каналов 1-8  
#define AdrFdsOut916        0x02  // регистр вывода сигналов каналов 9-16  
#define AdrFdsISP18         0x03  // регистр исправности каналов 1-8  
#define AdrFdsISP916        0x0F  // регистр исправности каналов 9-16  


typedef struct __attribute__((packed))
{
  unsigned char   typeVds;       // 0xc2 тип вдс32р                                   
  unsigned short  BoxLen;        // 0xff длина ПЯ, уменьшенная на 1                   
  unsigned char   vip;           // 0    флаг критически важного для системы модуля   
  unsigned char   AdrFds;        // 0x6  Адрес ФДС на МИСПА  
  unsigned char   typeFds;       // 0x04 тип Фдс16р                                   
} vencf8_inipar;

// Структура для считывания
typedef union  { 
    unsigned long l;
    unsigned short i[2];
    unsigned char  c[4];
  } enc_value_t;

typedef struct __attribute__((packed))
{  
  sslong    gray[8];       // исходная - код Грея 
  sslong    venc[8];       // декодированная   
  short     DiagnFDS;      // диагностика ФДС
  short     DiagnVDS;      // диагностика ВДС 
  short     numE;          // переменная для выбора цикла (выбирает следующий BUS)
  char emptyspace[64];
} vencf8_data;

#define VENCF 0x2

void vencf8_ini(table_drv* drv);
void vencf8_dr(table_drv* drv);



#endif /* VENCF8L_H */


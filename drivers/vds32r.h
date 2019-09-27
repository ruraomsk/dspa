#ifndef VDS32R_H
#define VDS32R_H

typedef struct __attribute__((packed))
{
  unsigned char type;       // default = 0xC2;  тип модуля 
  unsigned int  BoxLen;     // default = 0xFF;  длина ПЯ, уменьшенная на 1 
  unsigned char inv;        // флаг инверсии 0 - прямой 1 - инверстный 
  unsigned char tadr116;    // default = 0xFF;  Время антидребезга каналов 1-16  
  unsigned char tadr1732;   // default = 0xFF;  Время антидребезга каналов 17-32 
} vds32r_inipar;

typedef struct __attribute__((packed))
{
  ssbool SIGN[32]; // Результат счета каналов 1-8   
  short Diagn;  // Результат диагностики модуля
  ssbool PastValue[32]; // Значение прошлого цикла опроса
  char emptyspace[64]; // Зарезервированная память
} vds32r_data;

typedef struct __attribute__((packed))
{
  unsigned char stat[2];
  unsigned char sost[4];
  unsigned char obr[4];
  unsigned char kz[4];
} vds32r_str;

#define VDS32R 0xC2
#define VDS32R_SIZE sizeof(vds32r_data)

void vds32r_ini(table_drv* drv);
void vds32r_rd(table_drv* drv);

#endif /* VDS32R_H */


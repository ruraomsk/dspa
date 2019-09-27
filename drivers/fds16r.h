#ifndef FDS16R_H
#define FDS16R_H


typedef struct __attribute__((packed))
{
  unsigned char type;       // default = 0x96;  тип модуля 
  unsigned int  BoxLen;     // default = 0xFF;  длина ПЯ, уменьшенная на 1 
  unsigned char NumCh;      // default = 8;     количество каналов 
} fds16r_inipar;

typedef struct __attribute__((packed))
{
  sschar SIGN[16]; // Результат счета каналов 1-8  
  ssint ISP[2];    // Исправность каналов
  short Diagn;     // Результат диагностики модуля
  char emptyspace[64]; // Зарезервированная память
} fds16r_data;

#define FDS16R 0x96
#define FDS16R_SIZE sizeof(fds16r_data)

void fds16r_ini(table_drv* drv);
void fds16r_dw(table_drv* drv);

#endif /* FDS16R_H */


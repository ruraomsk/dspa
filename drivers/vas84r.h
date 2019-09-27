#ifndef VAS84R_H
#define VAS84R_H

typedef struct __attribute__((packed))
{
  unsigned char type;    // default = 0xC6; // тип модуля 
  unsigned int  BoxLen;  // default = 0xFF; // длина ПЯ, уменьшенная на 1 
} vas84r_inipar;


typedef struct __attribute__((packed))
{
  ssint SIGN[8];  // Результат счета каналов 1-8   
  sschar widesos; // расширенный байт состояния
  short Diagn;  // Результат диагностики модуля
  short NumK; // номер текущего канала
  char emptyspace[64];  // Зарезервированная память
} vas84r_data;

#define VAS84R 0xC6
#define VAS84R_SIZE sizeof(vas84r_data)

void vas84r_rd(table_drv* drv);
void vas84r_ini(table_drv* drv);


#endif /* VAS84R_H */


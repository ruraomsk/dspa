#ifndef VCHS2_H
#define VCHS2_H

typedef struct __attribute__((packed))
{
  unsigned char type;    // default = 0xC6;    тип модуля 
  unsigned int  BoxLen;  // default = 0xFF;    длина ПЯ, уменьшенная на 1 
  unsigned char NumCh;   // default = 32;      количество каналов 
  unsigned char UsMask;  // default = 0xFF;    маска использования каналов 1-первый 2-второй  
  unsigned char ChMask;  // default = 0x0;     флаги изменения каналов 1-2   
  unsigned char chn1d;   // default = 0x1;     диапазон канала1:  1 - 1-1000 с;  2 - 10мс-10с; 4 - 10мкс - 10мс; 8 - 1мкс - 1мс
  unsigned char chn2d;   // default = 0x1;     диапазон канала1:  1 - 1-1000 с;  2 - 10мс-10с; 4 - 10мкс - 10мс; 8 - 1мкс - 1мс
} vchs_inipar;

typedef struct __attribute__((packed))
{
  ssfloat         K01VCHS;       // частота 1 канал 
  ssfloat         K02VCHS;       // частота 2 канал 
  short Diagn;                   // результат диагностики модуля
  unsigned int tempI[2];         // расшифрованное значение
  float takt[2];                 // такт программы
  float cykl[2];                 // оставшееся время
  short perm[2];                 // разрешение на чтение данных
  unsigned char SVCHS[2];        // флаг переполнения канала
  float fvch[2];                 // посчитанная частота
  float cyklS[2];                // минимальное значение цикла
  char emptyspace[64];           // Зарезервированная память
} vchs_data;

#define VCHS 0xc4
#define VCHS_SIZE sizeof(vchs_data)

void vchs_ini(table_drv* drv);
void vchs_dr(table_drv* drv);

#endif /* VCHS2_H */

#ifndef VCHS2_H
#define VCHS2_H

typedef struct __attribute__((packed))
{
  unsigned char type;    // default = 0xC6;    тип модуля 
  unsigned int  BoxLen;  // default = 0xFF;    длина ПЯ, уменьшенная на 1 
  unsigned char vip;     // default = 0;       флаг критически важного для системы модуля 
  unsigned char NumCh;   // default = 32;      количество каналов 
  unsigned char UsMask;  // default = 0xFF;    маска использования каналов 1-первый 2-второй  
  unsigned char ChMask;  // default = 0x0;     флаги изменения каналов 1-2   
  unsigned char chn1d;   // default = 0x1;     диапазон канала1:  1 - 1-1000 с;  2 - 10мс-10с; 4 - 10мкс - 10мс; 8 - 1мкс - 1мс
  unsigned char chn2d;   // default = 0x1;     диапазон канала1:  1 - 1-1000 с;  2 - 10мс-10с; 4 - 10мкс - 10мс; 8 - 1мкс - 1мс
  float         Gmin1;   // default = 0;       нижняя граница измерения частоты 
  float         Gmin2;   // default = 0;       нижняя граница измерения частоты 
  float         Gmax1;   // default = 1000000; верхняя граница измерения частоты 
  float         Gmax2;   // default = 1000000; верхняя граница измерения частоты 
} vchs_inipar;

typedef struct __attribute__((packed))
{
  ssfloat         K01VCHS;       // частота 1 канал 
  ssfloat         K02VCHS;       // частота 2 канал 
  unsigned int tempI[2];
  float takt[2];
  float cykl[2];
  short perm[2];
  unsigned char SVCHS[2];
  float fvch[2];
  float cyklS[2];
//  unsigned int iMFast[2][4];
//  long lMSlow[2][20];
//  long lSmF[2];
//  long lSmS[2];
//  float fMFtim[2][4];
//  float fMStim[2][20];
//  float fTimF[2];
//  float fTimS[2];
//  int pMFast[2];
//  int pMSlow[2];
} vchs_data;

#define VCHS 0xc4
#define VCHS_SIZE sizeof(vchs_data)

void vchs_ini(table_drv* drv);
void vchs_dr(table_drv* drv);

#endif /* VCHS2_H */

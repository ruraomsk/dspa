/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vchs2.h
 * Author: rusin
 *
 * Created on 22 марта 2018 г., 9:58
 */

#ifndef VCHS2_H
#define VCHS2_H

#pragma pack(push,1)
typedef struct
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

typedef struct
{
  ssfloat K01VCHS;          // частота 1 канал 
  ssfloat K02VCHS;          // частота 2 канал 
  sschar  Cyklen;          // увеличение длины цикла контроллера   
  unsigned int iMFast1[4];      // массив значений импульсов за цикл от счетного канала1
  float   fMFtim1[4];      // массив времен цикла измерения счетного канала 1 
  unsigned long lMSlow1[20];// массив накопленных значений импульсов от счетного канала 1 
  float   fMStim1[20];      // массив накопленных значений времени циклов измерения счетного канала 1
  char    pMFast1;          // указатель текущей позиции массива импульсов за цикл от счетного канала 1
  char    pMSlow1;          // указатель текущей позиции массива накопленных импульсов от счетного канала 1
  float   fTimF1;           // суммарное время измерения импульсов за цикл от счетного канала 1
  unsigned long lSmF1;      // суммарное количество импульсов за цикл от счётного канала 1
  float   fTimS1;           // суммарное время измерения накопленных импульсов от счетного канала 1
  unsigned long lSmS1;      // суммарное измеренное количество импульсов от счётного канала 1
  unsigned int iMFast2[4];      // массив значений импульсов за цикл от счетного канала 2
  float   fMFtim2[4];      // массив времен цикла измерения счетного канала 2 
  unsigned long lMSlow2[20];// массив накопленных значений импульсов от счетного канала 2 
  float   fMStim2[20];      // массив накопленных значений времени циклов измерения счетного канала 2
  char    pMFast2;          // указатель текущей позиции массива импульсов за цикл от счетного канала 2
  char    pMSlow2;          // указатель текущей позиции массива накопленных импульсов от счетного канала 2
  float   fTimF2;           // суммарное время измерения импульсов за цикл от счетного канала 2
  unsigned long lSmF2;      // суммарное количество импульсов за цикл от счётного канала 2
  float   fTimS2;           // суммарное время измерения накопленных импульсов от счетного канала 2
  unsigned long lSmS2;      // суммарное измеренное количество импульсов от счётного канала 2
} vchs_data;
#pragma pack(pop)

void vchs_ini(table_drv* drv);
void vchs_dr(table_drv* drv);
#define VCHS 0xc4
#define VCHS_SIZE sizeof(vchs_data)

#endif /* VCHS2_H */

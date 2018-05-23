/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vds32r.h
 * Author: rusin
 *
 * Created on 14 февраля 2018 г., 9:30
 */

#ifndef VDS32R_H
#define VDS32R_H

typedef struct __attribute__((packed))
{
  unsigned char type;       // default = 0xC2;  тип модуля 
  unsigned int  BoxLen;     // default = 0xFF;  длина ПЯ, уменьшенная на 1 
//  unsigned char vip;        // default = 0;     флаг критически важного для системы модуля 
  unsigned char inv;        // флаг инверсии 0 - прямой 1 - инверстный 
  unsigned char NumCh;      // default = 8;     количество каналов 
  unsigned char tadr116;    // default = 0xFF;  Время антидребезга каналов 1-16  
  unsigned char tadr1732;   // default = 0xFF;  Время антидребезга каналов 17-32 
  unsigned char Dmask116;   // default = 0xFF;  маска диагностики каналов 1-16   
  unsigned char Dmask1732;  // default = 0xFF;  маска диагностики каналов 17-32 
  unsigned char UsMask18;   // default = 0xFF;  маска использования каналов 1-8  
  unsigned char UsMask916;  // default = 0xFF;  маска использования каналов 9-16 
  unsigned char UsMask1724; // default = 0xFF;  маска использования каналов 17-24
  unsigned char UsMask2532; // default = 0xFF;  маска использования каналов 25-32
  unsigned char ChMask18;   // default = 0x0;   маска изменения состояния каналов 1-8  
  unsigned char ChMask916;  // default = 0x0;   маска изменения состояния каналов 9-16 
  unsigned char ChMask1724; // default = 0x0;   маска изменения состояния каналов 17-24
  unsigned char ChMask2532; // default = 0x0;   маска изменения состояния каналов 25-32
} vds32r_inipar;

typedef struct __attribute__((packed))
{
  ssbool SIGN[32]; // Результат счета каналов 1-8   
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


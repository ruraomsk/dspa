/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   fds16r.h
 * Author: rusin
 *
 * Created on 13 февраля 2018 г., 12:39
 */

#ifndef FDS16R_H
#define FDS16R_H

#pragma pack(push,1)

typedef struct
{
  unsigned char type;       // default = 0xC2;  тип модуля 
  unsigned int  BoxLen;     // default = 0xFF;  длина ПЯ, уменьшенная на 1 
  unsigned char vip;        // default = 0;     флаг критически важного для системы модуля 
  unsigned char NumCh;      // default = 8;     количество каналов 
  unsigned char UsMask18;   // default = 0xFF;  маска использования каналов 1-8  
  unsigned char UsMask916;  // default = 0xFF;  маска использования каналов 9-16 
  unsigned char ChMask18;   // default = 0x0;   маска изменения состояния каналов 1-8  
  unsigned char ChMask916;  // default = 0x0;   маска изменения состояния каналов 9-16 
  unsigned char is11;       // default = 0x0;   история флагов исправности каналов 1-8  
  unsigned char is12;       // default = 0x0;   история флагов исправности каналов 1-8  
  unsigned char is91;       // default = 0x0;   история флагов исправности каналов 9-16 
  unsigned char is92;       // default = 0x0;   история флагов исправности каналов 9-16  
  unsigned char s11;        // default = 0x0;   история флагов состояния каналов 1-4  
  unsigned char s12;        // default = 0x0;   история флагов состояния каналов 1-4  
  unsigned char s51;        // default = 0x0;   история флагов состояния каналов 5-8  
  unsigned char s52;        // default = 0x0;   история флагов состояния каналов 5-8  
  unsigned char s91;        // default = 0x0;   история флагов состояния каналов 9-12  
  unsigned char s92;        // default = 0x0;   история флагов состояния каналов 9-12  
  unsigned char s131;       // default = 0x0;   история флагов состояния каналов 13-16 
  unsigned char s132;       // default = 0x0;   история флагов состояния каналов 13-16  
} fds16r_inipar;

typedef struct
{
  sschar SIGN[16]; // Результат счета каналов 1-8   
} fds16r_data;
#pragma pack(pop)
#define FDS16R 0x04
#define FDS16R_SIZE sizeof(fds16r_data)

void fds16r_ini(table_drv* drv);
void fds16r_dw(table_drv* drv);

#endif /* FDS16R_H */


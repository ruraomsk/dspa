/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vas84r.h
 * Author: rusin
 *
 * Created on 14 февраля 2018 г., 11:04
 */

#ifndef VAS84R_H
#define VAS84R_H

typedef struct __attribute__((packed))
{
  unsigned char type;    // default = 0xC6; // тип модуля 
  unsigned int  BoxLen;  // default = 0xFF; // длина ПЯ, уменьшенная на 1 
  unsigned char vip;     // default = 0;    // флаг критически важного для системы модуля 
  unsigned char NumCh;   // default = 8;    // количество каналов 
  unsigned char UsMask;  // default = 0xFF; // маска использования каналов
  unsigned char ChMask;  // default = 0x0;  // флаги изменения каналов 
  unsigned char Aprt;    // default = 0x17;  // апертура 
} vas84r_inipar;


typedef struct __attribute__((packed))
{
  ssint SIGN[8];   // Результат счета каналов 1-8   
  sschar widesos;  // расширенный байт состояния
  short Diagn;
} vas84r_data;

#define VAS84R 0xC6
#define VAS84R_SIZE sizeof(vas84r_data)

void vas84r_rd(table_drv* drv);
void vas84r_ini(table_drv* drv);


#endif /* VAS84R_H */


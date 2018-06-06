/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sbkfp7.h
 * Author: rusin
 *
 * Created on 22 РјР°СЂС‚Р° 2018 Рі., 10:00
 */

#ifndef SBKFP7_H
#define SBKFP7_H

typedef struct __attribute__((packed))
{
  unsigned char type;  // default = 0xC2;  
  unsigned int BoxLen; // default = 0xFF;  
  unsigned char vip;   // default = 0;     
  unsigned char NumCh; // default = 8;     
} sbk_inipar;

typedef struct __attribute__((packed))
{
  ssbool SbkSIGN[13];

  // ssbool SbkPower1; 
  // ssbool SbkPower2; 
  // ssbool SbkDoor;   
  // ssbool SbkT43;    
  // ssbool SbkT53;    
  // ssbool SbkPB124;  
  // ssbool SbkPB15;   
  // ssbool SbkPB224;  
  // ssbool SbkPB25;   

  // ssbool SbkMpPB124;
  // ssbool SbkMpPB15; 
  // ssbool SbkMpPB224;
  // ssbool SbkMpPB25; 
} sbk_data;

#define SBK 0x01
#define SBK_SIZE sizeof(sbk_data)

void sbkfp7_ini(table_drv *drv);
void sbkfp7_dw(table_drv *drv);

#endif /* SBKFP7_H */

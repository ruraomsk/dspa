/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sbkfp7.c
 * Author: rusin
 * 
 * Created on 22 РјР°СЂС‚Р° 2018 Рі., 10:00
 */

#include "../dspadef.h"
#include "../misparw.h"
#include "sbkfp7.h"

#define inipar ((sbk_inipar *)(drv->inimod))
#define NewDate ((sbk_data *)(drv->data))
#define LastIn ((char *)(&drv->time))
 
static int UpRead = 0;

//===========================================================
//	РРЅРёС†РёР°Р»РёР·Р°С†РёСЏ РјРѕРґСѓР»СЏ
//===========================================================

void sbkfp7_ini(table_drv *drv)
{
    log_init(drv);
  NewDate->SbkPower1.b = 0;
  NewDate->SbkPower1.error = 0;
  NewDate->SbkPower2.b = 0;
  NewDate->SbkPower2.error = 0;
  NewDate->SbkDoor.b = 0;
  NewDate->SbkDoor.error = 0;
  NewDate->SbkT43.b = 0;
  NewDate->SbkT43.error = 0;
  NewDate->SbkT53.b = 0;
  NewDate->SbkT53.error = 0;
  NewDate->SbkPB124.b = 0;
  NewDate->SbkPB124.error = 0;
  NewDate->SbkPB15.b = 0;
  NewDate->SbkPB15.error = 0;
  NewDate->SbkPB224.b = 0;
  NewDate->SbkPB224.error = 0;
  NewDate->SbkPB25.b = 0;
  NewDate->SbkPB25.error = 0;
  NewDate->SbkMpPB124.b = 0;
  NewDate->SbkMpPB124.error = 0;
  NewDate->SbkMpPB15.b = 0;
  NewDate->SbkMpPB15.error = 0;
  NewDate->SbkMpPB224.b = 0;
  NewDate->SbkMpPB224.error = 0;
  NewDate->SbkMpPB25.b = 0;
  NewDate->SbkMpPB25.error = 0;
  drv->error = 0;
}

//===========================================================/
//	РџСЂРёРµРј РґР°РЅРЅС‹С… РёР· РјРѕРґСѓР»СЏ SBK
//===========================================================
/*
//===========================================================
СЃС‚СЂСѓРєС‚СѓСЂР° Р±Р°Р№С‚Р° СЃРѕСЃС‚РѕСЏРЅРёСЏ
 
  Р‘РёС‚         Р—РЅР°С‡РµРЅРёРµ

   0   -  РќРµРёСЃРїСЂР°РІРЅРѕСЃС‚СЊ СЃРµС‚Рё РїРёС‚Р°РЅРёСЏ 1 РёР»Рё 2
   1   -  Р”РІРµСЂСЊ С€РєР°С„Р° РѕС‚РєСЂС‹С‚Р°
   2   -  РџСЂРµРІС‹С€РµРЅРёРµ РґРѕРїСѓСЃС‚РёРјРѕР№ С‚РµРјРїРµСЂР°С‚СѓСЂС‹
   3   -  РќРµРёСЃРїСЂР°РІРЅРѕСЃС‚СЊ РњРџ15/24
   4   -  РќРµРёСЃРїСЂР°РІРЅРѕСЃС‚СЊ Р‘Рџ5/24 
   5   -
   6   -
   7   -

//===========================================================
*/
void sbkfp7_dw(table_drv *drv)
{
  ssbool SBKTemp;
  // Port 110h
    log_step(drv);
  drv->error = 0;
  if (UpRead == 0){ // РџР•Р Р’Р«Р™ Р’РҐРћР”               РџРћР Рў 110 - 1
    WritePort(0x108, 0xd);
  }

  if (UpRead == 1){ // Р’РўРћР РћР™ Р’РҐРћР”
    SBKTemp.b = ReadPort(0x110);
    NewDate->SbkPower1.b = (SBKTemp.b >> 0) & 1;
    NewDate->SbkPower2.b = (SBKTemp.b >> 1) & 1;
    NewDate->SbkDoor.b = (SBKTemp.b >> 3) & 1;
    NewDate->SbkT43.b = (SBKTemp.b >> 4) & 1;
    NewDate->SbkT53.b = (SBKTemp.b >> 5) & 1;
    if (NewDate->SbkPower1.b == 1)
      NewDate->SbkPower1.error = 0x1;
    else
      NewDate->SbkPower1.error = 0x0;
    if (NewDate->SbkPower2.b == 1)
      NewDate->SbkPower2.error = 0x1;
    else
      NewDate->SbkPower2.error = 0x0;
    if (NewDate->SbkDoor.b == 1)
      NewDate->SbkDoor.error = 0x2;
    else
      NewDate->SbkDoor.error = 0x0;
    if (NewDate->SbkT43.b == 0)
      NewDate->SbkT43.error = 0x4;
    else
      NewDate->SbkT43.error = 0x0;
    if (NewDate->SbkT53.b == 1)
      NewDate->SbkT53.error = 0x4;
    else
      NewDate->SbkT53.error = 0x0;
  }

  if (UpRead == 2){ // РўР Р•РўРР™ Р’РҐРћР”      РџРћР Рў 110 - 2
    WritePort(0x108, 0xb);
  }

  if (UpRead == 3){// Р§Р•РўР’Р•Р РўР«Р™ Р’РҐРћР”
    SBKTemp.b = ReadPort(0x110);
    NewDate->SbkPB124.b = (SBKTemp.b >> 0) & 1;
    NewDate->SbkPB15.b = (SBKTemp.b >> 1) & 1;
    NewDate->SbkPB224.b = (SBKTemp.b >> 2) & 1;
    NewDate->SbkPB25.b = (SBKTemp.b >> 3) & 1;
    if (NewDate->SbkPB124.b == 1)
      NewDate->SbkPB124.error = 0x8;
    else
      NewDate->SbkPB124.error = 0x0;
    if (NewDate->SbkPB15.b == 1)
      NewDate->SbkPB15.error = 0x8;
    else
      NewDate->SbkPB15.error = 0x0;
    if (NewDate->SbkPB224.b == 1)
      NewDate->SbkPB224.error = 0x8;
    else
      NewDate->SbkPB224.error = 0x0;
    if (NewDate->SbkPB25.b == 1)
      NewDate->SbkPB25.error = 0x8;
    else
      NewDate->SbkPB25.error = 0x0;
    }

  if(UpRead!=3)
  UpRead++;  
  else
  UpRead=0;

  // Port 114h - С‡РёС‚Р°РµС‚ РєР°Р¶РґС‹Р№ СЂР°Р·

  SBKTemp.b = ReadPort(0x114);
  NewDate->SbkMpPB124.b = (SBKTemp.b >> 0) & 1;
  NewDate->SbkMpPB15.b = (SBKTemp.b >> 1) & 1;
  NewDate->SbkMpPB224.b = (SBKTemp.b >> 6) & 1;
  NewDate->SbkMpPB25.b = (SBKTemp.b >> 7) & 1;

  if (NewDate->SbkMpPB124.b == 1)
    NewDate->SbkMpPB124.error = 0x10;
  else
    NewDate->SbkMpPB124.error = 0x0;
  if (NewDate->SbkMpPB15.b == 1)
    NewDate->SbkMpPB15.error = 0x10;
  else
    NewDate->SbkMpPB15.error = 0x0;
  if (NewDate->SbkMpPB224.b == 1)
    NewDate->SbkMpPB224.error = 0x10;
  else
    NewDate->SbkMpPB224.error = 0x0;
  if (NewDate->SbkMpPB25.b == 1)
    NewDate->SbkMpPB25.error = 0x10;
  else
    NewDate->SbkMpPB25.error = 0x0;
}
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
#define sbkDate ((sbk_data *)(drv->data))
#define LastIn ((char *)(&drv->time))

static int UpRead = 0;

//===========================================================
//	РРЅРёС†РёР°Р»РёР·Р°С†РёСЏ РјРѕРґСѓР»СЏ
//===========================================================

void sbkfp7_ini(table_drv *drv)
{
  //    log_init(drv);
  sbkDate->SbkPower1.b = 0;
  sbkDate->SbkPower1.error = 0;
  sbkDate->SbkPower2.b = 0;
  sbkDate->SbkPower2.error = 0;
  sbkDate->SbkDoor.b = 0;
  sbkDate->SbkDoor.error = 0;
  sbkDate->SbkT43.b = 0;
  sbkDate->SbkT43.error = 0;
  sbkDate->SbkT53.b = 0;
  sbkDate->SbkT53.error = 0;
  sbkDate->SbkPB124.b = 0;
  sbkDate->SbkPB124.error = 0;
  sbkDate->SbkPB15.b = 0;
  sbkDate->SbkPB15.error = 0;
  sbkDate->SbkPB224.b = 0;
  sbkDate->SbkPB224.error = 0;
  sbkDate->SbkPB25.b = 0;
  sbkDate->SbkPB25.error = 0;
  sbkDate->SbkMpPB124.b = 0;
  sbkDate->SbkMpPB124.error = 0;
  sbkDate->SbkMpPB15.b = 0;
  sbkDate->SbkMpPB15.error = 0;
  sbkDate->SbkMpPB224.b = 0;
  sbkDate->SbkMpPB224.error = 0;
  sbkDate->SbkMpPB25.b = 0;
  sbkDate->SbkMpPB25.error = 0;
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
  //    log_step(drv);
  drv->error = 0;
  if (UpRead == 0)
  { // РџР•Р Р’Р«Р™ Р’РҐРћР”               РџРћР Рў 110 - 1
    SBKTemp.b = ReadPort(0x108);
    SBKTemp.b &= 0xfd;
    WritePort(0x108, SBKTemp.b);
  }

  if (UpRead == 1)
  { // Р’РўРћР РћР™ Р’РҐРћР”
    SBKTemp.b = ReadPort(0x110);
    sbkDate->SbkPower1.b = (SBKTemp.b >> 0) & 1;
    sbkDate->SbkPower2.b = (SBKTemp.b >> 1) & 1;
    sbkDate->SbkDoor.b = (SBKTemp.b >> 3) & 1;
    sbkDate->SbkT43.b = (SBKTemp.b >> 4) & 1;
    sbkDate->SbkT53.b = (SBKTemp.b >> 5) & 1;
    if (sbkDate->SbkPower1.b == 1)
      sbkDate->SbkPower1.error = 0x1;
    else
      sbkDate->SbkPower1.error = 0x0;
    if (sbkDate->SbkPower2.b == 1)
      sbkDate->SbkPower2.error = 0x1;
    else
      sbkDate->SbkPower2.error = 0x0;
    if (sbkDate->SbkDoor.b == 1)
      sbkDate->SbkDoor.error = 0x2;
    else
      sbkDate->SbkDoor.error = 0x0;
    if (sbkDate->SbkT43.b == 0)
      sbkDate->SbkT43.error = 0x4;
    else
      sbkDate->SbkT43.error = 0x0;
    if (sbkDate->SbkT53.b == 1)
      sbkDate->SbkT53.error = 0x4;
    else
      sbkDate->SbkT53.error = 0x0;
  }

  if (UpRead == 2)
  { // РўР Р•РўРР™ Р’РҐРћР”      РџРћР Рў 110 - 2
    SBKTemp.b = ReadPort(0x108);
    SBKTemp.b &= 0xfb;
    WritePort(0x108, SBKTemp.b);
  }

  if (UpRead == 3)
  { // Р§Р•РўР’Р•Р РўР«Р™ Р’РҐРћР”
    SBKTemp.b = ReadPort(0x110);
    sbkDate->SbkPB124.b = (SBKTemp.b >> 0) & 1;
    sbkDate->SbkPB15.b = (SBKTemp.b >> 1) & 1;
    sbkDate->SbkPB224.b = (SBKTemp.b >> 2) & 1;
    sbkDate->SbkPB25.b = (SBKTemp.b >> 3) & 1;
    if (sbkDate->SbkPB124.b == 1)
      sbkDate->SbkPB124.error = 0x8;
    else
      sbkDate->SbkPB124.error = 0x0;
    if (sbkDate->SbkPB15.b == 1)
      sbkDate->SbkPB15.error = 0x8;
    else
      sbkDate->SbkPB15.error = 0x0;
    if (sbkDate->SbkPB224.b == 1)
      sbkDate->SbkPB224.error = 0x8;
    else
      sbkDate->SbkPB224.error = 0x0;
    if (sbkDate->SbkPB25.b == 1)
      sbkDate->SbkPB25.error = 0x8;
    else
      sbkDate->SbkPB25.error = 0x0;
  }

  if (UpRead != 3)
    UpRead++;
  else
    UpRead = 0;

  // Port 114h - С‡РёС‚Р°РµС‚ РєР°Р¶РґС‹Р№ СЂР°Р·

  SBKTemp.b = ReadPort(0x114);
  sbkDate->SbkMpPB124.b = (SBKTemp.b >> 0) & 1;
  sbkDate->SbkMpPB15.b = (SBKTemp.b >> 1) & 1;
  sbkDate->SbkMpPB224.b = (SBKTemp.b >> 6) & 1;
  sbkDate->SbkMpPB25.b = (SBKTemp.b >> 7) & 1;

  if (sbkDate->SbkMpPB124.b == 1)
    sbkDate->SbkMpPB124.error = 0x10;
  else
    sbkDate->SbkMpPB124.error = 0x0;
  if (sbkDate->SbkMpPB15.b == 1)
    sbkDate->SbkMpPB15.error = 0x10;
  else
    sbkDate->SbkMpPB15.error = 0x0;
  if (sbkDate->SbkMpPB224.b == 1)
    sbkDate->SbkMpPB224.error = 0x10;
  else
    sbkDate->SbkMpPB224.error = 0x0;
  if (sbkDate->SbkMpPB25.b == 1)
    sbkDate->SbkMpPB25.error = 0x10;
  else
    sbkDate->SbkMpPB25.error = 0x0;
}
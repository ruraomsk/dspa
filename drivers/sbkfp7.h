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

#pragma pack(push,1)
typedef struct
{
  unsigned char type;       // default = 0xC2;  С‚РёРї РјРѕРґСѓР»СЏ 
  unsigned int  BoxLen;     // default = 0xFF;  РґР»РёРЅР° РџРЇ, СѓРјРµРЅСЊС€РµРЅРЅР°СЏ РЅР° 1 
  unsigned char vip;        // default = 0;     С„Р»Р°Рі РєСЂРёС‚РёС‡РµСЃРєРё РІР°Р¶РЅРѕРіРѕ РґР»СЏ СЃРёСЃС‚РµРјС‹ РјРѕРґСѓР»СЏ 
  unsigned char NumCh;      // default = 8;     РєРѕР»РёС‡РµСЃС‚РІРѕ РєР°РЅР°Р»РѕРІ 
} sbk_inipar;

typedef struct
{
 /* РџРѕСЂС‚ СѓРїСЂР°РІР»РµРЅРёСЏ 110h */

 ssbool  SbkPower1;  /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё СЃРµС‚Рё 1 */
 ssbool  SbkPower2;  /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё СЃРµС‚Рё 2 */
 ssbool  SbkDoor;    /* С„Р»Р°Рі РѕС‚РєСЂС‹С‚РёСЏ РґРІРµСЂРё */
 ssbool  SbkT43;     /* С„Р»Р°Рі РїРѕРІС‹С€РµРЅРёСЏ С‚РµРјРїРµСЂР°С‚СѓСЂС‹ РІС‹С€Рµ 43 */
 ssbool  SbkT53;     /* С„Р»Р°Рі РїРѕРІС‹С€РµРЅРёСЏ С‚РµРјРїРµСЂР°С‚СѓСЂС‹ РІС‹С€Рµ 53 */
 ssbool  SbkPB124;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 1 - 24 */
 ssbool  SbkPB15;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 1 - 5 */
 ssbool  SbkPB224;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 2 - 24 */
 ssbool  SbkPB25;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 2 - 5 */

 /* РџРѕСЂС‚ СѓРїСЂР°РІР»РµРЅРёСЏ 114h */

 ssbool  SbkMpPB124;  /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 1 - 24 */
 ssbool  SbkMpPB15;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 1 - 5 */
 ssbool  SbkMpPB224;  /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 2 - 24 */
 ssbool  SbkMpPB25;   /* С„Р»Р°Рі РЅРµРёСЃРїСЂР°РІРЅРѕСЃС‚Рё Р‘Рџ 2 - 5 */
} sbk_data;

#pragma pack(pop)

void sbkfp7_ini(table_drv* drv);
void sbkfp7_dr(table_drv* drv);
#define SBK 0xc4
#define SBK_SIZE sizeof(sbk_data)

#endif /* SBKFP7_H */

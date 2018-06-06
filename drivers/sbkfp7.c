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

void sbkfp7_ini(table_drv *drv) {
    drv->error = 0;
}

void sbkfp7_dw(table_drv *drv) {
    int i, j;
    unsigned char temp;

    // состояния шкафа
    if (UpRead == 0) {
        temp = ReadPort(0x108);
        temp &= 0xfd;
        WritePort(0x108, temp);
        for (i = 0; i < 13; i++)
            sbkDate->SbkSIGN[i].error = 0xff;
    }
    
    if (UpRead == 1) {
        temp = ReadPort(0x110);
        sbkDate->SbkSIGN[0].b = (temp >> 0) & 1;
        sbkDate->SbkSIGN[1].b = (temp >> 1) & 1;
        sbkDate->SbkSIGN[2].b = (temp >> 3) & 1;
        sbkDate->SbkSIGN[3].b = (temp >> 4) & 1;
        sbkDate->SbkSIGN[4].b = (temp >> 5) & 1;
        for (i = 0; i < 5; i++) {
            sbkDate->SbkSIGN[i].error = 0;
        }
    }
    // состояния БП
    if (UpRead == 2) {
        temp = ReadPort(0x108);
        temp &= 0xfb;
        WritePort(0x108, temp);
    }

    if (UpRead == 3) {
        temp = ReadPort(0x110);
        for (i = 5, j = 0x1; i < 8; i++) {
            if (temp & j)
                sbkDate->SbkSIGN[i].b = 1;
            else
                sbkDate->SbkSIGN[i].b = 0;
            j <<= 1;
            sbkDate->SbkSIGN[i].error = 0;
        }
    }

    temp = ReadPort(0x114);
    sbkDate->SbkSIGN[9].b = (temp >> 0) & 1;
    sbkDate->SbkSIGN[10].b = (temp >> 1) & 1;
    sbkDate->SbkSIGN[11].b = (temp >> 6) & 1;
    sbkDate->SbkSIGN[12].b = (temp >> 7) & 1;
    for (i = 9; i < 12; i++) {
        sbkDate->SbkSIGN[i].error = 0;
    }

    if (UpRead != 3)
        UpRead++;
    else
        UpRead = 0;

    //    ssbool SBKTemp;
    //
    //    drv->error = 0;
    //    if (UpRead == 0) {
    //        SBKTemp.b = ReadPort(0x108);
    //        SBKTemp.b &= 0xfd;
    //        WritePort(0x108, SBKTemp.b);
    //    }
    //
    //    if (UpRead == 1) {
    //        SBKTemp.b = ReadPort(0x110);
    //        sbkDate->SbkPower1.b = (SBKTemp.b >> 0) & 1;
    //        sbkDate->SbkPower2.b = (SBKTemp.b >> 1) & 1;
    //        sbkDate->SbkDoor.b = (SBKTemp.b >> 3) & 1;
    //        sbkDate->SbkT43.b = (SBKTemp.b >> 4) & 1;
    //        sbkDate->SbkT53.b = (SBKTemp.b >> 5) & 1;
    //        if (sbkDate->SbkPower1.b == 1)
    //            sbkDate->SbkPower1.error = 0x1;
    //        else
    //            sbkDate->SbkPower1.error = 0x0;
    //        if (sbkDate->SbkPower2.b == 1)
    //            sbkDate->SbkPower2.error = 0x1;
    //        else
    //            sbkDate->SbkPower2.error = 0x0;
    //        if (sbkDate->SbkDoor.b == 1)
    //            sbkDate->SbkDoor.error = 0x2;
    //        else
    //            sbkDate->SbkDoor.error = 0x0;
    //        if (sbkDate->SbkT43.b == 0)
    //            sbkDate->SbkT43.error = 0x4;
    //        else
    //            sbkDate->SbkT43.error = 0x0;
    //        if (sbkDate->SbkT53.b == 1)
    //            sbkDate->SbkT53.error = 0x4;
    //        else
    //            sbkDate->SbkT53.error = 0x0;
    //    }
    //
    //    if (UpRead == 2) {
    //        SBKTemp.b = ReadPort(0x108);
    //        SBKTemp.b &= 0xfb;
    //        WritePort(0x108, SBKTemp.b);
    //    }
    //
    //    if (UpRead == 3) {
    //        SBKTemp.b = ReadPort(0x110);
    //        sbkDate->SbkPB124.b = (SBKTemp.b >> 0) & 1;
    //        sbkDate->SbkPB15.b = (SBKTemp.b >> 1) & 1;
    //        sbkDate->SbkPB224.b = (SBKTemp.b >> 2) & 1;
    //        sbkDate->SbkPB25.b = (SBKTemp.b >> 3) & 1;
    //        if (sbkDate->SbkPB124.b == 1)
    //            sbkDate->SbkPB124.error = 0x8;
    //        else
    //            sbkDate->SbkPB124.error = 0x0;
    //        if (sbkDate->SbkPB15.b == 1)
    //            sbkDate->SbkPB15.error = 0x8;
    //        else
    //            sbkDate->SbkPB15.error = 0x0;
    //        if (sbkDate->SbkPB224.b == 1)
    //            sbkDate->SbkPB224.error = 0x8;
    //        else
    //            sbkDate->SbkPB224.error = 0x0;
    //        if (sbkDate->SbkPB25.b == 1)
    //            sbkDate->SbkPB25.error = 0x8;
    //        else
    //            sbkDate->SbkPB25.error = 0x0;
    //    }
    //
    //    if (UpRead != 3)
    //        UpRead++;
    //    else
    //        UpRead = 0;
    //
    //
    //
    //    SBKTemp.b = ReadPort(0x114);
    //    sbkDate->SbkMpPB124.b = (SBKTemp.b >> 0) & 1;
    //    sbkDate->SbkMpPB15.b = (SBKTemp.b >> 1) & 1;
    //    sbkDate->SbkMpPB224.b = (SBKTemp.b >> 6) & 1;
    //    sbkDate->SbkMpPB25.b = (SBKTemp.b >> 7) & 1;
    //
    //    if (sbkDate->SbkMpPB124.b == 1)
    //        sbkDate->SbkMpPB124.error = 0x10;
    //    else
    //        sbkDate->SbkMpPB124.error = 0x0;
    //    if (sbkDate->SbkMpPB15.b == 1)
    //        sbkDate->SbkMpPB15.error = 0x10;
    //    else
    //        sbkDate->SbkMpPB15.error = 0x0;
    //    if (sbkDate->SbkMpPB224.b == 1)
    //        sbkDate->SbkMpPB224.error = 0x10;
    //    else
    //        sbkDate->SbkMpPB224.error = 0x0;
    //    if (sbkDate->SbkMpPB25.b == 1)
    //        sbkDate->SbkMpPB25.error = 0x10;
    //    else
    //        sbkDate->SbkMpPB25.error = 0x0;
}
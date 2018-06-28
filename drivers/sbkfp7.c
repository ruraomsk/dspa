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
#include "linux/printk.h"

#define inipar ((sbk_inipar *)(drv->inimod))
#define sbkDate ((sbk_data *)(drv->data))

static int UpRead = 0;

//===========================================================
//	РРЅРёС†РёР°Р»РёР·Р°С†РёСЏ РјРѕРґСѓР»СЏ
//===========================================================

void sbkfp7_ini(table_drv *drv) {
    drv->error = 0;
}

void sbkfp7_dw(table_drv *drv) {
    int i, j;
    ssbool temp;
    unsigned char ti;
    // состояния шкафа
    if (UpRead == 0) {
        ti = ReadPort(0x108);
        ti &= 0xf9;
        ti |= 0xfd;
        WritePort(0x108, ti);
        //      WritePort(0x108, 0xfd);
        for (i = 0; i < 13; i++)
            sbkDate->SbkSIGN[i].error = 0xff;
    }

    if (UpRead == 1) {
        temp.b = ReadPort(0x110);
        sbkDate->SbkSIGN[0].b = (temp.b >> 0) & 1;
        sbkDate->SbkSIGN[1].b = (temp.b >> 1) & 1;
        sbkDate->SbkSIGN[2].b = (temp.b >> 3) & 1;
        sbkDate->SbkSIGN[3].b = (temp.b >> 4) & 1;
        sbkDate->SbkSIGN[4].b = (temp.b >> 5) & 1;
        for (i = 0; i < 5; i++) {
            sbkDate->SbkSIGN[i].error = 0;
        }
    }
    // состояния БП
    if (UpRead == 2) {
        ti = ReadPort(0x108);
        ti &= 0xf9;
        ti |= 0xfb;
        WritePort(0x108, ti);
        //        WritePort(0x108, 0xfb);
    }

    if (UpRead == 3) {
        temp.b = ReadPort(0x110);
        for (i = 5, j = 0x1; i < 8; i++) {
            if (temp.b & j)
                sbkDate->SbkSIGN[i].b = 1;
            else
                sbkDate->SbkSIGN[i].b = 0;
            j <<= 1;
            sbkDate->SbkSIGN[i].error = 0;
        }
    }

    temp.b = ReadPort(0x114);
    sbkDate->SbkSIGN[9].b = (temp.b >> 0) & 1;
    sbkDate->SbkSIGN[10].b = (temp.b >> 1) & 1;
    sbkDate->SbkSIGN[11].b = (temp.b >> 6) & 1;
    sbkDate->SbkSIGN[12].b = (temp.b >> 7) & 1;
    for (i = 9; i < 12; i++) {
        sbkDate->SbkSIGN[i].error = 0;
    }

    if (UpRead != 3)
        UpRead++;
    else
        UpRead = 0;
}
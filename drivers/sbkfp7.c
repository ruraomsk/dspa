#include "../dspadef.h"
#include "../misparw.h"
#include "sbkfp7.h"
#include "linux/printk.h"

#define inipar ((sbk_inipar *)(drv->inimod))
#define sbkDate ((sbk_data *)(drv->data))

static int UpRead = 0;

void sbkfp7_ini(table_drv *drv) {
    drv->error = 0;
}

void sbkfp7_dw(table_drv *drv) {
    int i;
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
        sbkDate->SbkSIGN[0].b = (temp.b >> 0) & 1;  // power 1
        sbkDate->SbkSIGN[1].b = (temp.b >> 1) & 1;  // power 2
        sbkDate->SbkSIGN[2].b = (temp.b >> 3) & 1;  // door
        sbkDate->SbkSIGN[3].b = (temp.b >> 4) & 1;  // t < 43
        sbkDate->SbkSIGN[4].b = (temp.b >> 5) & 1;  // t > 53
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
        sbkDate->SbkSIGN[5].b = (temp.b >> 0) & 1;  // MP15-3.1 - 1
        sbkDate->SbkSIGN[6].b = (temp.b >> 1) & 1;  // MP15-3.1 - 2
        sbkDate->SbkSIGN[7].b = (temp.b >> 2) & 1;  // MP15-3 - 3
        sbkDate->SbkSIGN[8].b = (temp.b >> 3) & 1;  // MP24-2 - 4
    }

    temp.b = ReadPort(0x114);
    sbkDate->SbkSIGN[9].b = (temp.b >> 0) & 1;  // PB5/24 - 5
    sbkDate->SbkSIGN[10].b = (temp.b >> 1) & 1; // PB5/24 - 6
    sbkDate->SbkSIGN[11].b = (temp.b >> 6) & 1; // PB5/24 - 7
    sbkDate->SbkSIGN[12].b = (temp.b >> 7) & 1; // PB5/24 - 8
    for (i = 9; i < 12; i++) {
        sbkDate->SbkSIGN[i].error = 0;
    }

    if (UpRead != 3)
        UpRead++;
    else
        UpRead = 0;
}
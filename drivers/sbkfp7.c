#include "../dspadef.h"
#include "../misparw.h"
#include "sbkfp7.h"
#include "linux/printk.h"

#define inipar ((sbk_inipar *)(drv->inimod))
#define sbkDate ((sbk_data *)(drv->data))

static int UpRead = 0;  // stepDate = 0;

void sbkfp7_ini(table_drv *drv) {
    int i;
    drv->error = 0;
    for (i = 0; i < 13; i++)
        sbkDate->SbkSIGN[i].error = 0;
}

void sbkfp7_dw(table_drv *drv) {
    // int j;
    ssbool temp;
    unsigned char ti;
    // состояния шкафа
    if (UpRead == 0) {
        ti = ReadPort(0x108);
        ti &= 0xf9;
        ti |= 0xfd;
        WritePort(0x108, ti);
        //      WritePort(0x108, 0xfd);
    }

    if (UpRead == 1) {
        temp.b = ReadPort(0x110);
        sbkDate->SbkSIGN[0].b = (temp.b >> 0) & 1;  // power 1
        sbkDate->SbkSIGN[1].b = (temp.b >> 1) & 1;  // power 2
        sbkDate->SbkSIGN[2].b = (temp.b >> 3) & 1;  // door
        sbkDate->SbkSIGN[3].b = (temp.b >> 4) & 1;  // t < 43
        sbkDate->SbkSIGN[4].b = (temp.b >> 5) & 1;  // t > 53
        // sbkDate->SbkSaveS[stepDate][0] = (temp.b >> 0) & 1;  // power 1
        // sbkDate->SbkSaveS[stepDate][1] = (temp.b >> 1) & 1;  // power 2
        // sbkDate->SbkSaveS[stepDate][2] = (temp.b >> 3) & 1;  // door
        // sbkDate->SbkSaveS[stepDate][3] = (temp.b >> 4) & 1;  // t < 43
        // sbkDate->SbkSaveS[stepDate][4] = (temp.b >> 5) & 1;  // t > 53
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
        // sbkDate->SbkSaveS[stepDate][5] = (temp.b >> 0) & 1;  // MP15-3.1 - 1
        // sbkDate->SbkSaveS[stepDate][6] = (temp.b >> 1) & 1;  // MP15-3.1 - 2
        // sbkDate->SbkSaveS[stepDate][7] = (temp.b >> 2) & 1;  // MP15-3 - 3
        // sbkDate->SbkSaveS[stepDate][8] = (temp.b >> 3) & 1;  // MP24-2 - 4
    }

    temp.b = ReadPort(0x114);
    sbkDate->SbkSIGN[9].b  = (temp.b >> 0) & 1; // PB5/24 - 5
    sbkDate->SbkSIGN[10].b = (temp.b >> 1) & 1; // PB5/24 - 6
    sbkDate->SbkSIGN[11].b = (temp.b >> 6) & 1; // PB5/24 - 7
    sbkDate->SbkSIGN[12].b = (temp.b >> 7) & 1; // PB5/24 - 8
    // sbkDate->SbkSaveS[stepDate][9]  = (temp.b >> 0) & 1; // PB5/24 - 5
    // sbkDate->SbkSaveS[stepDate][10] = (temp.b >> 1) & 1; // PB5/24 - 6
    // sbkDate->SbkSaveS[stepDate][11] = (temp.b >> 6) & 1; // PB5/24 - 7
    // sbkDate->SbkSaveS[stepDate][12] = (temp.b >> 7) & 1; // PB5/24 - 8

    // for(j = 0; j < 13; j++){
    //     if(sbkDate->SbkSaveS[1][j] == sbkDate->SbkSaveS[2][j] == sbkDate->SbkSaveS[3][j])
    //         sbkDate->SbkSIGN[j].b = sbkDate->SbkSaveS[stepDate][j];
    // }   

    // if(stepDate == 3)
    //     stepDate = 0;

    if (UpRead != 3)
        UpRead++;
    else{
        UpRead = 0;
        // stepDate++;
    }
}

#include "../misparw.h"
#include "../dspadef.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "vencf8l.h"
#include "linux/printk.h"

//#define   CHECK_WRITEPORT   1

#define DEBUG_DRV_STAGES  1
#define DEBUG_MODE_ON 1
#define USING_VDS_NEW_FW 1

#define inipar ((vencf8_inipar *)(tdrv->inimod))
#define devdata ((vencf8_data *)(tdrv->data))
#define LastIn ((char *)(&tdrv->time))

#define VENCF_DELAY usleep_range(20, 50)

//TODO Remove this after debug
sslong OldCoord[TOTAL_ENCODERS_COUNT];
//TODO Remove this
unsigned char g_nOldError = 0;
sschar g_sscRQbyte;
ssint g_iVencError[TOTAL_ENCODERS_COUNT]; // счетчики ошибок и ошибки в одном флаконе
enc_value_t TempEnc[3];
sslong g_sslCurCoordinate = {0, 0}, g_sslSavCoord = {0, 0};

inline void ConnMod(unsigned char addr) {
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, addr);
}

unsigned char vds32CheckData(unsigned char nByteAddrA, unsigned char nWaitForDataA, pschar pnRQbyteA) {
    unsigned long nTOut = 0;
    unsigned char rqbit = (nByteAddrA >= ADR_CONTACTS_STATE_CH17_24) ? VDS_RQ_HI_DATA_READY : VDS_RQ_LO_DATA_READY;
    while (1) {
        pnRQbyteA->error = ReadBx3w(ADR_REQUEST_REG, &pnRQbyteA->c);
        if (pnRQbyteA->error) {
            if (pnRQbyteA->error == BUSY_BOX) {
                return 0;
            }


            if (nWaitForDataA & RQ_NO_ERRORS) {
                VENCF_DELAY;
                continue;
            }
        }

        if (++nTOut > VDS_WAIT_TIMEOUT) {
            pnRQbyteA->error |= VENCF_ERR_TIME_OUT;
            break;
        }

        if (!(pnRQbyteA->c & rqbit) && (nWaitForDataA & RQ_WAIT_MODE)) {
            VENCF_DELAY;
        } else
            break;

    }
    return (pnRQbyteA->c & rqbit) != 0 ? 1 : 0;
}

sschar vds32GetData(unsigned char nByteAddrA) {
    sschar res = {0, 0};
    while (1) {
        res.error = ReadBx3w(nByteAddrA, &res.c);
        res.c = ~res.c;
        if (res.error) { // read failed - return
            if (res.error == BUSY_BOX)
                return res;
        }
        if (res.error) {
            VENCF_DELAY;
        } else
            break;
    }
    return res;
}

sschar vds32GetStat(unsigned char nModIdA, unsigned char nAddrA) {
    sschar res = {0, 0};
    if (nModIdA) {
#ifdef CHECK_WRITEPORT
        CLEAR_MEM
        WritePort(SPAPS_ADR_MISPA, nModIdA);
        if (ERR_MEM) {
            res.error = BUSY_BOX;
            return res;
        }
#else
        WritePort(SPAPS_ADR_MISPA, nModIdA);
#endif
    }
    //Регистры статуса
    res.error = ReadBx3w(nAddrA, &res.c);
    return res;
}

unsigned char vds32init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char temp = 0, RH = 0, Err = 0;
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, AdrMod);
    if (ERR_MEM)
        return VENCF_ERR_CRIT_MALFUNC;

    RH |= WriteSinglBox(6, 0);
    RH |= WriteSinglBox(4, 0);
    //    проверка типа модуля
    RH |= ReadBx3w(0x4, &temp);
    if (temp != TypeMod) {
        return 0x80;
    } //ошибка типа модуля
    RH |= WriteBox(0x20, 0xff); //  антидребезг каналы 1-16   0x20  AdrAntiTrembl0
    RH |= WriteBox(0x21, 0x0); // маска каналы 1-16   0x21          AdrChanlsMask0
    RH |= WriteBox(0x23, 0xff); // антидребезг каналы 17-32  0x23   AdrAntiTrembl1
    RH |= WriteBox(0x24, 0x0); // маска каналы 1-16   0x21          AdrChanlsMask1
    RH |= ReadBx3w(0x22, &temp); // статус0                   AdrStatus0
    RH |= ReadBx3w(0x25, &temp); // статус1                   AdrStatus1
    RH |= ReadBx3w(0x5, &temp); // RQ                        AdrRQ
    if (RH)
        Err = RH;
    return Err;
}

unsigned char fds16init(unsigned char AdrMod, unsigned char TypeMod) {
    unsigned char RH = 0, temp = 0, Err = 0, isp = 0;
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, AdrMod);
    if (ERR_MEM)
        return VENCF_ERR_CRIT_MALFUNC;
    RH |= ReadBx3w(ADR_MODULE_TYPE, &temp);
    if (temp != TypeMod)
        Err = 0x80;
    RH |= WriteBox(0x01, 0);
    RH |= WriteBox(0x02, 0);
    RH |= ReadBx3w(0x03, &temp);
    isp |= temp;
    RH |= ReadBx3w(0x0f, &temp);
    isp |= temp;
    if (RH || isp)
        Err = RH;
    return Err;
}

//===========================================================
//  Инициализация модуля VENCF8
//===========================================================
//

void vencf8_ini(table_drv *tdrv) {
    unsigned char RH = 0;

    tdrv->error = 0; // clear DRV error
    memset(&g_iVencError, 0, sizeof (g_iVencError));
    SetBoxLen(inipar->BoxLen);
    // инициализация модуля VDS32 - данные от ПТИ
    RH |= vds32init(tdrv->address, inipar->typeVds);
    // инициализация модуля VDS32 - синхронизация с ПТИ
    // RH |= vds32init(inipar->AdrVdsSinc, inipar->typeVds);
    // RH |= fds16init(inipar->AdrFds, inipar->typeFds);
    if (RH)
        tdrv->error = RH;
    // чтобы это не значило
    //проверка наличия ПТИ - режим работы с ПТИ или с PU-10
    //    if (!er.error && (er.c & (inipar->MaskChanSinc >> 1)) != 0) {
    //        inipar->model_present = 1; // работа с пти
    //    } else {
    //        inipar->model_present = 0; // работа с PU-10
    //    }
}



void vencf8_dr(table_drv *tdrv) {
    unsigned char RH = 0, i, j=0, temp;
    int k1, k2; // для считывания координат
    sschar er = {0, 0};

    SetBoxLen(inipar->BoxLen);

    // vds - 0x01 - tdrv->address
    // ConnMod(tdrv->address);
    // if (ERR_MEM) {
    //     tdrv->error = 0x80;
    //     return;
    // }
    // RH |= ReadBx3w(0x22, &temp); //AdrStatus0
    // RH |= ReadBx3w(0x25, &temp); //AdrStatus1
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }







    // // vds - 0x03  - inipar->AdrVdsSinc
    // ConnMod(inipar->AdrVdsSinc);
    // if (ERR_MEM) {
    //     tdrv->error = 0x80;
    //     return;
    // }
    // RH |= ReadBx3w(0x22, &temp); //AdrStatus0
    // RH |= ReadBx3w(0x25, &temp); //AdrStatus1
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }


    //  выставить latch

    // ConnMod(inipar->AdrFds);
    // if (ERR_MEM) {
    //     tdrv->error = 0x80;
    //     return;
    // }
    // RH = WriteBox(ADR_LATCH_FDS_CH9_16, 1); // rise LATCH 
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }






    // for (i = 0, j = 1; i < TOTAL_ENCODERS_COUNT; i++) { // выводим очередной BUS
    {
        // ConnMod(inipar->AdrFds);
        // if (ERR_MEM) {
        //     tdrv->error = 0x80;
        //     return;
        // }
        // // RH = WriteBox(ADR_BUS_FDS_CH1_8, j); // rise current BUS
        // RH = WriteBox(ADR_BUS_FDS_CH1_8, 1); // rise current BUS
        // if (RH) {
        //     tdrv->error = RH;
        //     return;
        // }


        ConnMod(tdrv->address); // вдс - 1
        if (ERR_MEM) {
            tdrv->error = 0x80;
            return;
        }
        ReadBox(0x5, &RH);
        if ((RH & 0x01) || (RH & 0x10)) {
            printk("-------------------------");
            ReadBx3w(0x10, &er.c);
            printk("0x10   -  %hhx",~er.c);
            ReadBx3w(0x11, &er.c);
            printk("0x11   -  %hhx",~er.c);
            ReadBx3w(0x40, &er.c);
            printk("0x40   -  %hhx",~er.c);
            ReadBx3w(0x41, &er.c);
            printk("0x41   -  %hhx",~er.c);
        }
        
        while (1) {
            for (k1 = 0; k1 < 2; k1++) {
                for (k2 = 0; k2 < 3; k2++) {
                    er.error = ReadBx3w(0x10 + k1, &er.c);
                    TempEnc[k2].c[k1] = ~er.c;
                    g_iVencError[k1].error |= er.error;
                }
                
            }
            for (k1 = 2; k1 < 4; k1++) {
                for (k2 = 0; k2 < 3; k2++) {
                    er.error = ReadBx3w(0x40 + (k1 - 2), &er.c);
                    TempEnc[k2].c[k1] = ~er.c;
                    g_iVencError[k1].error |= er.error;
                }
            }
            if (er.error == BUSY_BOX) {
                tdrv->error = er.error;
                return;
            }

            if ((TempEnc[0].l == TempEnc[1].l && TempEnc[0].l == TempEnc[2].l)){
                printk("TempEnc[0].l   -  %lx",TempEnc[0].l);
                printk("TempEnc[1].l   -  %lx",TempEnc[1].l);
                printk("TempEnc[2].l   -  %lx",TempEnc[2].l);
                break;
                }
        }

        g_sslCurCoordinate.l = (TempEnc[0].l & TempEnc[1].l) | (TempEnc[1].l & TempEnc[2].l) | (TempEnc[0].l & TempEnc[2].l);
        printk("g_sslCurCoordinate.l = %lx",g_sslCurCoordinate.l);


        g_sslSavCoord.error = g_iVencError[i].error;
        devdata->gray[i].l = (unsigned long) g_sslCurCoordinate.l & 0x00fffffful;
        printk("do dekoda = %d",devdata->gray[i].l);
        devdata->gray[i].error = 0;
        devdata->venc[i].l = decodegray(devdata->gray[i].l);
        printk("posle dekod = %d",devdata->venc[i].l);
        devdata->venc[i].error = 0;



        j <<= 1; // shift BUS variable to the next BUS

        // ConnMod(inipar->AdrFds);
        // if (ERR_MEM) {
        //     tdrv->error = 0x80;
        //     return;
        // }
        // RH = 0;
        // RH |= WriteBox(ADR_BUS_FDS_CH1_8, 0);
        // if (RH) {
        //     tdrv->error = RH;
        //     return;
        // }
    }
    // RH |= WriteBox(ADR_LATCH_FDS_CH9_16, 0);
    // if (RH) {
    //     tdrv->error = RH;
    //     return;
    // }
}
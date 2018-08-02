/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "../misparw.h"
#include "../dspadef.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "vencf8l.h"

//#define   CHECK_WRITEPORT   1

//#define   DEBUG_DRV_STAGES  1  
#define   DEBUG_MODE_ON     1
#define   USING_VDS_NEW_FW  1

#define inipar  ((vencf8_inipar*)(tdrv->inimod)) 
#define devdata ((vencf8_data*)(tdrv->data)) 
#define LastIn  ((char*)(&tdrv->time))

#define VENCF_DELAY usleep_range(20, 50)

//TODO Remove this after debug
sslong OldCoord[TOTAL_ENCODERS_COUNT];
//TODO Remove this
unsigned char g_nOldError = 0;
sschar g_sscRQbyte;
ssint g_iVencError[TOTAL_ENCODERS_COUNT]; // счетчики ошибок и ошибки в одном флаконе
enc_value_t g_encTmp[3];
sslong g_sslCurCoordinate = {0, 0}, g_sslSavCoord = {0, 0};

unsigned char vds32CheckData(unsigned char nByteAddrA, unsigned char nWaitForDataA, pschar pnRQbyteA) {
    sschar res; //, val={0,0};
    unsigned long nTOut = 0;
    unsigned char rqbit = (nByteAddrA >= ADR_CONTACTS_STATE_CH17_24) ? VDS_RQ_HI_DATA_READY : VDS_RQ_LO_DATA_READY;
    while (1) {
        res = vds32GetRq(0);
        //val.c |= res.c;    
        if (pnRQbyteA) {
            pnRQbyteA->c = res.c;
            pnRQbyteA->error |= res.error;
        }
        if (res.error) {
            if (res.error == BUSY_BOX) {
                #ifdef DEBUG_MODE_ON
                printk("[vencf::vds32CheckData] vds_getRQ_FAILED");
                #endif
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
        if (!(res.c & rqbit) && (nWaitForDataA & RQ_WAIT_MODE)) {
            #ifdef DEBUG_MODE_ON
            if (nTOut == 1)
                printk("[vencf::vds32CheckData] waiting for RQ..");
            #endif        
            VENCF_DELAY;
        }
        else
            break;
    }
    return (res.c & rqbit) != 0 ? 1 : 0;
}

sschar vds32GetData(unsigned char nByteAddrA) {
    sschar res = {0, 0};
    sslong lCnt = {0, 0};
    unsigned short nTOut = 0;
    while (1) {
        res.error = ReadBox3(nByteAddrA, &res.c);
        res.c = ~res.c;
        if (res.error) { // read failed - return
            #ifdef DEBUG_MODE_ON
            ++lCnt.l;
            lCnt.error = res.error;
            //if( lCnt.l > 1 )
            printk("[vencf::vds32GetData] getData FAILED cnt=%d err=%d", lCnt.l, lCnt.error);
            #endif
            if (res.error == BUSY_BOX)
                return res;
        }
        #ifdef DEBUG_MODE_ON         
        if (++nTOut > VDS_WAIT_TIMEOUT) {
            printk("[vencf::vds32GetData] ~~~ getData_TOut ~~~");
            res.error |= VDS_WAIT_TIMEOUT;
            break;
        }
        #endif        
        if (res.error) {
            #ifdef DEBUG_MODE_ON         
            if (nTOut == 1)
                printk("[vencf::vds32GetData] waiting for correct Data..");
            #endif                
            VENCF_DELAY;
        }
        else
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
    res.error = ReadBox3(nAddrA, &res.c);
    return res;
}

sschar vds32GetRq(unsigned char nModIdA) {
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
    res.error = ReadBox3(ADR_REQUEST_REG, &res.c);
    return res;
}

unsigned char vds32init(unsigned char nModIdA, unsigned char nVdsTypeId) {
    unsigned char iRes = 0;
    sschar er;
    unsigned char RL, RH, nIndx;

#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, nModIdA);
    if (ERR_MEM) {
#ifdef DEBUG_MODE_ON
        printk("[vencf::vds32init] WritePort mod 0x%x FAILED - BUSY_BOX!", nModIdA);
#endif
        return VENCF_ERR_CRIT_MALFUNC;
    }
#else
    WritePort(SPAPS_ADR_MISPA, nModIdA);
#endif

    for (nIndx = 0; nIndx < 2; nIndx++) {
        printk("[vencf::vds32init] ~~~ vds_ini mod=%d iter=%d ~~~", nModIdA, nIndx);
        //сброс регистров модуля
        iRes = WriteSinglBox(4, 0);
#ifdef DEBUG_MODE_ON
        if (iRes != SPAPS_OK)
            printk("[vencf::vds32init] reg 4 Rst Err 0x%x", iRes);
#endif
        if (iRes == BUSY_BOX) {
            if (nIndx == 0)
                continue;
            else
                return VENCF_ERR_CRIT_MALFUNC;
        }
        RH = WriteSinglBox(6, 0);
        if (RH != SPAPS_OK) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] reg 6 Rst Err 0x%x", RH);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC;
            }
            iRes |= RH;
        }
        // проверка типа модуля
        RH = ReadBox3(ADR_MODULE_TYPE, &RL);
        if (RH != SPAPS_OK) { //ошибка определения типа модуля
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] check ModType err=0x%x iRes=0x%x", RH, iRes);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC;
            }
            iRes |= RH;
        }
        if (RL != nVdsTypeId) {
            iRes |= VENCF_ERR_MOD_TYPE;
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] check ModType err=0x%x want=0x%x got=0x%x", iRes, nVdsTypeId, RL);
#endif
            continue;
        }
        //настроить регистры конфигурирования каналов
        //Регистры настройки времени антидребезга
        RH = WriteBox(ADR_ANTITREMBLE_CH1_16, 0); // 200 нс - minimum
        if (RH != SPAPS_OK) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] setTRMBL1err 0x%x", RH);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16;
            }
            iRes |= RH | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16;
        }
        //Регистры маски каналов - отключаем диагностику обрыва и КЗ
        RH = WriteBox(ADR_MASK_CH1_16, 0); // каналы 1-16 0x21
        if (RH) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] setMASK1err 0x%x", RH);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16;
            }
            iRes |= RH | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16;
        }
        //Регистры настройки времени антидребезга
        RH = WriteBox(ADR_ANTITREMBLE_CH17_32, 0); // 200 нс - minimum
        if (RH != SPAPS_OK) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] setTRMBL2err 0x%x", RH);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS17_32;
            }
            iRes |= RH | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS17_32;
        }
        //Регистры маски каналов - отключаем диагностику обрыва и КЗ
        RH = WriteBox(ADR_MASK_CH17_32, 0); // каналы 1-16 0x21
        if (RH) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] setMASK2err 0x%x", RH);
#endif
            if (RH == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS17_32;
            }
            iRes |= RH | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS17_32;
        }
        //Регистры статуса
        er = vds32GetStat(0, ADR_STATUSREGS_CH1_16);
        if (er.error || (er.c & VDS_STAT_CONFIG_ERROR)) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] getSTATE1 stat=0x%x err=0x%x", er.c, er.error);
#endif
            if (er.error == BUSY_BOX && nIndx)
                return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16;
            continue;
        }
        er = vds32GetStat(0, ADR_STATUSREGS_CH17_32);
        if (er.error || (er.c & VDS_STAT_CONFIG_ERROR)) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] getSTATE2 stat=0x%x err=0x%x", er.c, er.error);
#endif
            if (er.error == BUSY_BOX && nIndx)
                return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS17_32;
            continue;
        }
        // проверка запроса обслуживания
        er = vds32GetRq(0);
        if (er.error) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] get RQ err=0x%x", er.error);
#endif
            if (er.error == BUSY_BOX) {
                if (nIndx == 0)
                    continue;
                else
                    return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS;
            }
            iRes |= er.error;
        }
        if (er.c & VDS_RQ_CONFIG_ERROR) {
#ifdef DEBUG_MODE_ON
            printk("[vencf::vds32init] get RQ ConfigErr=0x%x", er.c);
#endif
            if (nIndx == 0)
                continue;
            else
                return VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SETUP_VDS;
        }
        if (iRes == SPAPS_OK)
            break;
    } // for
    return iRes;
}

/*
static unsigned long decodegray(unsigned long k)
{
//  unsigned long sk = k;
//  unsigned long rk = sk;
//  while( sk>>= 1 ) rk ^= sk;
//  return rk;
  unsigned long lk = k;
  unsigned char i;
  for(i = 1; i < 24; i <<= 1)
    lk ^= (lk >> i);
  return lk;
}   */

void set_all_errs(table_drv* tdrv, char nErrorA) {
    unsigned char i;
    if (tdrv)
        for (i = 0; i < TOTAL_ENCODERS_COUNT; i++)
            ((vencf8_data*) (tdrv->data))->venc[i].error = nErrorA;
};

//===========================================================
//  Инициализация модуля VENCF8
//===========================================================
//

void vencf8_ini(table_drv* tdrv) {
    unsigned char RH, RL, isp18, isp916;
    sschar er = {0, 0};
#ifdef DEBUG_MODE_ON
    printk("[vencf::vencf8_ini] ~~~ init start ~~~");
#endif
    tdrv->error = SPAPS_OK; // clear DRV error
    memset(&g_iVencError, 0, sizeof (g_iVencError));
    //настройка длины ПЯ
    SetBoxLen(inipar->BoxLen);
    // инициализация модуля VDS32 - данные от ПТИ
    er.c = vds32init(tdrv->address, inipar->typeVds);
    if (er.c) {
        tdrv->error = er.c | VENCF_ERR_SETUP_VDS | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] DataVDS (mod %d) init error 0x%x", tdrv->address, er.c);
#endif
    }
    // инициализация модуля VDS32 - синхронизация с ПТИ
    er.c = vds32init(inipar->AdrVdsSinc, inipar->typeVds);
    if (er.c) {
        tdrv->error |= er.c | VENCF_ERR_SETUP_VDS | VENCF_ERR_SYNC_VDS_FAILED;
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] SyncVDS (mod %d) init error 0x%x", inipar->AdrVdsSinc, er.c);
#endif
    }
#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
    if (ERR_MEM == SPAPS_OK)
#else    
    WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
#endif
        {
#ifdef USING_VDS_NEW_FW
            er = vds32GetData(ADR_CONTACTS_STATE_NEWFW_CH25_32);
#else
            er = vds32GetData(ADR_CONTACTS_STATE_CH1_8 + inipar->NumByteSinc);
#endif  
        }#ifdef CHECK_WRITEPORT   
    else {
        er.error = BUSY_BOX;
        tdrv->error |= VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] PTI detecting (WritePort) FAILED - mod %d!", inipar->AdrVdsSinc);
#endif     
        return;
    }
#endif
    //проверка наличия ПТИ - режим работы с ПТИ или с PU-10 
    if (!er.error && (er.c & (inipar->MaskChanSinc >> 1)) != 0) {
        inipar->model_present = 1; // работа с пти
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] PTI detected - vds=0x%x err=0x%x !", er.c, er.error);
#endif
    } else {
        inipar->model_present = 0; // работа с PU-10
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] No PTI detected vds=0x%x err=0x%x.", er.c, er.error);
#endif
    }
    // инициализация модуля FDS16R
#ifdef DEBUG_MODE_ON
    printk("[vencf::vencf8_ini] ~~~ FDS init start ~~~");
#endif

#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
    if (ERR_MEM) {
        tdrv->error |= VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_ini] FDS init (WritePort) FAILED - mod %d!", inipar->AdrFds);
#endif           
        return;
    }
    else
#else
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
        {
            RL = 0;
            RH = ReadBx3w(ADR_MODULE_TYPE, &RL);
            if (RH) {
                tdrv->error |= VENCF_ERR_MOD_TYPE;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] FDS TYPE check err 0x%x", RH);
#endif
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
                    return;
                }
            }
            if (RL != inipar->typeFds) { //ошибка типа модуля 
                tdrv->error |= VENCF_ERR_FDS_FAILED | VENCF_ERR_MOD_TYPE;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] FDS TYPE check FAILED got 0x%x wanted 0x%x", RL, inipar->typeFds);
#endif
            }
            // вывод сигналов
            RH = WriteBox(ADR_BUS_FDS_CH1_8, 0); // clear BUS
            if (RH) {
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] FDS BUS clear err=0x%x", RH);
#endif          
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
                    return;
                }
                tdrv->error |= VENCF_ERR_FDS_FAILED;
            }
            RH = WriteBox(ADR_LATCH_FDS_CH9_16, 0); // clear LATCH
            if (RH) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] FDS LATCH clear err=0x%x", RH);
#endif
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
                    return;
                }
            }
            // контроль исправности
            isp18 = 0;
            RH = ReadBx3w(ADR_ISPR_FDS_CH1_8, &isp18);
            if (RH) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] Checking FDS channels1_8 failed 0x%x drv.er=0x%x",
                        RH, tdrv->error);
#endif        
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
                    return;
                }
                //else
                // isp18 = 0; 
            }
            isp916 = 0;
            RH = ReadBx3w(ADR_ISPR_FDS_CH9_16, &isp916);
            if (RH) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] Checking FDS channels9_16 failed 0x%x drv.er=0x%x",
                        RH, tdrv->error);
#endif        
                if (RH == BUSY_BOX)
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
                //else
                // isp916 = 0x0; 
            }
            if (isp18 || isp916) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_ini] ~~~ FDS init FAILED ~~~");
#endif          
            }
#ifdef DEBUG_MODE_ON
            else
                printk("[vencf::vencf8_ini] ~~~ FDS init DONE ~~~");
#endif           
        }
#ifdef DEBUG_MODE_ON
    printk("[vencf::vencf8_ini] ~~~ init end, drv.error=0x%x ~~~", tdrv->error);
#endif
}

void vencf8_dr(table_drv* tdrv) {
    unsigned char indx2, nIter;
    unsigned char RH, RQ, i, j;
    sschar er = {0, 0}, res = {0, 0};

    SetBoxLen(inipar->BoxLen);

#ifdef DEBUG_MODE_ON
    if (tdrv->error & VENCF_ERR_CRIT_MALFUNC) {
        if (g_nOldError != tdrv->error) {
            printk("[vencf::vencf8_dr] ~~~ exit - because of drv.err=0x%x ~~~", tdrv->error);
            g_nOldError = tdrv->error;
        }
        return;
    }
#endif
#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, tdrv->address);
    if (ERR_MEM) {
        tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
        set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_dr] DataVds not accessible (WritePort failed) mod=%d!", tdrv->address);
#endif
        return;
    } else
#else
    WritePort(SPAPS_ADR_MISPA, tdrv->address);
#endif
        {
            // ПРОВЕРИМ СТАТУС   вдс-32 энкодеров  
            RQ = 0;
            res = vds32GetStat(tdrv->address, ADR_STATUSREGS_CH1_16);
            if (res.error) {
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] ENCwr_STATget_err1 0x%x", res.error);
#endif
                if (res.error == BUSY_BOX) {
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16;
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
                RQ |= res.error;
            }
            if (res.c & VDS_STAT_CONFIG_ERROR) { // ошибка инициализации модуля  
                RQ |= 0x01;
            }
            res = vds32GetStat(tdrv->address, ADR_STATUSREGS_CH17_32);
            if (res.error) {
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] ENCwr_STATget_err2 0x%x", res.error);
#endif
                if (res.error == BUSY_BOX) {
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS17_32;
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
                RQ |= res.error;
            }
            if (res.c & VDS_STAT_CONFIG_ERROR) { // ошибка инициализации модуля     
                RQ |= 0x02;
            }
            if (RQ) {
                er.error = vds32init(tdrv->address, inipar->typeVds);
                if (er.error) {
                    tdrv->error |= er.error;
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] vds32 mod %d  REINIT FAILED 0x%x drv.err=0x%x",
                            tdrv->address, er.error, tdrv->error);
#endif          
                    set_all_errs(tdrv, er.error);
                    return;
                }
            }
        }
    //************************************************************
    // ПРОВЕРИМ СТАТУС   вдс-32 PTI VALID   
#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
    if (ERR_MEM) //           WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);        
    {
        tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
        set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_dr] SyncVds not accessible (WritePort failed) mod=%d!", inipar->AdrVdsSinc);
#endif
        return;
    } else
#else
    WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
#endif      
        {
            RQ = 0;
            res = vds32GetStat(inipar->AdrVdsSinc, ADR_STATUSREGS_CH1_16);
            if (res.error) {
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] ENCwr_validSTATget_err1 0x%x", res.error);
#endif
                if (res.error == BUSY_BOX) {
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
                RQ |= res.error;
            }
            if (res.c & VDS_STAT_CONFIG_ERROR) { // ошибка инициализации модуля  
                RQ |= 0x01;
            }
            res = vds32GetStat(inipar->AdrVdsSinc, ADR_STATUSREGS_CH17_32);
            if (res.error) {
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] ENCwr_validSTATget_err2 0x%x", res.error);
#endif
                if (res.error == BUSY_BOX) {
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
                RQ |= res.error;
            }
            if (res.c & VDS_STAT_CONFIG_ERROR) { // ошибка инициализации модуля     
                RQ |= 0x02;
            }
            if (RQ) {
                er.error = vds32init(inipar->AdrVdsSinc, inipar->typeVds);
                if (er.error) {
                    tdrv->error |= er.error;
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] vds32 mod %d REINIT FAILED 0x%x drv.err=0x%x",
                            inipar->AdrVdsSinc, er.error, tdrv->error);
#endif          
                    set_all_errs(tdrv, er.error);
                    return;
                }
            }
        }
    //************************************************************
    inipar->puontime = 200;
    inipar->puofftime = 50;
    //************************************************************
    //  выставить latch   
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
    //~//if( inipar->model_present )
    printk("[vencf::vencf8_dr] riseLATCH");
#endif
#endif  
#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
    if (ERR_MEM) {
        tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_FDS_FAILED;
        set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_dr] SyncVds not accessible (WritePort failed) mod=%d!", inipar->AdrFds);
#endif
        return;
    }
#else
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
    RH = WriteBox(ADR_LATCH_FDS_CH9_16, 1 | 0x04); // rise LATCH & DebugOut(0x04-11channel)
    if (RH) {
        tdrv->error = VENCF_ERR_FDS_FAILED;
        if (RH == BUSY_BOX)
            tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
        set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
        printk("[vencf::vencf8_dr] FDS LATCH set err=0x%x drv.err=0x%x", RH, tdrv->error);
#endif
        return;
    }

    for (i = 0, j = 1; i < TOTAL_ENCODERS_COUNT; i++) {
        // выводим очередной BUS
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
        //~//if( inipar->model_present )
        printk("[vencf::vencf8_dr] rise BUS i=0x%x j=0x%x", i, j);
#endif
#endif  

#ifdef CHECK_WRITEPORT
        CLEAR_MEM
        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
        if (ERR_MEM) {
            tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_FDS_FAILED;
            set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
            printk("[vencf::vencf8_dr] SyncVds not accessible (WritePort failed) mod=%d!", inipar->AdrFds);
#endif
            return;
        }
#else
        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
        RH = WriteBox(ADR_BUS_FDS_CH1_8, j); // rise current BUS
        if (RH) {
            tdrv->error = VENCF_ERR_FDS_FAILED;
            if (RH == BUSY_BOX)
                tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
#ifdef DEBUG_MODE_ON
            printk("[vencf::vencf8_dr] FDS BUS set err=0x%x", RH);
#endif
            if (inipar->model_present || (tdrv->error & VENCF_ERR_CRIT_MALFUNC)) {
                set_all_errs(tdrv, tdrv->error);
                return;
            }
            else {
                devdata->venc[i].error = RH;
                j <<= 1; // shift BUS variable to the next BUS
                continue;
            }
        }
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//        
        if (inipar->model_present) {
#ifdef CHECK_WRITEPORT
            CLEAR_MEM
            WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
            if (ERR_MEM) {
                tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] wait4VALID SyncVDS not accessible (WritePort failed) mod=%d!", inipar->AdrVdsSinc);
#endif
                return;
            }
#else
            WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
#endif
            g_sscRQbyte.c = 0;
            g_sscRQbyte.error = 0;
            // Wait until VALID is changed
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
            //~//if( inipar->model_present )
            printk("[vencf::vencf8_dr] wait4VALID_byRQ");
#endif
#endif

#ifdef USING_VDS_NEW_FW
            vds32CheckData(ADR_CONTACTS_STATE_NEWFW_CH25_32, RQ_WAIT_MODE | RQ_NO_ERRORS, &g_sscRQbyte);
#else
            vds32CheckData(ADR_CONTACTS_STATE_CH1_8 + inipar->NumByteSinc, RQ_WAIT_MODE | RQ_NO_ERRORS, &g_sscRQbyte);
#endif

#ifdef DEBUG_MODE_ON
            if (g_sscRQbyte.error && g_sscRQbyte.error != 0xC0)
                printk("[vencf::vencf8_dr] get_VALID_RQ err=0x%x", g_sscRQbyte.error);
#endif
            if (g_sscRQbyte.error & VENCF_ERR_CRIT_MALFUNC) {
                tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                set_all_errs(tdrv, tdrv->error);
                return;
            }

#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
            //~//if( inipar->model_present )
            printk("[vencf::vencf8_dr] wait4VALID_byData");
#endif
#endif
            nIter = 0;
            indx2 = 0;
            res.c = 0x0;
            while (indx2 < VDS_WAIT_TIMEOUT) {
#ifdef USING_VDS_NEW_FW
                er = vds32GetData(ADR_CONTACTS_STATE_NEWFW_CH25_32);
#else
                er = vds32GetData(ADR_CONTACTS_STATE_CH1_8 + inipar->NumByteSinc);
#endif
                if (er.error) {
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] get_VALID_data err=0x%x", er.error);
#endif      
                    if (er.error == BUSY_BOX) {
                        tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                        set_all_errs(tdrv, tdrv->error);
                        // clear LATCH & BUS
#ifdef CHECK_WRITEPORT
                        CLEAR_MEM
                        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
                        if (ERR_MEM == SPAPS_OK)
#else
                        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
                            {
                                WriteBox(ADR_BUS_FDS_CH1_8, 0);
                                WriteBox(ADR_LATCH_FDS_CH9_16, 0);
                            }
                        return;
                    }
                }
                else
                    if (er.c & inipar->MaskChanSinc) {
                    if (++nIter > 3) {
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
                        //~//if( inipar->model_present )
                        printk("[vencf::vencf8_dr] gotVALID");
#endif
#endif
                        break;
                    }
                } else
                    nIter = 0;
#ifdef DEBUG_MODE_ON         
                if (++indx2 > VDS_WAIT_TIMEOUT) {
                    printk("[vencf::vencf8_dr] ~~~ wait4VALID_TOut ~~~");
                    g_iVencError[i].error |= VENCF_ERR_TIME_OUT;
                    break;
                }
                if (res.c != er.c) {
                    printk("[vencf::vencf8_dr] +++ VALID readings cur=0x%x old=0x%x", er.c, res.c);
                    res.c = er.c;
                    indx2 = 0;
                }
#endif
                VENCF_DELAY;
            } // while 1
            /*
                  if( indx2 >= VDS_WAIT_TIMEOUT )
                  {
                    g_iVencError[i].error |= (VENCF_ERR_SYNC_VDS_FAILED|VENCF_ERR_TIME_OUT); 
                    #ifdef DEBUG_MODE_ON
                      printk("[vencf.dr] wait4VALID_tout="); 
                    #endif
                  }  
             */
            //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        } else
            delaymcs(inipar->puontime);

#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
        //~//if( inipar->model_present )
        printk("[vencf::vencf8_dr] waitDATAhold RQ.c=0x%x RQ.er=0x%x", g_sscRQbyte.c, g_sscRQbyte.error);
#endif
#endif  
#ifdef CHECK_WRITEPORT
        CLEAR_MEM
        WritePort(SPAPS_ADR_MISPA, tdrv->address);
        if (ERR_MEM) {
            tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
            set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
            printk("[vencf::vencf8_dr] DataVDS not accessible mod=%d!", tdrv->address);
#endif
            // clear LATCH & BUS
            CLEAR_MEM
            WritePort(SPAPS_ADR_MISPA, inipar->AdrFds); //адрес модуля ФДС16р на миспа
            if (ERR_MEM == SPAPS_OK) {
                WriteBox(ADR_BUS_FDS_CH1_8, 0);
                WriteBox(ADR_LATCH_FDS_CH9_16, 0);
            }
            return;
        }
#else
        WritePort(SPAPS_ADR_MISPA, tdrv->address);
#endif
        indx2 = 0;
        while (1) {
            g_sscRQbyte.c = 0;
            g_sscRQbyte.error = 0;
            // Check if the Data still changing
            vds32CheckData(ADR_CONTACTS_STATE_CH1_8, RQ_NO_ERRORS, &g_sscRQbyte);
            if (g_sscRQbyte.error & VENCF_ERR_CRIT_MALFUNC) {
                tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
                set_all_errs(tdrv, tdrv->error);
                return;
            }
#ifdef DEBUG_MODE_ON        
            if (g_sscRQbyte.error && g_sscRQbyte.error != NEGC_BOX)
                printk("[vencf::vencf8_dr] waitDATAhold_RQ error RQ.c=0x%x RQ.er=0x%x", g_sscRQbyte.c, g_sscRQbyte.error);
#endif                
            if (g_sscRQbyte.c & VDS_RQ_LO_DATA_READY) {
                er = vds32GetData(ADR_CONTACTS_STATE_CH1_8);
                res = vds32GetData(ADR_CONTACTS_STATE_CH9_16);
#ifdef DEBUG_MODE_ON
                //  printk("[vencf::vencf8_dr] getRQ_byte12 dir=0x%x inv=0x%x", er.c, res.c); 
#endif        
                if (er.error & VENCF_ERR_CRIT_MALFUNC || res.error & VENCF_ERR_CRIT_MALFUNC) {
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] getRQ_byte12 FAILED - VENCF_ERR_CRIT_MALFUNC!");
#endif             
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
                    set_all_errs(tdrv, tdrv->error);
                    // clear LATCH & BUS
#ifdef CHECK_WRITEPORT
                    CLEAR_MEM
                    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
                    if (ERR_MEM == SPAPS_OK)
#else
                    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
                        {
                            WriteBox(ADR_BUS_FDS_CH1_8, 0);
                            WriteBox(ADR_LATCH_FDS_CH9_16, 0);
                        }
                    return;
                }
            }
            if (g_sscRQbyte.c & VDS_RQ_HI_DATA_READY) {
#ifdef USING_VDS_NEW_FW
                er = vds32GetData(ADR_CONTACTS_STATE_NEWFW_CH17_24);
                res = vds32GetData(ADR_CONTACTS_STATE_NEWFW_CH25_32);
#else
                er = vds32GetData(ADR_CONTACTS_STATE_CH17_24);
                res = vds32GetData(ADR_CONTACTS_STATE_CH25_32);
#endif
#ifdef DEBUG_MODE_ON
                //  printk("[vencf::vencf8_dr] getRQ_byte34 dir=0x%x inv=0x%x", er.c, res.c); 
#endif 
                if (er.error & VENCF_ERR_CRIT_MALFUNC || res.error & VENCF_ERR_CRIT_MALFUNC) {
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] getRQ_byte34 FAILED - VENCF_ERR_CRIT_MALFUNC!");
#endif                         
                    tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
                    set_all_errs(tdrv, tdrv->error);
                    // clear LATCH & BUS
#ifdef CHECK_WRITEPORT
                    CLEAR_MEM
                    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
                    if (ERR_MEM == SPAPS_OK)
#else
                    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
                        {
                            WriteBox(ADR_BUS_FDS_CH1_8, 0);
                            WriteBox(ADR_LATCH_FDS_CH9_16, 0);
                        }
                    return;
                }
            }
#ifdef DEBUG_MODE_ON
            if (++indx2 > VDS_WAIT_TIMEOUT) {
                printk("[vencf::vencf8_dr] ~~~ waitDataHold_TOut ~~~ RQ.c=0x%x RQ.er=0x%x", g_sscRQbyte.c, g_sscRQbyte.error);
                g_iVencError[i].error |= VENCF_ERR_TIME_OUT;
                break;
            }
#endif
            if (g_sscRQbyte.c & (VDS_RQ_HI_DATA_READY | VDS_RQ_LO_DATA_READY)) {
#ifdef DEBUG_MODE_ON
                if (indx2 == 1)
                    printk("[vencf::vencf8_dr]  wait for DataHold... ");
#endif          
                VENCF_DELAY;
            }
            else
                break;
        }

        indx2 = 0;
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
        //~//if( inipar->model_present )
        printk("[vencf::vencf8_dr] readDATA 0x%x", i);
#endif
#endif  
        while (1) {
            //Регистры состояния контактов датчиков
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH1_8, &er.c);
            g_encTmp[0].c[0] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH1_8, &er.c);
            g_encTmp[1].c[0] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH1_8, &er.c);
            g_encTmp[2].c[0] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH9_16, &er.c);
            g_encTmp[0].c[1] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH9_16, &er.c);
            g_encTmp[1].c[1] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH9_16, &er.c);
            g_encTmp[2].c[1] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH17_24, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH17_24, &er.c);
#endif  
            g_encTmp[0].c[2] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH17_24, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH17_24, &er.c);
#endif  
            g_encTmp[1].c[2] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH17_24, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH17_24, &er.c);
#endif  
            g_encTmp[2].c[2] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;

#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH25_32, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH25_32, &er.c);
#endif        
            g_encTmp[0].c[3] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH25_32, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH25_32, &er.c);
#endif        
            g_encTmp[1].c[3] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;
#ifdef USING_VDS_NEW_FW
            er.error = ReadBox3(ADR_CONTACTS_STATE_NEWFW_CH25_32, &er.c);
#else      
            er.error = ReadBox3(ADR_CONTACTS_STATE_CH25_32, &er.c);
#endif        
            g_encTmp[2].c[3] = ~er.c;
            g_iVencError[i].error |= er.error;
            if (er.error == BUSY_BOX)
                break;

#ifdef DEBUG_MODE_ON         
            if (++indx2 > VDS_WAIT_TIMEOUT) {
                printk("[vencf::vencf8_dr] ~~~ readData[%d]_TOut ~~~", i);
                g_iVencError[i].error |= VENCF_ERR_TIME_OUT;
                break;
            }
#endif    
            if ((g_encTmp[0].l != g_encTmp[1].l || g_encTmp[0].l != g_encTmp[2].l)) {
#ifdef DEBUG_MODE_ON         
                if (indx2 == 1)
                    printk("[vencf::vencf8_dr]  reading Data[%d]...", i);
#endif  
                VENCF_DELAY;
            }
            else
                break;
        }
        //while((g_encTmp[0].l != g_encTmp[1].l || g_encTmp[0].l != g_encTmp[2].l) /*&& ++indx2 < VDS_WAIT_TIMEOUT */ );          

#ifdef DEBUG_MODE_ON
        if (g_iVencError[i].error)
            printk("[vencf::vencf8_dr] Data[%d] READ failed err=0x%x", i, g_iVencError[i].error);
#endif

        if (er.error == BUSY_BOX) {
            tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_DATA_VDS1_16 | VENCF_ERR_DATA_VDS17_32;
            set_all_errs(tdrv, tdrv->error);
            // clear LATCH & BUS
#ifdef CHECK_WRITEPORT
            CLEAR_MEM
            WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
            if (ERR_MEM == SPAPS_OK)
#else
            WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
                {
                    WriteBox(ADR_BUS_FDS_CH1_8, 0);
                    WriteBox(ADR_LATCH_FDS_CH9_16, 0);
                }
            return;
        }

        g_sslCurCoordinate.l = (g_encTmp[0].l & g_encTmp[1].l) | (g_encTmp[1].l & g_encTmp[2].l) | (g_encTmp[0].l & g_encTmp[2].l);

        /*    #ifdef DEBUG_MODE_ON
            if( inipar->model_present &&  
                 g_sslCurCoordinate.l != 0x01010101ul && g_sslCurCoordinate.l != 0x02020202ul && g_sslCurCoordinate.l != 0x04040404ul && g_sslCurCoordinate.l != 0x08080808ul 
                  && g_sslCurCoordinate.l != 0x10101010ul && g_sslCurCoordinate.l != 0x20202020ul && g_sslCurCoordinate.l != 0x40404040ul && g_sslCurCoordinate.l != 0x80808080ul )
              printk("[vencf.dr] MALFUNC_DATA=0x%x er=0x%x", g_sslCurCoordinate.l, g_sslCurCoordinate.error ); 
            #endif  */

        g_sslSavCoord.error = g_iVencError[i].error;
        devdata->gray[i].l = (unsigned long) g_sslCurCoordinate.l & 0x00fffffful;
        devdata->gray[i].error = 0;
        g_sslSavCoord.l = decodegray(devdata->gray[i].l);

        if (g_encTmp[0].l != g_encTmp[1].l || g_encTmp[0].l != g_encTmp[2].l) {
            g_sslCurCoordinate.error = 0xff;
#ifdef DEBUG_MODE_ON
            printk("[vencf::vencf8_dr] RD errors coord=0x%x (0x%x)",
                    g_sslCurCoordinate.l, g_sslCurCoordinate.error);
            printk(" 0x%lx", g_encTmp[0].l);
            printk(" 0x%lx", g_encTmp[1].l);
            printk(" 0x%lx", g_encTmp[2].l);
#endif
        } else
            g_sslCurCoordinate.error = g_sslSavCoord.error;

#ifdef DEBUG_MODE_ON    
        if (OldCoord[i].l != g_sslCurCoordinate.l || g_sslCurCoordinate.error || g_sslSavCoord.error) {
            printk(" - - -  gray[%d]=%d 0x%x (0x%x)  - - - ", i, g_sslCurCoordinate.l,
                    g_sslCurCoordinate.l, g_sslCurCoordinate.error);

            printk(" + + +  coord[%d]=%d 0x%x (0x%x) + + + ", i, g_sslSavCoord.l,
                    g_sslSavCoord.l, g_sslSavCoord.error);
            OldCoord[i].l = g_sslCurCoordinate.l;
        }
#endif

        // TODO checkout the ERRORs processing!!!
        if (g_sslCurCoordinate.error) { // has got error
            if (++g_iVencError[i].i > ERRORS_COUNT_THRESHOLD) {
                devdata->venc[i].l = g_sslSavCoord.l;
                devdata->venc[i].error = g_sslCurCoordinate.error;
            }
        }
        else {
            g_iVencError[i].i = 0;
            devdata->venc[i].l = g_sslSavCoord.l;
            devdata->venc[i].error = 0;
        }
        // TODO not change while errors???
        ///    devdata->venc[i].l  = g_sslSavCoord.l;    

        j <<= 1; // shift BUS variable to the next BUS

#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
        //~//if( inipar->model_present )
        printk("[vencf::vencf8_dr] clear currBUS next BUS = %d", j);
#endif  
#endif

        //адрес модуля ФДС16р на миспа
#ifdef CHECK_WRITEPORT
        CLEAR_MEM
        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
        if (ERR_MEM == SPAPS_OK)
#else
        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif    
            { // clear BUS
                RH = WriteBox(ADR_BUS_FDS_CH1_8, 0);
            }
#ifdef CHECK_WRITEPORT
        else {
            RH = BUSY_BOX;
#ifdef DEBUG_MODE_ON                
            printk("[vencf::vencf8_dr] clr BUS - FDS not accessible (WritePort failed) mod=%d!", inipar->AdrFds);
#endif
        }
#endif
        if (RH) {
            tdrv->error |= VENCF_ERR_FDS_FAILED;
            if (RH == BUSY_BOX)
                tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
#ifdef DEBUG_MODE_ON
            printk("[vencf::vencf8_dr] FDS BUS clear err=0x%x drv.err=0x%x", RH, tdrv->error);
#endif
            set_all_errs(tdrv, tdrv->error);
            return;
        }

        if (inipar->model_present) { // wait for PTI to clear VALID 
#ifdef CHECK_WRITEPORT
            CLEAR_MEM
            WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
            if (ERR_MEM) {
                tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                set_all_errs(tdrv, tdrv->error);
#ifdef DEBUG_MODE_ON
                printk("[vencf::vencf8_dr] wait4clearVALID - SyncVds not accessible mod=%d!", inipar->AdrVdsSinc);
#endif
                return;
            }
#else
            WritePort(SPAPS_ADR_MISPA, inipar->AdrVdsSinc);
#endif

            g_sscRQbyte.c = 0;
            g_sscRQbyte.error = 0;
            // Wait until VALID is changed
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
            //~//if( inipar->model_present )
            printk("[vencf::vencf8_dr] wait4fallingVALID_byRQ");
#endif
#endif              
#ifdef USING_VDS_NEW_FW              
            vds32CheckData(ADR_CONTACTS_STATE_NEWFW_CH25_32, RQ_WAIT_MODE | RQ_NO_ERRORS, &g_sscRQbyte);
#else
            vds32CheckData(ADR_CONTACTS_STATE_CH1_8 + inipar->NumByteSinc, RQ_WAIT_MODE | RQ_NO_ERRORS, &g_sscRQbyte);
#endif
#ifdef DEBUG_MODE_ON
            if (g_sscRQbyte.error && g_sscRQbyte.error != NEGC_BOX)
                printk("[vencf::vencf8_dr] get_clrVALID_RQ err RQ.c=0x%x RQ.er=0x%x", g_sscRQbyte.c, g_sscRQbyte.error);
#endif      

            if (g_sscRQbyte.error & VENCF_ERR_CRIT_MALFUNC) {
                tdrv->error = VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                set_all_errs(tdrv, tdrv->error);
                return;
            }

#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
            if (inipar->model_present)
                printk("[vencf::vencf8_dr] wait4fallingVALID_byValue");
#endif
#endif      
            nIter = 0;
            indx2 = 0;
            while (indx2 < VDS_WAIT_TIMEOUT) {
                // синхронизация с ПТИ
#ifdef USING_VDS_NEW_FW
                er = vds32GetData(ADR_CONTACTS_STATE_NEWFW_CH25_32);
#else
                er = vds32GetData(ADR_CONTACTS_STATE_CH1_8 + inipar->NumByteSinc);
#endif          
                if (er.error) {
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] clrVALID_getData err=0x%x", er.error);
#endif
                    if (er.error == BUSY_BOX) { // clear LATCH&BUS          
                        tdrv->error |= VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_SYNC_VDS_FAILED;
                        set_all_errs(tdrv, tdrv->error);
#ifdef CHECK_WRITEPORT
                        CLEAR_MEM
                        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
                        if (ERR_MEM == SPAPS_OK)
#else
                        WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
                            {
                                WriteBox(ADR_BUS_FDS_CH1_8, 0);
                                WriteBox(ADR_LATCH_FDS_CH9_16, 0);
                            }
                        return;
                    }
                }
                else
                    if (!(er.c & inipar->MaskChanSinc)) {
                    if (++nIter > 3) {
                        // VALID is falled - lets continue                
#ifdef DEBUG_MODE_ON
#ifdef DEBUG_DRV_STAGES
                        //~//if( inipar->model_present )
                        printk("[vencf::vencf8_dr] VALID is falled");
#endif
#endif  
                        break;
                    }
                }
                else
                    nIter = 0;
#ifdef DEBUG_MODE_ON         
                if (++indx2 > VDS_WAIT_TIMEOUT) {
                    printk("[vencf::vencf8_dr] ~~~ wait4FallValid_TOut ~~~");
                    devdata->venc[i].error |= VENCF_ERR_TIME_OUT;
                    break;
                }
                if (indx2 == 1)
                    printk("[vencf::vencf8_dr] waiting for falling Valid...");
#endif          
                VENCF_DELAY;
            } // while 1
            /*    if( indx2 >= VDS_WAIT_TIMEOUT )
                  {
                    devdata->venc[i].error |= VENCF_ERR_SYNC_VDS_FAILED|VENCF_ERR_TIME_OUT;
                    #ifdef DEBUG_MODE_ON
                      printk("[vencf::vencf8_dr] wait4VALIDclr_tout="); 
                    #endif
                  }   */
        }//if inipar->model_present
        else
            delaymcs(inipar->puofftime);
    } //end for - end of encoders input

    // сброс Latch / BUS already falled down - but do it again - just in case
#ifdef CHECK_WRITEPORT
    CLEAR_MEM
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
    if (ERR_MEM == SPAPS_OK)
#else
    WritePort(SPAPS_ADR_MISPA, inipar->AdrFds);
#endif
        {
            RH = WriteBox(ADR_BUS_FDS_CH1_8, 0);
            if (RH != SPAPS_OK) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] Final BUS clear err=0x%x", RH);
#endif    
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
            }
            RH = WriteBox(ADR_LATCH_FDS_CH9_16, 0);
            if (RH != SPAPS_OK) {
                tdrv->error |= VENCF_ERR_FDS_FAILED;
                if (RH == BUSY_BOX) {
                    tdrv->error |= VENCF_ERR_CRIT_MALFUNC;
#ifdef DEBUG_MODE_ON
                    printk("[vencf::vencf8_dr] Final LATCH clear err=0x%x", RH);
#endif    
                    set_all_errs(tdrv, tdrv->error);
                    return;
                }
            }
        }
#ifdef CHECK_WRITEPORT
    else {
        tdrv->error |= VENCF_ERR_CRIT_MALFUNC | VENCF_ERR_FDS_FAILED;
#ifdef DEBUG_MODE_ON                
        printk("[vencf::vencf8_dr] Final clr L&B - FDS not accessible mod=%d!", inipar->AdrFds);
#endif
        set_all_errs(tdrv, tdrv->error);
    }
#endif

    // if( !inipar->model_present )
    //  delaymcs(100);

}
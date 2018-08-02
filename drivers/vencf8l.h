/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   vencf8l.h
 * Author: Mikl
 *
 * Created on 24 июля 2018 г., 12:25
 */

#ifndef VENCF8L_H
#define VENCF8L_H

#ifdef __cplusplus
extern "C" {
#endif

#define RQ_WAIT_MODE    (0x01)
#define RQ_NO_ERRORS    (0x02)
    
// тип модуля 
#define ADR_MODULE_TYPE           (0x04)
// регистр запроса обслуживания  
#define ADR_REQUEST_REG           (0x05)
#define VDS_WAIT_TIMEOUT          (64)
#define VDS_RQ_LO_DATA_READY      (0x01)
#define VDS_RQ_HI_DATA_READY      (0x10)
//Регистры состояния контактов датчиков
#define ADR_CONTACTS_STATE_CH1_8    (0x10)
#define ADR_CONTACTS_STATE_CH9_16   (0x11)
#define ADR_CONTACTS_STATE_CH17_24  (0x12)
#define ADR_CONTACTS_STATE_CH25_32  (0x13)

#define ADR_CONTACTS_STATE_NEWFW_CH17_24 (0x40)
#define ADR_CONTACTS_STATE_NEWFW_CH25_32 (0x41)

//Регистры обрывов линий связи с датчиками
#define ADR_BRAKEOFLINE_CH1_8       (0x14)
#define ADR_BRAKEOFLINE_CH9_16      (0x15)
#define ADR_BRAKEOFLINE_CH17_24     (0x16)
#define ADR_BRAKEOFLINE_CH25_32     (0x17)
//Регистры коротких замыканий линий связи с датчиками
#define ADR_SHORTLINE_CH1_8         (0x18)
#define ADR_SHORTLINE_CH9_16        (0x19)
#define ADR_SHORTLINE_CH17_24       (0x1A)
#define ADR_SHORTLINE_CH25_32       (0x1B)
//Регистры настройки времени антидребезга
#define ADR_ANTITREMBLE_CH1_16      (0x20)
//Регистры маски каналов
#define ADR_MASK_CH1_16             (0x21)
//Регистрыы статуса
#define ADR_STATUSREGS_CH1_16       (0x22)
//Регистры настройки времени антидребезга
#define ADR_ANTITREMBLE_CH17_32     (0x23)
//Регистры маски каналов
#define ADR_MASK_CH17_32            (0x24)
//Регистрыы статуса
#define ADR_STATUSREGS_CH17_32      (0x25)

    
#define VDS_RQ_INPUT1_CHANGED      (0x01)
#define VDS_RQ_BREAK_SHRT1_ERROR   (0x02)
#define VDS_RQ_REGS1_ERROR         (0x04)
#define VDS_RQ_CONFIG1_ERROR       (0x08)
#define VDS_RQ_INPUT2_CHANGED      (0x10)
#define VDS_RQ_BREAK_SHRT2_ERROR   (0x20)
#define VDS_RQ_REGS2_ERROR         (0x40)
#define VDS_RQ_CONFIG2_ERROR       (0x80)    
#define VDS_RQ_INPUTS_CHANGED      (VDS_RQ_INPUT1_CHANGED|VDS_RQ_INPUT2_CHANGED) 
#define VDS_RQ_CONFIG_ERROR        (VDS_RQ_REGS1_ERROR|VDS_RQ_REGS2_ERROR|VDS_RQ_CONFIG1_ERROR|VDS_RQ_CONFIG2_ERROR)        

#define VDS_STAT_INPUT1_CHANGED    (0x01)
#define VDS_STAT_INPUT2_CHANGED    (0x02)
#define VDS_STAT_INPUT_CHANGED     (VDS_STAT_INPUT1_CHANGED|VDS_STAT_INPUT2_CHANGED)
#define VDS_STAT_BREAK1_ERROR      (0x04)
#define VDS_STAT_BREAK2_ERROR      (0x08)    
#define VDS_STAT_BREAK_ERROR       (VDS_STAT_BREAK1_ERROR|VDS_STAT_BREAK2_ERROR)
#define VDS_STAT_SHORT1_ERROR      (0x10)
#define VDS_STAT_SHORT2_ERROR      (0x20)
#define VDS_STAT_SHORT_ERROR       (VDS_STAT_SHORT1_ERROR|VDS_STAT_SHORT2_ERROR)
#define VDS_STAT_MASK_ERROR        (0x40)
#define VDS_STAT_UNTTR_ERROR       (0x80)
#define VDS_STAT_CONFIG_ERROR      (VDS_STAT_MASK_ERROR|VDS_STAT_UNTTR_ERROR)


//FDS addr
// регистр вывода сигналов каналов 1-8  
#define ADR_BUS_FDS_CH1_8           (0x01)
// регистр вывода сигналов каналов 9-16  
#define ADR_LATCH_FDS_CH9_16        (0x02)
// регистр исправности каналов 1-8  
#define ADR_ISPR_FDS_CH1_8          (0x03)
// регистр исправности каналов 9-16  
#define ADR_ISPR_FDS_CH9_16         (0x0F)
//Регистры состояния выходов
//#define AdrSost14        0x05 // регистр состояния каналов 1-4   
//#define AdrSost58        0x06 // регистр состояния каналов 5-8   
//#define AdrSost912       0x07 // регистр состояния каналов 9-12   
//#define AdrSost1316      0x08 // регистр состояния каналов 13-16   

/*
 //=========================================================== 0001  1
 //диагностика модуля                                          0010  2
 //структура байта достоверности                               0011  3
 //                                                            0100  4
 //  Бит         Значение                                      0101  5
 //                                                            0110  6
 //   0   -   ошибки настройки вдс                             0111  7
 //   1   -   таймаут                                          1001  9
 //   2   -   ошибки каналов 1-16    вдс                       0111  7
 //   3   -   ошибка каналов 17 - 32 вдс                       1001  9
 //   4   -   ошибка фдс каналов 1-9                           1011  B
 //   5   -   ошибка вдс синхронизации                         1100  C
 //   6   -   ошибка типа модуля                               1101  D
 //   7   -   критическая ошибка или нет доступа к ПЯ          1110  E
 //                                                            1111  F
 //===========================================================
*/

#define VENCF_ERR_SETUP_VDS       0x01
#define VENCF_ERR_TIME_OUT        0x02
#define VENCF_ERR_DATA_VDS1_16    0x04
#define VENCF_ERR_DATA_VDS17_32   0x08
#define VENCF_ERR_FDS_FAILED      0x10
#define VENCF_ERR_SYNC_VDS_FAILED 0x20
#define VENCF_ERR_MOD_TYPE        0x40
#define VENCF_ERR_CRIT_MALFUNC    0x80

#define TOTAL_ENCODERS_COUNT      (8) 
#define ERRORS_COUNT_THRESHOLD    (2)

typedef struct __attribute__((packed))
{
  unsigned char   typeVds;       // тип вдс32р                                   
  unsigned short  BoxLen;        // длина ПЯ, уменьшенная на 1                   
  unsigned char   vip;           // флаг критически важного для системы модуля   
  unsigned char   AdrFds;        // Адрес ФДС на МИСПА  
  unsigned char   typeFds;       // тип Фдс16р                                   
  unsigned char   model_present; // флаг работы с моделью                        
  unsigned char   AdrVdsSinc;    // Адрес Вдс синхронизации с моделью на миспа   
  unsigned char   NumByteSinc;   // Номер байта канала синхронизации с моделью   
  unsigned char   MaskChanSinc;  // маска канала синхронизации с моделью         
  unsigned short  puontime;      // 1850 время появления данных в мкс                 
  unsigned short  datareadtime;  // 250 время стояния данных в мкс                   
  unsigned short  puofftime;     // 350 время исчезания данных в мкс                 
  unsigned char   workok;                                                        
} vencf8_inipar;

typedef union  { 
    unsigned long l;
    unsigned short i[2];
    unsigned char  c[4];
  } enc_value_t;

typedef struct __attribute__((packed))
{  
  sslong        gray[TOTAL_ENCODERS_COUNT];       // исходная - код Грея 
  sslong        venc[TOTAL_ENCODERS_COUNT];       // декодированная   
} vencf8_data;

#define VENCF 0x2

void vencf8_ini(table_drv* drv);
void vencf8_dr(table_drv* drv);
sschar vds32GetStat(unsigned char nModIdA, unsigned char nAddr);
sschar vds32GetRq(unsigned char nModIdA);
unsigned char vds32CheckData(unsigned char nByteAddrA, unsigned char nWaitForDataA, pschar pnRQbyteA);
sschar vds32GetData(unsigned char nByteAddrA);    


#ifdef __cplusplus
}
#endif

#endif /* VENCF8L_H */


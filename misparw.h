/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   misparw.h
 * Author: rusin
 *
 * Created on 13 февраля 2018 г., 13:07
 */

#ifndef MISPARW_H
#define MISPARW_H
#define ADR_BOX     0xD0000
#define SIZE_BOX    0xff

#define MRQ         0x05    //; запрос на обслуживание
#define SV          0x06    //; флаг захвата ПЯ со стороны ФП
#define SVE         0x07    //; флаг завершения обслуживания ПЯ
#define MASTER      0x08    //; флаг задатчика времени
#define newtime     0x09    //; флаг нового времени
#define NOTINV      0x0A    //; слово - счетчик количества неинверсий ПЯ
//; обнаруженных модулем
#define NINVFP      0x0C    //; слово - счетчик количества неинверсий ПЯ
//; обнаруженных ФП
#define CT_GLOB     0x0e    //; счетчик переворотов ПЯ
#define MSTAT        0x0f    //; байт состояния модуля
#define FLNET       0x10    //; длина таблицы сетевых передач
#define ININET      0x12    //; запрос на инициализацию сетевого процессора

#define BUSY_BOX    0x80
#define NEGC_BOX    0xC0

typedef unsigned char uspaint8;
typedef signed char spaint8;

//#define delaymcs(x) DelaySec(((double)(x))/1000000.0)
//#define delaymcs(x)  inline sleep_spa(int ms){

typedef struct {
  uspaint8 c;
  uspaint8 error;
}sschar;
typedef struct {
  uspaint8 b;
  uspaint8 error;
}ssbool;

int init_memory(void);
void free_memory(void);
void SetBoxLen (int);

unsigned char ReadPort( int);
void WritePort(int , unsigned char );

int WriteBox(unsigned char ptr,unsigned char value);
int ReadBox(unsigned char ptr,unsigned char *value);
int ReadSinglBox(unsigned char ptr,unsigned char *value);
int WriteSinglBox(unsigned char ptr, unsigned char value);
int ReadBx3w(unsigned char ptr,unsigned char *value);
int ReadBox3(unsigned char ptr, unsigned char *value);
int CatchBox(void);
int FreeBox(void);
unsigned long decodegray(unsigned long k);
void delaymcs(int x);
#endif /* MISPARW_H */


/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   dev.h
 * Author: rusin
 *
 * Created on 1 февраля 2018 г., 11:06
 */

#ifndef DEV_H
#define DEV_H

#include <linux/fs.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include "dspadef.h"

#include "misparw.c"
#include "drivers/fds16r.c"
#include "drivers/vds32r.c"
#include "drivers/vas84r.c"

typedef struct {
    int lenght; // размер буфера ввода/вывода
    unsigned char *inimod; // данные инициализации в области данных пользователя
    unsigned char *data; // область ввода-вывода данных в пространнстве пользователя
    void * init;
    void * step;
    char * td; // указатель в пространстве памяти на структуру данных table_drv

} user_area;

typedef struct {
    int type;
    void (*init)(table_drv*);
    void (*step)(table_drv*);
} type_drivers;

static type_drivers tab_t[MAX_DRIVERS] = {
    { FDS16R,
        &fds16r_ini,
        &fds16r_dw,},

    { VDS32R,
        &vds32r_ini,
        &vds32r_dw,},

    { VAS84R,
        &vas84r_ini,
        &vas84r_dw,},

    {-1, NULL, NULL},
};
static user_area drv_len_data[MAX_DRIVERS];
static table_drv table_drvs[MAX_DRIVERS];

#endif /* DEV_H */


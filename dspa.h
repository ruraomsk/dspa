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




MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yury Rusinov <ruraomsk@list.ru>");
MODULE_VERSION("1.0");
#define MAX_DRIVERS 100
static int drv_count=0;
static unsigned int irq_count;
#include "dspadef.h"

static unsigned int drv_len_data[MAX_DRIVERS];
static table_drv table_drvs[MAX_DRIVERS];    

#include "drivers/testdrv1.c"
#include "drivers/testdrv2.c"
#include "drivers/testdrv3.c"

#endif /* DEV_H */


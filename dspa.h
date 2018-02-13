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



#include "drivers/testdrv1.c"
#include "drivers/testdrv2.c"
#include "drivers/testdrv3.c"

#endif /* DEV_H */


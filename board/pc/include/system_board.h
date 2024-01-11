/*
Copyright (c) 2019-2023 Aysi 773917760@qq.com. All right reserved
Official site: www.mtf123.club

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

It under the terms of the Apache as published;
either version 2 of the License,
or (at your option) any later version.
*/

#ifndef _SYSTEM_BOARD_H
#define _SYSTEM_BOARD_H

#include "types_base.h"

/******************************************************
 * Note: F1C100S is big endian
 * FLASH空间划分(v1.0): 0~892KB 程序储存;
 *                     894~894.25KB 激活信息(256B);
 *                     894.3~894.6KB 设备信息(256B);
 *                     894.7~895KB 触屏信息(256B);
 *                     896~928KB cfg文件;
 *                     928~1024KB 临时储存;
 * **************************************************/

/*FLASH ADDR*/
#define JSON_FLASH_ADDR (896*1024) //前896K用于程序储存
#define JSON_FLASH_FLAG 0X13 //文件标记,确认为用户写入

void active_warning(void);
uint8_t check_active_state(void);
void personalise_function(void);
void FilesRenew(void);
void licence_timing_check(void);

#endif

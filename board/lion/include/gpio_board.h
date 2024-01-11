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

#ifndef _GPIO_BOARD_H
#define _GPIO_BOARD_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "GPIO_port.h"

void gpio_test(void);

/****************************
应用定义接口
*****************************/
//ctiic.c io定义
#ifdef __MYCT_IIC_H

//IO方向设置
#define CT_SDA_IN()  MTF_GPIO_SetDir(MTF_GPIO_A, 2, GPIO_DIRECTION_INPUT)
#define CT_SDA_OUT() MTF_GPIO_SetDir(MTF_GPIO_A, 2, GPIO_DIRECTION_OUTPUT)

//IO操作函数	 
#define CT_IIC_SCL(x)   MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_1, x)		//SCL     
#define CT_IIC_SDA(x)    MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_2, x) //SDA	 
#define CT_READ_SDA   MTF_GPIO_ReadPin(MTF_GPIO_A, MTF_PIN_2)	//输入SDA

static __INLINE void CT_IIC_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_1; //scl
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_2; //sda
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);
}
#endif
///////////////////////////////

//gt9147.c io定义
#ifdef __GT9147_H

#define GT_RST(x)    MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_3, x)	//GT9147复位引脚
#define GT_INT    	MTF_GPIO_ReadPin(MTF_GPIO_A, MTF_PIN_0)	//GT9147中断引脚
#define GT_INT_SET(x)    MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_0, x) //GT9147中断引脚

static __INLINE void GT9147_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_3; //ret
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_0; //int
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);
}

static __INLINE void GT9147_int_pin_set(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_0; //int
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_NOPULL;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);
}

#endif
//////////////////////////////

//ft5206.c io定义
#ifdef __FT5206_H

#define FT_RST(x)    	MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_0, x)	//FT5206复位引脚
#define FT_INT    	MTF_GPIO_ReadPin(MTF_GPIO_A, MTF_PIN_3)	//FT5206中断引脚

static __INLINE void FT5206_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_0; //ret
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_3; //int
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);
}

#endif
/////////////////////////////

//ott2001a.c IO定义
#ifdef __OTT2001A_H

#define OTT_RST(x)   MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_0, x) //OTT2001A复位引脚
#define OTT_INT    	MTF_GPIO_ReadPin(MTF_GPIO_A, MTF_PIN_3)	//OTT2001A中断引脚	

static __INLINE void OTT2001A_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_0; //ret
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_3; //int
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);
}

#endif
////////////////////////////

//system_board.c IO定义
#ifdef _SYSTEM_BOARD_H

#define TF_CHECK() MTF_GPIO_ReadPin(MTF_GPIO_E, MTF_PIN_4) //TF卡检查脚

static __INLINE void system_board_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_4; // TF卡检查脚
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_PULLDOWN;
  MTF_GPIO_Init(MTF_GPIO_E, &GPIO_Initure);
}

#endif
////////////////////////////

//main.c IO定义
#ifdef _SAGITTARIUS_H

static __INLINE void main_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_2; //UART BUSY脚, 485 RE/DE脚
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_NOPULL;
  MTF_GPIO_Init(MTF_GPIO_E, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_3; //USB ID脚
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_E, &GPIO_Initure);
}

#endif
////////////////////////////

//dgus2Com.c IO定义
#ifdef _DGUSII_COM_H
#define RDorDE_PIN(x) MTF_GPIO_WritePin(MTF_GPIO_E, MTF_PIN_2, x) //485 RD or DE PIN
#endif
///////////////////////////

//T5UIC2_agreement.c IO定义
#ifdef _T5UIC2_H
#define T5L_BUSY_PIN(x) MTF_GPIO_WritePin(MTF_GPIO_E, MTF_PIN_2, x)
#endif
///////////////////////////

//MTF_ComProtocol.c
#ifdef _MTF_COMPROTOCOL_H
#define RDorDE_PIN(x) MTF_GPIO_WritePin(MTF_GPIO_E, MTF_PIN_2, x) //485 RD or DE PIN
#endif
//////////////////////////

//XPT2046.c
#ifdef _XPT2046_H

#define PEN MTF_GPIO_ReadPin(MTF_GPIO_F, MTF_PIN_1)	//INT
#define TDOUT MTF_GPIO_ReadPin(MTF_GPIO_A, MTF_PIN_3)	//MISO
#define TDIN(x) MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_1, X) //MOSI
#define TCLK(x) MTF_GPIO_WritePin(MTF_GPIO_A, MTF_PIN_2, X) //SCLK
#define TCS(x) MTF_GPIO_WritePin(MTF_GPIO_F, MTF_PIN_0, X) // CS

static __INLINE void XPT2046_IO_init(void)
{
  MTF_GPIO_Type GPIO_Initure;

  GPIO_Initure.Pin = MTF_PIN_1;
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_F, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_3;
  GPIO_Initure.Mode = MTF_PIN_MODE_INPUT;
  GPIO_Initure.Pull = MTF_PIN_NOPULL;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_1;
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_NOPULL;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_2;
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_NOPULL;
  MTF_GPIO_Init(MTF_GPIO_A, &GPIO_Initure);

  GPIO_Initure.Pin = MTF_PIN_0;
  GPIO_Initure.Mode = MTF_PIN_MODE_OUTPUT_OD;
  GPIO_Initure.Pull = MTF_PIN_PULLUP;
  MTF_GPIO_Init(MTF_GPIO_F, &GPIO_Initure);
}

#endif
//////////////////////////

#define TEST_TOGGLE_PIN() MTF_GPIO_TogglePin(MTF_GPIO_E, MTF_PIN_2) //测试引脚, 和busy共用

#ifdef __cplusplus
}
#endif

#endif

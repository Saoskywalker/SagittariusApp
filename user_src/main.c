#include "Sagittarius.h"
#include "Sagittarius_global.h"
#include "MTF_io.h"
#include "ROM_port.h"
#include "RAM_port.h"
#include "gpio_board.h"
#include "uart_board.h"
#include "usb_port.h"
#include "timer_port.h"
#include "watch_dog_port.h"
#include "delay.h"
#include "piclib.h"	
#include "touch.h"
#include "text.h"
#include "UI_engine.h"
#include "cJSON_extend.h"
#include "ComPort.h"
#include "HMI_Command.h"
#include "MTF_HMI.h"
#include "file_type.h"
#include "beep.h"
#include "Sagittarius_timer.h"
#include "music_play.h"
#include "system_port.h"
#include "system_board.h"

#define Sagittarius_debug(...) //printf(__VA_ARGS__)

static int user_text_backup, user_direction = 0;
static int user_touch_switch, user_touch_mode, user_touch_down_keep_up; //触摸配置
static u8 touch_switch_data[8*1024] = {0}; //HMIConfig.bin文件缓存

void cfg_load(void);
void touch_process(void);

/*T5L和扩展命令运行*/
static u8 Buffer_command[T5_BUFF_LEN];
static int T5UIC2_commandNum = 0;
void T5L_Command_Run(u8 *data, int *num);
void PLAY_MUSIC_process(u8 *data, int *num);
void SHOW_PIC_process(u8 *data, int *num);

#if 0

/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Used as a loop counter to create a very crude delay. */
#define mainDELAY_LOOP_COUNT		( 0xffffff )

/* The task functions. */
void vTask1( void *pvParameters );
void vTask2( void *pvParameters );

/*-----------------------------------------------------------*/

int test_freertos( void )
{
	/* Create one of the two tasks. */
	xTaskCreate(	vTask1,		/* Pointer to the function that implements the task. */
					"Task 1",	/* Text name for the task.  This is to facilitate debugging only. */
					1000,		/* Stack depth - most small microcontrollers will use much less stack than this. */
					NULL,		/* We are not using the task parameter. */
					1,			/* This task will run at priority 1. */
					NULL );		/* We are not using the task handle. */

	/* Create the other task in exactly the same way. */
	xTaskCreate( vTask2, "Task 2", 1000, NULL, 1, NULL );

	/* Start the scheduler to start the tasks executing. */
	vTaskStartScheduler();	

	/* The following line should never be reached because vTaskStartScheduler() 
	will only return if there was not enough FreeRTOS heap memory available to
	create the Idle and (if configured) Timer tasks.  Heap management, and
	techniques for trapping heap exhaustion, are described in the book text. */
	for( ;; );
	return 0;
}
/*-----------------------------------------------------------*/

void vTask1( void *pvParameters )
{
const char *pcTaskName = "Task 1 is running\r\n";
volatile uint32_t ul;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task. */
		Sagittarius_debug("%s", pcTaskName );

		/* Delay for a period. */
		for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
		{
			/* This loop is just a very crude delay implementation.  There is
			nothing to do in here.  Later exercises will replace this crude
			loop with a proper delay/sleep function. */
		}
	}
}
/*-----------------------------------------------------------*/

void vTask2( void *pvParameters )
{
const char *pcTaskName = "Task 2 is running\r\n";
volatile uint32_t ul;

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		/* Print out the name of this task. */
		Sagittarius_debug("%s", pcTaskName );

		/* Delay for a period. */
		for( ul = 0; ul < mainDELAY_LOOP_COUNT; ul++ )
		{
			/* This loop is just a very crude delay implementation.  There is
			nothing to do in here.  Later exercises will replace this crude
			loop with a proper delay/sleep function. */
		}
	}
}

#endif

__weak uint8_t hardware_init_weak(void)
{
    return 0;
}

__weak uint8_t system_exit_weak(void)
{
    return 0;
}

__weak uint8_t system_process_weak(void)
{
    return 0;
}

void hardware_init(void)
{
    hardware_init_weak(); //some platform need set at start

    main_IO_init(); //system io set to safety state and some pin init at start

    MTF_RAM_init(); /* Do initial mem pool */
    MTF_ROM_init(); //flash init

#ifdef _USER_DEBUG
    uart_hmi.addr = UART_D_HMI;                      //串口端口
    uart_hmi.Init.BaudRate = 115200;                   //波特率
    uart_hmi.Init.WordLength = MTF_UART_WORDLENGTH_8B; //字长为8位数据格式
    uart_hmi.Init.StopBits = MTF_UART_STOPBITS_1;      //一个停止位
    uart_hmi.Init.Parity = MTF_UART_PARITY_NONE;       //无奇偶校验位
    uart_hmi.Init.HwFlowCtl = MTF_UART_HWCONTROL_NONE; //无硬件流控
    uart_hmi.Init.Mode = MTF_UART_MODE_TX_RX;          //收发模式
    MTF_UART_Init(&uart_hmi);                          //uart init, note: use to debug
#endif
Sagittarius_debug("BCB...\r\n");
    // beep_init(); //beep init
    LCD_Init(); //lcd driver init
    MTF_timer_init_handle(); //timer init
    MTF_watch_dog_init(); //start WDOG
    Sagittarius_debug("BCB...\r\n");
}

void app_init(void)
{
    dirPath = "./MTF/"; //默认路径
    personalise_function();
}

int main(int argc, char **argv) 
{
    u8 start_command[] = {PLAY_MUSIC, 120, 0XFF, 0XAA};
    char res = 0;
    u8 keyDownLock = 0;

    hardware_init();
    app_init();

    delay_ms(35); //延时点亮背光, 避免显示没准备好而闪烁
    LCD_BackLightSet(100); //点亮背光

    cfg_load(); //载入ini文件,并初始化系统

    if (HMI_parsed != NULL)
    {
        //HMI mode executes the run code at start
        HMI_run_code(HMI_parsed,
                     json_read_string(HMI_parsed, "run", "FFFFFF", &res));
    }
    else
    {
        //command mode executes this at start
        PLAY_MUSIC_process(&start_command[0], NULL); //开机播放120号音乐
        start_command[0] = SHOW_PIC;
        start_command[1] = 0;
        SHOW_PIC_process(&start_command[0], NULL); //开机显示0号图片
    }

    FilesRenew(); //文件更新

    while (1)
    {
        if (system_process_weak() == 1)
        {
            system_exit_weak();
            break;
        }

        // _WAV_Play2(); //放于定时中断
        if(Sagittarius_timer_flag._6s)
        {
            Sagittarius_timer_flag._6s = 0;
        }
        if(Sagittarius_timer_flag._450ms)
        {
            Sagittarius_timer_flag._450ms = 0;
/* 
            //程序没更新的,这里更新
            //note: 自适应需用此更新显示
            if (LCD_disAdaptFlag && LCD_renewRequire && framebuffer_var_info.scale_flag == 0)
                UI_disAdaptScale(&LCD_rectNow, &LCD_disAdapt, adaptScaleX, adaptScaleY);
            LCD_renewRequire = 0;
             */
        }
        if(Sagittarius_timer_flag._9ms)
        {
            Sagittarius_timer_flag._9ms = 0;
            if (HMI_parsed != NULL)
            {
                if (!((keyDownLock == 1) && (UI_pressState)))
                {
                    keyDownLock = 0; //锁定后, 只有先松开, 才能正常使用触摸处理
                    HMI_touch_process(UI_pressTouchX, UI_pressTouchY, UI_pressState);
                }
            }
            else
            {
                touch_process();
            }

            gif_decode_loop();
            licence_timing_check();
        }
        if(Sagittarius_timer_flag._3ms)
        {
            Sagittarius_timer_flag._3ms = 0;
            while (ComModel.read(&Buffer_command[0], &T5UIC2_commandNum) == 0) //读指令
            {
                //DGUSII模式下, 防止有按键效果时, 当按下时, 后面用户指令显示的区域和按键效果区域重叠,
                // 造成松开后, 按键自动还原其区域, 将用户设置的显示破坏
                // (主要是全屏显示图片, 也可只针对其处理)
                if (HMI_parsed != NULL && keyDownLock == 0)
                {
                    keyDownLock = 1;            //模拟松开锁定, 防止正常触摸处理错误
                    HMI_touch_process(0, 0, 0); //模拟触摸松开
                }
                T5L_Command_Run(&Buffer_command[0], &T5UIC2_commandNum);
            }
            TP_Adjust_trigger(); //约24S检查周期
        }
        delay_ms(1);
    }

    return 0;
}

//cfg配置错误, 刷除FLASH上的CFG文件, 待重新上电时更新CFG文件(仅cfg_load()使用)
static void cfg_error_renew(void)
{
    MTF_remove("./FLASH/MTF/");
    Show_Str2(0, 16, (u8 *)"Please renew cfg.json (+_+)?", 16, 0);
    LCD_Exec();
}

//载入ini文件,并初始化系统
void cfg_load(void)
{
    int ini_int = 0;
    int ini_uart = 0, ini_check_sum = 0;
    char *strings = "", *str_temp = "";

    mFILE *f_bin = NULL; //user_touch_switch定义
    char path[PATH_LEN];
    u8 res = 0;
    long size = 0;
    cJSON *parsed = NULL, *item = NULL;

    memset(path, 0, sizeof(path));
    strcat(path, dirPath);
    strcat(path, "cfg.json");
    parsed = json_file_parse(path);
    if (parsed != NULL)
    {
        item = cJSON_GetObjectItem(parsed, "INIT");
        if(item!=NULL)
        {
            //打授权信息
            strings = json_read_string(item, "Copyright", "none", (char *)&res);
            if (strcmp(strings, "none") != 0)
            {
                POINT_COLOR=LIGHT_RED; 
                Show_Str2(0, 0, (u8 *)"Copyright Inc MTF company (Aysi)", 24, 0);
                Show_Str2(0, 24, (u8 *)"All rights reserved", 24, 0); 
                LCD_Exec();
                delay_ms(3000);
            }

            //核对版本和协议
            strings = json_read_string(item, "sys", "none vvv", (char *)&res);
            if (strcmp(strings, "none") != 0)
            {
                POINT_COLOR=LIGHT_RED; 
                Show_Str2(0, 0, (u8 *)"sys software version error", 16, 0); 
                LCD_Exec();
                cfg_error_renew();
                while (1);
            }
            strings = json_read_string(item, "gui", "dalta vvv", (char *)&res);
            if (strcmp(strings, "dalta_HMI") == 0) //组态模式
            {
                memset(path, 0, sizeof(path));
                strcat(path, dirPath);
                strcat(path, "HMIConfig.json");
                HMI_parsed = json_file_parse(path);
                strings = json_read_string(HMI_parsed, "version", "dalta vvv", (char *)&res);
                if (HMI_parsed == NULL || strcmp(strings, "MTF_HMI") != 0)
                {
                    POINT_COLOR = LIGHT_RED;
                    Show_Str2(0, 0, (u8 *)"gui version error", 16, 0);
                    LCD_Exec();
                    cfg_error_renew();
                    while (1);
                }
            }
            else if (strcmp(strings, "dalta") == 0) //指令模式
            {

            }
            else
            {
                POINT_COLOR=LIGHT_RED; 
                Show_Str2(0, 0, (u8 *)"gui version error", 16, 0); 
                LCD_Exec();
                cfg_error_renew();
                while (1);
            }

            /*通信配置*/
            ini_check_sum = json_read_bool(item, "check_sum", 0, (char *)&res);
            ini_uart = json_read_int(item, "com_band", 115200, (char *)&res);

            strings = json_read_string(item, "connect_way", "UART", (char *)&res);
#ifndef _USER_DEBUG
            str_temp = strrchr(strings, '_'); //获取uart端口号
            if (str_temp != NULL)
            {
                str_temp++;
                uart_hmi.addr = *str_temp - '0'; //串口端口
            }
            else
            {
                uart_hmi.addr = UART_D_HMI;                      //串口端口
            }
            uart_hmi.Init.BaudRate = (uint32_t)ini_uart;       //波特率
            uart_hmi.Init.WordLength = MTF_UART_WORDLENGTH_8B; //字长为8位数据格式
            uart_hmi.Init.StopBits = MTF_UART_STOPBITS_1;      //一个停止位
            uart_hmi.Init.Parity = MTF_UART_PARITY_NONE;       //无奇偶校验位
            uart_hmi.Init.HwFlowCtl = MTF_UART_HWCONTROL_NONE; //无硬件流控
            uart_hmi.Init.Mode = MTF_UART_MODE_TX_RX;          //收发模式
            MTF_UART_Init(&uart_hmi);                          //uart init
#endif
            if (strcmp(strings, "USB") == 0) //USB虚拟串口通信, 默认UART0端口通信
            {
                USB_UART_INIT; //因软件架构原因, UART必须再此前开启
            }
            strings = json_read_string(item, "agreement", "T5 vvv", (char *)&res);
            if (strcmp(strings, "T5l") == 0) //T5L指令模式
            {
                T5UIC2_init((u8)ini_check_sum);
                ComModel.read = T5UIC2_read;
                ComModel.send = T5UIC2_sendData;
                ComModel.recProcess = T5UIC2_recData;
            }
            else if (strcmp(strings, "MTF_Protocol") == 0) //MTF通信模式
            {
                MTF_Com_init(0XA1, 0X66, (u8)ini_check_sum);
                ComModel.read = MTF_Com_read;
                ComModel.send = MTF_Com_sendData;
                ComModel.recProcess = MTF_Com_recData;
            }
            else if (strcmp(strings, "DGUSII") == 0) //DGUS通信模式
            {
                DGUSII_Com_init(0X5A, 0XA5, (u8)ini_check_sum);
                ComModel.read = DGUSII_Com_read;
                ComModel.send = DGUSII_Com_sendData;
                ComModel.recProcess = DGUSII_Com_recData;
            }
            else if (strcmp(strings, "Modbus_Slave") == 0) //modbus从机模式
            {
            }
            else if (strcmp(strings, "Modbus_Master") == 0) //modbus主机模式
            {
            }
            else if (strcmp(strings, "RaspberryPi") == 0) //树莓派屏幕模式
            {
            }
            else
            {
                POINT_COLOR=LIGHT_RED; 
                Show_Str2(0, 0, (u8 *)"agreement version error", 16, 0); 
                LCD_Exec();
                cfg_error_renew();
                while (1);
            }

            //这里FLASH源结果肯定相同并前面已更新文件, 这用于当为SD源, 将SD和flash上的cfg对比
            if(strcmp(dirPath, "./MTF/")==0) //当前标记为SD源
            {
                strings = json_read_string(item, "file_source", "none", (char *)&res); //读SD标记
                if (strcmp(strings, "FLASH") == 0)
                {
                    cfg_error_renew();
                    Show_Str2(0, 0, (u8 *)"cfg.json will renew after reboot :)", 16, 0); 
                    LCD_Exec();
                    while (1);
                }
                else if (strcmp(strings, "SD") == 0)
                {
                    
                }
                else 
                {
                    Show_Str2(0, 0, (u8 *)"file source error", 16, 0); 
                    LCD_Exec();
                    while (1);
                }
            }

            if (strcmp(dirPath, "./FF/MTF/") == 0) //当前标记为FLASH源
            {
                ini_int = json_read_bool(item, "flash_format", 0, (char *)&res);
                if (ini_int != 0) //低格式化FATFS
                {
                    POINT_COLOR = LIGHT_RED;
                    Show_Str2(0, 0, (u8 *)"Bottom Formatting...(long time, please wait)", 16, 0);
                    LCD_Exec();
                    cfg_error_renew();
                    MTF_ROM_user_data_erase();
                    Show_Str2(0, 0, (u8 *)"Bottom Format complete(please reboot system)", 16, 0);
                    LCD_Exec();
                    while (1);
                }

                beep_enable = json_read_bool(item, "beep_output", 0, (char *)&res);
                if (beep_enable) //使用蜂鸣器
                {
                    beep_control = json_read_bool(item, "beep_control", 0, (char *)&res);
                }
            }

            /*显示参数初始化*/
            user_direction = json_read_int(item, "display_direction", 0, (char *)&res); //显示方向设置
            if(user_direction==90)
                LCD_Display_Dir(DIS_DIR_90);
            else if(user_direction==180)
                LCD_Display_Dir(DIS_DIR_180);
            else if(user_direction==270)
                LCD_Display_Dir(DIS_DIR_270);
            else 
                LCD_Display_Dir(DIS_DIR_0);
            tp_dev.init(); //touch init, note need flash initialized, and dir init
            piclib_init(); //初始化解码画图
            UI_init(); //gui init
            if(check_active_state()) //检查激活
                active_warning(); //激活警告

            framebuffer_var_info.xres_virtual = json_read_int(item, "width", lcddev.width, (char *)&res);
            framebuffer_var_info.yres_virtual = json_read_int(item, "height", lcddev.height, (char *)&res);
            if(framebuffer_var_info.yres_virtual!=lcddev.height||framebuffer_var_info.xres_virtual!=lcddev.width)
            {
                //if(framebuffer_var_info.yres_virtual<=600&&framebuffer_var_info.xres_virtual<=800&&
                //   lcddev.height<=600&&lcddev.width<=800)
                    //note: 注意内存过大
                    Sagittarius_debug("adapt: %d\r\n", UI_disAdapt(framebuffer_var_info.xres_virtual, framebuffer_var_info.yres_virtual));
            }

            /*操作设置*/
            user_text_backup = json_read_bool(item, "text_backup", 1, (char *)&res);
            user_touch_mode = json_read_bool(item, "touch_mode", 1, (char *)&res);
            user_touch_down_keep_up = json_read_bool(item, "touch_down_keep_up", 0, (char *)&res);
            user_touch_switch = json_read_bool(item, "touch_switch", 0, (char *)&res);
            //背光自动控制
            back_light_control_enable = json_read_bool(item, "back_light_control", 0, (char *)&res);
            if(back_light_control_enable)
            {
                //获取背光自动控制参数
            }

            if(user_touch_switch)
            {
                memset(path, 0, sizeof(path));
                strcat(path, dirPath);
                strcat(path, "HMIConfig.bin");
                f_bin = MTF_open(path, "rb");
                if (f_bin != NULL)
                {
                    size = MTF_size(f_bin);
                    if (size > 0)
                    {
                        if (size <= 1024 * 8 - 32) //touch_switch_data为8KB
                            MTF_read(touch_switch_data, 1, size, f_bin);
                    }
                    else
                    {
                        touch_switch_data[0] = 0XFF; //touch_switch_data结束标志;
                        touch_switch_data[1] = 0XFF;
                    }
                    MTF_close(f_bin);
                }
                else
                {
                    touch_switch_data[0] = 0XFF;
                    touch_switch_data[1] = 0XFF;
                }
            }
            
            page_num = 0;
        }
        else //失败, 不运行
        {
            POINT_COLOR=LIGHT_RED; 
            Show_Str2(0, 0, (u8 *)"Without cfg.json, sys shutdown :(", 24, 0); 
            LCD_Exec();
            while (1);
        }
    } 
    else //失败, 不运行
    {
        POINT_COLOR=LIGHT_RED; 
        Show_Str2(0, 0, (u8 *)"Without cfg.json, sys shutdown :(", 24, 0); 
        LCD_Exec();
        while (1);
    }
    cJSON_Delete(parsed);
}

void path_product(const char *dir, const u8 *num, const char *type, char *result)
{
    char i[PATH_LEN];
    memset(i, 0, sizeof(i));
    sprintf(i, "%d", *num);
    result[0] = 0;
    strcat(result, dir);
    strcat(result, i);
    strcat(result, type);
}

u8 pic_type_adapt(const char *dir, const u8 *num1, const u8 *num2, char *result)
{
    char i[8] = {0};
    char k[PATH_LEN] = {0};
    mFILE *ff = NULL;

    if(num2==NULL)
        sprintf(i, "%d", *num1);
    else
        sprintf(i, "%d_%d", *num1, *num2);

    result[0] = 0;
    strcat(result, dir);
    k[0] = 0;
    strcat(k, i);
    strcat(k, ".bmp");
    strcat(result, k);
    ff = MTF_open(result, "rb");
    if (ff != NULL)
    {
        MTF_close(ff);
        return T_BMP;
    }

    result[0] = 0;
    strcat(result, dir);
    k[0] = 0;
    strcat(k, i);
    strcat(k, ".png");
    strcat(result, k);
    ff = MTF_open(result, "rb");
    if (ff != NULL)
    {
        MTF_close(ff);
        return T_PNG;
    }

    result[0] = 0;
    strcat(result, dir);
    k[0] = 0;
    strcat(k, i);
    strcat(k, ".jpg");
    strcat(result, k);
    ff = MTF_open(result, "rb");
    if (ff != NULL)
    {
        MTF_close(ff);
        return T_JPG;
    }

    result[0] = 0;
    strcat(result, dir);
    k[0] = 0;
    strcat(k, i);
    strcat(k, ".gif");
    strcat(result, k);
    ff = MTF_open(result, "rb");
    if (ff != NULL)
    {
        MTF_close(ff);
        return T_GIF;
    }

    return 0X27; //非图片格式!!!
}

static u8 key_code[3], coord_code[5];
void touch_switch_check(int *Xin, int *Yin, u8 state)
{
    static u8 s = 0;
    static u16 dest = 0;
    u16 x1, y1, x2, y2;
    char pic_path[PATH_LEN];
    int com_len = 3;

    RectInfo pic;
    pic.pixelByte = 4;
    pic.crossWay = 0;
    pic.alpha = 255;

    if (state) //按下
    {
        if (s == 1)
        {
            if (touch_switch_data[dest + 14] != 0XFF) //判断是否发送键值
            { //按下连续发送
                key_code[0] = KEY_DOWN;
                key_code[1] = touch_switch_data[dest + 14];
                key_code[2] = touch_switch_data[dest + 15];
                ComModel.send(&key_code[0], &com_len);
            }
            return;
        }
        //按下时搜索坐标所在页面号对应的按钮
        for (dest = 0; dest < 8 * 1024 && touch_switch_data[dest] != 0XFF && touch_switch_data[dest + 1] != 0XFF; dest += 16)
        {
            if (touch_switch_data[dest + 1] == page_num)
            {
                x1 = ((u16)touch_switch_data[dest + 2] << 8) + touch_switch_data[dest + 3];
                y1 = ((u16)touch_switch_data[dest + 4] << 8) + touch_switch_data[dest + 5];
                x2 = ((u16)touch_switch_data[dest + 6] << 8) + touch_switch_data[dest + 7];
                y2 = ((u16)touch_switch_data[dest + 8] << 8) + touch_switch_data[dest + 9];
                if (*Xin >= x1 && *Xin <= x2 &&
                    *Yin >= y1 && *Yin <= y2)
                {
                    if (((u16)touch_switch_data[dest + 12] << 8) + touch_switch_data[dest + 13] != 0XFF00)
                    { //有按下效果
                        pic_type_adapt(dirPath, &touch_switch_data[dest + 13], NULL, pic_path);
                        if (UI_pic_cut(pic_path, &pic, x1, y1, x2, y2) == 0)
                        {
                            UI_disRegionCrossAdapt(&pic, x1, y1);
                            LCD_Exec();
                            free(pic.pixelDatas);
                        }
                    }
                    if (touch_switch_data[dest + 14] != 0XFF) //判断是否发送键值
                    {
                        key_code[0] = KEY_DOWN;
                        key_code[1] = touch_switch_data[dest + 14];
                        key_code[2] = touch_switch_data[dest + 15];
                        ComModel.send(&key_code[0], &com_len);
                    }
                    s = 1;
                    break;
                }
            }
        }
    }
    else //松开
    {
        if (s == 1) //之前已按下按钮
        {
            s = 0;
            if (((u16)touch_switch_data[dest + 10] << 8) + touch_switch_data[dest + 11] != 0XFF00)
            { //跳转页面
                if (pic_type_adapt(dirPath, &touch_switch_data[dest + 11], NULL, pic_path) == T_GIF)
                {
                    gif_decode((u8 *)pic_path, 0, 0, &pic);
                    UI_LCDBackupRenew();
                    page_num = touch_switch_data[dest + 11];
                }
                else if (UI_pic(pic_path, &pic) == 0)
                {
                    UI_disRegionCrossAdapt(&pic, 0, 0);
                    free(pic.pixelDatas);
                    LCD_Exec();
                    UI_LCDBackupRenew();
                    page_num = touch_switch_data[dest + 11];
                }
            }
            else if (((u16)touch_switch_data[dest + 12] << 8) + touch_switch_data[dest + 13] != 0XFF00)
            { //恢复按钮
                pic_type_adapt(dirPath, &page_num, NULL, pic_path);
                x1 = ((u16)touch_switch_data[dest + 2] << 8) + touch_switch_data[dest + 3];
                y1 = ((u16)touch_switch_data[dest + 4] << 8) + touch_switch_data[dest + 5];
                x2 = ((u16)touch_switch_data[dest + 6] << 8) + touch_switch_data[dest + 7];
                y2 = ((u16)touch_switch_data[dest + 8] << 8) + touch_switch_data[dest + 9];
                if (UI_pic_cut(pic_path, &pic, x1, y1, x2, y2) == 0)
                {
                    UI_disRegionCrossAdapt(&pic, x1, y1);
                    LCD_Exec();
                    free(pic.pixelDatas);
                }
            }
            if (touch_switch_data[dest + 14] != 0XFF) //判断是否发送键值
            {
                key_code[0] = KEY_UP;
                key_code[1] = touch_switch_data[dest + 14];
                key_code[2] = touch_switch_data[dest + 15];
                ComModel.send(&key_code[0], &com_len);
            }
        }
    }
}

void touch_process(void)
{
    static u8 touch_down = 0;
    int com_len = 0;

    if (UI_pressState) //触摸屏被按下
    {
        if (touch_down == 0)
        {
        }
        if (user_touch_mode) //按下上传
        {
            coord_code[0] = COORDS_DOWN;
            coord_code[1] = UI_pressTouchX >> 8;
            coord_code[2] = (u8)UI_pressTouchX;
            coord_code[3] = UI_pressTouchY >> 8;
            coord_code[4] = (u8)UI_pressTouchY;
            com_len = 5;
            if (user_touch_down_keep_up) //按下持续上传
            {
                if (user_touch_switch)
                {
                    touch_switch_check(&UI_pressTouchX, &UI_pressTouchY, 1);
                }
                else
                {
                    ComModel.send(&coord_code[0], &com_len);
                }
            }
            else
            {
                if (touch_down == 0)
                {
                    if (user_touch_switch)
                    {
                        touch_switch_check(&UI_pressTouchX, &UI_pressTouchY, 1);
                    }
                    else
                    {
                        ComModel.send(&coord_code[0], &com_len);
                    }
                }
            }
        }
        touch_down = 1;
    }
    else
    {
        if (touch_down)
        {
            if (user_touch_switch)
            {
                touch_switch_check(&UI_pressTouchX, &UI_pressTouchY, 0);
            }
            else
            {
                coord_code[0] = COORDS_UP;
                coord_code[1] = UI_pressTouchX >> 8;
                coord_code[2] = (u8)UI_pressTouchX;
                coord_code[3] = UI_pressTouchY >> 8;
                coord_code[4] = (u8)UI_pressTouchY;
                com_len = 5;
                ComModel.send(&coord_code[0], &com_len);
            }
        }
        touch_down = 0;
    }
}

void HAND_IN_process(u8 *data, int *num)
{
    u8 d[] = {HAND_IN, '\"', 'O', 'K', '_', 'V', '1', '0', '\"', 0, 0, 0, 0};
    int i = 0;
    i = sizeof(d);
    d[i-1] = page_num;
    ComModel.send(d, &i);
}

void PEN_COLOR_process(u8 *data, int *num)
{
    //RGB 565
    u16 color1 = 0, color2 = 0;
    color1 = ((u16)data[1]<<8)+data[2];
    color2 = ((u16)data[3]<<8)+data[4];
    POINT_COLOR = RGB565to888(&color1);
    BACK_COLOR = RGB565to888(&color2);
}

void CHAR_GAP_process(u8 *data, int *num)
{
    
}

void GET_COLOR_TO_FRONT_process(u8 *data, int *num)
{
    u16 x, y;
    x = (int)data[1]<<8|data[2];
    y = (int)data[3]<<8|data[4];
    POINT_COLOR = UI_readPoint(&LCD_disAdapt, x, y);
}

void GET_COLOR_TO_BACK_process(u8 *data, int *num)
{
    u16 x, y;
    x = (int)data[1]<<8|data[2];
    y = (int)data[3]<<8|data[4];
    BACK_COLOR = UI_readPoint(&LCD_disAdapt, x, y);
}

void SHOW_CHAR_SIZE_8_process(u8 *data, int *num)
{
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    s = (u8 *)malloc(*num-4);
    if(s==NULL)
        return;
    for(i = 0; i<*num-5; i++)
        s[i] = data[i+5];
    s[i] = 0;
    UI_showStr2(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 8, user_text_backup); 
    free(s);
}

void SHOW_CHAR_SIZE_12_process(u8 *data, int *num)
{
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    s = (u8 *)malloc(*num-4);
    if(s==NULL)
        return;
    for(i = 0; i<*num-5; i++)
        s[i] = data[i+5];
    s[i] = 0;
    UI_showStr2(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 12, user_text_backup); 
    free(s);
}

void SHOW_CHAR_SIZE_16_process(u8 *data, int *num)
{
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    s = (u8 *)malloc(*num-4);
    if(s==NULL)
        return;
    for(i = 0; i<*num-5; i++)
        s[i] = data[i+5];
    s[i] = 0;
    UI_showStr2(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 16, user_text_backup); 
    free(s);
}

void SHOW_CHAR_SIZE_24_process(u8 *data, int *num)
{
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    s = (u8 *)malloc(*num-4);
    if(s==NULL)
        return;
    for(i = 0; i<*num-5; i++)
        s[i] = data[i+5];
    s[i] = 0;
    UI_showStr2(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 24, user_text_backup);
    free(s);  
}

void SHOW_CHAR_SIZE_32_process(u8 *data, int *num)
{
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    s = (u8 *)malloc(*num-4);
    if(s==NULL)
        return;
    for(i = 0; i<*num-5; i++)
        s[i] = data[i+5];
    s[i] = 0;
    UI_showStr2(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 32, user_text_backup);  
    free(s);
}

void SHOW_CHAR_USER_process(u8 *data, int *num)
{
    ColorClass f = 0, b = 0;
    u16 ft = 0, bt = 0;
    u16 x, y;
    u8 *s = NULL;
    u16 i = 0;
    u8 xsize = 0, ysize = 0;
    x = (u16)data[1]<<8|data[2];
    y = (u16)data[3]<<8|data[4];
    ft = (u16)data[8]<<8|data[9];
    bt = (u16)data[10]<<8|data[11];
    s = (u8 *)malloc(*num-11);
    f = POINT_COLOR;
    b = BACK_COLOR;
    POINT_COLOR = RGB565to888(&ft);
    BACK_COLOR = RGB565to888(&bt);
    if(s==NULL)
        return;
    for(i = 0; i<*num-12; i++)
        s[i] = data[i+12];
    s[i] = 0;
    if(i>5) //防止T5L连收时出错, 导致显示多个字符
        return;
    if((data[6]&0x0f)>=1&&(data[6]&0x0f)<=4)
    {
        switch (data[7])
        {
        case 0: ysize = 12; break;
        case 1: ysize = 16; break;
        case 2: ysize = 24; break;
        case 3: ysize = 32; break;
        case 4: ysize = 40; break;
        case 5: ysize = 48; break;
        case 6: ysize = 56; break;
        case 7: ysize = 64; break;
        default: ysize = data[7]; break; //按用户定义大小
        }
        if((data[6]&0x10)) //开启则宽高相等, 否则宽为高一半, 只对ASCII码的范围有效, GBK范围长宽始终相等
            xsize = ysize;
        else
            xsize = ysize / 2;
        UI_showStrDefine1(&LCD_disAdapt, &LCD_rectBackup, x, y, s, 
                        data[5], ((u16)ysize<<8)+xsize, data[6]); 
    } 
    POINT_COLOR = f;
    BACK_COLOR = b;
    free(s);
}

void CLEAR_DIS_process(u8 *data, int *num)
{
    for(u32 p = 0; p<LCD_disAdapt.totalPixels; p++)
		((ColorClass *)LCD_disAdapt.pixelDatas)[p] = BACK_COLOR;
	UI_disRegionCrossAdapt(&LCD_disAdapt, 0, 0); //更新显示
	LCD_Exec();
    UI_LCDBackupRenew();
}

void CLOSE_LIGHT_process(u8 *data, int *num)
{
    LCD_BackLightSet(0);
}

void ADJ_LIGHT_process(u8 *data, int *num)
{
    u8 i;
    i = data[1]/0.64+0.5; //T5范围: 0~0x40
    if(i>100)
        i = 100;
    LCD_BackLightSet(i);
}

void PLAY_MUSIC_process(u8 *data, int *num)
{
    u8 i = 0;
    char music_path[PATH_LEN];
    path_product(dirPath, &data[1], ".wav", music_path);
    wav_init(music_path, data[2]);
    i = 0.247*data[3];
    MTF_audio_vol(i);
}

void PLAY_STOP_process(u8 *data, int *num)
{
    MP3WAVplay_exit();
}

void PLAY_VOL_process(u8 *data, int *num)
{
    u8 i = 0;
    i = 0.247*data[1];
    MTF_audio_vol(i);
}

void SHOW_PIC_process(u8 *data, int *num)
{
    RectInfo pic;
    char pic_path[PATH_LEN];

    pic.pixelByte = 4;
    pic.crossWay = 0;
    pic.alpha = 255;

    if(pic_type_adapt(dirPath, &data[1], NULL, pic_path)==T_GIF)
    {
        gif_decode((u8 *)pic_path, 0, 0, &pic);
        UI_LCDBackupRenew();
        page_num = data[1];
    }
    else if(UI_pic(pic_path, &pic)==0)
    {
        UI_disRegionCrossAdapt(&pic, 0, 0);
        free(pic.pixelDatas);
        LCD_Exec();
        UI_LCDBackupRenew(); //更新背景
        page_num = data[1];
    }
}

void CUT_PIC_process(u8 *data, int *num)
{
    RectInfo pic;
    char pic_path[PATH_LEN];
    int x, y;
    u16 x1, y1, x2, y2;

    x = (int)data[10]<<8|data[11];
    y = (int)data[12]<<8|data[13];
    x1 = (u16)data[2]<<8|data[3];
    x2 = (u16)data[6]<<8|data[7];
    y1 = (u16)data[4]<<8|data[5];
    y2 = (u16)data[8]<<8|data[9];
    pic.pixelByte = 4;
    pic.crossWay = 0;
    pic.alpha = 255;

    pic_type_adapt(dirPath, &data[1], NULL, pic_path);
    if (UI_pic_cut(pic_path, &pic, x1, y1, x2, y2) == 0)
    {
        UI_disRegionCrossAdapt(&pic, x, y);
        LCD_Exec();
        free(pic.pixelDatas);
    }
}

#if 0 //功能还没调试好

static u8 user_ram_data[1024 * 128];
void WRITE_DATA_process(u8 *data, int *num)
{
    u32 addr = ((u32)data[5] << 24) + ((u32)data[6] << 16) + ((u32)data[7] << 8) + data[8];
    u8 d[] = {WRITE_DATA, 0X4F, 0X4B};
    int i = 0;
    i = sizeof(d);
    mFILE *f_user = NULL;
    UINT fr = 0;
    char path[PATH_LEN] = {0};
    uint32_t total, freeStorage;
    u8 dp[] = {0, ':', 0};

    strcat(path, dirPath);
    strcat(path, "user_rom_data");

    if (addr >= 128 * 1024 + 1 * 1024 * 1024)
        return; //128KB+1MB
    if (data[1] == 0X55 && data[2] == 0XAA && data[3] == 0X5A && data[4] == 0XA5)
    {
        if (addr <= 128 * 1024 - 1)
        {
            fr = *num - 7;
            if (addr + fr - 1 <= 128 * 1024 - 1)
            {
                memcpy(&user_ram_data[addr], &data[9], fr);
            }
            else //超过数量, 截取并整理留给下面
            {
                fr = 128 * 1024 - 1 - addr + 1;
                memcpy(&user_ram_data[addr], &data[9], fr);
                addr = 128 * 1024;
                *num = *num - 7 - fr;
                for (u16 cnt = 0; cnt < *num; cnt++)
                    data[9 + cnt] = data[9 + fr - 1 + cnt];
            }
        }
        if (addr > 128 * 1024 - 1)
        {
            f_user = MTF_open((const char *)path, "rb+");
            if (f_user == NULL) //不存在
            {             //创建
                if (strcmp(dirPath, "1:/") == 0)
                    dp[0] = '1';
                else
                    dp[0] = '0';
                MTF_disk_get_free(&dp[0], &total, &freeStorage); //获取磁盘容量情况
                if (freeStorage - 32 * 1024 >= 1024)       //1MB 保留32KB
                {
                    f_user = MTF_open((const char *)path, "wb+");
                    fr = 1024 * 1024 * 1; //创建1MB
                    MTF_write(user_ram_data, 1, fr, f_user);
                    MTF_seek(f_user, 0, SEEK_SET);
                }
            }

            if (f_user != NULL)
            {
                MTF_seek(f_user, addr - 128 * 1024 - 1, SEEK_SET);
                fr = *num - 7;
                if (addr + fr - 1 <= 128 * 1024 + 1 * 1024 * 1024 - 1)
                {
                    MTF_write(&data[9], 1, fr, f_user);
                }
                else //超过数量, 后面不要
                {
                    fr = 128 * 1024 + 1 * 1024 * 1024 - 1 - addr + 1;
                    MTF_write(&data[9], 1, fr, f_user);
                }
                MTF_close(f_user);
            }
        }
        ComModel.send(d, &i);
    }
}

void READ_DATA_process(u8 *data, int *num)
{
    u32 addr = ((u32)data[1] << 24) + ((u32)data[2] << 16) + ((u32)data[3] << 8) + data[4];
    int len = ((u16)data[5] << 8) + data[6];
    u8 *d;
    d = (u8 *)malloc(len + 7);
    if (d == NULL)
        return;
    d[0] = data[0];
    d[1] = data[1];
    d[2] = data[2];
    d[3] = data[3];
    d[4] = data[4];
    d[5] = data[5];
    d[6] = data[6];

    mFILE *f_user = NULL;
    UINT fr = 0;
    char path[PATH_LEN] = {0};
    u32 local_now = 0;

    strcat(path, dirPath);
    strcat(path, "user_rom_data");

    if (addr >= 128 * 1024 + 1 * 1024 * 1024)
        return; //128KB+1MB

    if (addr <= 128 * 1024 - 1)
    {
        fr = len;
        if (addr + fr - 1 <= 128 * 1024 - 1)
        {
            memcpy(&d[7], &user_ram_data[addr], fr);
        }
        else //超过数量, 截取并整理留给下面
        {
            fr = 128 * 1024 - 1 - addr + 1;
            memcpy(&d[7], &user_ram_data[addr], fr);
            addr = 128 * 1024;
            len = len - fr;
            local_now = fr - 1;
        }
    }
    if (addr > 128 * 1024 - 1)
    {
        f_user = MTF_open((const char *)path, "rb");
        if (f_user != NULL)
        {
            MTF_seek(f_user, addr - 128 * 1024 - 1, SEEK_SET);
            fr = len;
            if (addr + fr - 1 <= 128 * 1024 + 1 * 1024 * 1024 - 1)
            {
                MTF_read(&d[7 + local_now], 1, fr, f_user);
            }
            else //超过数量, 后面不要
            {
                fr = 128 * 1024 + 1 * 1024 * 1024 - 1 - addr + 1;
                MTF_read(&d[7 + local_now], 1, fr, f_user);
            }
            MTF_close(f_user);
        }
    }
    len += 7;
    ComModel.send(d, &len);
}

#endif

void DIS_SAVE_process(u8 *data, int *num)
{
    Sagittarius_debug("saving\r\n");
    MTF_fb_write_back_start();
    Sagittarius_debug("end: %d\r\n", MTF_fb_write_back_state());
    LCD_DrawBitmap(0, 0, lcddev.width-1, lcddev.height-1, (ColorClass *)framebuffer_var_info.write_back_addr);
}

void VIDEO_INPUT_process(u8 *data, int *num)
{
    u32 p = 0;
    MTF_fb_TV_input(&framebuffer_var_info, data[1], 128, 32, 128, 0);
    if (data[1]) //开启AV输入 默认cvbs显示在UI界面下面
    {
        for (p = 0; p < LCD_disAdapt.totalPixels; p++)
            ((ColorClass *)LCD_disAdapt.pixelDatas)[p] = FULL_TRANS_BLACK; //UI层全白透明, 不遮盖cvbs层
    }
    else //关闭av输入
    {
        for (p = 0; p < LCD_disAdapt.totalPixels; p++)
            ((ColorClass *)LCD_disAdapt.pixelDatas)[p] = BLACK; //UI层全黑不透, 遮盖cvbs层
    }
    UI_disRegionCrossAdapt(&LCD_disAdapt, 0, 0); //更新显示
    LCD_Exec();
    UI_LCDBackupRenew();
}

void VIDEO_INPUT_PLUS_process(u8 *data, int *num)
{
    u32 p = 0;
    MTF_fb_TV_input(&framebuffer_var_info, data[1], data[2], data[3], data[4], data[5]);
    if (data[1]) //开启AV输入 默认cvbs显示在UI界面下面
    {
        for (p = 0; p < LCD_disAdapt.totalPixels; p++)
            ((ColorClass *)LCD_disAdapt.pixelDatas)[p] = FULL_TRANS_BLACK; //UI层全白透明, 不遮盖cvbs层
    }
    else //关闭av输入
    {
        for (p = 0; p < LCD_disAdapt.totalPixels; p++)
            ((ColorClass *)LCD_disAdapt.pixelDatas)[p] = BLACK; //UI层全黑不透, 遮盖cvbs层
    }
    UI_disRegionCrossAdapt(&LCD_disAdapt, 0, 0); //更新显示
    LCD_Exec();
    UI_LCDBackupRenew();
}

void ADJ_TOUCH_process(u8 *data, int *num)
{
    u8 d[] = {ADJ_TOUCH, 0X4F, 0X4B};
    int i = 0;
    i = sizeof(d);

    TP_Adjust();
    ComModel.send(d, &i);
}

void BEEP_PLAY_process(u8 *data, int *num)
{
    beep.Mod = 0;
    beep.Time = data[1];
    beep.Vol = data[2]*0.7/2.55; //禁止VOL:100, 必须方波
}

void GIF_SHOW_process(u8 *data, int *num)
{
    char _path[PATH_LEN] = {0};

    if (data[9] < 8)
    {
        path_product(dirPath, &data[2], ".gif", _path);
        if (gif_decode_init(_path, (char)data[9]) >= -1)
            gif_decode_set(data[9], data[7], data[8], ((int)data[3] << 8) + data[4], ((int)data[5] << 8) + data[6]);
    }
}

void SHOW_PIC_PLUS_process(u8 *data, int *num)
{
    RectInfo pic, pic2;
    char pic_path[PATH_LEN];
    u16 angle = ((u16)data[9]<<8)+data[10];
    int x, y;

    if ((data[3] & 0X80) == 0) //最高位0为正, 1为负
        x = ((int)data[3] << 8) + data[4];
    else
        x = -((((int)data[3] & 0X7F) << 8) + data[4]);
    if ((data[5] & 0X80) == 0) //最高位0为正, 1为负
        y = ((int)data[5] << 8) + data[6];
    else
        y = -((((int)data[5] & 0X7F) << 8) + data[6]);

    pic2.pixelDatas = NULL;
    pic.pixelByte = 4;
    pic.crossWay = data[7];
    pic.alpha = data[8];

    if(pic_type_adapt(dirPath, &data[2], NULL, pic_path)==T_GIF)
    {
        gif_decode((u8 *)pic_path, ((u16)data[3]<<8)+data[4], ((u16)data[5]<<8)+data[6], &pic);
    }
    else if(UI_pic(pic_path, &pic)==0)
    {
        if(angle!=0&&angle<=360)
        {
            if(angle<=180)
                UI_imrotate(&pic2, &pic, angle);
            else
                UI_imrotate(&pic2, &pic, angle-360);
            UI_disRegionCrossAdapt(&pic2, x, y);
        }
        else
        {
            UI_disRegionCrossAdapt(&pic, x, y);
        }
        free(pic.pixelDatas);
        free(pic2.pixelDatas);
        LCD_Exec();
    }
}

void CONTROL_SHOW_process(u8 *data, int *num)
{
    RectInfo pic, pic2;
    char pic_path[PATH_LEN];
    u16 angle = ((u16)data[9]<<8)+data[10];
    u8 conID = 0;
    int x, y;
    int *temp = NULL;

    // if (LCD_disAdaptFlag == 1) //自适应不支持
    //     return;
    if ((data[3] & 0X80) == 0) //最高位0为正, 1为负
        x = ((int)data[3] << 8) + data[4];
    else
        x = -((((int)data[3] & 0X7F) << 8) + data[4]);
    if ((data[5] & 0X80) == 0) //最高位0为正, 1为负
        y = ((int)data[5] << 8) + data[6];
    else
        y = -((((int)data[5] & 0X7F) << 8) + data[6]);

    pic2.pixelDatas = NULL; //初始化付NULL, 保证代码健壮
    pic.pixelByte = 4;
    pic.crossWay = data[7];
    pic.alpha = data[8];

    conID = control_table_check(data[2]);
    if(conID!=255) //之前已生成
    {
        if(angle!=0&&angle<=360)
        {
            if(angle<=180)
                UI_imrotate(&pic2, &controller_table[conID].rect, angle);
            else
                UI_imrotate(&pic2, &controller_table[conID].rect, angle-360);
            
            temp = controller_table[conID].rect.pixelDatas;
            controller_table[conID].rect.pixelDatas = pic2.pixelDatas;
            control_move(&controller_table[conID], x, y);
            free(pic2.pixelDatas);
            controller_table[conID].rect.pixelDatas = temp;
        }
        else
        {
            control_move(&controller_table[conID], x, y);
        }
        LCD_Exec();
        return;
    }
    if(pic_type_adapt(dirPath, &data[2], NULL, pic_path)==T_GIF)
    {
        gif_decode((u8 *)pic_path, ((u16)data[3]<<8)+data[4], ((u16)data[5]<<8)+data[6], &pic);
        UI_FillFrontToDest(&pic, ((u16)data[3]<<8)+data[4], ((u16)data[5]<<8)+data[6]);
    }
    else if(UI_pic(pic_path, &pic)==0)
    {
        // UI_color_replace(&pic);
        if(angle!=0&&angle<=360)
        {
            if(angle<=180)
                UI_imrotate(&pic2, &pic, angle);
            else
                UI_imrotate(&pic2, &pic, angle-360);
            UI_disRegionCrossAdapt(&pic2, x, y);
            free(pic.pixelDatas);
            pic.pixelDatas = pic2.pixelDatas;
        }
        else
        {
            UI_disRegionCrossAdapt(&pic, x, y);
        }
        LCD_Exec();
    }
    control_table_malloc(&pic, data[2], x, y);
} 

void CONTROL_RELEASE_process(u8 *data, int *num)
{
    control_table_free(data[2]);
} 

void RELEASE_ALL_process(u8 *data, int *num)
{
    control_table_free(255);
} 

void RENEW_BACK_process(u8 *data, int *num)
{
    UI_LCDBackupRenew(); //更新背景
}

void BACK_RECT_process(u8 *data, int *num)
{
/*     //有问题, 执行后, 再次画图像卡死
    u8 conID = 0;
    if (LCD_disAdaptFlag == 1)
        return;
    conID = control_table_check(data[2]);
    if(conID!=255) //之前已生成
    {
            render_front->pixels = LCD_rectBackup.pixelDatas; //要切换目标地址
            UI_disRegionCrossAdapt(&controller_table[conID].rect,
                                   controller_table[conID].x, controller_table[conID].y);
            render_front->pixels = LCD_disAdapt.pixelDatas; //恢复地址
    } */
} 

void REST_SYS_process(u8 *data, int *num)
{
    if(data[2]=='r'&&data[3]=='e'&&data[4]=='s'&&data[5]=='e'&&data[6]=='t')
    {
        Sagittarius_debug("reset\r\n");
        reset_flag = 1;
    }
}

void BACK_DIS_PART_process(u8 *data, int *num)
{
    int x1, y1;
    u16 width, height;
    x1 = ((int)data[2] << 8) + data[3];
    y1 = ((int)data[4] << 8) + data[5];
    width = (((int)data[6] << 8) + data[7]) - x1 + 1;
    height = (((int)data[8] << 8) + data[9]) - y1 + 1;
    backRectRecovery(x1, y1, width, height);
}

void PART_COLOR_REPLACE_process(u8 *data, int *num)
{
    // UI_color_replace_set(data[2], (ColorClass)data[3]<<24|(ColorClass)data[4]<<16|(ColorClass)data[5]<<8|data[6], 
    // (ColorClass)data[7]<<24|(ColorClass)data[8]<<16|(ColorClass)data[9]<<8|data[10]);
}

void KEY_STATE_process(u8 *data, int *num)
{
    int com_len = sizeof(key_code);
    ComModel.send(&key_code[0], &com_len);
}

void COORD_STATE_process(u8 *data, int *num)
{
    int com_len = sizeof(coord_code);
    ComModel.send(&coord_code[0], &com_len);
}

void SET_COLOR_process(u8 *data, int *num)
{
    //ARGB
    POINT_COLOR = ((ColorClass)data[2]<<24)+((ColorClass)data[3]<<16)+ \
                    ((ColorClass)data[4]<<8)+data[5];
    BACK_COLOR = ((ColorClass)data[6]<<24)+((ColorClass)data[7]<<16)+ \
                    ((ColorClass)data[8]<<8)+data[9];
}

void testBoard(unsigned char *data, int *num);

/* void ComBurnFirmware(unsigned char *data, int *num)
{
#ifdef _USER_DEBUG
    unsigned char i = 0;
    unsigned char code[] = {'b', 'u', 'r', 'n', ' ', 'f', 'i', 'r', 'm', 'w', 'a', 'r', 'e'};
    u8 *temp_data = NULL;
    u32 count = 0, size = 1*1024*1024; //最大1MB
    int _num1 = 0, _num2 = 0, timeout = 0;

    for (; i < sizeof(code); i++)
    {
        if(data[i+2]!=code[i])
            return; //不对码, 退出
    }
    temp_data = (u8 *)malloc(size);
    if (temp_data == NULL)
        return;
    Sagittarius_debug("please download the firmware***\r\n");
    usb_com_open = 0;
    while(1)
    {
        usb_cdc_in_ep_callback(temp_data+_num1, &_num2);
        _num1 += _num2;
        if(_num2==0)
            timeout++;
        else
            timeout = 0;
        if(timeout>150000000)
            break; //超时退出
    }
    size = _num1;
    usb_com_open = 1;
    Sagittarius_debug("file size: %d KB\r\n", size/1024);
    if(size<1024)
    {          
        Sagittarius_debug("download error....\r\n");
        free(temp_data);
        return; //过小退出
    }
    else
    {
        Sagittarius_debug("burning to flash!!!\r\n");
    }
    while (1)
    {
        MTF_ROM_write(&temp_data[count], count, 4096);
        count += 4096;
        if (count + 4096 >= size - 4096)
        {
            MTF_ROM_write(&temp_data[count], count, size - count);
            break;
        }
    }
    free(temp_data);
    Sagittarius_debug("download OK>>>>>\r\n");
#endif
} */

void EXTEND_COMMAND_process(u8 *data, int *num)
{
    switch (data[1])
    {
    case GIF_SHOW: GIF_SHOW_process(data, num); break;
    case SHOW_PIC_PLUS: SHOW_PIC_PLUS_process(data, num); break;
    case CONTROL_SHOW: CONTROL_SHOW_process(data, num); break;
    case CONTROL_RELEASE: CONTROL_RELEASE_process(data, num); break;
    case BACK_RECT: BACK_RECT_process(data, num); break;
    case RELEASE_ALL: RELEASE_ALL_process(data, num); break;
    case RENEW_BACK: RENEW_BACK_process(data, num); break;
    case REST_SYS: REST_SYS_process(data, num); break;
    case BACK_DIS: UI_LCDBackupDis(); break;
    case BACK_DIS_PART: BACK_DIS_PART_process(data, num); break;
    case PART_COLOR_REPLACE: PART_COLOR_REPLACE_process(data, num); break;
    case TEST_SYS: testBoard(data, num); break;
    case KEY_STATE: KEY_STATE_process(data, num); break;
    case COORD_STATE: COORD_STATE_process(data, num); break; 
    case SET_COLOR: SET_COLOR_process(data, num); break;
    // case 0XF2: ComBurnFirmware(data, num); break;
    default: break;
    }
}

void T5L_Command_Run(u8 *data, int *num)
{
#ifdef _USER_DEBUG
        Sagittarius_debug(" com: ");
        for (u8 ttt = 0; ttt < *num; ttt++)
            Sagittarius_debug("%#X, ", data[ttt]);
        Sagittarius_debug("\r\n");
#endif
    //处理指令
    switch (data[0])
    {
        case HAND_IN: HAND_IN_process(data, num); break;
        case PEN_COLOR: PEN_COLOR_process(data, num); break;
        case CHAR_GAP: CHAR_GAP_process(data, num); break;
        case GET_COLOR_TO_FRONT: GET_COLOR_TO_FRONT_process(data, num); break;
        case GET_COLOR_TO_BACK: GET_COLOR_TO_BACK_process(data, num); break;
        case SHOW_CHAR_SIZE_8: SHOW_CHAR_SIZE_8_process(data, num); break;
        case SHOW_CHAR_SIZE_12: SHOW_CHAR_SIZE_12_process(data, num); break;
        case SHOW_CHAR_SIZE_16: SHOW_CHAR_SIZE_16_process(data, num); break;
        case SHOW_CHAR_SIZE_24: SHOW_CHAR_SIZE_24_process(data, num); break;
        case SHOW_CHAR_SIZE_32: SHOW_CHAR_SIZE_32_process(data, num); break;
        case SHOW_CHAR_USER: SHOW_CHAR_USER_process(data, num); break;
        case CLEAR_DIS: CLEAR_DIS_process(data, num); break;
        case CLOSE_LIGHT: CLOSE_LIGHT_process(data, num); break;
        case ADJ_LIGHT: ADJ_LIGHT_process(data, num); break;
        case PLAY_MUSIC: PLAY_MUSIC_process(data, num); break;
        case PLAY_STOP: PLAY_STOP_process(data, num); break;
        case PLAY_VOL: PLAY_VOL_process(data, num); break;
        case SHOW_PIC: SHOW_PIC_process(data, num); break;
        case CUT_PIC: CUT_PIC_process(data, num); break;
        // case WRITE_DATA: WRITE_DATA_process(data, num); break;
        // case READ_DATA: READ_DATA_process(data, num); break;
        case DIS_SAVE: DIS_SAVE_process(data, num); break;
        case VIDEO_INPUT: VIDEO_INPUT_process(data, num); break;
        case VIDEO_INPUT_PLUS: VIDEO_INPUT_PLUS_process(data, num); break;
        case ADJ_TOUCH: ADJ_TOUCH_process(data, num); break;
        case BEEP_PLAY: BEEP_PLAY_process(data, num); break;
        case DGUSII_READ: HMI_read_process(data, num); break;
        case DGUSII_WRITE: HMI_write_process(data, num); break;
        case EXTEND_COMMAND: EXTEND_COMMAND_process(data, num); break;
        default: break;
    }
}

void __fatal_error(const char *msg) {
    while (1);
}

#ifndef NDEBUG
void __assert_func(const char *file, int line, const char *func, const char *expr) {
    //Sagittarius_debug("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

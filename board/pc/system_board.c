#include "system_board.h"
#include "sha256.h"
#include "licence.h"
#include "file_operate_hal.h"
#include "file_type.h"
#include "Sagittarius_global.h"
#include "Sagittarius_timer.h"
#include "cJSON_extend.h"
#include "UI_engine.h"
#include "MTF_io.h"
#include "gpio_board.h"
#include "ROM_port.h"
#include "delay.h"

#define system_board_debug(...) //printf(__VA_ARGS__)

//激活警告
extern unsigned char _binary_ff_gif_start[], _binary_ff_gif_size[], _binary_ff_gif_end[];
void active_warning(void)
{
    RectInfo pic;
    pic.pixelByte = 4;
    pic.crossWay = 0;
    pic.alpha = 255;

    unsigned char *buf = _binary_ff_gif_start; //文件由ld生成并由gcc连接到代码
    size_t total = (size_t)_binary_ff_gif_size;
    mFILE *fp = NULL;

    fp = MTF_open("./MTF/logo", "wb+"); //读写, 重新生成打开
    if (fp != NULL)
    {
        MTF_write(buf, total, 1, fp);
        MTF_close(fp);
        gif_decode((u8 *)"./MTF/logo", 0, 0, &pic);
        UI_LCDBackupRenew();
        delay_ms(300);
    }
    else
    {
        while (1); //防止盗版修改logo
    }
}

//检测激活状态
uint8_t check_active_state(void)
{
    return 1;
}

void personalise_function(void)
{
}

//文件复制和FATFS挂载(仅当存储源为FLASH时)
void FilesRenew(void)
{
    return;
}

//定时激活检查
void licence_timing_check(void)
{
/*     static uint8_t i = 0;

    if (Sagittarius_run_time() <= 12000) //函数反回时间单位为10ms, 需2min后执行判断
        return;

    //用于随机检查激活, 增强防破解
    srand(Sagittarius_run_time());
    if (rand() / 6000 % 2)
    {
        if (i == 1)
        {
            i = 0;
            if (sys_integrity_check())
            {
                LCD_Clear(RED); //清屏
                reset_flag = 1; //激活有问题, 重启
                delay_ms(1000);
            }
        }
    }
    else
    {
        i = 1;
    } */
}

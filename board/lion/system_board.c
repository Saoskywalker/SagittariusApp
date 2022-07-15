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
    // RectInfo pic;
    // pic.pixelByte = 4;
    // pic.crossWay = 0;
    // pic.alpha = 255;

    // unsigned char *buf = _binary_ff_gif_start; //文件由ld生成并由gcc连接到代码
    // size_t total = (size_t)_binary_ff_gif_size;
    // mFILE *fp = NULL;

    // fp = MTF_open("./MTF/logo", "wb+"); //读写, 重新生成打开
    // if (fp != NULL)
    // {
    //     MTF_write(buf, total, 1, fp);
    //     MTF_close(fp);
    //     gif_decode((u8 *)"./MTF/logo", 0, 0, &pic);
    //     UI_LCDBackupRenew();
    //     delay_ms(300);
    // }
    // else
    // {
    //     while (1); //防止盗版修改logo
    // }
}

//检测激活状态
uint8_t check_active_state(void)
{
    mFILE *f_licence = NULL, *f_UID = NULL;
    char licence[32];
    uint64_t UID = 0;
    u8 temp[] = {0, 0, 0, 0, 0, 0, 0, 0};
return 0;
    // active_code_generate(MTF_unique_id()); //运行激活程序
    if(sys_licence_check()==0) //检查激活
    {
        system_board_debug("--actived--\r\n");
        return 0;
    }
    else //激活码在SD卡
    {
        if(MTF_mount(&FILE_SYS_SD)!=0) //挂载SD卡失败 
        {
            Show_Str2(0, 0, (u8 *)"mount SD card fail :(", 16, 0); 
            while (1);
        }
        else
        {
            UID = MTF_unique_id(); //读UID
            for(u8 j = 0; j<8; j++)
                temp[j] = UID>>8*j;
            f_UID = MTF_open("./MTF/UID", "rb+");
            if(f_UID != NULL)
            {
                MTF_write(temp, 1, 8, f_UID);
                MTF_close(f_UID);
            }

            f_licence = MTF_open("./MTF/licence", "rb");
            if(f_licence == NULL)
            {
                Show_Str2(0, 0, (u8 *)"active fail", 16, 0); 
                while (1);
            }
            MTF_read(licence, 1, 32, f_licence);
            MTF_close(f_licence);
            sys_active(licence);
            Show_Str2(0, 0, (u8 *)"active success, sys will reset :)", 16, 0);
            reset_flag = 1;
            while(1); //重启
        }
        return 1;
    }
}

//固件更新,,,
static void firmware_update(void)
{
    mFILE *f_firmware = NULL;
    long size = 0;
    u8 *temp_data = NULL;
    u32 count = 0;
    u8 sha256value_1[32], sha256value_2[32];

    if(strcmp(dirPath, "./MTF/")==0) //当前为SD源
    {
        f_firmware = MTF_open((const char *)"./MTF/firmware.bin", "rb");
        if (f_firmware == NULL)
            return;
        size = MTF_size(f_firmware); //get file size
        if ((size <= 4096)||(size >= JSON_FLASH_ADDR))
            return;
        temp_data = (u8 *)malloc(size);
        if(temp_data==NULL)
            return;
        Show_Str2(0, 0, (u8 *)"Updating... $_$", 16, 0);
        LCD_Exec();
        MTF_read(temp_data, 1, size, f_firmware); //load file
        MTF_close(f_firmware);
        count = 0;
        while (1)
        {
            MTF_ROM_write(&temp_data[count], count, 4096);
            count+=4096;
            if(count+4096>=size-4096)
            {
                MTF_ROM_write(&temp_data[count], count, size-count);
                break;
            }
        }
        sha256_hash(temp_data, size, &sha256value_1[0]); //生成SHA256码
        count = 0;
        while (1)
        {
            MTF_ROM_read(&temp_data[count], count, 4096);
            count+=4096;
            if(count+4096>=size-4096)
            {
                MTF_ROM_read(&temp_data[count], count, size-count);
                break;
            }
        }
        sha256_hash(temp_data, size, &sha256value_2[0]); //生成SHA256码
        for (u8 i = 0; i < 32; i++)
        {
            if(sha256value_1[i]!=sha256value_2[i])
            {
                Show_Str2(0, 16, (u8 *)"Update fail, please use OTG update T_T", 16, 0);
                LCD_Exec();
                while(1);
            }
        }
        Show_Str2(0, 16, (u8 *)"Update success, please reboot ^_^", 16, 0);
        LCD_Exec();
        while(1);
    }
}

//更新cfg文件
static uint8_t cfg_file_renew(void)
{
    mFILE *f_ini = NULL;
    long size = 0;
    uint8_t *flash_data = NULL;

    f_ini = MTF_open((const char *)"./MTF/cfg.json", "rb");
    if (f_ini == NULL)
        return 4;
    size = MTF_size(f_ini); //get file size
    if ((size <= 0)||(size >= 32768))
        return 1;
    //data+size(2B)+JSON_FLASH_FLAG(1B)
    flash_data = (uint8_t *)malloc(size+3);
    if(flash_data==NULL)
        return 2;
    flash_data[0] = (uint8_t)size;
    flash_data[1] = (uint8_t)(size>>8);
    flash_data[2] = JSON_FLASH_FLAG;
    MTF_read(&flash_data[3], 1, size, f_ini); //load file
    MTF_close(f_ini);

    MTF_ROM_write(flash_data, JSON_FLASH_ADDR, size+3);
    free(flash_data);
    return 0;
}

//检查储存位置
static void storage_local_check(void)
{
    char *strings = "";
    const char *cfg_path = "./FLASH/MTF/cfg.json";
    u8 res = 0;
    cJSON *parsed = NULL, *item = NULL;

    parsed = json_file_parse(cfg_path);
    if (parsed != NULL)
    {
        //检查存储位置
        item = cJSON_GetObjectItem(parsed, "INIT");
        if (item != NULL)
        {
            strings = json_read_string(item, "file_source", "none", (char *)&res);
            system_board_debug("LOCAL: %s\r\n", strings);
            if (strcmp(strings, "SD") == 0)
            {
                dirPath = "./MTF/"; //选定存储设备
            }
            else if (strcmp(strings, "FLASH") == 0)
            {
                dirPath = "./FF/MTF/"; //选定存储设备
            }
            else
            { //失败, 不运行
                POINT_COLOR = LIGHT_RED;
                Show_Str2(0, 0, (u8 *)"cfg.json file not found !", 16, 0);
                LCD_Exec();
                res = MTF_mount(&FILE_SYS_SD); //挂载SD卡
                if (res == 0)                  //挂载成功
                {
                    cfg_file_renew();
                    Show_Str2(0, 0, (u8 *)"cfg.json file renew ok ! ", 16, 0);
                    LCD_Exec();
                    while (1);
                }
                else
                {
                    while (1);
                }
            }
        }
    } 
    else //失败, 不运行
    {
        POINT_COLOR=LIGHT_RED; 
        Show_Str2(0, 0, (u8 *)"Without cfg.json, sys shutdown :(", 24, 0); 
        LCD_Exec();
        res = MTF_mount(&FILE_SYS_SD); //挂载SD卡 
        if(res==0) //挂载成功
        {
            cfg_file_renew();
            Show_Str2(0, 24, (u8 *)"cfg.json file renew ok ! ", 16, 0); 
            LCD_Exec();
            while (1);
        }
        else
        {
            while(1);
        }
    }
    cJSON_Delete(parsed);
}

//按储存源挂载文件系统
static void FileSysInit(void)
{
    u8 res = 0;
    uint32_t total = 0, freeStorage = 0;

    POINT_COLOR = LIGHT_RED; //设置字体颜色
    BACK_COLOR = BLACK;      //背景黑色

    if (strcmp(dirPath, "./MTF/") == 0) //SD
    {
        if (MTF_mount(&FILE_SYS_SD) != 0) //挂载SD卡失败
        {
            Show_Str2(0, 0, (u8 *)"mount SD card fail :(", 16, 0);
            while (1);
        }
    }
    else
    {
        res = MTF_mount(&FILE_SYS_FLASH); //挂载FLASH.
        if (res == 0X0D)               //FLASH磁盘,FAT文件系统错误,重新格式化FLASH
        {
            POINT_COLOR = LIGHT_RED;
            Show_Str2(0, 0, (u8 *)"Flash Formatting...", 16, 0); //格式化FLASH
            LCD_Exec();
            // MTF_ROM_user_data_erase();
            res = MTF_disk_fromat((uint8_t *)"1:", 1, 4096); //格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
            if (res == 0)
            {
                MTF_disk_set_label((uint8_t *)"1:Aysi");                  //设置Flash磁盘的名字
                Show_Str2(0, 0, (u8 *)"Flash Format Succeed", 16, 0); //格式化完成
            }
            else
            {
                Show_Str2(0, 0, (u8 *)"Flash Format Fail   ", 16, 0); //格式化失败
            }
            LCD_Exec();
        }
        while (MTF_disk_get_free((u8 *)"1:", &total, &freeStorage)) //得到flash卡的总容量和剩余容量
        {
            //格式化失败
            POINT_COLOR = LIGHT_RED;
            Show_Str2(0, 0, (u8 *)"Flash Format Fail --Flash FileSystem Error!", 16, 0);
            LCD_Exec();
            delay_ms(200);
            Show_Str2(0, 0, (u8 *)"                                           ", 16, 0);
            LCD_Exec();
            delay_ms(200);
        }
    }
}

void personalise_function(void)
{
    //文件路径确认流程: 默认设置->检查FLASH上的CFG文件和其文件路径并设置(如检查错误或打开失败则通过SD卡更新CFG文件)
    // storage_local_check(); //检查储存位置
    FileSysInit(); //按储存源挂载文件系统
    // firmware_update(); //固件更新

    // #ifdef _USER_DEBUG
    //     uint32_t total, freeStorage;
    //     MTF_disk_get_free((u8 *)dirPath, &total, &freeStorage);
    //     mf_scan_files((u8 *)dirPath);
    //     system_board_debug("storage: total %dKB, free %dKB\r\n", total, freeStorage);
    // #endif
}

//文件复制和FATFS挂载(仅当存储源为FLASH时)
void FilesRenew(void)
{
    u8 res = 0;
    uint32_t total = 0,freeStorage = 0;
    char *sd_files[100], *flash_files[100];
    u8 sd_files_cnt = 0, flash_files_cnt = 0;
    u8 i = 0, j = 0;
    char temp_path[32], temp_path2[32];
return;
    system_board_IO_init();

    if ((strcmp(dirPath, "./FF/MTF/") != 0) || (!TF_CHECK())) //当前标记为FLASH源, 并且SD卡已插入
        return;

    memset(&temp_path[0], 0, 32);
    memset(&temp_path2[0], 0, 32);

    POINT_COLOR=LIGHT_RED; //设置字体颜色
    BACK_COLOR=BLACK; //背景黑色

    res = MTF_mount(&FILE_SYS_SD); //挂载SD卡 
    if(res==0) //挂载成功
    {
        if(MTF_disk_get_free((u8 *)"0:",&total,&freeStorage))	//得到SD卡的总容量和剩余容量
        {

        }		
        else
        {
            POINT_COLOR=LIGHT_RED; 
            Show_Str2(0, 0, (u8 *)"^0^ LOAD SUCCESS SD", 16, 0);  
            POINT_COLOR=LIGHT_GREEN; 
            Show_Str2(0, 16, (u8 *)"SD Total Size:     MB", 16, 0); 
            Show_Str2(0, 32, (u8 *)"SD  Free Size:     MB", 16, 0); 	    
            LCD_ShowNum(100, 16,total>>10,5,16); //显示SD卡总容量 MB
            LCD_ShowNum(100, 32,freeStorage>>10,5,16);	//显示SD卡剩余容量 MB
            LCD_Exec();

            for(i = 0; i<100; i++)
                sd_files[i] = NULL;    
            sd_files[0] = (char *)malloc(1024*3); //字符串池, 3K容量
            memset(sd_files[0], 0, 1024*3); //字符串池初始化需清0
            mf_scan_files2((u8 *)"0:/MTF", sd_files, &sd_files_cnt); //读目录
            if(sd_files_cnt>0) //有文件
            {
                POINT_COLOR=LIGHT_ORANGE; 
                Show_Str2(0, 48, (u8 *)"files copying......", 16, 0);  
                LCD_Exec();
               
                for(i = 0; i<100; i++)
                    flash_files[i] = NULL;    
                flash_files[0] = (char *)malloc(1024*3); //字符串池, 3K容量
                memset(flash_files[0], 0, 1024*3); //字符串池初始化需清0
                mf_scan_files2((u8 *)"1:", flash_files, &flash_files_cnt); //读目录
                // if(flash_files_cnt>0)   //先删除所有
                // {   
                //     for(j = 0; j<flash_files_cnt; j++)
                //     {
                //         strcpy (temp_path, "./");
                //         strcat(temp_path, flash_files[j]);
                //         f_unlink((const TCHAR *)temp_path);
                //     }
                // }
                for(j = 0; j<sd_files_cnt; j++)
                {
                    strcpy (temp_path, "0:/MTF/");
                    strcat(temp_path, sd_files[j]);
                    strcpy (temp_path2, "./");
                    strcat(temp_path2, sd_files[j]);
                    if (strcmp("cfg.json", sd_files[j]) == 0) 
                        res = cfg_file_renew(); //cfg文件用此更新
                    else
                        res = mf_copy((u8 *)temp_path, (u8 *)temp_path2, 1);  //复制
                    system_board_debug("r: %d  --", res);
                    system_board_debug("COPY TO: %s\r\n", temp_path2);
                    if(res==7) //磁盘满
                        break;
                }
                MTF_disk_get_free((u8 *)"1:",&total,&freeStorage);
                POINT_COLOR=SUN_YELLOW; 
                Show_Str2(0, 48, (u8 *)"files copy completed ^3^", 16, 0);  
                Show_Str2(0, 64, (u8 *)"Free Size:         MB", 16, 0); 	
                LCD_ShowNum(100,64,freeStorage>>10,5,16); //显示剩余容量 MB
                LCD_Exec();
                free(sd_files[0]); //释放文件目录池
                free(flash_files[0]);
                if(res==7)
                {
                    Show_Str2(0, 80, (u8 *)"sorry, disk full :(", 16, 0); 
                    Show_Str2(0, 96, (u8 *)sd_files[j], 16, 0);
                }
                else
                {
                    Show_Str2(0, 96, (u8 *)"Please unload SD card and reboot!", 24, 0);
                }
                system_board_debug("copy completed\r\n");
                LCD_Exec();
                while(1); //提示需重新上电
            }
        }
    }
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

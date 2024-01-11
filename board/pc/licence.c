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

#include "types_plus.h"
#include "sha256.h"
#include "crc32.h"
#include <stdio.h>
#include <string.h>
#include "ROM_port.h"
#include <malloc.h>
#include "licence.h"

#define licence_debug(...) //printf(__VA_ARGS__)

/*FLASH ADDR*/
#define licence_addr 0X000DF800
#define CRC32_addr 0X000DF820

char sys_licence_check(void)
{
    char *key = "SaoskywalkerAtomMTF";
    uint64_t UID = 0;
    u8 iooi[128];
    u8 sha256value[32];
    u8 licence_code[32];

    // struct w25q_flash_dev flash_dev;
    // flash_dev = W25QXX_Init();
    // licence_debug("type: %#X, size %d, sector: %d\r\n", flash_dev.type, flash_dev.size, flash_dev.sector);
    // licence_debug("w25q jedc id: %#X\r\n", ReadFlashJedecID());
    UID = MTF_unique_id(); //读UID

    memset(iooi, 0, 128); //清0
    for (u8 i = 0; i < 8; i++) //UID+KEY拼接
    {
        iooi[i] = UID>>(8*(7-i));
        licence_debug("%#X,", iooi[i]);
    }
    for (u8 j = 0; key[j] != 0; j++)
        iooi[8+j] = key[j];
    
    sha256_hash(iooi, sizeof(iooi), &sha256value[0]); //生成SHA256码
    MTF_ROM_read(licence_code, licence_addr, 32); //获取licence
    licence_debug("\r\n sha256:\r\n");
    for(u8 jj = 0; jj<32; jj++)
    {
        licence_debug("%#X, ", sha256value[jj]);
        if(sha256value[jj]!=licence_code[jj])
            return 1; //失败
    }
    licence_debug("\r\n");
    return 0; //成功
}

#define check_file_size 0 //校验内容:FLASH前n字节+32Beyt激活码

char sys_integrity_check(void)
{
    u8 *flash_data;
    u32 crc32 = 0;
    u8 integrity_code[4];
    u8 licence_code[32];

    flash_data = (u8 *)malloc(check_file_size+32);
    if(flash_data==NULL)
        return 2;

    MTF_ROM_read(flash_data, 0, check_file_size);
    MTF_ROM_read(licence_code, licence_addr, 32);
    for (u8 j = 0; j < 32; j++)
        flash_data[check_file_size+j] = licence_code[j];
    crc32 = crc32_sum(0, flash_data, check_file_size+32);
    free(flash_data);
    licence_debug("%#X\r\n", crc32);
    
    MTF_ROM_read(integrity_code, CRC32_addr, 4);
    if (crc32==((u32)integrity_code[0]<<24)+((u32)integrity_code[1]<<16)+
                ((u32)integrity_code[2]<<8)+integrity_code[3])
        return 0; //成功
    else
        return 1; //失败
}

char sys_active(char *active_code)
{
    u8 *flash_data;
    u32 crc32 = 0;
    u8 crc32_code[4];

    MTF_ROM_write((u8 *)active_code, licence_addr, 32);

    flash_data = (u8 *)malloc(check_file_size+32);
    if(flash_data==NULL)
        return 2;

    MTF_ROM_read(flash_data, 0, check_file_size);
    for (u8 j = 0; j < 32; j++)
        flash_data[check_file_size+j] = active_code[j];
    crc32 = crc32_sum(0, flash_data, check_file_size+32);
    crc32_code[0] = crc32>>24;
    crc32_code[1] = crc32>>16;
    crc32_code[2] = crc32>>8;
    crc32_code[3] = crc32;
    MTF_ROM_write(crc32_code, CRC32_addr, 4);
    free(flash_data);
    licence_debug("%#X\r\n", crc32);
    
    return 0;
}

void active_code_generate(uint64_t code)
{
    char *key = "SaoskywalkerAtomMTF";
    u8 iooi[128];
    u8 sha256value[32];

    memset(iooi, 0, 128); //清0
    for (u8 i = 0; i < 8; i++) //code+KEY拼接
    {
        iooi[i] = code>>(8*(7-i));
        licence_debug("%#X,", iooi[i]);
    }
    for (u8 j = 0; key[j] != 0; j++)
        iooi[8+j] = key[j];
    
    sha256_hash(iooi, sizeof(iooi), &sha256value[0]); //生成SHA256码
    
    licence_debug("\r\n active code:\r\n");
    for(u8 jj = 0; jj<32; jj++)
    {
        licence_debug("%#X, ", sha256value[jj]);
    }
    licence_debug("\r\n");

    sys_active((char *)sha256value);
}

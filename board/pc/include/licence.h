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

#ifndef _LICENCE_H_
#define _LICENCE_H_

#include "types_base.h"

char sys_licence_check(void);
char sys_integrity_check(void);
char sys_active(char *active_code);
void active_code_generate(uint64_t code);

#endif

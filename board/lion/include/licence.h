#ifndef _LICENCE_H_
#define _LICENCE_H_

#include "types_base.h"

char sys_licence_check(void);
char sys_integrity_check(void);
char sys_active(char *active_code);
void active_code_generate(uint64_t code);

#endif

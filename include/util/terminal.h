#pragma once

#define CONFIG_ENABLE_UTF8          1
#define CONFIG_ENABLE_FULL_BUFFERED 1

extern int NO_COLOR;

void terminal_setup(void);
void terminal_setdown(void);
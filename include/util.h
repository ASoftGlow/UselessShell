#pragma once

#include "base.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef _WIN32
#define ETR_CHAR '\r'
#define DIR_SEP "\\"
#else
#define ETR_CHAR '\n'
#define DIR_SEP "/"
#endif
#if !defined(errno_t)
typedef int errno_t;
#endif

ENUM(USChar, int16_t,
	Escape = 27,
	BackWord = 127,
	Undo = 26,
	Redo = 25,

	Unknown = 256,
	Exit,

	Left,
	Right,
	Up,
	Down,

	LeftWord,
	RightWord,
	UpWord,
	DownWord,

	Home,
	End,
	PageUp,
	PageDown,
	PageUpWord,
	PageDownWord,

	Delete,
	DeleteWord,

	Details
);

USChar us_getchar(void);
bool us_chrdy(void);
// Gets and creates config directory with name
// @throws EIO
// @throws ENFILE
// @throws EEXIST
errno_t get_cfg_path(_Out_writes_(_MAX_PATH) char* buffer, _In_z_ const char* name);
errno_t create_directory(_In_z_ const char* path);
errno_t delete_directory(_Inout_z_ char* path);
void delete_self(void);
// @returns count
int get_directory_contents(_Inout_z_ char* path, _Out_writes_(max) char contents[][16], int max, bool is_dir);

int strcatc(_Inout_z_ char* str, char c);
bool strislwr(_In_z_ const char* str);
bool iswordend(const char c);
void us_sleep(unsigned long ms);


// puts without newline
static inline int put(_In_z_ const char* str)
{
	return fputs(str, stdout);
}

#pragma once
#include "base.h"

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

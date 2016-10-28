#ifndef _INPUT_3DS_H_
#define _INPUT_3DS_H_

#include "akey.h"

bool N3DS_IsControlPressed();
void N3DS_DrawKeyboard(sf2d_texture *tex);

typedef struct
{
	u16 x, y, w, h;
	s16 keycode;
	u8 flags;
} touch_area_t;

#define TA_FLAG_SLANTED 1

#endif /* _INPUT_3DS_H */

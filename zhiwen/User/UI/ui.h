#ifndef __UI_H__
#define __UI_H__

#include "sys.h"

u8 scan_key(void);
u8 Menu_active(char **menuText, u8 numItems);
u8 scan_key(void);
void Packet_Handle();
void play_Packet_Handle();
void debug_mode(void);
void play(void);
void mode_play(void);

#endif


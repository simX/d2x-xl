/*
 * $Source: /cvs/cvsroot/d2x/arch/ggi/key.c,v $
 * $Revision: 1.2 $
 * $Author: btb $
 * $Date: 2003/02/27 22:07:21 $
 *
 * GGI keyboard input support
 *
 * $Log: key.c,v $
 * Revision 1.2  2003/02/27 22:07:21  btb
 * use timer_delay instead of d_delay
 *
 * Revision 1.1  2001/10/24 09:25:05  bradleyb
 * Moved input stuff to arch subdirs, as in d1x.
 *
 * Revision 1.3  2001/01/29 14:03:57  bradleyb
 * Fixed build, minor fixes
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <ggi/gii.h> 

#include "event.h"
#include "error.h"
#include "key.h"
#include "timer.h"
#include "mono.h"


#define KEY_BUFFER_SIZE 16

static unsigned char Installed = 0;

//-------- Variable accessed by outside functions ---------
unsigned char 		keyd_bufferType;		// 0=No buffer, 1=buffer ASCII, 2=buffer scans
unsigned char 		keyd_repeat;
unsigned char 		keyd_editor_mode;
volatile unsigned char 	keyd_last_pressed;
volatile unsigned char 	keyd_last_released;
volatile unsigned char	keyd_pressed[256];
volatile int		xLastKeyPressTime;

typedef struct Key_info {
	ubyte		state;			// state of key 1 == down, 0 == up
	ubyte		lastState;		// previous state of key
	int		counter;		// incremented each time key is down in handler
	fix		timewentdown;	// simple counter incremented each time in interrupt and key is down
	fix		timehelddown;	// counter to tell how long key is down -- gets reset to 0 by key routines
	ubyte		downcount;		// number of key counts key was down
	ubyte		upcount;		// number of times key was released
} Key_info;

typedef struct keyboard	{
	unsigned short		keybuffer[KEY_BUFFER_SIZE];
	Key_info		keys[256];
	fix			time_pressed[KEY_BUFFER_SIZE];
	unsigned int 		keyhead, keytail;
} keyboard;

static /*volatile*/ keyboard key_data;

char * key_text[256];

unsigned char ascii_table[128] = 
{ 255, 255, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',255,255,
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 255, 255,
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`',
  255, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 255,'*',
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

unsigned char shifted_ascii_table[128] = 
{ 255, 255, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',255,255,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 255, 255,
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 
  255, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 255,255,
  255, ' ', 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,255,255,
  255, 255, 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255 };

int giiKeyTranslate (int keylabel) {
 switch (keylabel)
 {
  case GIIUC_0: return KEY_0;
  case GIIUC_1: return KEY_1;
  case GIIUC_2: return KEY_2;
  case GIIUC_3: return KEY_3;
  case GIIUC_4: return KEY_4;
  case GIIUC_5: return KEY_5;
  case GIIUC_6: return KEY_6;
  case GIIUC_7: return KEY_7;
  case GIIUC_8: return KEY_8;
  case GIIUC_9: return KEY_9;

  case GIIUC_A: return KEY_A;
  case GIIUC_B: return KEY_B;
  case GIIUC_C: return KEY_C;
  case GIIUC_D: return KEY_D;
  case GIIUC_E: return KEY_E;
  case GIIUC_F: return KEY_F;
  case GIIUC_G: return KEY_G;
  case GIIUC_H: return KEY_H;
  case GIIUC_I: return KEY_I;
  case GIIUC_J: return KEY_J;
  case GIIUC_K: return KEY_K;
  case GIIUC_L: return KEY_L;
  case GIIUC_M: return KEY_M;
  case GIIUC_N: return KEY_N;
  case GIIUC_O: return KEY_O;
  case GIIUC_P: return KEY_P;
  case GIIUC_Q: return KEY_Q;
  case GIIUC_R: return KEY_R;
  case GIIUC_S: return KEY_S;
  case GIIUC_T: return KEY_T;
  case GIIUC_U: return KEY_U;
  case GIIUC_V: return KEY_V;
  case GIIUC_W: return KEY_W;
  case GIIUC_X: return KEY_X;
  case GIIUC_Y: return KEY_Y;
  case GIIUC_Z: return KEY_Z;

  case GIIUC_Minus: return KEY_MINUS;
  case GIIUC_Equal: return KEY_EQUAL;
  case GIIUC_Slash: return KEY_DIVIDE;
  case GIIUC_BackSlash: return KEY_SLASH;
  case GIIUC_Comma: return KEY_COMMA;
  case GIIUC_Period: return KEY_PERIOD;
  case GIIUC_Semicolon: return KEY_SEMICOL;

  case GIIUC_BracketLeft: return KEY_LBRACKET;
  case GIIUC_BracketRight: return KEY_RBRACKET;
 
  case GIIUC_Apostrophe: return KEY_RAPOSTRO;
  case GIIUC_Grave:  return KEY_LAPOSTRO;

  case GIIUC_Escape: return KEY_ESC;
  case GIIK_Enter: return KEY_ENTER;
  case GIIUC_BackSpace: return KEY_BACKSP;
  case GIIUC_Tab: return KEY_TAB;
  case GIIUC_Space: return KEY_SPACEBAR;

  case GIIK_NumLock: return KEY_NUMLOCK;
  case GIIK_ScrollLock: return KEY_SCROLLOCK;
  case GIIK_CapsLock: return KEY_CAPSLOCK;

  case GIIK_ShiftL: return KEY_LSHIFT;
  case GIIK_ShiftR: return KEY_RSHIFT;

  case GIIK_AltL: return KEY_LALT;
  case GIIK_AltR: return KEY_RALT;

  case GIIK_CtrlL: return KEY_LCTRL;
  case GIIK_CtrlR: return KEY_RCTRL;

  case GIIK_F1: return KEY_F1;
  case GIIK_F2: return KEY_F2;
  case GIIK_F3: return KEY_F3;
  case GIIK_F4: return KEY_F4;
  case GIIK_F5: return KEY_F5;
  case GIIK_F6: return KEY_F6;
  case GIIK_F7: return KEY_F7;
  case GIIK_F8: return KEY_F8;
  case GIIK_F9: return KEY_F9;
  case GIIK_F10: return KEY_F10;
  case GIIK_F11: return KEY_F11;
  case GIIK_F12: return KEY_F12;

  case GIIK_P0: return KEY_PAD0;
  case GIIK_P1: return KEY_PAD1;
  case GIIK_P2: return KEY_PAD2;
  case GIIK_P3: return KEY_PAD3;
  case GIIK_P4: return KEY_PAD4;
  case GIIK_P5: return KEY_PAD5;
  case GIIK_P6: return KEY_PAD6;
  case GIIK_P7: return KEY_PAD7;
  case GIIK_P8: return KEY_PAD8;
  case GIIK_P9: return KEY_PAD9;
  case GIIK_PMinus: return KEY_PADMINUS;
  case GIIK_PPlus: return KEY_PADPLUS;
  case GIIK_PDecimal: return KEY_PADPERIOD;
  case GIIK_PSlash: return KEY_PADDIVIDE;
  case GIIK_PAsterisk: return KEY_PADMULTIPLY;
  case GIIK_PEnter: return KEY_PADENTER;

  case GIIK_Insert: return KEY_INSERT;
  case GIIK_Home: return KEY_HOME;
  case GIIK_PageUp: return KEY_PAGEUP;
  case GIIK_Delete: return KEY_DELETE;
  case GIIK_End: return KEY_END;
  case GIIK_PageDown: return KEY_PAGEDOWN;
  case GIIK_Up: return KEY_UP;
  case GIIK_Down: return KEY_DOWN;
  case GIIK_Left: return KEY_LEFT;
  case GIIK_Right: return KEY_RIGHT;

  case GIIK_PrintScreen: return KEY_PRINT_SCREEN;
  case GIIK_Pause: return KEY_PAUSE;
 }
 return 0;
}

//killed on 10/03/98 by Matt Mueller
//unsigned char KeyToASCII(int a)
//{
// if (!isprint(a)) return 255;
// if (a & KEY_SHIFTED) {
//  return (toupper((unsigned char) a);
// } else {
//  return ((unsigned char) a);
// }
//}
//end kill -MM

//added on 10/03/98 by Matt Mueller to fix shifted keys (copied from dos/key.c)
unsigned char KeyToASCII(int keycode)
{
	int shifted;

	shifted = keycode & KEY_SHIFTED;
	keycode &= 0xFF;

	if ( keycode>=127 )
		return 255;

	if (shifted)
		return shifted_ascii_table[keycode];
	else
		return ascii_table[keycode];
}
//end addition -MM

void keyboard_handler(int button, ubyte state)
{
	ubyte keyState;
	int i, keycode;
	unsigned short event_key;
	Key_info *key;
	unsigned char temp;

	keyState = state;
	event_key = giiKeyTranslate(button);
	//con_printf (CONDBG,"keyboard_handler(%i,%i):%i\n",button,state,event_key);

	//=====================================================
	//Here a translation from win keycodes to mac keycodes!
	//=====================================================

	for (i = 255; i >= 0; i--) {

		keycode = i;
		key = &(key_data.keys[keycode]);
                if (i == event_key)
			state = keyState;
		else
			state = key->lastState;
			
		if ( key->lastState == state )	{
			if (state) {
				key->counter++;
				keyd_last_pressed = keycode;
				xLastKeyPressTime = TimerGetFixedSeconds();
			}
		} else {
			if (state)	{
				keyd_last_pressed = keycode;
				keyd_pressed[keycode] = 1;
				key->downcount += state;
				key->state = 1;
				key->timewentdown = xLastKeyPressTime = TimerGetFixedSeconds();
				key->counter++;
			} else {	
				keyd_pressed[keycode] = 0;
				keyd_last_released = keycode;
				key->upcount += key->state;
				key->state = 0;
				key->counter = 0;
				key->timehelddown += TimerGetFixedSeconds() - key->timewentdown;
			}
		}
		if ( (state && !key->lastState) || (state && key->lastState && (key->counter > 30) && (key->counter & 0x01)) ) {
			if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT])
				keycode |= KEY_SHIFTED;
			if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT])
				keycode |= KEY_ALTED;
			if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL])
				keycode |= KEY_CTRLED;
                        if ( keyd_pressed[KEY_DELETE] )
                                keycode |= KEYDBGGED;
			temp = key_data.keytail+1;
			if ( temp >= KEY_BUFFER_SIZE ) temp=0;
			if (temp!=key_data.keyhead)	{
				key_data.keybuffer[key_data.keytail] = keycode;
				key_data.time_pressed[key_data.keytail] = xLastKeyPressTime;
				key_data.keytail = temp;
			}
		}
		key->lastState = state;
	}
}

void key_close()
{
	Installed = 0;
}

void key_init()
{
	Installed=1;

	xLastKeyPressTime = TimerGetFixedSeconds();
	keyd_bufferType = 1;
	keyd_repeat = 1;

// Clear the keyboard array
	KeyFlush();
	atexit(key_close);
}

void KeyFlush()
{
 	int i;
	fix curtime;

	if (!Installed)
		key_init();

	key_data.keyhead = key_data.keytail = 0;

	//Clear the keyboard buffer
	for (i=0; i<KEY_BUFFER_SIZE; i++ )	{
		key_data.keybuffer[i] = 0;
		key_data.time_pressed[i] = 0;
	}

	curtime = TimerGetFixedSeconds();

	for (i=0; i<256; i++ )	{
		keyd_pressed[i] = 0;
		key_data.keys[i].state = 1;
		key_data.keys[i].lastState = 0;
		key_data.keys[i].timewentdown = curtime;
		key_data.keys[i].downcount=0;
		key_data.keys[i].upcount=0;
		key_data.keys[i].timehelddown = 0;
		key_data.keys[i].counter = 0;
	}
}

int add_one(int n)
{
 n++;
 if ( n >= KEY_BUFFER_SIZE ) n=0;
 return n;
}

int key_checkch()
{
	int is_one_waiting = 0;
	event_poll();
	if (key_data.keytail!=key_data.keyhead)
		is_one_waiting = 1;
	return is_one_waiting;
}

int KeyInKey()
{
	int key = 0;
	if (!Installed)
		key_init();
        event_poll();
	if (key_data.keytail!=key_data.keyhead) {
		key = key_data.keybuffer[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}
//added 9/3/98 by Matt Mueller to D2_FREE cpu time instead of hogging during menus and such
//	else timer_delay(1);
//end addition - Matt Mueller
        return key;
}

int KeyInKeyTime(fix * time)
{
	int key = 0;

	if (!Installed)
		key_init();
        event_poll();
	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
		*time = key_data.time_pressed[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}
	return key;
}

int key_peekkey()
{
	int key = 0;
        event_poll();
	if (key_data.keytail!=key_data.keyhead)
		key = key_data.keybuffer[key_data.keyhead];

	return key;
}

int key_getch()
{
	int dummy=0;

	if (!Installed)
		return 0;
//		return getch();

	while (!key_checkch())
		dummy++;
	return KeyInKey();
}

unsigned int key_get_shift_status()
{
	unsigned int shift_status = 0;

	if ( keyd_pressed[KEY_LSHIFT] || keyd_pressed[KEY_RSHIFT] )
		shift_status |= KEY_SHIFTED;

	if ( keyd_pressed[KEY_LALT] || keyd_pressed[KEY_RALT] )
		shift_status |= KEY_ALTED;

	if ( keyd_pressed[KEY_LCTRL] || keyd_pressed[KEY_RCTRL] )
		shift_status |= KEY_CTRLED;

#ifdef _DEBUG
	if (keyd_pressed[KEY_DELETE])
		shift_status |=KEYDBGGED;
#endif

	return shift_status;
}

// Returns the number of seconds this key has been down since last call.
fix KeyDownTime(int scancode)
{
	fix timeDown, time;

	event_poll();
        if ((scancode<0)|| (scancode>255)) return 0;

	if (!keyd_pressed[scancode]) {
		timeDown = key_data.keys[scancode].timehelddown;
		key_data.keys[scancode].timehelddown = 0;
	} else {
		time = TimerGetFixedSeconds();
		timeDown = time - key_data.keys[scancode].timewentdown;
		key_data.keys[scancode].timewentdown = time;
	}

	return timeDown;
}

unsigned int keyDownCount(int scancode)
{
	int n;
        event_poll();
        if ((scancode<0)|| (scancode>255)) return 0;

	n = key_data.keys[scancode].downcount;
	key_data.keys[scancode].downcount = 0;

	return n;
}

unsigned int key_upCount(int scancode)
{
	int n;
        event_poll();
        if ((scancode<0)|| (scancode>255)) return 0;

	n = key_data.keys[scancode].upcount;
	key_data.keys[scancode].upcount = 0;

	return n;
}

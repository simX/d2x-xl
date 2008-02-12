/*
 * $Source: /cvs/cvsroot/d2x/arch/svgalib/key.c,v $
 * $Revision: 1.2 $
 * $Author: btb $
 * $Date: 2003/02/27 22:07:21 $
 *
 * SVGALib keyboard input support
 *
 * $Log: key.c,v $
 * Revision 1.2  2003/02/27 22:07:21  btb
 * use timer_delay instead of d_delay
 *
 * Revision 1.1  2001/10/24 09:25:05  bradleyb
 * Moved input stuff to arch subdirs, as in d1x.
 *
 * Revision 1.2  2001/01/29 14:03:57  bradleyb
 * Fixed build, minor fixes
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <vgakeyboard.h> 

#include "event.h"
#include "error.h"
#include "key.h"
#include "timer.h"


#define KEY_BUFFER_SIZE 16

static unsigned char Installed = 0;

//-------- Variable accessed by outside functions ---------
unsigned char 		gameStates.input.keys.nBufferType;		// 0=No buffer, 1=buffer ASCII, 2=buffer scans
unsigned char 		gameStates.input.keys.bRepeat;
unsigned char 		gameStates.input.keys.bEditorMode;
volatile unsigned char 	gameStates.input.keys.nLastPressed;
volatile unsigned char 	gameStates.input.keys.nLastReleased;
volatile unsigned char	gameStates.input.keys.pressed[256];
volatile int		gameStates.input.keys.xLastPressTime;

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

char *pszKeyText[256];

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

void KeyHandler(int scancode, int press)
{
	ubyte state, keyState;
	int i, keycode, event_key;
	Key_info *key;
	unsigned char temp;

	if (press == KEY_EVENTPRESS)
		keyState = 1;
	else if (press == KEY_EVENTRELEASE)
		keyState = 0;
	else
		return;

	event_key = scancode;

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
				gameStates.input.keys.nLastPressed = keycode;
				gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds();
			}
		} else {
			if (state)	{
				gameStates.input.keys.nLastPressed = keycode;
				gameStates.input.keys.pressed[keycode] = 1;
				key->downcount += state;
				key->state = 1;
				key->timewentdown = gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds();
				key->counter++;
			} else {
				gameStates.input.keys.pressed[keycode] = 0;
				gameStates.input.keys.nLastReleased = keycode;
				key->upcount += key->state;
				key->state = 0;
				key->counter = 0;
				key->timehelddown += TimerGetFixedSeconds() - key->timewentdown;
			}
		}
		if ( (state && !key->lastState) || (state && key->lastState && (key->counter > 30) && (key->counter & 0x01)) ) {
			if ( gameStates.input.keys.pressed[KEY_LSHIFT] || gameStates.input.keys.pressed[KEY_RSHIFT])
				keycode |= KEY_SHIFTED;
			if ( gameStates.input.keys.pressed[KEY_LALT] || gameStates.input.keys.pressed[KEY_RALT])
				keycode |= KEY_ALTED;
			if ( gameStates.input.keys.pressed[KEY_LCTRL] || gameStates.input.keys.pressed[KEY_RCTRL])
				keycode |= KEY_CTRLED;
                        if ( gameStates.input.keys.pressed[KEY_DELETE] )
                                keycode |= KEYDBGGED;
			temp = key_data.keytail+1;
			if ( temp >= KEY_BUFFER_SIZE ) temp=0;
			if (temp!=key_data.keyhead)	{
				key_data.keybuffer[key_data.keytail] = keycode;
				key_data.time_pressed[key_data.keytail] = gameStates.input.keys.xLastPressTime;
				key_data.keytail = temp;
			}
		}
		key->lastState = state;
	}
}

void KeyClose()
{
	Installed = 0;
	keyboard_close();
}

void KeyInit()
{
	if (keyboard_init())
		Error ("SVGAlib Keyboard Init Failed");
	Installed=1;

	keyboard_seteventhandler (KeyHandler);
	gameStates.input.keys.xLastPressTime = TimerGetFixedSeconds();
	gameStates.input.keys.nBufferType = 1;
	gameStates.input.keys.bRepeat = 1;

// Clear the keyboard array
	KeyFlush();
	atexit(KeyClose);
}

void KeyFlush()
{
 	int i;
	fix curtime;

	if (!Installed)
		KeyInit();

	key_data.keyhead = key_data.keytail = 0;

	//Clear the keyboard buffer
	for (i=0; i<KEY_BUFFER_SIZE; i++ )	{
		key_data.keybuffer[i] = 0;
		key_data.time_pressed[i] = 0;
	}

//use gettimeofday here:
	curtime = TimerGetFixedSeconds();

	for (i=0; i<256; i++ )	{
		gameStates.input.keys.pressed[i] = 0;
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

int KeyCheckChar()
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
		KeyInit();
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
		KeyInit();
        event_poll();
	if (key_data.keytail!=key_data.keyhead)	{
		key = key_data.keybuffer[key_data.keyhead];
		*time = key_data.time_pressed[key_data.keyhead];
		key_data.keyhead = add_one(key_data.keyhead);
	}
	return key;
}

int KeyPeekKey()
{
	int key = 0;
        event_poll();
	if (key_data.keytail!=key_data.keyhead)
		key = key_data.keybuffer[key_data.keyhead];

	return key;
}

int KeyGetChar()
{
	int dummy=0;

	if (!Installed)
		return 0;
//		return getch();

	while (!KeyCheckChar())
		dummy++;
	return KeyInKey();
}

unsigned int KeyGetShiftStatus()
{
	unsigned int shift_status = 0;

	if ( gameStates.input.keys.pressed[KEY_LSHIFT] || gameStates.input.keys.pressed[KEY_RSHIFT] )
		shift_status |= KEY_SHIFTED;

	if ( gameStates.input.keys.pressed[KEY_LALT] || gameStates.input.keys.pressed[KEY_RALT] )
		shift_status |= KEY_ALTED;

	if ( gameStates.input.keys.pressed[KEY_LCTRL] || gameStates.input.keys.pressed[KEY_RCTRL] )
		shift_status |= KEY_CTRLED;

#ifdef _DEBUG
	if (gameStates.input.keys.pressed[KEY_DELETE])
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

	if (!gameStates.input.keys.pressed[scancode]) {
		timeDown = key_data.keys[scancode].timehelddown;
		key_data.keys[scancode].timehelddown = 0;
	} else {
		time = TimerGetFixedSeconds();
		timeDown = time - key_data.keys[scancode].timewentdown;
		key_data.keys[scancode].timewentdown = time;
	}

	return timeDown;
}

unsigned int KeyDownCount(int scancode)
{
	int n;
        event_poll();
        if ((scancode<0)|| (scancode>255)) return 0;

	n = key_data.keys[scancode].downcount;
	key_data.keys[scancode].downcount = 0;

	return n;
}

unsigned int KeyUpCount(int scancode)
{
	int n;
        event_poll();
        if ((scancode<0)|| (scancode>255)) return 0;

	n = key_data.keys[scancode].upcount;
	key_data.keys[scancode].upcount = 0;

	return n;
}

#include "stc12c5a60s2.h"

#ifndef	KEYBOARD
#define KEYBOARD
						 
#define KeyReturn	10
#define KeyUp		11
#define KeyMenu		12

#define	KeyLeft		13
#define	KeyEnter	14
#define KeyRight	15

#define KeyF1		16
#define KeyDown		17
#define KeyF2		18
#define KeyF3		19
#define KeyF4		20

#define NULLKey		0xFF
#define KeyDelay	300

uchar GetKey(uint Delayms);
void InitKeyboard();
#endif
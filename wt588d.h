#include "stc12c5a60s2.h"
#include "delay.h"

#ifndef WT588D
#define WT588D

sbit WT588D_SDA = P1^4;
sbit WT588D_CS = P1^6;
sbit WT588D_SCL = P1^3;
sbit WT588D_BUSY = P2^4;

#define FunctionMaxWait 250			//250 = 2500ms
#define	WaitIfBusy		1
#define NotWait			0
#define BUSY			0
#define NOTBUSY			1
#define	Dot				11
#define Miao			12
#define ReadyToGoBeep	13
#define DaoDianBeep		14
#define JiShuFanGuiBeep 15
#define ReadyToGo		16
#define JiShuFanGui		17

void SendThreeLines(unsigned char addr, uchar Wait);
#endif
#include "stc12c5a60s2.h"
#include "delay.h"
#include "WT588D.h"


void SendThreeLines(unsigned char addr, uchar Wait) //三线发码子程序,程序最长执行2.5S
{
	unsigned char i;

	if(Wait == WaitIfBusy)
	{
		i = 0;
		if(WT588D_BUSY != BUSY) delayms(20);
		while((WT588D_BUSY == BUSY) && (i++ < FunctionMaxWait))
		{
			delayms(10);
		}
	}

	WT588D_CS = 0;
	delayms(5);
	for(i = 0; i < 8; i++)
	{
		WT588D_SCL = 0;
		WT588D_SDA = (addr & 1) ? 1 : 0; 
		addr >>= 1;
		delay100us();
		WT588D_SCL = 1;
		delay100us();
	}
	WT588D_CS = 1;
	if(Wait == WaitIfBusy)
	{
		delayms(30);
	}
}

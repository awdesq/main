#include "KeyBoard.h"
#include "Delay.h"
#include "main.h"
#include "wt588d.h"
#include "define.h"

void InitKeyboard()
{
	//将行设置为只读
	P1M1 |= 0x07;	//P1M1低3位设置成1
	P1M0 &= 0xF8;	//P1M0低3位设置成0

	//将列设置为强上拉
	P0M1 = 0x00;
	P0M0 = 0x7F;

	//将列设置为低
	P0 = 0x7F;
}

//三行连在P1.0 P1.1 P1.2上,用一个10K电阻拉低,而且是只读,所以常态读出为低
//七列连在P0.0 --- P0.6 上,如果将其设置为高电平,只要任意一个按键按下,就会使那一行读出为高
uchar GetKey(uint Delayms)
{
	//Key[行][列] 这个为参数表
	uchar code Key[3][7] = {{KeyReturn, KeyUp, KeyMenu, 6, 7, 8, 9}, \
							{KeyLeft, KeyEnter, KeyRight, 2, 3, 4, 5}, \
							{KeyF1, KeyDown, KeyF2, 1, 0, KeyF3, KeyF4}};

/*
	#ifndef SINGLEDEBUG
	uchar code Sound[3][7] = {{0xFF, 0xFF, 0xFF, 6, 7, 8, 9}, \
							  {0xFF, 0xFF, 0xFF, 2, 3, 4, 5}, \
							  {0xFF, 0xFF, 0xFF, 1, 0, 0xFF, 0xFF}};
	#endif
*/							
	uchar Row, Colum;

	//为保证检测的速度, 在此加入一个快速检测,没有按键时快速返回
	if(((P1 & 0x07) == 0) && (Delayms == 0))
		return NULLKey;

	for(Colum = 0; Colum < 7; Colum++)
	{
		P0 = (1 << Colum);
		if((P1 & 0x07) != 0)
		{
			Row = (P1 & 0x07);
			break;
		}
	}
	switch(Row)
	{
		case 1: Row = 0; break;
		case 2: Row = 1; break;
		case 4: Row = 2; break;
		default: Row = 3; break;
	}
	delayms(Delayms); 					//延时,不然读得太快
	P0 = 0x7F;							//仍然设置为高,以低于随时读出有没有按键
	if((Row != 3) && (Colum != 7))
	{
		Beep(10);
		return Key[Row][Colum];
	}
	else
		return NULLKey;
}
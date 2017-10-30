#include "KeyBoard.h"
#include "Delay.h"
#include "main.h"
#include "wt588d.h"
#include "define.h"

void InitKeyboard()
{
	//��������Ϊֻ��
	P1M1 |= 0x07;	//P1M1��3λ���ó�1
	P1M0 &= 0xF8;	//P1M0��3λ���ó�0

	//��������Ϊǿ����
	P0M1 = 0x00;
	P0M0 = 0x7F;

	//��������Ϊ��
	P0 = 0x7F;
}

//��������P1.0 P1.1 P1.2��,��һ��10K��������,������ֻ��,���Գ�̬����Ϊ��
//��������P0.0 --- P0.6 ��,�����������Ϊ�ߵ�ƽ,ֻҪ����һ����������,�ͻ�ʹ��һ�ж���Ϊ��
uchar GetKey(uint Delayms)
{
	//Key[��][��] ���Ϊ������
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

	//Ϊ��֤�����ٶ�, �ڴ˼���һ�����ټ��,û�а���ʱ���ٷ���
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
	delayms(Delayms); 					//��ʱ,��Ȼ����̫��
	P0 = 0x7F;							//��Ȼ����Ϊ��,�Ե�����ʱ������û�а���
	if((Row != 3) && (Colum != 7))
	{
		Beep(10);
		return Key[Row][Colum];
	}
	else
		return NULLKey;
}
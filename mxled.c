#include "stc12c5a60s2.h"
#include "mxled.h"
#include "lcd12864.h"
#include "SoftReset.h"
#include "delay.h"
#include "main.h"

/*
void InitUartForMXLED(void)		//9600bps@12MHz
{
	EA = 0;				//关中断
	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	BRT = 0xD9;			//设定独立波特率发生器重装值
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	RI = TI = 0;		//清零标志位

//	UART_busy = 0;		//忙标志位
	ES = 1;				//开中断
	EA = 1;				//开中断
}
*/	
void SendMXString(uchar *myStr, uchar where, uchar IsWaitOK)
{
	uchar code MXGuide[3] = {0x16, 0xa5, 0x04}, MXconstMXCode[3][2] = {{0xA1, 0xA4},{0xA2, 0xA5},{0xA3, 0xA6}};
	uchar MXDataLen, MXAddress, MXCode, MXOrder, MXxor[2], *pStart, i;

	MXDataLen = MXOrder = i = MXxor[0] = MXxor[1] = 0;
	MXAddress = 0x01;
	pStart = myStr;
	while(1)
	{
		if(*myStr != 0) MXDataLen += ((*myStr) & 0x80) ? 1 : 2;										//移动指针,记录长度								
		if((MXDataLen == 32) || (*myStr == 0))														//是否满足发送条件
		{
			MXCode = (MXDataLen == 32) ? MXconstMXCode[where - 1][0] : MXconstMXCode[where - 1][1];	//写入MXCode
			WriteUARTSB(MXGuide[0]);
			WriteUARTSB(MXGuide[1]);
			WriteUARTSB(MXGuide[2]);
			WriteUARTSB(MXDataLen);
			WriteUARTSB(MXAddress);
			WriteUARTSB(MXCode);
			WriteUARTSB(MXOrder);
			MXxor[0] = MXDataLen ^ MXCode;
			MXxor[1] = MXAddress ^ MXOrder;
			while((pStart <= myStr) && (*pStart != 0))
			{
				if(((*pStart) & 0x80) == 0){ WriteUARTSB(0); MXxor[i++ % 2] ^= 0; } 
				WriteUARTSB(*pStart);
				MXxor[i++ % 2] ^= *pStart++;
			}
			WriteUARTSB(MXxor[0]);
			WriteUARTSB(MXxor[1]);
			TrigeUARTSend();
			if(IsWaitOK) WaitReturnOK();
		   	if(MXDataLen < 32) break;
			if(MXDataLen == 0) break;
			MXDataLen = i = 0;
			MXOrder++;
			MXxor[0] = MXxor[1] = 0;
		}
		myStr++;
	} 
}

void SendMXTime(uint t)
{
	uchar str[] = {"00.00 秒"}; //1123
	str[4] = t % 10 + '0';
	str[3] = (t / 10) % 10 + '0';
	str[1] = (t / 100) % 10 + '0';
	str[0] = (t / 1000) % 10 + '0';
	if(str[0] == '0') str[0] = ' ';
	SendMXString(str, 3, 0);
}

void SendMXCode(uchar myMXCode, uchar myMXContent, uchar IsWaitOK)
{

	uchar code MXGuide[3] = {0x16, 0xa5, 0x04};
	uchar MXDataLen, MXAddress, MXCode, MXOrder, MXxor[2];

	MXDataLen = (myMXContent == MXCodeNULLContent) ? 0 : 1;
	MXAddress = 1;
	MXCode = myMXCode;
	MXOrder = 0x00;
	MXxor[0] = MXxor[1] = 0;

	WriteUARTSB(MXGuide[0]);
	WriteUARTSB(MXGuide[1]);
	WriteUARTSB(MXGuide[2]);
	WriteUARTSB(MXDataLen);
	WriteUARTSB(MXAddress);
	WriteUARTSB(MXCode);
	WriteUARTSB(MXOrder);
	MXxor[0] = MXDataLen ^ MXCode;
	MXxor[1] = MXAddress ^ MXOrder;
	if(myMXContent != MXCodeNULLContent)
	{
		WriteUARTSB(myMXContent);
		MXxor[0] ^= myMXContent;
	}
	WriteUARTSB(MXxor[0]);
	WriteUARTSB(MXxor[1]);
	TrigeUARTSend();
	if(IsWaitOK) WaitReturnOK();
}

void WaitReturnOK()
{
	uchar _Count = 250;
	MXIsOK = 0;
	while(--_Count)
	{
		if(MXIsOK)
		{
			MXIsOK = 0;
			delayms(100);
			break;
		}
		delayms(4);
	}
}
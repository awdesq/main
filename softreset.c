#include "stc12c5a60s2.h"
#include "Delay.h"
#include "SoftReset.h"
#include "LCD12864.h"
#include "mxled.h"
#include "main.h"

#ifdef SOFTRESET
//pUARTSBCur 	用户程序往发送缓冲区存数据的指针
//pUARTSBSend 	串口中断从发送缓冲区取数据的指针
//pUARTRBCur	用户程序从接收缓冲区取数据的指针
//pUARTRBRecv	串口中断往接收缓冲区存数据的指针

uchar UARTSB[UARTSBLens], *pUARTSBCur, *pUARTSBSend;
//uchar UARTRB[UARTRBLens], *pUARTRBCur, *pUARTRBRecv;

//复位程序和铭星显示屏临时用到,因为不想占用时间来查询
bit MXIsOK;
uchar UARTMXLedBuf[2];

void InitUART()		//9600bps@12MHz
{
	EA = 0;				//关中断
	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	BRT = 0xD9;			//设定独立波特率发生器重装值
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	RI = TI = 0;		//清零标志位

	pUARTSBCur = pUARTSBSend = UARTSB;				//初始化发送指针和发送缓冲区数据指针
//	pUARTRBCur = pUARTRBRecv = UARTRB;				//初始化接收指针和接收缓冲区数据指针

	UARTMXLedBuf[0] = UARTMXLedBuf[1] = MXIsOK = 0;	//显示屏和复位程序临时用到

	ES = 1;				//开中断
	EA = 1;				//开中断
}

void UARTInterruptForDownload() interrupt 4
{
	if(RI)
	{
		/*
		*pUARTRBRecv = SBUF;						//存储收数据
		if(pUARTRBRecv == UARTRB + UARTRBLens - 1)	//移动存数据指针
		{
			pUARTRBRecv = UARTRB;					
		}
		else
		{
			pUARTRBRecv++;
		}
		if(pUARTRBCur == pUARTRBRecv)				//如有必要,移动取数据指针
		{
			if(pUARTRBCur == UARTRB + UARTRBLens - 1)
			{
				pUARTRBCur = UARTRB;
			}
			else
			{
				pUARTRBCur++;
			}
		}
		*/
		UARTMXLedBuf[0] = UARTMXLedBuf[1];
		UARTMXLedBuf[1] = SBUF;
		//刷程序用到
		if(UARTMXLedBuf[0] == SelfDefineISPDownloadCommand)
		{
			delayms(2000);
			SoftResetToISPMonitor();
		}
		//与LED屏通讯用到		
		if((UARTMXLedBuf[0] == 0x4F) && (UARTMXLedBuf[1] == 0x4B))
		{
			MXIsOK = 1;
			UARTMXLedBuf[0] = UARTMXLedBuf[1] = 0;
		}

		RI = 0;
	}
	if(TI)
	{
		if(pUARTSBSend != pUARTSBCur)
		{
			SBUF = *pUARTSBSend;						//发送1个字节
			if(pUARTSBSend == UARTSB + UARTSBLens - 1)	//移动缓冲区指针
			{
				pUARTSBSend = UARTSB;
			}
			else
			{
				pUARTSBSend++;
			}
		}
		TI = 0;
	}
}

void WriteUARTSB(unsigned char c)
{
	if((pUARTSBCur < UARTSB + UARTSBLens - 1) && (pUARTSBCur + 1 != pUARTSBSend))		//存数据指针没有到尾部,且可以继续存入
	{
		*pUARTSBCur = c;				//存入缓冲区
		pUARTSBCur++;					//移动指针
	}
	else if((pUARTSBCur == UARTSB + UARTSBLens - 1) && (pUARTSBSend != UARTSB))			//已经到了尾部,但发送指针没有在首部
	{
		*pUARTSBCur = c;				//存入缓冲区
		pUARTSBCur = UARTSB;			//移动指针
	}
}

void TrigeUARTSend()
{
	TI = 1;
}

void SoftResetToISPMonitor()
{
	IAP_CONTR = 0x60;				//IAP控制符
}
#endif
#include "stc12c5a60s2.h"
#include "Delay.h"
#include "SoftReset.h"
#include "LCD12864.h"
#include "mxled.h"
#include "main.h"

#ifdef SOFTRESET
//pUARTSBCur 	�û����������ͻ����������ݵ�ָ��
//pUARTSBSend 	�����жϴӷ��ͻ�����ȡ���ݵ�ָ��
//pUARTRBCur	�û�����ӽ��ջ�����ȡ���ݵ�ָ��
//pUARTRBRecv	�����ж������ջ����������ݵ�ָ��

uchar UARTSB[UARTSBLens], *pUARTSBCur, *pUARTSBSend;
//uchar UARTRB[UARTRBLens], *pUARTRBCur, *pUARTRBRecv;

//��λ�����������ʾ����ʱ�õ�,��Ϊ����ռ��ʱ������ѯ
bit MXIsOK;
uchar UARTMXLedBuf[2];

void InitUART()		//9600bps@12MHz
{
	EA = 0;				//���ж�
	PCON &= 0x7f;		//�����ʲ�����
	SCON = 0x50;		//8λ����,�ɱ䲨����
	BRT = 0xD9;			//�趨���������ʷ�������װֵ
	AUXR |= 0x04;		//���������ʷ�����ʱ��ΪFosc,��1T
	AUXR |= 0x01;		//����1ѡ����������ʷ�����Ϊ�����ʷ�����
	AUXR |= 0x10;		//�������������ʷ�����
	RI = TI = 0;		//�����־λ

	pUARTSBCur = pUARTSBSend = UARTSB;				//��ʼ������ָ��ͷ��ͻ���������ָ��
//	pUARTRBCur = pUARTRBRecv = UARTRB;				//��ʼ������ָ��ͽ��ջ���������ָ��

	UARTMXLedBuf[0] = UARTMXLedBuf[1] = MXIsOK = 0;	//��ʾ���͸�λ������ʱ�õ�

	ES = 1;				//���ж�
	EA = 1;				//���ж�
}

void UARTInterruptForDownload() interrupt 4
{
	if(RI)
	{
		/*
		*pUARTRBRecv = SBUF;						//�洢������
		if(pUARTRBRecv == UARTRB + UARTRBLens - 1)	//�ƶ�������ָ��
		{
			pUARTRBRecv = UARTRB;					
		}
		else
		{
			pUARTRBRecv++;
		}
		if(pUARTRBCur == pUARTRBRecv)				//���б�Ҫ,�ƶ�ȡ����ָ��
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
		//ˢ�����õ�
		if(UARTMXLedBuf[0] == SelfDefineISPDownloadCommand)
		{
			delayms(2000);
			SoftResetToISPMonitor();
		}
		//��LED��ͨѶ�õ�		
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
			SBUF = *pUARTSBSend;						//����1���ֽ�
			if(pUARTSBSend == UARTSB + UARTSBLens - 1)	//�ƶ�������ָ��
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
	if((pUARTSBCur < UARTSB + UARTSBLens - 1) && (pUARTSBCur + 1 != pUARTSBSend))		//������ָ��û�е�β��,�ҿ��Լ�������
	{
		*pUARTSBCur = c;				//���뻺����
		pUARTSBCur++;					//�ƶ�ָ��
	}
	else if((pUARTSBCur == UARTSB + UARTSBLens - 1) && (pUARTSBSend != UARTSB))			//�Ѿ�����β��,������ָ��û�����ײ�
	{
		*pUARTSBCur = c;				//���뻺����
		pUARTSBCur = UARTSB;			//�ƶ�ָ��
	}
}

void TrigeUARTSend()
{
	TI = 1;
}

void SoftResetToISPMonitor()
{
	IAP_CONTR = 0x60;				//IAP���Ʒ�
}
#endif
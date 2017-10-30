#include "STC12C5A60S2.h"
#include "main.h"
#include "KeyBoard.h"
#include "LCD12864.h"
#include "MyMenu.h"
#include "delay.h"
#include "24c02.h"
#include "define.h"
#include "usecommunication.h"
#include "SoftReset.h"
#include "MXLED.h"
#include "WT588D.h"

/*
20121017	��Menu�м���һ��Beep(10) ��Ϊ���޸���1��׮������Beepʱ,����˵���һֱ����ȥ��Bug
20121017	��Ϊ�Ķ���Test10T�Ĺ���ʱ��,���Բ��Եȴ���Ϣ�иĶ���һ����delayms(10), ��Ϊdelayms(20)
20121102	ɾ���������
20121103	ɾ��һЩ�˵�,���µ����˵�
20121113	��Menu��ɾ����Beep(10)���,��GetKey()�м�����Beep(10),�ṩ������ʾ��
*/	

extern uchar isRoot;
extern uchar code ThisUser[];
typedef void(*VoidFunctionPoint)(void);
VoidFunctionPoint code FunctionTable[] = {
	FactorySet,				// 0	�ָ�Ĭ������
	SetIDs,					// 1	���ôӻ�ID
	SetBeepType,			// 2	����Ϊ���������������С�������������
	ViewMsg2,				// 3	�鿴��Ϣ
	MsgTest,				// 4	���Է�����Ϣ���ȶ���
	SetTimeOver,			// 5	���ó�ʱ
	SetPassword,			// 6	��������
	SetWorkMode,			// 7	���ù���ģʽ
	SetRemainTimes,			// 8	����ʣ�����
	SetLiangDu,				// 9	��������
	SetYingLiang,			// 10	��������
	SetMDSJXSFF,			// 11	����ÿ��ʱ����ʾ����
	QuickSetIDs,			// 12	��������ID
	ZhuangSelfTest,			// 13	�ϰ汾��׮�Բ��Գ���
	ZhuJiTest				// 14 	�����Բ���
	};

void Menu(void)
{
	uchar MenuDraw;
	//20121017 �޸�
	//Beep(20);
	//20140929���������в˵�ɾ�������������û����õ��ĺ�һ���߼����ã�
	//�ڳ����н���һ��������񣬴��ж�ȡ�����в�ͬ�Ĺ���
	//����Ҫ����ʲô���ܣ�ֻ��ͨ�������
Menu1:
	MenuDraw = 0;
	delayms(500);
	while(1)
	{
		if(MenuDraw == 0)
		{
			MenuDraw = 1;
			LCDWrite(WriteCode, LCD_CLEAR);
			delayms(10);
			LCDSetXY(1, 1);
			LCDWriteString("1 ��ʼ����");
			LCDSetXY(1, 2);
			LCDWriteString("2 ����ϵͳ");
			LCDSetXY(1, 3);
			LCDWriteString("3 ��׮���");
			if(IsRoot){
				LCDSetXY(1, 4);
				LCDWriteString("4 �߼�����");
			}else{
				LCDSetXY(1, 4);
				LCDWriteString("4 ͳ����Ϣ");
			}
		}
		switch(GetKey(KeyDelay))
		{
			//�������أ���ʼ���Թ�����
			case 1: LCDWrite(WriteCode, LCD_CLEAR);delayms(3);return;
			//������Ƶ���ϵͳ
			case 2: 
				CommunicationTest(); 

				LCDWrite(WriteCode, LCD_CLEAR);
				delayms(1000);
				InfraredTest(); 

				LCDWrite(WriteCode, LCD_CLEAR);
				delayms(1000);
				IRPositionTest();
				
				LCDWrite(WriteCode, LCD_CLEAR);
				delayms(1000);				 
				
				goto Menu1;
			//��׮����ȫ����
			case 3: IRRecTimeLine(); goto Menu1;
			//�߼������������
		   	case 4: 
				if(isRoot){
					RunFunction();
					delayms(500);
				}else{
					DisplaySumary();
					delayms(500);
				}
				goto Menu1;

			case KeyReturn:
				return;
			//��ʾͳ������
			case KeyF1:
				DisplaySumary();
				delayms(500);
				goto Menu1;
			//�鿴�ɼ�
			case KeyF2:
				ViewRecord();
				delayms(500);
				goto Menu1;
			//�رձ���
			case KeyF3:
				LCD_PSW = LCDBKLightON;
				break;
			//�򿪱���
			case KeyF4:
				LCD_PSW = LCDBKLightOFF;
				break;
			default:break;
		}

	}
	delayms(500);
}


//�뱣֤pBuffer��N��һ���ֽ�
//���û�����룬����0
uchar NumberInput(uchar *pBuffer, uchar N, char IsPassword, uchar x, uchar y)
{
	uchar myKey, i = 0, j;
	LCDSetXY(x, y);
	while(1)
	{
		myKey = GetKey(KeyDelay);
		switch(myKey)
		{
			case KeyLeft: 	//�ø��ӵ�ɾ�ַ�����
				if(i > 0)
				{
					i--;
					*(--pBuffer) = 0;
					if(IsPassword == 0)
					{
						LCDSetXY(x, y);
						LCDWriteString((pBuffer - i)); 
						LCDWrite(WriteData, ' ');
						LCDSetXY(x, y);
						LCDWriteString((pBuffer - i)); 
					}
					else
					{
						LCDSetXY(x, y);
						for(j = 0; j < i; j++)
							LCDWrite(WriteData, '*');
						LCDWrite(WriteData, ' ');
						LCDSetXY(x, y);
						for(j = 0; j < i; j++)
							LCDWrite(WriteData, '*');
					}
				}
				break;

			case KeyReturn:
				return 0;

			case KeyEnter:
				*pBuffer = 0;
				return i;
				break;

			default:
				if((myKey >= 0) && (myKey <= 9) && (i < N ))
				{
					*pBuffer = myKey + '0';
					if(IsPassword == 1)
						LCDWrite(WriteData, '*');
					else
						LCDWrite(WriteData, *pBuffer);
					i++;
					pBuffer++;
				}
				break;
		} 
	}
}

/*
��¼����ַ������Ϊ����
*/
uint CharToNum(uchar* pData)
{
	uint myNum = 0;
	do
	{
		myNum *= 10;
		if((*pData >= '0') && (*pData <= '9'))
		{
			myNum += (*pData - '0');
		}
		pData++;
	}while(*pData != 0);
	return myNum;
}



void SetIDs()
{
	uchar pData[3], tmp;

	RestartSub();
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(2);
	//����׮����1����������
	if(GetOnLineIDs() > 1){
		LCDSetXY(1, 1);
		LCDWriteString("����: ����׮̫��");
		LCDSetXY(1, 2);
		LCDWriteString("��ֻ����һ��������");
		LCDSetXY(1, 3);
		LCDWriteString("���������ϣ�");
		delayms(2000);
		return;
	}

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("������ID: ");
	LCDSetXY(1, 2);
	LCDWriteString("A:1-6 B:9-14");
	LCDSetXY(1, 3);
	LCDWriteString("����: ϵͳ���Ի�");
	LCDSetXY(1, 4);
	LCDWriteString("������ȡ������");

	//��������˷��ؼ�����ִ�в���
	if(NumberInput(pData, 2, 0, 6, 1) == 0)
		return;

	tmp = CharToNum(pData);
	LCDSetXY(1, 4);
	if(((tmp >= 1) && (tmp <= 6)) ||  ((tmp >= 9) && (tmp <= 14)))
	{
		SendMsg(MergeMsg(tmp, CHANGEID));
		SendMsg(MergeMsg(tmp, CHANGEID));
		SendMsg(MergeMsg(tmp, CHANGEID));
		SendMsg(MergeMsg(tmp, CHANGEID));
		LCDWriteString("���óɹ�        ");
	}
	else
	{
		LCDWriteString("���ò���ȷ      ");	
	}
	delayms(1000);
}

void SetTimeOver()
{
	uchar tmp;
	uchar pData[3];
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("Now:");
	LCDPrintfWord((uint)SendMsgWaitTimeDefault * 10 + WorkTime * 100, 0);
	LCDWriteString("ms");
	LCDSetXY(1, 4);
	LCDWriteString("������ǰ��λ");
	LCDSetXY(1, 2);
	LCDWriteString("New:");
	LCDSetXY(4, 2);
	LCDWriteString("00ms");
	NumberInput(pData, 2, 0, 3, 2);
	tmp = (uchar)CharToNum(pData);
	LCDSetXY(1, 4);
	if((tmp >= TimeOverMin) && (tmp <= TimeOverMax))
	{
		WriteByteEEPROM(TimeOverAddr, tmp);
		WorkTime = tmp - SendMsgWaitTimeDefault / 10;
		RestartSub();
		SendMsg(SETWORKTIME);
		SendMsg(WorkTime);
		LCDWriteString("���óɹ�    ");
	}
	else
	{
		LCDWriteString("���ò���ȷ    ");	
	}
	delayms(2000);
}

uchar CommunicationTest()
{
	uchar i, myMsg, ErrorCount = 0, state[12];

	LCDWrite(WriteCode, LCD_CLEAR);								//����
	//��ʼ������
	for(i = 0; i < 12; i++){
		state[i] = 0;		
	}
	while(1){
		ErrorCount = 0;
		for(i = 1; i < 7; i++)									//��1��6��׮����һ����ID��COMMUNICATIONTEST��Ϣ.������������Ϣ,˵��ͨѶ����
		{
			ClearMsg();											//����Ϣ����
			myMsg = MergeMsg(i, COMMUNICATIONTEST);				//׼����Ҫ���Ĳ�����Ϣ
			SendMsg(myMsg);										//��i��׮A����һ��COMMUNICATION_TEST�ź�
			delayms(10);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
			LCDSetXY(((i + 1) % 2) * 5 + 1, ((i + 1) / 2));	  	//���ô�ӡλ��
			if(myMsg == GetMsg())								//ȡ��Ϣ
			{
				//��洢��״̬��ͬʱ��������
				if(state[i - 1] == 0){
					Beep(10);
				}
				//�洢���ڵ�״̬
				state[i - 1] = 1;
				LCDWriteString("OK");							//�ص���Ϣ�ͷ��͵�һ��,��ӡOK
			}
			else
			{
				//��洢��״̬��ͬʱ��������
				if(state[i - 1] == 1){
					Beep(10);
				}
				//�洢���ڵ�״̬
				state[i - 1] = 0;
				ErrorCount++;									//û����Ϣ,��Ϣ��ΪNULLMsg,��ӡ��׮�ĺ���,����¼�������
				LCDWrite(WriteData, i + '0');
				LCDWrite(WriteData, 'A');
			}
	
	
			ClearMsg();													//����Ϣ����
			myMsg = (MergeMsg(GetReflectID(i), COMMUNICATIONTEST));  	//׼����Ҫ���Ĳ�����Ϣ,ע��Ҫ��i�Ļ������Ϸ���׮�ı�־
			SendMsg(myMsg);												//��i��׮B����һ��COMMUNICATION_TEST�ź�
			delayms(10);												//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
			LCDSetXY(((i + 1) % 2) * 5 + 3, ((i + 1) / 2));				//���ô�ӡλ��
			if(myMsg == GetMsg())										//ȡ��Ϣ
			{
				//��洢��״̬��ͬʱ��������
				if(state[i + 6] == 0){
					Beep(10);
				}
				//�洢���ڵ�״̬
				state[i + 6] = 1;
				LCDWriteString("OK");	 								//�ص���Ϣ�ͷ��͵�һ��,��ӡOK
			}
			else
			{
				//��洢��״̬��ͬʱ��������
				if(state[i + 6] == 1){
					Beep(10);
				}
				//�洢���ڵ�״̬
				state[i + 6] = 0;
				ErrorCount++;										 	//û����Ϣ,��Ϣ��ΪNULLMsg,��ӡ��׮�ĺ���,����¼�������
				LCDWrite(WriteData, i + '0');							
				LCDWrite(WriteData, 'B');
			}		
		}
	
		LCDSetXY(1, 4);
		if(ErrorCount > 0)												 //��ӡͳ����Ϣ
		{
			LCDPrintfByte(ErrorCount, 1);
			LCDWriteString(" ��׮ͨѶ�쳣");
		}
		else
		{
			LCDWriteString("    ͨѶ����ͨ��");
			Beep(10);
		}
		if(GetKey(0) != NULLKey){
			return 0;
		}
	}
}

uchar InfraredTest()
{
	uchar i, myMsg, ErrorCount = 0, state[6];

	LCDWrite(WriteCode, LCD_CLEAR);							//����
	for(i = 0; i < 6; i++){
		state[i] = 0;
	}

	while(1){
		ErrorCount = 0;
		for(i = 1; i < 7; i++)
		{
			ClearMsg();											//����Ϣ����
			SendMsg(MergeMsg(GetReflectID(i), TESTOVER));		//��i��׮B�˳�����,����Reflect();							
			SendMsg(MergeMsg(GetReflectID(i), STARTWORK));  	//��i��׮B��ʼ����
			myMsg = MergeMsg(i, INFRAREDTEST);					//׼���ò�����Ϣ
			SendMsg(myMsg);										//��i��׮A����һ��INFRARED_TEST�ź�
			//delayms(10);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
			delayms(20);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms���Ժ�����ͨ��,20ms�㹻�յ��ص���Ϣ��
			SendMsg(MergeMsg(GetReflectID(i), STOPWORK));		//��i��׮B����һ��STOPWORK�ź�
	
			LCDSetXY(((i + 1) % 2) * 5 + 1, ((i + 1) / 2));		//���ô�ӡλ��
			if(myMsg == GetMsg())
			{	
				if(state[i - 1] == 0){
					Beep(10);
				}
				state[i - 1] = 1;									  			
				LCDWriteString("  OK   ");						//�ص���Ϣ�ͷ��͵�һ��,��ӡOK
			}
			else
			{												   	
				ErrorCount++;									//û����Ϣ,��Ϣ��ΪNULLMsg,��ӡ��׮�ĺ���,����¼�������
				if(state[i - 1] == 1){
					Beep(10);
				}
				state[i - 1] = 0;
				LCDWrite(WriteData, i + '0');
				LCDWriteString("A ");
				LCDWrite(WriteData, i + '0');
				LCDWrite(WriteData, 'B');
			}		
		}
	
		LCDSetXY(1, 4);											  //��ӡͳ����Ϣ
		if(ErrorCount > 0)
		{
			LCDPrintfByte(ErrorCount, 1);
			LCDWriteString(" ��׮�����쳣");
		}
		else
		{
			LCDWriteString("    �������ͨ��");
			Beep(10);
		}
		if(GetKey(0) != NULLKey){
			return 0;
		}
	}
}

void FactorySet()
{	
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("�ָ�����������?");
	LCDSetXY(1, 3);
	LCDWriteString("ȷ��--�ָ�");
	LCDSetXY(1, 4);
	LCDWriteString("����--���ָ�");

	while(1)
	{
		switch(GetKey(KeyDelay))
		{
			case KeyEnter:
				LCDWrite(WriteCode, LCD_CLEAR);					//����
				delayms(10);
				LCDSetXY(1, 2);
				LCDWriteString("�ָ���......");						//��ʾ
				WriteByteEEPROM(TimeOverAddr, TimeOverDefault);		//�ָ�TimeOver
				WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			//�ָ�ʹ�ü���
				WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		//�ָ��������
				WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		//�ָ���ѳɼ�
				delayms(100);		
				goto ResetOK;
	
			case NULLKey:
				break;

			default:
				goto NotReset;
		}
	}
ResetOK:
	SoftResetToISPMonitor();
NotReset:
	return;
}


void DisplaySumary()
{
	//���岢��ʼ������
	uchar RecPoint = 0, Count = 0, BackupCount = 0, Success = 0, myKey;
	uint Avg = 0, Best = 0;
	unsigned long Sum = 0; //��ΪҪͳ��NoOfMaxSaved��uint��¼ 100 * 2500 = 250000,����Ӧ����һ��usigned long
		
	//����ƽ���ɼ�
	if(NoSuccess > 0)
	{
		//�ȼ��ϵ�ǰ��ַ�ĳɼ�
		RecPoint = GetCurrentRecordAddr();
		Sum += (unsigned long)ReadWordEEPROM(RecPoint);
		Count++;
		//���ۼӵ�ǰ��ַ֮ǰ�ĳɼ�
		while(1)
		{
			RecPoint = GetPrevRecordAddr(RecPoint);
			if(RecPoint != NULLPoint)
			{
				Sum += (unsigned long)ReadWordEEPROM(RecPoint);
				Count++;
			}
			else
			{
				break;
			}
		}
		Avg = (uint)((unsigned long)Sum  / (unsigned long)Count);
	}

	//���㷸�����
	if(NoUsed > 0)
		Success = (NoUsed - NoSuccess) * 100 / NoUsed;
  
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(100);
	
	//��ӡ����ʹ�ô���
	LCDSetXY(1, 1);
	LCDWriteString("ʹ�ô���: ");
	LCDPrintfWord(NoUsed, 0);
	LCDSetXY(1, 2);
	LCDWriteString("�������: ");
	LCDPrintfWord(Success, 0);
	LCDWriteString("%");
	LCDSetXY(1, 3);
	LCDWriteString("��ѳɼ�: ");
	printftime(FastestRec, 6, 3);
	LCDSetXY(1, 4);
	LCDWriteString("ƽ���ɼ�: ");
	printftime(Avg, 6, 4);
	//�ȴ�1S��ȴ�����
	delayms(1000);
	while(GetKey(KeyDelay) == NULLKey);
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("����ʹ���ܴ���: ");
	LCDSetXY(1, 2);
	LCDPrintfWord(CanotResetNo, 0);
	LCDSetXY(1, 3);
	LCDWriteString("��F3�������");
	LCDSetXY(1, 4);
	//�������ַ����ƶ�����Main.C��
	LCDWriteString(ThisUser);
	delayms(1000);
	while(1){
		myKey = GetKey(KeyDelay);
		if(myKey != NULLKey){
			if(myKey == KeyF3){
				WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			//�ָ�ʹ�ü���
				WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		//�ָ��������
				WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		//�ָ���ѳɼ�
				NoUsed = ReadWordEEPROM(NoUsedAddr);
				NoSuccess = ReadWordEEPROM(NoSuccessAddr);
				FastestRec = ReadWordEEPROM(FastestRecAddr);
			}
			delayms(500);
			break;
		}	
	}
}

void ViewRecord()
{
	uchar RecPoint = 0, j = 0, k = 0;
	uchar tmp[15], zushu = 0;
	//����
	LCDWrite(WriteCode, LCD_CLEAR);	
	delayms(100);

   	//��һ��tmp�����¼����Ҫ��ʾ�ĵ�һ���ɼ��ĵ�ַ
	if(NoSuccess > 0)
	{
		tmp[0] = RecPoint = GetCurrentRecordAddr();
		zushu = 1;
	}
	else
	{
		//û������������ʾ
		LCDSetXY(2, 2);
		LCDWriteString("�����ݼ�¼");
		delayms(500);
		return;
	}

	//ͳ��һ��8������,���ж�����,ÿһ����ʼ��ַ��ʲô
	for(j = 1; j < 15; j++)
	{
		for(k = 0; k < 8; k++)
		{
			if(GetPrevRecordAddr(RecPoint) != 0xFF)
				RecPoint = GetPrevRecordAddr(RecPoint);
			else
				break;
		}
		if(k == 8)
		{
			tmp[j] = RecPoint;
			zushu++;
		}
	}
	
	k = 0;
DisplayRecord:
	{
		RecPoint = tmp[k];
		LCDSetXY(1, 1);
		LCDPrintfByte(k + 1, 0);
		for(j = 2; j < 10; j++)
		{
			printftime(ReadWordEEPROM(RecPoint), ((j % 2) + 1) * 3, j / 2);
			if(GetPrevRecordAddr(RecPoint) != NULLPoint)
				RecPoint = GetPrevRecordAddr(RecPoint);
			else
				break;
		}
		
		while(1)
		{
			switch(GetKey(KeyDelay))
			{
				case KeyRight:
				case KeyDown:
					if(k == 0)
						k = zushu - 1;
					else
						k--;
					LCDWrite(WriteCode, LCD_CLEAR);	
					delayms(100);
					goto DisplayRecord;

				case KeyUp:
				case KeyLeft:
					if(k == zushu - 1)
						k = 0;
					else
						k++;
					LCDWrite(WriteCode, LCD_CLEAR);	
					delayms(100);
					goto DisplayRecord;

				case KeyReturn:
					return;
				
				default:
					break;
			}
		}	
	}	
}

void IRPositionTest()
{
	uchar myID, debugMsg, lastID = 0;

StartIRPositionTest:	
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	RestartSub();									//������������׮
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("��Ҫ�����ĸ�׮��");
	LCDSetXY(1, 2);
	LCDWriteString("�ߵ�׼ȷ��:");
   	//�õ��û�����,���û�а����κμ�һֱ�ȴ�
	//��ʱ��һ��debugMsg,�õȴ�������ʱ������Ϊ5����
	debugMsg = 5 * 5;
	while(debugMsg--)
	{
		//��ʾ����ʱ10��
		LCDSetXY(8, 4);
		LCDPrintfByte(debugMsg / 5, 0);
		//���԰���
		myID = GetKey(KeyDelay);
		//�жϰ���
		if(myID != NULLKey){
			//��������˷��ؼ������˳�
			if(myID == KeyReturn){
				return;
			}
			lastID = myID;
			break;
		}
	}
	if(myID == NULLKey){
		//Ĭ�ϴ�1��׮��ʼ����
		myID = ++lastID;
	}
	//������ǰ���1-6�����˳�����
	if((myID >= 1) && (myID <= 6))
	{
		//��ID��׮����׮�˳����Ժ�����
		SendMsg(MergeMsg(GetReflectID(myID), TESTOVER));
		delayms(100); 
		//��ID��׮�ķ���׮��ʼ����
		SendMsg(MergeMsg(GetReflectID(myID), STARTWORK));
		//���� 
		LCDWrite(WriteCode, LCD_CLEAR);		
		delayms(100);
		//��ӡ��ʾ
		LCDSetXY(2, 2);
	   	LCDPrintfByte(myID, 0);
		LCDWriteString(" ��׮������..");
		//��ID��׮�ķ���׮��ʼ����׼�Ȳ���
		SendMsg(MergeMsg(myID, IRPOSITIONTEST));
		SendMsg(MergeMsg(myID, STARTWORK));
		//����ID��׮��������Ϣ
		while(1)
		{
			//����Ϣ����������Ϣ
			if(GetMsgCount() > 0)
			{
				debugMsg = GetMsg();
				//��Ӧ׮��������Ϣ
				if(GetIDFromMsg(debugMsg) == myID)
				{
					if(GetMsgStyle(debugMsg) == REACHED)
					{
						Speak = StartBeep;
					}
					else if(GetMsgStyle(debugMsg) == LEAVED)
					{
						Speak = StopBeep;
					}
				}
			}
			else if(GetKey(0) != NULLKey)
			{
				//ֹͣ����
				Speak = StopBeep;
				//��ID��׮�ķ���׮ֹͣ����
				SendMsg(MergeMsg(myID, STOPWORK));
				//��ID��׮�ķ���׮Ҳֹͣ����
				SendMsg(MergeMsg(GetReflectID(myID), STOPWORK));
				delayms(200);
				//ȡ��������Ϣ�����¿�ʼ
				while(GetMsg() != NULLMsg);
				Beep(10);
				delayms(1000);
				//Ϊ�򻯳�����ƣ�������һ��׮����gogo���ص�ǰ�����µ�����һ��׮
				goto StartIRPositionTest;
			}
		}
	}
}

void SetBeepType()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("�밴����ѡ����Դ");
	LCDSetXY(1, 2);
	LCDWriteString("��ǰ:");

	if(BeepType == BeepBuzzer)
		LCDWriteString(" С������");
	else
		LCDWriteString(" �������");

	LCDSetXY(1, 3);
	LCDWriteString("0 �������");
	LCDSetXY(1, 4);
	LCDWriteString("1 С������");

	while(1)
	{
		switch(GetKey(KeyDelay))
		{
			case 0:
				WriteByteEEPROM(BeepTypeAddr, BeepDAC);
				BeepType = BeepDAC;
				goto SetOK;

			case 1:
				WriteByteEEPROM(BeepTypeAddr, BeepBuzzer);
				BeepType = BeepBuzzer;
				goto SetOK;
	
			case KeyReturn:
				goto SetOK;

			case NULLKey:
				break;

			default:
				break;
		}
	}
SetOK:
	return;
}

/*
�˶������EEPROM�洢�������Ƿ���ͬ
*/
uchar CheckPassword()
{
	uchar Password[PasswordLen + 2], InputPassword[PasswordLen + 2], i;

	//����
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);

	//��ջ�����
	for(i = 0; i < PasswordLen; i++) Password[i] = 0;

	//����EEPROM���뵽�ڴ�
	ReadEEPROM(ATMEL24C02, PasswordAddr, Password, PasswordLen);		//�������뵽Password

	//�������õ����볤��
	for(i = 0, Password[PasswordLen] = 0; i < PasswordLen; i++)
		if((Password[i] >= '0') && (Password[i] <= '9'))
			Password[PasswordLen]++;

	//���벻��PasswordLen, ��Ϊû����������
	if(Password[PasswordLen] != PasswordLen)
	{
		LCDSetXY(1, 4);
		LCDWriteString("����Ϊ�ղ��ܲ���");
		delayms(2000);
		return NULLPassword;					//����Ϊ��
	}
	//��ʾ��������
	LCDSetXY(1, 1);
	LCDWriteString("����������: ");
	for(i = 0; i < PasswordLen; i++) InputPassword[i] = 0;
	NumberInput(InputPassword, 6, 1, 1, 2);										//��������
	//�Ƚ�����
	for(i = 0; i < PasswordLen; i++)
	{
		if(Password[i] != InputPassword[i])
			break;
	}
	//��ʾ��������ؽ��
	LCDSetXY(1, 4);
	if(i == PasswordLen)
	{
		LCDWriteString("������ȷ");
		delayms(2000);
		return RightPassword; 											//���벻��ȷ
	}
	else
	{
		LCDWriteString("�������");
		delayms(2000);
		return WrongPassword;											//������ȷ
	}
}

void SetPassword()
{
	uchar pData[8], i;
	uchar CheckResult;
	CheckResult = CheckPassword();
	if((CheckResult == RightPassword) || (CheckResult == NULLPassword))
	{
		LCDWrite(WriteCode, LCD_CLEAR);
		delayms(10);
		LCDSetXY(1, 1);
		LCDWriteString("����������:");
		for(i = 0; i < PasswordLen; i++)
		{
			pData[i] = 0;
		}
		NumberInput(pData, 6, 1, 1, 2);
		for(i = 0; i < PasswordLen; i++)
		{
			if((pData[i] < '0') || (pData[i] > '9'))	break;
		}
		if(i == PasswordLen)
		{
			WriteEEPROM(ATMEL24C02, PasswordAddr, pData, PasswordLen);
			LCDSetXY(5, 4);
			LCDWriteString("���óɹ�");
		}
		else
		{
			LCDSetXY(5, 4);
			LCDWriteString("����ʧ��");
		}	
		delayms(2000);
	}
}

void SetWorkMode()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("�밴����ѡ��ģʽ");
	LCDSetXY(1, 2);
	LCDWriteString("��ǰ:");

	if(WorkMode == WORKMODE_NOLIMIT)
		LCDWriteString(" ����ģʽ");
	else
		LCDWriteString(" �ƴ�ģʽ");

	LCDSetXY(1, 3);
	LCDWriteString("0 ����ģʽ");
	LCDSetXY(1, 4);
	LCDWriteString("1 �ƴ�ģʽ");

	while(1)
	{
		switch(GetKey(KeyDelay))
		{
			case 0:
				if(WorkMode == WORKMODE_NOLIMIT)
				return;

				if(CheckPassword() != RightPassword)
					return;

				WriteByteEEPROM(WorkModeAddr, WORKMODE_NOLIMIT);

				WorkMode = WORKMODE_NOLIMIT;
				goto SetOK;

			case 1:
				if(WorkMode == WORKMODE_COUNTTIMES)
				return;

				if(CheckPassword() != RightPassword)
					return;

				WriteByteEEPROM(WorkModeAddr, WORKMODE_COUNTTIMES);

				WorkMode = WORKMODE_COUNTTIMES;
				goto SetOK;
	
			case KeyReturn:
				goto NotSet;

			case KeyMenu:
				goto NotSet;
		}
	}
SetOK:
	LCDSetXY(5, 4);
	LCDWriteString("���óɹ�");
	delayms(2000);
NotSet:
	return;
}

void SetRemainTimes()
{
	uint tmp;
	uchar pData[6];
	
	//��ȡ,����ʾ����,����������
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("Now:");
	RemainTimes = ReadWordEEPROM(RemainTimesAddr);
	LCDPrintfWord((uint)RemainTimes, 0);
	LCDSetXY(6, 1);
	LCDWriteString("��");
	LCDSetXY(1, 4);
	LCDWriteString("���������");
	LCDSetXY(1, 2);
	LCDWriteString("New:");
	LCDSetXY(6, 2);
	LCDWriteString("��");
	if(NumberInput(pData, 5, 0, 3, 2) == 0)			//û����������ʱ����
		return;
	tmp = CharToNum(pData);

	//ȷ������
	if(CheckPassword() != RightPassword)
		return;

	//д�����
	WriteWordEEPROM(RemainTimesAddr, tmp);
	RemainTimes = tmp;

	//��ʾ
	LCDSetXY(5, 4);
	LCDWriteString("���óɹ�");
	delayms(2000);
}

void SetLiangDu()
{
	//��������
	uchar tmp;
	uchar pData[3];

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("��������λ��0-15");
	LCDSetXY(1, 2);
	LCDWriteString("New:");
	NumberInput(pData, 2, 0, 3, 2);
	tmp = (uchar)CharToNum(pData);
	LCDSetXY(1, 4);
	if((tmp >= MXCodeLiangDuMin) && (tmp <= MXCodeLiangDuMax))
	{
		SendMXCode(MXCodeSetLiangDu, MXCodeLiangDuBase + tmp, MXCodeWAITOK);
		LCDWriteString("���óɹ�");
	}
	else
	{
		LCDWriteString("���ò���ȷ");	
	}
	delayms(2000);

}

void SetYingLiang()
{
	//��������
	uchar tmp;
	uchar pData[3];

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("������һλ��0-7");
	LCDSetXY(1, 2);
	LCDWriteString("Now:");
	LCDPrintfByte(ReadByteEEPROM(YingLiangAddr), 0);
	LCDSetXY(1, 3);
	LCDWriteString("New:");
	if(NumberInput(pData, 1, 0, 3, 3) == 0){
		return;
	}
	tmp = (uchar)CharToNum(pData);
	LCDSetXY(1, 4);
	if((tmp >= YingLiangMin) && (tmp <= YingLiangMax))
	{
		SendThreeLines(tmp + 0xE0, WaitIfBusy);
		SendThreeLines(DaoDianBeep, WaitIfBusy);
		if((tmp >= YingLiangMin) && (tmp <= YingLiangMax))
		{
			WriteByteEEPROM(YingLiangAddr, tmp);
		}
		LCDWriteString("���óɹ�");
	}
	else
	{
		LCDWriteString("���ò���ȷ");	
	}
	delayms(2000);

}

void IRRecTimeLine(){
	uchar myMsg, i, j, power, key;
	uint start,end;
	
	//����
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(2);										//�ȴ��������
	LCDSetXY(1, 1);
	LCDWriteString("�ɰ棺0 ���� A׮");
	LCDSetXY(1, 2);										  
	LCDWriteString("      1 ���� B׮");
	LCDSetXY(1, 3);
	LCDWriteString("�°棺0 - 4 ����");
	delayms(1000);

	//���ù���
	power = 1;
	
	//һֱ��ʾ����״̬							   
	while(1)
	{
		//�鿴�ж��ٴ���������
		j = GetOnLineIDs();
		if(j > 1){
			LCDWrite(WriteCode, LCD_CLEAR);					//����
			delayms(2);										//�ȴ��������
			LCDSetXY(1, 1);
			LCDWriteString("��ֻ��һ��������");
			if(GetKey(0) == KeyReturn){
				return;
			}
			continue;				
		}

		//�����Ϣ����
		ClearMsg();

		//����Ϣ���ӻ��������ӻ�����
		SendMsg(IRLEDTIMELINTTEST);

		//����Ҫ���ԵĹ���
		SendMsg(power);

		//ÿ������ͷ4����Ϣ��3��,���1��ID��Ϣ,��13����Ϣ
		//�յ�13����Ϣ�㿪ʼ����
		//���Ҫ��������Ϣ
		i = 200;
		while(GetMsgCount() < 13){
			key = GetKey(0);
			switch(key){
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					power = key;
					break;

				case KeyReturn:
					delayms(1000);
					ClearMsg();
					return;
			}

			delayms(2);	
			if(i-- == 0){
				ClearMsg();
				LCDWrite(WriteCode, LCD_CLEAR);					//����
				delayms(2);										//�ȴ��������
				LCDSetXY(1, 1);
				LCDWriteString("����������");
				LCDSetXY(1, 2);
				LCDWriteString("��������Ŀ: ");
				LCDPrintfByte(j, 0);
				break;
			}
		}

		//��ѭ����Ϣ����գ�˵������ʧ�ܣ���������
		if(GetMsgCount() == 0){
			continue;
		}
		
		//��Ϣ���꣬������ʾ
		LCDWrite(WriteCode, LCD_CLEAR);					//����
		delayms(2);										//�ȴ��������

		//ÿ�ζ�4�����ݣ��ܹ���3������Ҫ��
		for(j = 0; j < 3;){
			//��ȡ��������,һ�ζ�1������ͷ�����ݣ�����ʾ
			for(i = 0; i < 4;){
				myMsg = GetMsg();
				//�ֱ�д��start �� end
				switch(i){
					case 0:
						start = myMsg;
						start <<= 8;
					break;
		
					case 1:
						start |= myMsg;
					break;
		
					case 2:
						end = myMsg;
						end <<= 8;
					break;
		
					case 3:
						end |= myMsg;
					break;	
				}
				i++;
			}
	
			//��ӡ��ȡ����ֵ
			LCDSetXY(1,2 + j);
			LCDPrintfWord(start, 0);
			LCDWriteString("us");
			LCDSetXY(5,2 + j);
			LCDPrintfWord(end - start, 0);
			LCDWriteString("us");
			j++;
		}
		
		//��ʾID
		LCDSetXY(1, 1);
		LCDWriteString("ID=");
		myMsg = GetMsg();
		if((GetNumFromIDorMsg(myMsg) >= 1) && (GetNumFromIDorMsg(myMsg) <= 6)){
			LCDPrintfByte(GetNumFromIDorMsg(myMsg), 0);

			if(IsReflect(myMsg)){
				LCDWriteString("B");
			}else{
				LCDWriteString("A");
			}
		}
		else{
			LCDPrintfByte(myMsg, 0);
		}

		//��ʾ��ǰ����ܹ���
		LCDSetXY(5, 1);
		LCDWriteString("Power=");
		LCDPrintfByte(power, 0);
		ClearMsg();
		delayms(200);
	}
}

void ViewMsg2()
{
	uchar code XYTable[24] = {0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x82, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, 0x83, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84};
	uchar code HEXCode[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	uchar i, myMsg;

RestartViewMsg:
	i = 0;
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(2);										//�ȴ��������
	LCDSetXY(1, 1);
	LCDWriteString("��Ϣ��: ");
	LCDPrintfByte(i, 1);

	while(1)
	{
		if(GetMsgCount() != 0)
		{
			myMsg = GetMsg();
			LCDSetXY((XYTable[i % 24] & 0xF0) >> 4, XYTable[i % 24] & 0x0F);
			LCDWrite(WriteData, HEXCode[(myMsg & 0xF0) >> 4]);
			LCDWrite(WriteData, HEXCode[(myMsg & 0x0F)]);
			LCDSetXY(5, 1);
			LCDPrintfByte(++i, 1);
		}
		else if(GetKey(0) == KeyMenu)
		{
			goto RestartViewMsg;
		}
		else if(GetKey(0) == KeyReturn)
		{
			delayms(500);
			return;	
		}
	}
}

//����ÿ��ʱ����ʾ����
void SetMDSJXSFF()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("�밴����ѡ�񷽷�");
	LCDSetXY(1, 2);
	LCDWriteString("��ǰ:");

	if(Mdsjxsff == MDSJJSXS)
		LCDWriteString(" ��ʱ��ʾ");
	else
		LCDWriteString(" �ͺ���ʾ");

	LCDSetXY(1, 3);
	LCDWriteString("0 ��ʱ��ʾ");
	LCDSetXY(1, 4);
	LCDWriteString("1 �ͺ���ʾ");

	while(1)
	{
		switch(GetKey(KeyDelay))
		{
			case 0:
				WriteByteEEPROM(MdsjxsffAddr, MDSJJSXS);
				Mdsjxsff = MDSJJSXS;
				goto SetOK;

			case 1:
				WriteByteEEPROM(MdsjxsffAddr, MDSJZHXS);
				Mdsjxsff = MDSJZHXS;
				goto SetOK;
	
			case KeyReturn:
				goto SetOK;

			case NULLKey:
				break;

			default:
				break;
		}
	}
SetOK:
	return;
}

void RunFunction(){
	//0��������ر���
	uchar buf[3];
	
	//1����������ʾ��ʾ��Ϣ
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(3);
	LCDSetXY(1, 1);
	LCDWriteString("���������д��룺");	
	
	//2�����ܰ���
	LCDSetXY(1, 4);
	LCDPrintfByte(sizeof(FunctionTable) / sizeof(VoidFunctionPoint), 0);
	LCDWriteString(" Functions");	
//	NumberInput(buf, 2, 0, 1, 3);
	
	
	//3��У�鰴������Ч��
	if(NumberInput(buf, 2, 0, 1, 3) == 0)
		return;

	//4���ٴμ��麯���Ƿ�Ϸ�
	buf[2] = CharToNum(buf);
	if(buf[2] < sizeof(FunctionTable)){	
		//4��������Ӧ����
		FunctionTable[buf[2]]();
	}	
}

void MsgTest(){
	uchar LastMsg = 0, tmp;
	uint err_count = 0, count = 0;

	//��ʾ��ʾ��Ϣ
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(3);
	LCDSetXY(2, 1);
	LCDWriteString("Message Test");
	LCDSetXY(1, 2);
	LCDWriteString("Total: ");
	LCDSetXY(4, 2);
	LCDPrintfWord(count, 1);

	LCDSetXY(1, 3);
	LCDWriteString("Error: ");
	LCDSetXY(4, 3);
	LCDPrintfWord(err_count, 1);

	//�ôӻ�ѡ����ģʽ
	SendMsg(MSGTEST);

	//ѭ���Ƚ�
	/*
	������һ������ֵ�����
	uchar a, b ,c;
	a = 0;
	b = 0xFF;
	if(a != b + 1) Ϊ��

	uchar a, b, c;
	a = 0;
	b = 0xFF��
	c = b + 1;
	if(a != b) Ϊ��
	*/
	while(1){
		//����Ϣ�ʹ�����Ϣ
		if(GetMsgCount() != 0){		 
			 tmp = GetMsg();
			 count++;
			 LastMsg++;
			 if(tmp != LastMsg){
				LastMsg = tmp;
				err_count++;
				LCDSetXY(4, 3);
				LCDPrintfWord(err_count, 1);
			 } 
		}
		//��Ϣ��ʱ(����10��)�͸�����Ļ
		if((GetMsgCount() <= 10) && (GetMsgCount() != 0)){
			LCDSetXY(4, 2);
			LCDPrintfWord(count, 1);	
		}
		//�а���ʱ�˳�
		if(GetKey(0) == KeyReturn){
			delayms(1000);
			return;
		}
	}
}

void QuickSetIDs(){
	uchar onLine, id, i, myMsg;
	uchar code screen_addr[][4] = {
	{1,1,3,1},{5,1,7,1},
	{1,2,3,2},{5,2,7,2},
	{1,3,3,3},{5,3,7,3}};

	LCDWrite(WriteCode, LCD_CLEAR);							//����
	delayms(2);
	LCDSetXY(1, 1);
	LCDWriteString("����׮����: ");
	onLine = GetOnLineIDs();
	LCDPrintfByte(onLine, 0);
	if(onLine > 1){
		LCDSetXY(1, 2);
		LCDWriteString("��ֻ��һ��������");
		delayms(1000);
		return;
	}
	
	LCDWrite(WriteCode, LCD_CLEAR);							//����
	delayms(2);
	for(id = 1; id <= 6; id++){
		//��ʾҪ���õ�ID
		LCDSetXY(screen_addr[id - 1][0], screen_addr[id - 1][1]);
		LCDPrintfByte(id, 0);
		LCDWrite(WriteData, 'A');
		
		//����ʱ5S
		i = 5;
		while(i){
			LCDSetXY(8, 4);
			LCDPrintfByte(i--, 0);
			delayms(1000);
		}
		LCDSetXY(8, 4);
		LCDPrintfByte(0, 0);

		//����A׮ID
		{
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			delayms(200);
		}

		//��ȡID
		myMsg = MergeMsg(id, COMMUNICATIONTEST);				//׼����Ҫ���Ĳ�����Ϣ
		ClearMsg();
		SendMsg(myMsg);
		delayms(100);
		LCDSetXY(screen_addr[id - 1][0] + 1, screen_addr[id - 1][1]);

		//��ʾ���
		if((GetMsgCount() != 0) && (GetMsg() == myMsg)){
			LCDWrite(WriteData, '*');
			Beep(10);
		}else{
			LCDWrite(WriteData, '?');
			Beep(30);
		}

		//��ʾҪ���õ�ID
		//��ʾҪ���õ�ID
		LCDSetXY(screen_addr[id - 1][2], screen_addr[id - 1][3]);
		LCDPrintfByte(id, 0);
		LCDWrite(WriteData, 'B');

		//����ʱ5S
		i = 5;
		while(i){
			LCDSetXY(8, 4);
			LCDPrintfByte(i--, 0);
			delayms(1000);
		}
		LCDSetXY(8, 4);
		LCDPrintfByte(0, 0);

		//����B׮ID
		{
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			delayms(200);
		}
		
		//��ȡID
		myMsg = MergeMsg(GetReflectID(id), COMMUNICATIONTEST);				//׼����Ҫ���Ĳ�����Ϣ
		ClearMsg();
		SendMsg(myMsg);
		delayms(100);
		LCDSetXY(screen_addr[id - 1][2] + 1, screen_addr[id - 1][3]);

		//��ʾ���
		if((GetMsgCount() != 0) && (GetMsg() == myMsg)){
			LCDWrite(WriteData, '*');
			Beep(10);
		}else{
			LCDWrite(WriteData, '?');
			Beep(30);
		}	
	}
	while(GetKey(0) == NULLKey);
	delayms(1000);	
}

uchar GetOnLineIDs(){
	uchar id, myMsg, onLine = 0;
	for(id = 1; id <= 6; id++){
		//����һ��������Ϣ�����Ƿ����ߣ���A׮
		myMsg = MergeMsg(id, COMMUNICATIONTEST);			//׼����Ҫ���Ĳ�����Ϣ
		SendMsg(myMsg);										//��i��׮A����һ��COMMUNICATION_TEST�ź�
		delayms(10);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
		if(myMsg == GetMsg()){								//ȡ��Ϣ
			//��������׮
			onLine++;
		}
		//����һ��������Ϣ�����Ƿ����ߣ���B׮
		myMsg = MergeMsg(GetReflectID(id), COMMUNICATIONTEST);			//׼����Ҫ���Ĳ�����Ϣ
		SendMsg(myMsg);										//��i��׮A����һ��COMMUNICATION_TEST�ź�
		delayms(10);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
		if(myMsg == GetMsg()){								//ȡ��Ϣ
			//��������׮
			onLine++;
		}
	}
	return onLine;
}

void ZhuangSelfTest()
{
	uchar tCount;
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("׮�Բ����ڽ���..");
	ClearMsg();
	SendMsg(SELFTEST);

	//LED��ˮ�� tCountӦ����Ϊ200ms��������
	tCount = 30;
	LCDSetXY(1, 2);
	LCDWriteString("1 LED ��");
	while(tCount--)
	{
		LCDSetXY(8, 2);
		LCDPrintfByte(tCount / 5, 0);
		delayms(200);
	}
	LCDSetXY(8, 2);
	LCDWriteString(" ");

	//������ղ���
	tCount = 5 * 6;
	LCDSetXY(1, 3);
	LCDWriteString("2 �����");
	while(tCount--)
	{
		LCDSetXY(8, 3);
		LCDPrintfByte(tCount / 5, 0);
		delayms(200);
	}
	LCDSetXY(8, 3);
	LCDWriteString(" ");

	//��Ϣ�������
	tCount = 5 * 2;
	LCDSetXY(1, 4);
	LCDWriteString("3 ����Ϣ");
	while(tCount--)
	{
		LCDSetXY(8, 4);
		LCDPrintfByte(tCount / 5, 0);
		delayms(200);
	}
	
	LCDSetXY(6, 4);
	tCount = GetMsg();
	if(GetMsgStyle(tCount) == SELFTEST)
	{
		LCDPrintfByte(GetNumFromIDorMsg(tCount), 0);
		if(IsReflect(tCount))
			LCDWriteString("B");
		else if(IsRadiant(tCount))
			LCDWriteString("A");
		else
			LCDWriteString("?");
		LCDWriteString("ͨ��");	
	}
	else
	{
		LCDSetXY(7, 4);
		LCDWriteString("�쳣");	
	}

	//20140222ɾ���������,��Ϊ�Զ��˻ص��˵�
	while(GetKey(0) == NULLKey);
	//delayms(1200);
}

void ZhuJiTest(){
	uchar i, myMsg;
	//����
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);

	//������ȫ����һ��
	LCDSetXY(1, 1);
	LCDWriteString("������⣺");
	for(i = 0; i <= 17; i++){
		LCDSetXY(6, 1);
		LCDPrintfByte(i, 1);
		SendThreeLines(i, WaitIfBusy);
		delayms(1000);
	}

	//��������
	LCDSetXY(1, 2);
	LCDWriteString("�������: ������");
	Beep(20);

	//��ⷢ��Ϣ������Ϣ
	LCDSetXY(1, 3);
	LCDWriteString("ͨѶ��������ӻ�");
	delayms(1000);
	ClearMsg();
	for(i = 1; i <= 6; i++){
		myMsg = MergeMsg(i, COMMUNICATIONTEST);
		SendMsg(myMsg);
		delayms(10);
		if(GetMsg() == myMsg){
			i = 0;
			break;
		}

		myMsg = MergeMsg(GetReflectID(i), COMMUNICATIONTEST);
		SendMsg(myMsg);
		delayms(10);
		if(GetMsg() == myMsg){
			i = 0;
			break;
		}
	}
	if(i == 0){
		ClearMsg();
		LCDSetXY(5, 3);
		LCDWriteString("��ͨ��  ");
	}else{
		LCDSetXY(5, 3);
		LCDWriteString("��δͨ��");
	}

	//��ⰴ��
	LCDSetXY(1, 4);
	LCDWriteString("������⣺");
	while(1){
		myMsg = GetKey(KeyDelay);
		if(myMsg != NULLKey){
			LCDSetXY(6, 4);
			LCDPrintfByte(myMsg, 1);

			if(myMsg == KeyReturn)
				break;
		}
	}
}
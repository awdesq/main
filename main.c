#include "stc12c5a60s2.h"
#include "main.h"
#include "UseCommunication.h"
#include "Delay.h"
#include "SoftReset.h"
#include "define.h"
#include "LCD12864.h"
#include "24C02.h"
#include "KeyBoard.h"
#include "MyMenu.h"
#include "WT588D.h"
#include "mxled.h"

//20121107 main() ���ִ�����ǰ,��ʱ�ں�,��100�ĳ�500
//20121107 main() GetKey(0) != NULLKey ԭ���� GetKey(0) == KeyMenu �����Ѱ��жϲ����İ������������
//20121107 main() �������ط�����һЩ������,�����ݲ˵�,����������
//20121112 InitForStartTest() ��������TestOver˳��,�÷���׮�и����ʱ�俪ʼ����
//20121213 InitForStartTest() �м�����delayms(150),Ϊ�����ý���ֹͣ���ٿ�ʼ����
//20130110 main���յ�1��׮Reachedʱ����ͣ��
//20130115 ����WT588D.c MXLED.c ����KeyBoard.c,����������ʾ
//20130122 �����ܱ������ʾ��
//20130122 ����һ�����ɸ�λ�Ĵ�������
//20140222 Ϊ�����ϻ����ܱ���,�޸���SpeakTime�����е�delay100 Ϊ delay300 ��ע�͵���,�ָ�ԭ��
//20171014 ��Menu��һ���ַ����Ƶ�������

uchar code ThisUser[] = "�Ƹ�20171029";
uchar code Sequence[] = {3, 4, 3, 5, 6};
uchar code strtimeover[] = "����";
uchar code strYJW[] = "�Ѿ�λ";
uchar code strtotaltime[] = "��ʱ��: ";
uchar code strerror[] = "ͨѶ����";
uchar code strreadytogo[] = "׼����ʼ";
uchar code strJSFG[] = "��������";
uchar code strinit[] = "��ʼ��...";
uchar code strZXJL[] = "��ԭ��¼";
uchar code WellcomeMsg[] = "��ӭʹ�ó�ɳ���Ƕ����������������,����֧��: 15399900725";

extern unsigned char code Photo[1024];

uchar WorkTime, RecordPoint, BeepType, WorkMode, Mdsjxsff, IsRoot;		//�����ļ�Ҫ�õ�
uint NoUsed, NoSuccess, FastestRec, CanotResetNo, RemainTimes;			//�����ļ�Ҫ�õ�

uchar SpeakCount, PointReached, MsgSend, i;
uint time[6], prev_t, t, t_Copy, t_LastHere, t_Every5M;

void main()
{	 
	uchar myMsg;
	LCD_PSW = LCDBKLightON;							//��LCD����
	InitUART();										//��ʼ�������������غ���ʾ
	InitForCommunication();							//��ʼ��IO������ͨ��
	LCDInit();										//��ʼ��Һ��															   
	InitT0_For_CLOCK();								//��ʼ��Time0,����ʱ��
	InitGlobalVal();								//��ʼ��һЩȫ�ֱ���
	InitForBeep();									//������PCA����BEEPʱ�����
	InitKeyboard();									//��ʼ�����̶˿�
	FirstPowerOntoResetValues();					//��һ���ϵ��ʼ��EEPROM
	IsRoot = 0;										//��ʼ����־����
	if(GetKey(0) == KeyF4){
		IsRoot = 1;									//�����Ƿ��а���F4������ʾͼƬ֮ǰ����F4�����ˡ�����Ҫ���ܾ�
		LCD_PSW = LCDBKLightOFF;					//����Ļ�����ڣ���������rootģʽ��
	}
	LCDPhotoDisplay(Photo);							//��ʾ��ʼ����
	delayms(1000);
	SoundFunction(BeepTimePointReached, DaoDianBeep)//������ʾ
	SendMXString(WellcomeMsg, 1, 1);				//���ͻ�ӭ��Ϣ
	SendMXCode(MXCodeDisplay, MXCodeDisplayContent1, 1);//��ʾ����1
	SendMXCode(MXCodeContent1Move, MXCodeLeftShift, 1);//��ʾ����1����
	LCDWrite(WriteCode, LCD_BASE_CODE);				//�л�LCD��ʾģʽ
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(2);										//�ȴ��������
	Menu();											//����˵�
	SendMXString(" ", 3, MXCodeWAITOK);				//����ʾ�������
	SendMXCode(MXCodeRealtimeDisplay, MXCodeNULLContent, MXCodeWAITOK);//תΪʵʱ��ʾģʽ
	SendThreeLines(ReadyToGo, 1);					//��������
	InitForStartTest();								//��ʼ�����Ի���
	
	while(1)
	{
		//������û����Ҫ��ע�İ�������,�����,�ʹ���������,��������������ʱ���Գ�
		uchar KeyPressed;

		if(GetMsgCount() > 0)
		{
			myMsg = GetMsg();										//��ȡ��Ϣ
			switch(GetMsgStyle(myMsg))
			{
				case REACHED:
					switch(GetIDFromMsg(myMsg))
					{
						case 2:
						case 3:
						case 4:
						case 5: 														//case 2, 3, 4, 5
							PointReached++;												//��¼����
							ET0 = 0;													//�ض�ʱ��0�ж�,��Ϊ���漰t�ĸ���
							time[PointReached - 2] = t - prev_t;						//��¼ʱ��
							prev_t = t;													//��¼ʱ��,�����ж�ʲôʱ����Ϣ
							ET0 = 1;													//����ʱ��0�ж�
							SoundFunction(BeepTimePointReached, DaoDianBeep)			//������ʾ
							if(Mdsjxsff == MDSJJSXS)									//ÿ��ʱ�伴ʱ��ʾ����ʾ����ɼ�
							{
								printftime(time[PointReached - 2], (PointReached % 2) * 4 + 1, PointReached / 2);//��ӡʱ��
							}
							MsgSend = MergeMsg(Sequence[PointReached - 2], STARTWORK);	//׼����Ҫ���͸���һ����׮����Ϣ
							break;

						case 6:															//�������жϴ������
							TR0 = 0;													//ֹͣ��ʱ
							PointReached++;												//��¼����
							time[PointReached - 2] = t - prev_t;						//��¼ʱ��
							prev_t = t;													//��¼ʱ��,�����ж�ʲôʱ����Ϣ
							SoundFunction(BeepTimePointReached, DaoDianBeep)			//������ʾ
							if(Mdsjxsff == MDSJJSXS)									//ÿ��ʱ�伴ʱ��ʾ����ʾ����ɼ�
							{
								printftime(t, 5, 4);									//��ӡ�����ʱ��
								printftime(time[PointReached - 2], (PointReached % 2) * 4 + 1, PointReached / 2);//��ӡ���һ������ʱ��
							}
							else
							{
								LCDWrite(WriteCode, LCD_CLEAR);				//����
								delayms(2);									//�ȴ�����
								for(i = 0; i < 6; i++)
								{
									printftime(time[i], (i % 2) * 4 + 1, i / 2 + 1);	//��ӡʱ��
								}
								LCDSetXY(1, 4);											//���ô�ӡλ��
								LCDWriteString(strtotaltime);							//��LCD����ʾ"��ʱ��"
								printftime(t, 5, 4);									//��ӡ�����ʱ��

							}
							delayms(100);												//�����������,��ô�п��ܵ�����������SendMXTimeʱ��̫��,��ʾ����
							SendMXTime(t);												//�������ʱ���LED

							//���ݸ��²���
							{
								//д��ɼ�
								WriteWordEEPROM(GetANewAddrToSaveRecord(), t);			//д��ɼ�
								WriteWordEEPROM(NoSuccessAddr, NoSuccess + 1);			//д��ɹ�����
								NoSuccess++;											//�ɹ�������1
								WriteWordEEPROM(NoUsedAddr, NoUsed + 1);				//д��ʹ�ô���
								NoUsed++;												//ʹ�ô�����1
								if((++CanotResetNo) >= MaxCanotResetNo)					//д��һ�����ɸ�λ�Ĵ���
									CanotResetNo = 0;
								if((WorkMode == WORKMODE_COUNTTIMES) && (RemainTimes > 0))//�ƴ�ģʽʱ,������1
								{
									WriteWordEEPROM(RemainTimesAddr, --RemainTimes);
								}
								WriteWordEEPROM(CanotResetNoAddr, CanotResetNo);
								/* ����¼���·�����������У�ֻ��׮�ź�û�������������óɼ�
								if((t < FastestRec) || (FastestRec == (uint)0))			//�Ƿ�Ϊ�¼�¼
								{
									FastestRec = t;
									WriteWordEEPROM(FastestRecAddr, FastestRec);		//д�����¼�¼
								}
								*/
							}
						    SpeakTime(t);												//������ʱ
							//���������������쳣�ɼ�,������쳣�ɼ����п�����ĳ��׮����û�ж�����
							{
								uchar *p, TestStr[] = "XXXXXX�Ÿ�Ӧ��������!";
								p = TestStr + 5;						//ָ�����һ��X
								
								if(time[5] < (uint)Every5MMinTime) *p-- = '6';
								if(time[4] < (uint)Every5MMinTime) *p-- = '5';
								if(time[2] < (uint)Every5MMinTime) *p-- = '4';
								if((time[3] < (uint)Every5MMinTime) || ((time[1] < (uint)Every5MMinTime))) *p-- = '3';
								if(time[0] < (uint)Every5MMinTime) *p-- = '2';								

								if(p != (TestStr + 5))					//p���Ķ�,˵�����쳣
								{
									SendMXString(" ", 3, MXCodeWAITOK);
									SendMXCode(MXCodeContent3Move, MXCodeLeftShift, MXCodeWAITOK);
									SendMXCode(MXCodeDisplay, MXCodeDisplayContent3, MXCodeWAITOK);
									SendMXString(p+1, 3, 1);
									delayms(8000);
									SendMXCode(MXCodeRealtimeDisplay, MXCodeNULLContent, MXCodeWAITOK);
									SendMXTime(t);												//������ʾʱ��
								}
								else if((t < FastestRec) || (FastestRec == (uint)0))			//�Ƿ�Ϊ�¼�¼
								{
									FastestRec = t;
									WriteWordEEPROM(FastestRecAddr, FastestRec);				//д�����¼�¼
									SendMXString(strZXJL, 3, 0);								//������ѳɼ�����ʾ��
								}
							}
							SendMsg(MergeMsg(1, STARTWORK));									//��1��׮���¿�ʼ����
							break;

						case 1:							
ViewWorkMode:				if((WorkMode == WORKMODE_COUNTTIMES) && (RemainTimes == 0))			//�Ǽƴ�ģʽ,��ʣ�����Ϊ0,����жϲ�ȥ����û����Ϣ
							{
								DisplayRemainTimesOverInfo();					//��ʾ��Ϣ
								while(GetKey(KeyDelay) == NULLKey);				//�ȴ�����
								Menu();
								goto ViewWorkMode;
							}
							t = prev_t = t_LastHere =0;						//��ʱ�����
							TR0 = 0;										//ͣ��
							PointReached = 0;								//��û�����κε�,�뿪1��׮ʱ,Ϊ������1�ŵ�
							MsgSend = NULLMsg;								//��ʾû����Ϣ�ɷ���
							if(Mdsjxsff == MDSJJSXS)						//Ҫʵʱ��ʾÿ��ʱ�������,��������
							{
								LCDWrite(WriteCode, LCD_CLEAR);				//����
								delayms(2);									//�ȴ�����
								LCDSetXY(3, 2);								//���ô�ӡλ��
								LCDWriteString(strYJW);						//��LCD����ʾ�Ѿ�λ
							}
							SendMXString(strYJW, 3, 0);						//�����Ѿ�λ����ʾ��
							SoundFunction(BeepTimeStart, ReadyToGoBeep) 	//������ʾ
							break;
					}
					break;

				case LEAVED:											//��1��׮�뿪ʱ,1��׮�ᷢ�������Ϣ
					TR0 = 1;											//��ʼ��ʱ
					PointReached = 1;									//1��׮���뿪
					SoundFunction(BeepTimePointReached, DaoDianBeep)	//�뿪ʱ����
					MsgSend = MergeMsg(2, STARTWORK);					//����Ҫ���͵���Ϣ,�����ڲ��ᷢ,Ҫ��һ��ʱ��֮��Żᷢ
					if(Mdsjxsff == MDSJJSXS)							//ÿ��ʱ����Ҫ��ʱ��ʾʱ��������ӡ"��ʱ��:"����
					{
						LCDWrite(WriteCode, LCD_CLEAR);					//����
						delayms(2);										//�ȴ�����
						LCDSetXY(1, 4);									//���ô�ӡλ��
						LCDWriteString(strtotaltime);					//��LCD����ʾ"��ʱ��"����
					}
					break;

				case TIMEOVER:
					TR0 = 0;											//ֹͣ��ʱ
					SoundFunction(BeepTimeTimeOver, JiShuFanGuiBeep)	//������ʾ
					if(Mdsjxsff == MDSJJSXS)							//������Ǽ�ʱ��ʾ,����ʾ��ʱ�ͻָ�ʱ��
					{
				   		LCDSetXY(((PointReached - 1) % 2) * 4 + 1, (PointReached - 1) / 2 + 1); //���ó�ʱ��ӡλ��
						LCDWriteString(strtimeover);						//��ӡ��ʱ
						printftime(0, 5, 4);								//��ʱ���ӡΪ0
					}
					delayms(100);										//�п�������ʱ�����̫����else�е���ʾ���������ʾ, 
					SendMXString(strJSFG, 3, 0);					   	//�����ʾ����Ϊ��������
					delayms(30);										//�ȴ�ǰ���������ʾ���ٷ���һ��������ʾ
					SendThreeLines(JiShuFanGui, WaitIfBusy);			//����������
					
					//���ݸ��²���
					{
						if(PointReached > 1)							//�Ѿ�����2��׮
						{
							WriteWordEEPROM(NoUsedAddr, ++NoUsed);		//д�����ʹ�ô���	
						}
					}
					SendMsg(MergeMsg(1, STARTWORK));					//���´�һ��
					break;
			}
		}
		else
		{
			//����1������ʱ�䡡�õ��뿪��һ�㵽����ʱ���,������һ�ε������ʱ��,���Ƚ�,ÿ�ٷ�֮һ��ֻ����һ��else
			ET0 = 0;t_Copy = t;ET0 = 1;									//�ض�ʱ��0�ж�,��Ϊ���漰t�ĸ���
			t_Every5M = t_Copy - prev_t;								//�õ�һ���뿪��һ���ʱ��

			//����2��������Ϣ��������Ϣ
			if((MsgSend != NULLMsg) && (t_Every5M > SendMsgWaitTimeDefault))		
			{
				SendMsg(MsgSend);
				MsgSend = NULLMsg;
			}

			//����3�������ܱ�
			if((t_Copy % FreqOfUpdateDisplay == 0) && (PointReached >= 1) && (t_LastHere != t_Copy))	//�ܱ�,�ٷ�֮һ��ֻˢ��һ��,����ˢ�¶��
			{
				if(Mdsjxsff == MDSJJSXS)			 								//ֻ�м�ʱ��ʾ��ʽ�Ÿ���LCD�ܱ�
					printftime_IfNoMsg(t_Copy, 5, 4);
				SendMXTime(t_Copy);													//����ʱ��������ʾ��
				t_LastHere = t_Copy;												//��������������if��ʱ��
			}

			//���񣴣��ж�ͨ�ų���
			if(t_Every5M > ErrorTime)		
			{
				TR0 = 0;									//ֹͣ��ʱ
				if(BeepType == BeepBuzzer)					//����Beep
					ErrorBeep();
				else
					SendThreeLines(JiShuFanGuiBeep, NotWait);//������ʾ
			   	LCDSetXY(((PointReached - 1) % 2) * 4 + 1, (PointReached - 1) / 2 + 1); //���ô����ӡλ��
				LCDWriteString(strerror);					//��ӡ������
				printftime(0, 5, 4);						//��ʱ���ӡΪ0
				delayms(100);								//�����������,��ô�п��ܵ�����������SendMXTimeʱ��̫��,��ʾ����
				SendMXString("ͨѶ����", 3, 0);				//�����ʾ����ʾ��Ϣ
				t = prev_t = t_LastHere = 0;				//��ֹ�ٴν���
				delayms(2000);								//�ȴ�һ��ʱ��,��ʼ��һ��
				SendMsg(MergeMsg(1, STARTWORK));			//���´�һ��
			}
			
			//���񣵣���Ӧ����
			if(GetKey(0) != NULLKey)
			{
				KeyPressed = GetKey(0);
				if(KeyPressed == KeyF3)
				{
					LCD_PSW = LCDBKLightON;
					goto NotReset;
				}
				else if(KeyPressed == KeyF4)
				{
					LCD_PSW = LCDBKLightOFF;
					goto NotReset;
				}
				else
				{	
					switch(KeyPressed)
					{
						case KeyF1:
							DisplaySumary();
							break;
		
						case KeyF2:
							ViewRecord();
							break;
		
						case KeyMenu:
							RestartSub();
							SendMXString("������...", 3, 0);
							Menu();
							SendMXString(" ", 3, 0);
							break;
		
						default:
							goto NotReset;							
					}
				}
				InitForStartTest();				//��ʼ��,���ڲ���
			} 
NotReset:;
		}
	}
}

void ErrorBeep()
{
	Beep(BeepTimeError);							//������ʾ
	delayms(BeepTimeError * 20);					//ֹͣһ��ʱ��
	Beep(BeepTimeError);							//������ʾ
	delayms(BeepTimeError * 20);					//ֹͣһ��ʱ��
	Beep(BeepTimeError);							//������ʾ
}

void InitT0_For_CLOCK(void)
{
	EA = 0;
	Set0(AUXR, 7);			//12T Mode
    Set1(TMOD, 0);			//16Bit Timer
	Set0(TMOD, 1);
    TH0 = CLOCK_TH0;
    TL0 = CLOCK_TL0;																												
    EA = 1;
    ET0 = 1;
}

void InitForStartTest()
{
	uchar i;
	//����������ʾ
	LCDWrite(WriteCode, LCD_CLEAR);							//����
	delayms(2);												//�ȴ�����
	LCDSetXY(3, 2);	
	LCDWriteString(strinit);								//��ʼ��

	//������׮
	RestartSub();											//����������׮

	//��ʼ������
	InitGlobalVal();										//����һЩȫ�ֱ���
	PointReached = 0;										//û�е�������׮
	MsgSend = NULLMsg;										//û���κ���Ϣ��Ҫ����
	TR0 = 0;												//��ʱ��������
	t = prev_t = t_LastHere = 0;							//��ʼ��ʱ��

	//������Ϣ����ʼ��׮
	ClearMsg();
	SendMsg(SETWORKTIME);									//����׮����ʱ������
	SendMsg(WorkTime);										//���ù���ʱ��
	for(i = 1; i < 7; i++)
	{
		SendMsg(MergeMsg(GetReflectID(i), TESTOVER));		//������B׮��TestOver�ź�
		SendMsg(MergeMsg(i, TESTOVER));						//������A׮��TestOver�ź�	
	}

	//����������ʾ
	LCDWrite(WriteCode, LCD_CLEAR);							//����
	delayms(2);												//�ȴ�����
	LCDSetXY(3, 2);	
	LCDWriteString(strreadytogo);							//��LCD����ʾ׼����ʼ
	SendMXString(strreadytogo, 3, 1);						//����׼����ʼ����ʾ��
	SendMsg(MergeMsg(1, STARTWORK));						//��1��׮���Ϳ�ʼ�����ź�
}

void Timer0_Routine(void) interrupt 1
{
    TH0 = CLOCK_TH0 + TH0;
    TL0 = CLOCK_TL0 + TL0;
	t++;
}

void printftime_IfNoMsg(uint time, uchar x, uchar y)	 					//��x,y ��ӡ��ʱ��
{
	uchar s, s100;
	s = (uchar)(time / 100);
	s100 = (uchar)(time % 100);
	if(GetMsgCount() != 0) return; LCDSetXY(x, y);
	if(GetMsgCount() != 0) return; LCDWrite(WriteData, ((s / 10) % 10) + '0');
	if(GetMsgCount() != 0) return; LCDWrite(WriteData, (s % 10) + '0');
	if(GetMsgCount() != 0) return; LCDWrite(WriteData, ':');
	if(GetMsgCount() != 0) return; LCDWrite(WriteData, ((s100 / 10) % 10) + '0');
	if(GetMsgCount() != 0) return; LCDWrite(WriteData, (s100 % 10) + '0');
}

void printftime(uint time, uchar x, uchar y)	 					//��x,y ��ӡ��ʱ��
{
	uchar s, s100;
	s = (uchar)(time / 100);
	s100 = (uchar)(time % 100);
	LCDSetXY(x, y);
	LCDWrite(WriteData, ((s / 10) % 10) + '0');
	LCDWrite(WriteData, (s % 10) + '0');
	LCDWrite(WriteData, ':');
	LCDWrite(WriteData, ((s100 / 10) % 10) + '0');
	LCDWrite(WriteData, (s100 % 10) + '0');
}

void SpeakTime(uint time)
{
	uchar s, s100, strtime[7] = {0, 0, 0, Dot, 0, 0, Miao};
	s = (uchar)(time / 100);
	s100 = (uchar)(time % 100);

	strtime[1] = (s / 10) % 10;
	strtime[2] = s % 10;
	strtime[4] = (s100 / 10) % 10;
	strtime[5] = (s100 % 10);
	
	if(strtime[1] > 0)
	{
		if(strtime[2] != 0)
		{
			strtime[0] = strtime[1];
			strtime[1] = 10;
			s100 = 0;
		}
		else
		{
			strtime[2] = 10;
			s100 = 1;
		}
	}
	else
	{
		s100 = 2;
	}
	for(s = s100; s < 7; s++)
	{
		SendThreeLines(strtime[s], WaitIfBusy);	
		//delayms(1000);
		delayms(100);
	}
}

void InitGlobalVal()
{
	uchar Data;

	//��ʱʱ��
	Data = ReadByteEEPROM(TimeOverAddr);								//���볬ʱʱ��,�������׮�Ĺ���ʱ��
	if((Data >= TimeOverMin) && (Data <= TimeOverMax))					//0.1���������
	{
		if(Data > (SendMsgWaitTimeDefault / 10))
			WorkTime = Data - SendMsgWaitTimeDefault / 10;				//׮�Ĺ���ʱ�� = ��ʱʱ�� - ����Ϣ��ʱʱ��
	}
	else
	{
		WorkTime = WorkTimeDefault;										//��ȡ������,����Ĭ��ʱ��
	}
	//���ʹ�ô���
	NoUsed = ReadWordEEPROM(NoUsedAddr);

	//����ɹ�����
	NoSuccess = ReadWordEEPROM(NoSuccessAddr);

	//��óɼ�
	FastestRec = ReadWordEEPROM(FastestRecAddr);

	//�õ�BeepType
	BeepType = ReadByteEEPROM(BeepTypeAddr);

	//�õ�CannotResetNo
	CanotResetNo = ReadWordEEPROM(CanotResetNoAddr);
	if(CanotResetNo >= MaxCanotResetNo)
	{
		CanotResetNo = 0;
		WriteWordEEPROM(CanotResetNoAddr, 0);
	}
	//�õ�����ģʽ
	WorkMode = ReadByteEEPROM(WorkModeAddr);
	if((WorkMode != WORKMODE_NOLIMIT) && (WorkMode != WORKMODE_COUNTTIMES))		
	{
		WorkMode = WORKMODE_COUNTTIMES;
		WriteByteEEPROM(WorkModeAddr, WorkMode);
	}
	//�õ�ʣ�����
	RemainTimes = ReadWordEEPROM(RemainTimesAddr);

   //��������
   Data = ReadByteEEPROM(YingLiangAddr);
   if((Data >= YingLiangMin) && (Data <= YingLiangMax))
   {
   		delayms(100);
		SendThreeLines(YingLiangBase + Data, WaitIfBusy);   
   }

   //�õ�ÿ��ʱ����ʾ��ʽ
	Mdsjxsff = ReadByteEEPROM(MdsjxsffAddr);
}

void InitForBeep()
{
#define CONST_CCAP0H	0x27
#define CONST_CCAP0L	0x10
	Set0(P3M1, 4);		//����BeepΪǿ����
	Set1(P3M0, 4);

	CMOD = 0x00;		// SYS/12,����������ж�
	CCON = 0x00;		//���־λCF CR / / / / CCF1 CCF0
	CCAPM0 = 0x49;		//16λ��ʱ�� ƥ��ʱ�ж�
	CH =  0;			//��������0
	CL = 0;
	CCAP0L = CONST_CCAP0L;
	CCAP0H = CONST_CCAP0H;
	Speak = StopBeep;
	SpeakCount = 0;
	
}

void InterruptPCA() interrupt 7
{
	CH = 0;				//��������0
	CL = 0;
	CCF0 = 0;			//���жϱ�־λ
	if(SpeakCount-- == 0)
	{
		STOPPCA;
		Speak = StopBeep;
	}
}

void Beep(uchar Ten_ms)
{
	CCF0 = 0;				//���жϱ�־λ
	CH = 0;					//��������0
	CL = 0;
	SpeakCount = Ten_ms;
	Speak = StartBeep;
	STARTPCA;
}

void RestartSub()
{
	uchar i;
	for(i = 0; i < 4; i++)
		SendMsg(RESTART);
	delayms(500);
}

uchar ReadByteEEPROM(uchar Addr)
{
	uchar Data;
	ReadEEPROM(ATMEL24C02, Addr, &Data, 1);
	return Data;
}				

uchar WriteByteEEPROM(uchar Addr, uchar Data)
{
	return WriteEEPROM(ATMEL24C02, Addr, &Data, 1);	
}

uint ReadWordEEPROM(uchar Addr)
{
	uint Data;
	ReadEEPROM(ATMEL24C02, Addr, (uchar*)&Data, 2);
	return Data;
}

uchar WriteWordEEPROM(uchar Addr, uint Data)
{
	return WriteEEPROM(ATMEL24C02, Addr, (uchar*)&Data, 2);
}

uchar GetANewAddrToSaveRecord()
{
	return (NoSuccess % NoOfMaxSaved) * 2 + RecordStartAddr;
}

uchar GetCurrentRecordAddr()
{
	if(NoSuccess == 0)
		return NULLPoint;

	return (NoSuccess % NoOfMaxSaved) * 2 + RecordStartAddr - 2;
}

uchar GetPrevRecordAddr(uchar CurAddr)
{
	//û�д���100��,���ݶ���[RecordStartAddr, RecordStartAddr + (NoSuccess - 1) * 2]֮��
	if(NoSuccess < NoOfMaxSaved)
	{
		//����ʼ��ַ�󣬱�����¼С������1�����ϵ�����,����������������
		if((CurAddr > RecordStartAddr) && (CurAddr <= GetCurrentRecordAddr()) && (NoSuccess > 1))		//����ʼ��ַ �� ��ǰ��¼ ֮������ݵ�ַ
			return CurAddr - 2;
		else													//û��ǰһ��������
			return NULLPoint;								
	}
	//���д洢������������
	else
	{
		if(CurAddr != RecordStartAddr)							//�����Ƶ��洢��֮ǰȥ
		{
			CurAddr -= 2;										//�����Ƶ��洢��֮ǰ,��ǰ�ƶ�
		}
		else
		{
			CurAddr = RecordStartAddr + (NoOfMaxSaved - 1) * 2;	//���ƶ����洢��֮ǰ,�Ƶ����һ��
		}

		if(CurAddr == GetCurrentRecordAddr())					//���ǰһ���ǵ�ǰ���¼�¼,˵����ͷ��
			return NULLPoint;
		else
			return CurAddr;
	}
}

/*
void TestBatteryTime()
{
	//��ز��Գ���
	uchar FailCount = 0, myMsg;								//ʧ�ܲ���
	uint min1, min2;										//��¼�������еķ�����

	RestartSub();											//�������ӻ�
	LCDWrite(WriteCode, LCD_CLEAR);							//����
	delayms(2);	
	t = 0;
	TR0 = 1;												//����ʱ��
	LCDSetXY(1, 1);
	LCDWriteString("����Ϣ��:");
	LCDSetXY(6, 1);
	LCDPrintfByte(FailCount, 1);							//��ӡ����Ϣ��

	LCDSetXY(1, 2);
	LCDWriteString("����ʱ��1:");							//��ӡ����ʱ��
	min1 = ReadWordEEPROM(Min1Addr);
	LCDSetXY(6, 2);
	LCDPrintfWord(min1, 1);

	LCDSetXY(1, 3);
	LCDWriteString("����ʱ��2:");
	min2 = ReadWordEEPROM(Min2Addr);
	LCDSetXY(6, 3);
	LCDPrintfWord(min2, 1);

	LCDSetXY(1, 4);
	LCDWriteString("F1����  F2�ؼ�");

	while(1)
	{
		switch(GetKey(0))									//���԰�����
		{
			case KeyF1:
				goto NotClearToZero;
			
			case KeyF2:
				goto ClearToZero; 
		}
	}
ClearToZero:
	WriteWordEEPROM(Min1Addr, 0);							//��ʼ��EEPROM
	WriteWordEEPROM(Min2Addr, 0);							//��ʼ��EEPROM

	min1 = 0;
	min2 = 0;
	FailCount = 0;

NotClearToZero:
	delayms(1000);

	LCDSetXY(6, 1);
	LCDPrintfByte(FailCount, 1);							//��ӡ����Ϣ��

	LCDSetXY(6, 2);
	LCDPrintfWord(min1, 1);

	LCDSetXY(6, 3);
	LCDPrintfWord(min2, 1);

	LCDSetXY(1, 4);
	LCDWriteString("                ");
	while(1)
	{
		ClearMsg();											//����Ϣ����
		myMsg = MergeMsg(1, COMMUNICATIONTEST);				//1AͨѶ������Ϣ
		SendMsg(myMsg);										//��1A������Ϣ
		delayms(10);										//�ȴ���Ϣ,2ms����Ϣ,2ms����Ϣ,10ms�㹻i��׮���ص���Ϣ��
		if(GetMsg() != myMsg)
		{
			FailCount++;					//��¼����Ϣ��Ŀ
			LCDSetXY(6, 1);
			LCDPrintfByte(FailCount, 1);					//��ӡ����Ϣ��
		}
		ET0 = 0;											//����t
		t_Copy = t;
		ET0 = 1;
		
		if(t_Copy >= 6000)									//ÿһ����дһ��EEPROM
		{
			ET0 = 0;										//дt
			t = 0;	
			ET0 = 1;									  	//ʱ���0
			min1++;											//����+1
			WriteWordEEPROM(Min1Addr, min1);				//������д��EEPROM
			LCDSetXY(6, 2);									//��ӡ����ʱ��
			LCDPrintfWord(min1, 1);

			min2++;											//����+1
			WriteWordEEPROM(Min2Addr, min2);				//������д��EEPROM
			LCDSetXY(6, 3);									//��ӡ����ʱ��
			LCDPrintfWord(min2, 1);

			LCDSetXY(6, 1);    								//��ӡ����Ϣ��
			LCDPrintfByte(FailCount, 1);

		}
		if((t_Copy % 10) == 0)								//1�����ʮ��LED,
		{
			SendMXTime(t_Copy);								//����ʱ��		
		}
		if((t_Copy % 1000) == 0)							//10�뷢һ������
		{
			SendThreeLines(DaoDianBeep, WaitIfBusy);
		}
		if(FailCount > 5)
		{
			LCDSetXY(1, 4);
			LCDWriteString("�������");
			while(GetKey(KeyDelay) == NULLKey);
			return;
		}
		if(GetKey(0) != NULLKey)
		{
			break;
		} 
	}
}
*/

void DisplayRemainTimesOverInfo()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//����
	delayms(2);										//�ȴ�����
	LCDSetXY(1, 1);									//���ô�ӡλ��
	LCDWriteString("�������ʹ�����");		
	LCDSetXY(1, 2);									//���ô�ӡλ��
	LCDWriteString("����ϵ�豸�ṩ��");
	LCDSetXY(1, 3);									//���ô�ӡλ��
	LCDWriteString("���ʹ����Ȩ");
	LCDSetXY(1, 4);
	LCDWriteString("TEL:15399900725");				//��ӡ�绰
}

void FirstPowerOntoResetValues(){
	if(ReadByteEEPROM(FirstPowerOn) != 'X'){
		//�ָ�TimeOver
		WriteByteEEPROM(TimeOverAddr, TimeOverDefault);		
		//�ָ�ʹ�ü���
		WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			
		//�ָ��������
		WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		
		//�ָ���ѳɼ�
		WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		
		//�������������
		WriteByteEEPROM(BeepTypeAddr, BeepDAC);
		//��������	111111
		WriteEEPROM(ATMEL24C02, PasswordAddr, "111111", PasswordLen);
		//���ù���ģʽ ����ģʽ
		WriteByteEEPROM(WorkModeAddr, WORKMODE_NOLIMIT);
		//����ʣ�����
		WriteWordEEPROM(RemainTimesAddr, (uint)50000);
		//��������
		WriteByteEEPROM(YingLiangAddr, 4);
		//���õڵ�ʱ����ʾ��ʽ
		WriteByteEEPROM(MdsjxsffAddr, MDSJJSXS);
		//���������ܴ��������ɸ�λ�ģ�
		WriteWordEEPROM(CanotResetNoAddr, 0);
		//��FirstPowerOn��־��Ϊ'N'
		WriteByteEEPROM(FirstPowerOn, 'X');
	}
}
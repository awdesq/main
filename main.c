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

//20121107 main() 发现错误在前,超时在后,将100改成500
//20121107 main() GetKey(0) != NULLKey 原来是 GetKey(0) == KeyMenu 这样把按中断操作的按键大大增加了
//20121107 main() 在上述地方加了一些程序处理,进入快捷菜单,增加易用性
//20121112 InitForStartTest() 调换发送TestOver顺序,让反射桩有更多的时间开始工作
//20121213 InitForStartTest() 中加入了delayms(150),为的是让叫声停止后再开始测试
//20130110 main接收到1号桩Reached时，不停表
//20130115 加入WT588D.c MXLED.c 更改KeyBoard.c,加入声音提示
//20130122 发送跑表到外接显示屏
//20130122 加入一个不可复位的次数计数
//20140222 为了让老机子能报数,修改了SpeakTime函数中的delay100 为 delay300 已注释掉了,恢复原样
//20171014 将Menu中一个字符串移到这里来

uchar code ThisUser[] = "黄岗20171029";
uchar code Sequence[] = {3, 4, 3, 5, 6};
uchar code strtimeover[] = "犯规";
uchar code strYJW[] = "已就位";
uchar code strtotaltime[] = "总时间: ";
uchar code strerror[] = "通讯错误";
uchar code strreadytogo[] = "准备开始";
uchar code strJSFG[] = "技术犯规";
uchar code strinit[] = "初始化...";
uchar code strZXJL[] = "破原记录";
uchar code WellcomeMsg[] = "欢迎使用长沙望城二中五米三向测试仪,技术支持: 15399900725";

extern unsigned char code Photo[1024];

uchar WorkTime, RecordPoint, BeepType, WorkMode, Mdsjxsff, IsRoot;		//其它文件要用到
uint NoUsed, NoSuccess, FastestRec, CanotResetNo, RemainTimes;			//其它文件要用到

uchar SpeakCount, PointReached, MsgSend, i;
uint time[6], prev_t, t, t_Copy, t_LastHere, t_Every5M;

void main()
{	 
	uchar myMsg;
	LCD_PSW = LCDBKLightON;							//打开LCD背光
	InitUART();										//初始化串口用于下载和显示
	InitForCommunication();							//初始化IO等用于通信
	LCDInit();										//初始化液晶															   
	InitT0_For_CLOCK();								//初始化Time0,用于时钟
	InitGlobalVal();								//初始化一些全局变量
	InitForBeep();									//初初化PCA用于BEEP时间控制
	InitKeyboard();									//初始化键盘端口
	FirstPowerOntoResetValues();					//第一次上电初始化EEPROM
	IsRoot = 0;										//初始化标志变量
	if(GetKey(0) == KeyF4){
		IsRoot = 1;									//测试是否有按下F4，在显示图片之前接下F4就行了。不需要按很久
		LCD_PSW = LCDBKLightOFF;					//当屏幕背光变黑，代表启动root模式。
	}
	LCDPhotoDisplay(Photo);							//显示起始画面
	delayms(1000);
	SoundFunction(BeepTimePointReached, DaoDianBeep)//声音提示
	SendMXString(WellcomeMsg, 1, 1);				//发送欢迎信息
	SendMXCode(MXCodeDisplay, MXCodeDisplayContent1, 1);//显示内容1
	SendMXCode(MXCodeContent1Move, MXCodeLeftShift, 1);//显示内容1左移
	LCDWrite(WriteCode, LCD_BASE_CODE);				//切换LCD显示模式
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(2);										//等待清屏完成
	Menu();											//进入菜单
	SendMXString(" ", 3, MXCodeWAITOK);				//将显示内容清空
	SendMXCode(MXCodeRealtimeDisplay, MXCodeNULLContent, MXCodeWAITOK);//转为实时显示模式
	SendThreeLines(ReadyToGo, 1);					//播放声音
	InitForStartTest();								//初始化测试环境
	
	while(1)
	{
		//测试有没有需要关注的按键按下,如果有,就处理按键事情,处理按键事情程序耗时可以长
		uchar KeyPressed;

		if(GetMsgCount() > 0)
		{
			myMsg = GetMsg();										//读取消息
			switch(GetMsgStyle(myMsg))
			{
				case REACHED:
					switch(GetIDFromMsg(myMsg))
					{
						case 2:
						case 3:
						case 4:
						case 5: 														//case 2, 3, 4, 5
							PointReached++;												//记录到点
							ET0 = 0;													//关定时器0中断,因为它涉及t的更新
							time[PointReached - 2] = t - prev_t;						//记录时间
							prev_t = t;													//记录时间,用于判断什么时候发消息
							ET0 = 1;													//开定时器0中断
							SoundFunction(BeepTimePointReached, DaoDianBeep)			//声音提示
							if(Mdsjxsff == MDSJJSXS)									//每道时间即时显示才显示这道成绩
							{
								printftime(time[PointReached - 2], (PointReached % 2) * 4 + 1, PointReached / 2);//打印时间
							}
							MsgSend = MergeMsg(Sequence[PointReached - 2], STARTWORK);	//准备好要发送给下一个号桩的消息
							break;

						case 6:															//不存在中断打断问题
							TR0 = 0;													//停止计时
							PointReached++;												//记录到点
							time[PointReached - 2] = t - prev_t;						//记录时间
							prev_t = t;													//记录时间,用于判断什么时候发消息
							SoundFunction(BeepTimePointReached, DaoDianBeep)			//声音提示
							if(Mdsjxsff == MDSJJSXS)									//每道时间即时显示才显示这道成绩
							{
								printftime(t, 5, 4);									//打印最后总时间
								printftime(time[PointReached - 2], (PointReached % 2) * 4 + 1, PointReached / 2);//打印最后一个５米时间
							}
							else
							{
								LCDWrite(WriteCode, LCD_CLEAR);				//清屏
								delayms(2);									//等待清屏
								for(i = 0; i < 6; i++)
								{
									printftime(time[i], (i % 2) * 4 + 1, i / 2 + 1);	//打印时间
								}
								LCDSetXY(1, 4);											//设置打印位置
								LCDWriteString(strtotaltime);							//在LCD上显示"总时间"
								printftime(t, 5, 4);									//打印最后总时间

							}
							delayms(100);												//如果满足条件,那么有可能导致连续两次SendMXTime时间太近,显示不了
							SendMXTime(t);												//发送最后时间给LED

							//数据更新部分
							{
								//写入成绩
								WriteWordEEPROM(GetANewAddrToSaveRecord(), t);			//写入成绩
								WriteWordEEPROM(NoSuccessAddr, NoSuccess + 1);			//写入成功次数
								NoSuccess++;											//成功次数加1
								WriteWordEEPROM(NoUsedAddr, NoUsed + 1);				//写入使用次数
								NoUsed++;												//使用次数加1
								if((++CanotResetNo) >= MaxCanotResetNo)					//写入一个不可复位的次数
									CanotResetNo = 0;
								if((WorkMode == WORKMODE_COUNTTIMES) && (RemainTimes > 0))//计次模式时,计数减1
								{
									WriteWordEEPROM(RemainTimesAddr, --RemainTimes);
								}
								WriteWordEEPROM(CanotResetNoAddr, CanotResetNo);
								/* 将记录更新放入下面程序中，只有桩信号没有问题才与入最好成绩
								if((t < FastestRec) || (FastestRec == (uint)0))			//是否为新记录
								{
									FastestRec = t;
									WriteWordEEPROM(FastestRecAddr, FastestRec);		//写入最新记录
								}
								*/
							}
						    SpeakTime(t);												//语音报时
							//下面程序用来检测异常成绩,如果有异常成绩很有可能是某个桩光线没有对正了
							{
								uchar *p, TestStr[] = "XXXXXX号感应器有问题!";
								p = TestStr + 5;						//指向最后一个X
								
								if(time[5] < (uint)Every5MMinTime) *p-- = '6';
								if(time[4] < (uint)Every5MMinTime) *p-- = '5';
								if(time[2] < (uint)Every5MMinTime) *p-- = '4';
								if((time[3] < (uint)Every5MMinTime) || ((time[1] < (uint)Every5MMinTime))) *p-- = '3';
								if(time[0] < (uint)Every5MMinTime) *p-- = '2';								

								if(p != (TestStr + 5))					//p被改动,说明有异常
								{
									SendMXString(" ", 3, MXCodeWAITOK);
									SendMXCode(MXCodeContent3Move, MXCodeLeftShift, MXCodeWAITOK);
									SendMXCode(MXCodeDisplay, MXCodeDisplayContent3, MXCodeWAITOK);
									SendMXString(p+1, 3, 1);
									delayms(8000);
									SendMXCode(MXCodeRealtimeDisplay, MXCodeNULLContent, MXCodeWAITOK);
									SendMXTime(t);												//继续显示时间
								}
								else if((t < FastestRec) || (FastestRec == (uint)0))			//是否为新记录
								{
									FastestRec = t;
									WriteWordEEPROM(FastestRecAddr, FastestRec);				//写入最新记录
									SendMXString(strZXJL, 3, 0);								//发送最佳成绩给显示屏
								}
							}
							SendMsg(MergeMsg(1, STARTWORK));									//让1号桩重新开始工作
							break;

						case 1:							
ViewWorkMode:				if((WorkMode == WORKMODE_COUNTTIMES) && (RemainTimes == 0))			//是计次模式,且剩余次数为0,这个判断不去管有没有消息
							{
								DisplayRemainTimesOverInfo();					//显示信息
								while(GetKey(KeyDelay) == NULLKey);				//等待按键
								Menu();
								goto ViewWorkMode;
							}
							t = prev_t = t_LastHere =0;						//将时间归零
							TR0 = 0;										//停表
							PointReached = 0;								//还没到达任何点,离开1号桩时,为到达了1号点
							MsgSend = NULLMsg;								//表示没有消息可发送
							if(Mdsjxsff == MDSJJSXS)						//要实时显示每道时间才清屏,否则不清屏
							{
								LCDWrite(WriteCode, LCD_CLEAR);				//清屏
								delayms(2);									//等待清屏
								LCDSetXY(3, 2);								//设置打印位置
								LCDWriteString(strYJW);						//在LCD上显示已就位
							}
							SendMXString(strYJW, 3, 0);						//发送已就位给显示屏
							SoundFunction(BeepTimeStart, ReadyToGoBeep) 	//声音提示
							break;
					}
					break;

				case LEAVED:											//从1号桩离开时,1号桩会发出这个信息
					TR0 = 1;											//开始计时
					PointReached = 1;									//1号桩已离开
					SoundFunction(BeepTimePointReached, DaoDianBeep)	//离开时声音
					MsgSend = MergeMsg(2, STARTWORK);					//这是要发送的信息,但现在不会发,要等一段时间之后才会发
					if(Mdsjxsff == MDSJJSXS)							//每道时间需要即时显示时才清屏打印"总时间:"字样
					{
						LCDWrite(WriteCode, LCD_CLEAR);					//清屏
						delayms(2);										//等待清屏
						LCDSetXY(1, 4);									//设置打印位置
						LCDWriteString(strtotaltime);					//在LCD上显示"总时间"字样
					}
					break;

				case TIMEOVER:
					TR0 = 0;											//停止计时
					SoundFunction(BeepTimeTimeOver, JiShuFanGuiBeep)	//声音提示
					if(Mdsjxsff == MDSJJSXS)							//如果不是即时显示,不显示超时和恢复时间
					{
				   		LCDSetXY(((PointReached - 1) % 2) * 4 + 1, (PointReached - 1) / 2 + 1); //设置超时打印位置
						LCDWriteString(strtimeover);						//打印超时
						printftime(0, 5, 4);								//将时间打印为0
					}
					delayms(100);										//有可能两个时间相隔太近，else中的显示与这里的显示, 
					SendMXString(strJSFG, 3, 0);					   	//外接显示屏设为技术犯规
					delayms(30);										//等待前面的声音提示完再发另一个声音提示
					SendThreeLines(JiShuFanGui, WaitIfBusy);			//报技术犯规
					
					//数据更新部分
					{
						if(PointReached > 1)							//已经过了2号桩
						{
							WriteWordEEPROM(NoUsedAddr, ++NoUsed);		//写入最近使用次数	
						}
					}
					SendMsg(MergeMsg(1, STARTWORK));					//重新打开一号
					break;
			}
		}
		else
		{
			//任务1：备份时间　得到离开上一点到现在时间差,保存上一次到这里的时间,并比较,每百分之一秒只运行一次else
			ET0 = 0;t_Copy = t;ET0 = 1;									//关定时器0中断,因为它涉及t的更新
			t_Every5M = t_Copy - prev_t;								//得到一个离开上一点的时间

			//任务2：如有消息，发送消息
			if((MsgSend != NULLMsg) && (t_Every5M > SendMsgWaitTimeDefault))		
			{
				SendMsg(MsgSend);
				MsgSend = NULLMsg;
			}

			//任务3：更新跑表
			if((t_Copy % FreqOfUpdateDisplay == 0) && (PointReached >= 1) && (t_LastHere != t_Copy))	//跑表,百分之一秒只刷新一次,避免刷新多次
			{
				if(Mdsjxsff == MDSJJSXS)			 								//只有即时显示方式才更新LCD跑表
					printftime_IfNoMsg(t_Copy, 5, 4);
				SendMXTime(t_Copy);													//发送时间给外接显示屏
				t_LastHere = t_Copy;												//保存最近进入这个if的时间
			}

			//任务４：判断通信出错
			if(t_Every5M > ErrorTime)		
			{
				TR0 = 0;									//停止计时
				if(BeepType == BeepBuzzer)					//出错Beep
					ErrorBeep();
				else
					SendThreeLines(JiShuFanGuiBeep, NotWait);//声音提示
			   	LCDSetXY(((PointReached - 1) % 2) * 4 + 1, (PointReached - 1) / 2 + 1); //设置错误打印位置
				LCDWriteString(strerror);					//打印出错标记
				printftime(0, 5, 4);						//将时间打印为0
				delayms(100);								//如果满足条件,那么有可能导致连续两次SendMXTime时间太近,显示不了
				SendMXString("通讯错误", 3, 0);				//外接显示屏提示信息
				t = prev_t = t_LastHere = 0;				//防止再次进入
				delayms(2000);								//等待一段时间,开始下一次
				SendMsg(MergeMsg(1, STARTWORK));			//重新打开一号
			}
			
			//任务５：响应按键
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
							SendMXString("设置中...", 3, 0);
							Menu();
							SendMXString(" ", 3, 0);
							break;
		
						default:
							goto NotReset;							
					}
				}
				InitForStartTest();				//初始化,用于测试
			} 
NotReset:;
		}
	}
}

void ErrorBeep()
{
	Beep(BeepTimeError);							//声音提示
	delayms(BeepTimeError * 20);					//停止一段时间
	Beep(BeepTimeError);							//声音提示
	delayms(BeepTimeError * 20);					//停止一段时间
	Beep(BeepTimeError);							//声音提示
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
	//清屏，并提示
	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	delayms(2);												//等待清屏
	LCDSetXY(3, 2);	
	LCDWriteString(strinit);								//初始化

	//重启动桩
	RestartSub();											//重启动所有桩

	//初始化变量
	InitGlobalVal();										//读入一些全局变量
	PointReached = 0;										//没有到达作何桩
	MsgSend = NULLMsg;										//没有任何消息需要发送
	TR0 = 0;												//计时器不启动
	t = prev_t = t_LastHere = 0;							//初始化时间

	//发送消息，初始化桩
	ClearMsg();
	SendMsg(SETWORKTIME);									//设置桩工作时间命令
	SendMsg(WorkTime);										//设置工作时间
	for(i = 1; i < 7; i++)
	{
		SendMsg(MergeMsg(GetReflectID(i), TESTOVER));		//给所有B桩发TestOver信号
		SendMsg(MergeMsg(i, TESTOVER));						//给所有A桩发TestOver信号	
	}

	//清屏，并提示
	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	delayms(2);												//等待清屏
	LCDSetXY(3, 2);	
	LCDWriteString(strreadytogo);							//在LCD上显示准备开始
	SendMXString(strreadytogo, 3, 1);						//发送准备开始给显示屏
	SendMsg(MergeMsg(1, STARTWORK));						//给1号桩发送开始工作信号
}

void Timer0_Routine(void) interrupt 1
{
    TH0 = CLOCK_TH0 + TH0;
    TL0 = CLOCK_TL0 + TL0;
	t++;
}

void printftime_IfNoMsg(uint time, uchar x, uchar y)	 					//在x,y 打印出时间
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

void printftime(uint time, uchar x, uchar y)	 					//在x,y 打印出时间
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

	//超时时长
	Data = ReadByteEEPROM(TimeOverAddr);								//读入超时时长,并计算出桩的工作时间
	if((Data >= TimeOverMin) && (Data <= TimeOverMax))					//0.1秒的整数倍
	{
		if(Data > (SendMsgWaitTimeDefault / 10))
			WorkTime = Data - SendMsgWaitTimeDefault / 10;				//桩的工作时间 = 超时时长 - 发消息延时时间
	}
	else
	{
		WorkTime = WorkTimeDefault;										//读取不正常,设置默认时间
	}
	//最近使用次数
	NoUsed = ReadWordEEPROM(NoUsedAddr);

	//最近成功次数
	NoSuccess = ReadWordEEPROM(NoSuccessAddr);

	//最好成绩
	FastestRec = ReadWordEEPROM(FastestRecAddr);

	//得到BeepType
	BeepType = ReadByteEEPROM(BeepTypeAddr);

	//得到CannotResetNo
	CanotResetNo = ReadWordEEPROM(CanotResetNoAddr);
	if(CanotResetNo >= MaxCanotResetNo)
	{
		CanotResetNo = 0;
		WriteWordEEPROM(CanotResetNoAddr, 0);
	}
	//得到工作模式
	WorkMode = ReadByteEEPROM(WorkModeAddr);
	if((WorkMode != WORKMODE_NOLIMIT) && (WorkMode != WORKMODE_COUNTTIMES))		
	{
		WorkMode = WORKMODE_COUNTTIMES;
		WriteByteEEPROM(WorkModeAddr, WorkMode);
	}
	//得到剩余次数
	RemainTimes = ReadWordEEPROM(RemainTimesAddr);

   //设置音量
   Data = ReadByteEEPROM(YingLiangAddr);
   if((Data >= YingLiangMin) && (Data <= YingLiangMax))
   {
   		delayms(100);
		SendThreeLines(YingLiangBase + Data, WaitIfBusy);   
   }

   //得到每道时间显示方式
	Mdsjxsff = ReadByteEEPROM(MdsjxsffAddr);
}

void InitForBeep()
{
#define CONST_CCAP0H	0x27
#define CONST_CCAP0L	0x10
	Set0(P3M1, 4);		//设置Beep为强推挽
	Set1(P3M0, 4);

	CMOD = 0x00;		// SYS/12,不允许溢出中断
	CCON = 0x00;		//清标志位CF CR / / / / CCF1 CCF0
	CCAPM0 = 0x49;		//16位定时器 匹配时中断
	CH =  0;			//计数器置0
	CL = 0;
	CCAP0L = CONST_CCAP0L;
	CCAP0H = CONST_CCAP0H;
	Speak = StopBeep;
	SpeakCount = 0;
	
}

void InterruptPCA() interrupt 7
{
	CH = 0;				//计数器置0
	CL = 0;
	CCF0 = 0;			//清中断标志位
	if(SpeakCount-- == 0)
	{
		STOPPCA;
		Speak = StopBeep;
	}
}

void Beep(uchar Ten_ms)
{
	CCF0 = 0;				//清中断标志位
	CH = 0;					//计数器置0
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
	//没有存满100个,数据都在[RecordStartAddr, RecordStartAddr + (NoSuccess - 1) * 2]之间
	if(NoSuccess < NoOfMaxSaved)
	{
		//比起始地址大，比最大记录小，且有1个以上的数据,即至少有两个数据
		if((CurAddr > RecordStartAddr) && (CurAddr <= GetCurrentRecordAddr()) && (NoSuccess > 1))		//从起始地址 到 当前记录 之间的数据地址
			return CurAddr - 2;
		else													//没有前一个数据了
			return NULLPoint;								
	}
	//所有存储区存满了数据
	else
	{
		if(CurAddr != RecordStartAddr)							//不会移到存储区之前去
		{
			CurAddr -= 2;										//不会移到存储区之前,向前移动
		}
		else
		{
			CurAddr = RecordStartAddr + (NoOfMaxSaved - 1) * 2;	//会移动到存储区之前,移到最后一个
		}

		if(CurAddr == GetCurrentRecordAddr())					//如果前一个是当前最新记录,说明到头了
			return NULLPoint;
		else
			return CurAddr;
	}
}

/*
void TestBatteryTime()
{
	//电池测试程序
	uchar FailCount = 0, myMsg;								//失败测试
	uint min1, min2;										//记录正常运行的分钟数

	RestartSub();											//重启动从机
	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	delayms(2);	
	t = 0;
	TR0 = 1;												//开定时器
	LCDSetXY(1, 1);
	LCDWriteString("丢消息数:");
	LCDSetXY(6, 1);
	LCDPrintfByte(FailCount, 1);							//打印丢消息数

	LCDSetXY(1, 2);
	LCDWriteString("运行时间1:");							//打印运行时间
	min1 = ReadWordEEPROM(Min1Addr);
	LCDSetXY(6, 2);
	LCDPrintfWord(min1, 1);

	LCDSetXY(1, 3);
	LCDWriteString("运行时间2:");
	min2 = ReadWordEEPROM(Min2Addr);
	LCDSetXY(6, 3);
	LCDPrintfWord(min2, 1);

	LCDSetXY(1, 4);
	LCDWriteString("F1继续  F2重计");

	while(1)
	{
		switch(GetKey(0))									//测试按按键
		{
			case KeyF1:
				goto NotClearToZero;
			
			case KeyF2:
				goto ClearToZero; 
		}
	}
ClearToZero:
	WriteWordEEPROM(Min1Addr, 0);							//初始化EEPROM
	WriteWordEEPROM(Min2Addr, 0);							//初始化EEPROM

	min1 = 0;
	min2 = 0;
	FailCount = 0;

NotClearToZero:
	delayms(1000);

	LCDSetXY(6, 1);
	LCDPrintfByte(FailCount, 1);							//打印丢消息数

	LCDSetXY(6, 2);
	LCDPrintfWord(min1, 1);

	LCDSetXY(6, 3);
	LCDPrintfWord(min2, 1);

	LCDSetXY(1, 4);
	LCDWriteString("                ");
	while(1)
	{
		ClearMsg();											//清消息队列
		myMsg = MergeMsg(1, COMMUNICATIONTEST);				//1A通讯测试消息
		SendMsg(myMsg);										//给1A发送消息
		delayms(10);										//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
		if(GetMsg() != myMsg)
		{
			FailCount++;					//记录丢消息数目
			LCDSetXY(6, 1);
			LCDPrintfByte(FailCount, 1);					//打印丢消息数
		}
		ET0 = 0;											//复制t
		t_Copy = t;
		ET0 = 1;
		
		if(t_Copy >= 6000)									//每一分钟写一次EEPROM
		{
			ET0 = 0;										//写t
			t = 0;	
			ET0 = 1;									  	//时间归0
			min1++;											//分钟+1
			WriteWordEEPROM(Min1Addr, min1);				//分钟数写入EEPROM
			LCDSetXY(6, 2);									//打印运行时间
			LCDPrintfWord(min1, 1);

			min2++;											//分钟+1
			WriteWordEEPROM(Min2Addr, min2);				//分钟数写入EEPROM
			LCDSetXY(6, 3);									//打印运行时间
			LCDPrintfWord(min2, 1);

			LCDSetXY(6, 1);    								//打印丢消息数
			LCDPrintfByte(FailCount, 1);

		}
		if((t_Copy % 10) == 0)								//1秒更新十次LED,
		{
			SendMXTime(t_Copy);								//更新时间		
		}
		if((t_Copy % 1000) == 0)							//10秒发一次声音
		{
			SendThreeLines(DaoDianBeep, WaitIfBusy);
		}
		if(FailCount > 5)
		{
			LCDSetXY(1, 4);
			LCDWriteString("测试完成");
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
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(2);										//等待清屏
	LCDSetXY(1, 1);									//设置打印位置
	LCDWriteString("购买次数使用完毕");		
	LCDSetXY(1, 2);									//设置打印位置
	LCDWriteString("请联系设备提供者");
	LCDSetXY(1, 3);									//设置打印位置
	LCDWriteString("获得使用授权");
	LCDSetXY(1, 4);
	LCDWriteString("TEL:15399900725");				//打印电话
}

void FirstPowerOntoResetValues(){
	if(ReadByteEEPROM(FirstPowerOn) != 'X'){
		//恢复TimeOver
		WriteByteEEPROM(TimeOverAddr, TimeOverDefault);		
		//恢复使用计数
		WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			
		//恢复犯规计数
		WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		
		//恢复最佳成绩
		WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		
		//从音箱输出声音
		WriteByteEEPROM(BeepTypeAddr, BeepDAC);
		//设置密码	111111
		WriteEEPROM(ATMEL24C02, PasswordAddr, "111111", PasswordLen);
		//设置工作模式 无限模式
		WriteByteEEPROM(WorkModeAddr, WORKMODE_NOLIMIT);
		//设置剩余次数
		WriteWordEEPROM(RemainTimesAddr, (uint)50000);
		//设置音量
		WriteByteEEPROM(YingLiangAddr, 4);
		//设置第道时间显示方式
		WriteByteEEPROM(MdsjxsffAddr, MDSJJSXS);
		//设置仪器总次数（不可复位的）
		WriteWordEEPROM(CanotResetNoAddr, 0);
		//将FirstPowerOn标志设为'N'
		WriteByteEEPROM(FirstPowerOn, 'X');
	}
}
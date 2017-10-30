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
20121017	在Menu中加入一句Beep(10) 是为了修复在1号桩让主机Beep时,进入菜单会一直叫下去的Bug
20121017	因为改动了Test10T的工作时间,所以测试等待消息中改动了一名将delayms(10), 改为delayms(20)
20121102	删除密码机制
20121103	删除一些菜单,重新调整菜单
20121113	在Menu中删除了Beep(10)这句,在GetKey()中加入了Beep(10),提供按键提示音
*/	

extern uchar isRoot;
extern uchar code ThisUser[];
typedef void(*VoidFunctionPoint)(void);
VoidFunctionPoint code FunctionTable[] = {
	FactorySet,				// 0	恢复默认设置
	SetIDs,					// 1	设置从机ID
	SetBeepType,			// 2	设置为音箱输出声音还是小蜂鸣器输出声音
	ViewMsg2,				// 3	查看消息
	MsgTest,				// 4	测试发送消息的稳定性
	SetTimeOver,			// 5	设置超时
	SetPassword,			// 6	设置密码
	SetWorkMode,			// 7	设置工作模式
	SetRemainTimes,			// 8	设置剩余次数
	SetLiangDu,				// 9	设置亮度
	SetYingLiang,			// 10	设置音量
	SetMDSJXSFF,			// 11	设置每道时间显示方法
	QuickSetIDs,			// 12	快速设置ID
	ZhuangSelfTest,			// 13	老版本的桩自测试程序
	ZhuJiTest				// 14 	主机自测试
	};

void Menu(void)
{
	uchar MenuDraw;
	//20121017 修改
	//Beep(20);
	//20140929将几乎所有菜单删除，留下三个用户能用到的和一个高级设置，
	//在程序中建立一个函数表格，从中读取而运行不同的功能
	//至于要运行什么功能，只能通过查表获得
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
			LCDWriteString("1 开始测试");
			LCDSetXY(1, 2);
			LCDWriteString("2 调试系统");
			LCDSetXY(1, 3);
			LCDWriteString("3 单桩检测");
			if(IsRoot){
				LCDSetXY(1, 4);
				LCDWriteString("4 高级设置");
			}else{
				LCDSetXY(1, 4);
				LCDWriteString("4 统计信息");
			}
		}
		switch(GetKey(KeyDelay))
		{
			//函数返回，开始测试工作。
			case 1: LCDWrite(WriteCode, LCD_CLEAR);delayms(3);return;
			//重新设计调试系统
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
			//单桩功能全面检测
			case 3: IRRecTimeLine(); goto Menu1;
			//高级函数运行入口
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
			//显示统计数据
			case KeyF1:
				DisplaySumary();
				delayms(500);
				goto Menu1;
			//查看成绩
			case KeyF2:
				ViewRecord();
				delayms(500);
				goto Menu1;
			//关闭背光
			case KeyF3:
				LCD_PSW = LCDBKLightON;
				break;
			//打开背光
			case KeyF4:
				LCD_PSW = LCDBKLightOFF;
				break;
			default:break;
		}

	}
	delayms(500);
}


//请保证pBuffer比N多一个字节
//如果没有输入，返回0
uchar NumberInput(uchar *pBuffer, uchar N, char IsPassword, uchar x, uchar y)
{
	uchar myKey, i = 0, j;
	LCDSetXY(x, y);
	while(1)
	{
		myKey = GetKey(KeyDelay);
		switch(myKey)
		{
			case KeyLeft: 	//好复杂的删字符程序
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
将录入的字符串输出为数字
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
	//在线桩超过1个，不设置
	if(GetOnLineIDs() > 1){
		LCDSetXY(1, 1);
		LCDWriteString("错误: 在线桩太多");
		LCDSetXY(1, 2);
		LCDWriteString("请只连接一个传感器");
		LCDSetXY(1, 3);
		LCDWriteString("器在主机上！");
		delayms(2000);
		return;
	}

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("请输入ID: ");
	LCDSetXY(1, 2);
	LCDWriteString("A:1-6 B:9-14");
	LCDSetXY(1, 3);
	LCDWriteString("警告: 系统会自毁");
	LCDSetXY(1, 4);
	LCDWriteString("按返回取消设置");

	//如果按下了返回键，不执行操作
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
		LCDWriteString("设置成功        ");
	}
	else
	{
		LCDWriteString("设置不正确      ");	
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
	LCDWriteString("请输入前两位");
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
		LCDWriteString("设置成功    ");
	}
	else
	{
		LCDWriteString("设置不正确    ");	
	}
	delayms(2000);
}

uchar CommunicationTest()
{
	uchar i, myMsg, ErrorCount = 0, state[12];

	LCDWrite(WriteCode, LCD_CLEAR);								//清屏
	//初始化数组
	for(i = 0; i < 12; i++){
		state[i] = 0;		
	}
	while(1){
		ErrorCount = 0;
		for(i = 1; i < 7; i++)									//给1至6号桩发送一个带ID的COMMUNICATIONTEST消息.如果返回这个消息,说明通讯正常
		{
			ClearMsg();											//清消息队列
			myMsg = MergeMsg(i, COMMUNICATIONTEST);				//准备好要发的测试消息
			SendMsg(myMsg);										//给i号桩A发送一个COMMUNICATION_TEST信号
			delayms(10);										//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
			LCDSetXY(((i + 1) % 2) * 5 + 1, ((i + 1) / 2));	  	//设置打印位置
			if(myMsg == GetMsg())								//取消息
			{
				//与存储的状态不同时发出声音
				if(state[i - 1] == 0){
					Beep(10);
				}
				//存储现在的状态
				state[i - 1] = 1;
				LCDWriteString("OK");							//回的消息和发送的一致,打印OK
			}
			else
			{
				//与存储的状态不同时发出声音
				if(state[i - 1] == 1){
					Beep(10);
				}
				//存储现在的状态
				state[i - 1] = 0;
				ErrorCount++;									//没有消息,消息会为NULLMsg,打印出桩的号码,并记录出错次数
				LCDWrite(WriteData, i + '0');
				LCDWrite(WriteData, 'A');
			}
	
	
			ClearMsg();													//清消息队列
			myMsg = (MergeMsg(GetReflectID(i), COMMUNICATIONTEST));  	//准备好要发的测试消息,注意要在i的基础置上反射桩的标志
			SendMsg(myMsg);												//给i号桩B发送一个COMMUNICATION_TEST信号
			delayms(10);												//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
			LCDSetXY(((i + 1) % 2) * 5 + 3, ((i + 1) / 2));				//设置打印位置
			if(myMsg == GetMsg())										//取消息
			{
				//与存储的状态不同时发出声音
				if(state[i + 6] == 0){
					Beep(10);
				}
				//存储现在的状态
				state[i + 6] = 1;
				LCDWriteString("OK");	 								//回的消息和发送的一致,打印OK
			}
			else
			{
				//与存储的状态不同时发出声音
				if(state[i + 6] == 1){
					Beep(10);
				}
				//存储现在的状态
				state[i + 6] = 0;
				ErrorCount++;										 	//没有消息,消息会为NULLMsg,打印出桩的号码,并记录出错次数
				LCDWrite(WriteData, i + '0');							
				LCDWrite(WriteData, 'B');
			}		
		}
	
		LCDSetXY(1, 4);
		if(ErrorCount > 0)												 //打印统计信息
		{
			LCDPrintfByte(ErrorCount, 1);
			LCDWriteString(" 个桩通讯异常");
		}
		else
		{
			LCDWriteString("    通讯测试通过");
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

	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	for(i = 0; i < 6; i++){
		state[i] = 0;
	}

	while(1){
		ErrorCount = 0;
		for(i = 1; i < 7; i++)
		{
			ClearMsg();											//清消息队列
			SendMsg(MergeMsg(GetReflectID(i), TESTOVER));		//让i号桩B退出测试,进入Reflect();							
			SendMsg(MergeMsg(GetReflectID(i), STARTWORK));  	//让i号桩B开始工作
			myMsg = MergeMsg(i, INFRAREDTEST);					//准备好测试消息
			SendMsg(myMsg);										//给i号桩A发送一个INFRARED_TEST信号
			//delayms(10);										//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
			delayms(20);										//等待消息,2ms收消息,2ms回消息,10ms测试红外连通性,20ms足够收到回的消息了
			SendMsg(MergeMsg(GetReflectID(i), STOPWORK));		//给i号桩B发送一个STOPWORK信号
	
			LCDSetXY(((i + 1) % 2) * 5 + 1, ((i + 1) / 2));		//设置打印位置
			if(myMsg == GetMsg())
			{	
				if(state[i - 1] == 0){
					Beep(10);
				}
				state[i - 1] = 1;									  			
				LCDWriteString("  OK   ");						//回的消息和发送的一致,打印OK
			}
			else
			{												   	
				ErrorCount++;									//没有消息,消息会为NULLMsg,打印出桩的号码,并记录出错次数
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
	
		LCDSetXY(1, 4);											  //打印统计信息
		if(ErrorCount > 0)
		{
			LCDPrintfByte(ErrorCount, 1);
			LCDWriteString(" 对桩红外异常");
		}
		else
		{
			LCDWriteString("    红外测试通过");
			Beep(10);
		}
		if(GetKey(0) != NULLKey){
			return 0;
		}
	}
}

void FactorySet()
{	
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("恢复出厂设置吗?");
	LCDSetXY(1, 3);
	LCDWriteString("确定--恢复");
	LCDSetXY(1, 4);
	LCDWriteString("返回--不恢复");

	while(1)
	{
		switch(GetKey(KeyDelay))
		{
			case KeyEnter:
				LCDWrite(WriteCode, LCD_CLEAR);					//清屏
				delayms(10);
				LCDSetXY(1, 2);
				LCDWriteString("恢复中......");						//提示
				WriteByteEEPROM(TimeOverAddr, TimeOverDefault);		//恢复TimeOver
				WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			//恢复使用计数
				WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		//恢复犯规计数
				WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		//恢复最佳成绩
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
	//定义并初始化变量
	uchar RecPoint = 0, Count = 0, BackupCount = 0, Success = 0, myKey;
	uint Avg = 0, Best = 0;
	unsigned long Sum = 0; //因为要统计NoOfMaxSaved个uint记录 100 * 2500 = 250000,所以应该用一个usigned long
		
	//计算平均成绩
	if(NoSuccess > 0)
	{
		//先加上当前地址的成绩
		RecPoint = GetCurrentRecordAddr();
		Sum += (unsigned long)ReadWordEEPROM(RecPoint);
		Count++;
		//再累加当前地址之前的成绩
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

	//计算犯规比率
	if(NoUsed > 0)
		Success = (NoUsed - NoSuccess) * 100 / NoUsed;
  
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(100);
	
	//打印仪器使用次数
	LCDSetXY(1, 1);
	LCDWriteString("使用次数: ");
	LCDPrintfWord(NoUsed, 0);
	LCDSetXY(1, 2);
	LCDWriteString("犯规比率: ");
	LCDPrintfWord(Success, 0);
	LCDWriteString("%");
	LCDSetXY(1, 3);
	LCDWriteString("最佳成绩: ");
	printftime(FastestRec, 6, 3);
	LCDSetXY(1, 4);
	LCDWriteString("平均成绩: ");
	printftime(Avg, 6, 4);
	//等待1S后等待按键
	delayms(1000);
	while(GetKey(KeyDelay) == NULLKey);
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("仪器使用总次数: ");
	LCDSetXY(1, 2);
	LCDPrintfWord(CanotResetNo, 0);
	LCDSetXY(1, 3);
	LCDWriteString("按F3清空数据");
	LCDSetXY(1, 4);
	//将下列字符串移动到了Main.C中
	LCDWriteString(ThisUser);
	delayms(1000);
	while(1){
		myKey = GetKey(KeyDelay);
		if(myKey != NULLKey){
			if(myKey == KeyF3){
				WriteWordEEPROM(NoUsedAddr, (uint)0x0000);			//恢复使用计数
				WriteWordEEPROM(NoSuccessAddr, (uint)0x0000);		//恢复犯规计数
				WriteWordEEPROM(FastestRecAddr, (uint)0x0000);		//恢复最佳成绩
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
	//清屏
	LCDWrite(WriteCode, LCD_CLEAR);	
	delayms(100);

   	//用一个tmp数组记录各屏要显示的第一个成绩的地址
	if(NoSuccess > 0)
	{
		tmp[0] = RecPoint = GetCurrentRecordAddr();
		zushu = 1;
	}
	else
	{
		//没有数据用于显示
		LCDSetXY(2, 2);
		LCDWriteString("无数据记录");
		delayms(500);
		return;
	}

	//统计一组8个数据,共有多少组,每一组起始地址是什么
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
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	RestartSub();									//重新启动所有桩
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("您要调试哪个桩光");
	LCDSetXY(1, 2);
	LCDWriteString("线的准确度:");
   	//得到用户按键,如果没有按下任何键一直等待
	//临时用一下debugMsg,让等待按键的时间设置为5秒钟
	debugMsg = 5 * 5;
	while(debugMsg--)
	{
		//显示倒计时10秒
		LCDSetXY(8, 4);
		LCDPrintfByte(debugMsg / 5, 0);
		//测试按键
		myID = GetKey(KeyDelay);
		//判断按键
		if(myID != NULLKey){
			//如果按下了返回键，则退出
			if(myID == KeyReturn){
				return;
			}
			lastID = myID;
			break;
		}
	}
	if(myID == NULLKey){
		//默认从1号桩开始调试
		myID = ++lastID;
	}
	//如果不是按的1-6，则退出调试
	if((myID >= 1) && (myID <= 6))
	{
		//让ID号桩反射桩退出测试和设置
		SendMsg(MergeMsg(GetReflectID(myID), TESTOVER));
		delayms(100); 
		//让ID号桩的反射桩开始工作
		SendMsg(MergeMsg(GetReflectID(myID), STARTWORK));
		//清屏 
		LCDWrite(WriteCode, LCD_CLEAR);		
		delayms(100);
		//打印提示
		LCDSetXY(2, 2);
	   	LCDPrintfByte(myID, 0);
		LCDWriteString(" 号桩调试中..");
		//让ID号桩的发射桩开始光线准度测试
		SendMsg(MergeMsg(myID, IRPOSITIONTEST));
		SendMsg(MergeMsg(myID, STARTWORK));
		//处理ID号桩发来的消息
		while(1)
		{
			//有消息来，处理消息
			if(GetMsgCount() > 0)
			{
				debugMsg = GetMsg();
				//对应桩发来的消息
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
				//停止叫声
				Speak = StopBeep;
				//让ID号桩的发射桩停止工作
				SendMsg(MergeMsg(myID, STOPWORK));
				//让ID号桩的反射桩也停止工作
				SendMsg(MergeMsg(GetReflectID(myID), STOPWORK));
				delayms(200);
				//取完所有消息再重新开始
				while(GetMsg() != NULLMsg);
				Beep(10);
				delayms(1000);
				//为简化程序设计，调试完一个桩后，用gogo语句回到前面重新调试另一个桩
				goto StartIRPositionTest;
			}
		}
	}
}

void SetBeepType()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("请按数字选择音源");
	LCDSetXY(1, 2);
	LCDWriteString("当前:");

	if(BeepType == BeepBuzzer)
		LCDWriteString(" 小蜂鸣器");
	else
		LCDWriteString(" 音箱输出");

	LCDSetXY(1, 3);
	LCDWriteString("0 音箱输出");
	LCDSetXY(1, 4);
	LCDWriteString("1 小蜂鸣器");

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
核对密码和EEPROM存储的密码是否相同
*/
uchar CheckPassword()
{
	uchar Password[PasswordLen + 2], InputPassword[PasswordLen + 2], i;

	//清屏
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);

	//清空缓存区
	for(i = 0; i < PasswordLen; i++) Password[i] = 0;

	//读入EEPROM密码到内存
	ReadEEPROM(ATMEL24C02, PasswordAddr, Password, PasswordLen);		//读入密码到Password

	//读出设置的密码长度
	for(i = 0, Password[PasswordLen] = 0; i < PasswordLen; i++)
		if((Password[i] >= '0') && (Password[i] <= '9'))
			Password[PasswordLen]++;

	//密码不足PasswordLen, 视为没有设置密码
	if(Password[PasswordLen] != PasswordLen)
	{
		LCDSetXY(1, 4);
		LCDWriteString("密码为空不能操作");
		delayms(2000);
		return NULLPassword;					//密码为空
	}
	//提示输入密码
	LCDSetXY(1, 1);
	LCDWriteString("请输入密码: ");
	for(i = 0; i < PasswordLen; i++) InputPassword[i] = 0;
	NumberInput(InputPassword, 6, 1, 1, 2);										//输入密码
	//比较密码
	for(i = 0; i < PasswordLen; i++)
	{
		if(Password[i] != InputPassword[i])
			break;
	}
	//提示结果并返回结果
	LCDSetXY(1, 4);
	if(i == PasswordLen)
	{
		LCDWriteString("密码正确");
		delayms(2000);
		return RightPassword; 											//密码不正确
	}
	else
	{
		LCDWriteString("密码错误");
		delayms(2000);
		return WrongPassword;											//密码正确
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
		LCDWriteString("请输新密码:");
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
			LCDWriteString("设置成功");
		}
		else
		{
			LCDSetXY(5, 4);
			LCDWriteString("设置失败");
		}	
		delayms(2000);
	}
}

void SetWorkMode()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("请按数字选择模式");
	LCDSetXY(1, 2);
	LCDWriteString("当前:");

	if(WorkMode == WORKMODE_NOLIMIT)
		LCDWriteString(" 无限模式");
	else
		LCDWriteString(" 计次模式");

	LCDSetXY(1, 3);
	LCDWriteString("0 无限模式");
	LCDSetXY(1, 4);
	LCDWriteString("1 计次模式");

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
	LCDWriteString("设置成功");
	delayms(2000);
NotSet:
	return;
}

void SetRemainTimes()
{
	uint tmp;
	uchar pData[6];
	
	//读取,并显示次数,并接受输入
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("Now:");
	RemainTimes = ReadWordEEPROM(RemainTimesAddr);
	LCDPrintfWord((uint)RemainTimes, 0);
	LCDSetXY(6, 1);
	LCDWriteString("次");
	LCDSetXY(1, 4);
	LCDWriteString("请输入次数");
	LCDSetXY(1, 2);
	LCDWriteString("New:");
	LCDSetXY(6, 2);
	LCDWriteString("次");
	if(NumberInput(pData, 5, 0, 3, 2) == 0)			//没有输入数字时返回
		return;
	tmp = CharToNum(pData);

	//确认密码
	if(CheckPassword() != RightPassword)
		return;

	//写入次数
	WriteWordEEPROM(RemainTimesAddr, tmp);
	RemainTimes = tmp;

	//提示
	LCDSetXY(5, 4);
	LCDWriteString("设置成功");
	delayms(2000);
}

void SetLiangDu()
{
	//接受输入
	uchar tmp;
	uchar pData[3];

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("请输入两位数0-15");
	LCDSetXY(1, 2);
	LCDWriteString("New:");
	NumberInput(pData, 2, 0, 3, 2);
	tmp = (uchar)CharToNum(pData);
	LCDSetXY(1, 4);
	if((tmp >= MXCodeLiangDuMin) && (tmp <= MXCodeLiangDuMax))
	{
		SendMXCode(MXCodeSetLiangDu, MXCodeLiangDuBase + tmp, MXCodeWAITOK);
		LCDWriteString("设置成功");
	}
	else
	{
		LCDWriteString("设置不正确");	
	}
	delayms(2000);

}

void SetYingLiang()
{
	//接受输入
	uchar tmp;
	uchar pData[3];

	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);
	LCDSetXY(1, 1);
	LCDWriteString("请输入一位数0-7");
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
		LCDWriteString("设置成功");
	}
	else
	{
		LCDWriteString("设置不正确");	
	}
	delayms(2000);

}

void IRRecTimeLine(){
	uchar myMsg, i, j, power, key;
	uint start,end;
	
	//清屏
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(2);										//等待清屏完成
	LCDSetXY(1, 1);
	LCDWriteString("旧版：0 测试 A桩");
	LCDSetXY(1, 2);										  
	LCDWriteString("      1 测试 B桩");
	LCDSetXY(1, 3);
	LCDWriteString("新版：0 - 4 功率");
	delayms(1000);

	//设置功率
	power = 1;
	
	//一直显示测试状态							   
	while(1)
	{
		//查看有多少传感器在线
		j = GetOnLineIDs();
		if(j > 1){
			LCDWrite(WriteCode, LCD_CLEAR);					//清屏
			delayms(2);										//等待清屏完成
			LCDSetXY(1, 1);
			LCDWriteString("请只连一个传感器");
			if(GetKey(0) == KeyReturn){
				return;
			}
			continue;				
		}

		//清除消息队列
		ClearMsg();

		//发消息给从机，启动从机测试
		SendMsg(IRLEDTIMELINTTEST);

		//设置要测试的功率
		SendMsg(power);

		//每个接收头4条信息，3个,外加1条ID信息,共13条信息
		//收到13条信息便开始处理
		//其间要处理按键信息
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
				LCDWrite(WriteCode, LCD_CLEAR);					//清屏
				delayms(2);										//等待清屏完成
				LCDSetXY(1, 1);
				LCDWriteString("传感器故障");
				LCDSetXY(1, 2);
				LCDWriteString("传感器数目: ");
				LCDPrintfByte(j, 0);
				break;
			}
		}

		//出循环消息被清空，说明测试失败，继续测试
		if(GetMsgCount() == 0){
			continue;
		}
		
		//消息收完，清屏显示
		LCDWrite(WriteCode, LCD_CLEAR);					//清屏
		delayms(2);										//等待清屏完成

		//每次读4个数据，总共有3组数据要读
		for(j = 0; j < 3;){
			//读取保存数据,一次读1个接收头的数据，并显示
			for(i = 0; i < 4;){
				myMsg = GetMsg();
				//分别写入start 和 end
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
	
			//打印读取到的值
			LCDSetXY(1,2 + j);
			LCDPrintfWord(start, 0);
			LCDWriteString("us");
			LCDSetXY(5,2 + j);
			LCDPrintfWord(end - start, 0);
			LCDWriteString("us");
			j++;
		}
		
		//显示ID
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

		//显示当前发射管功率
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
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(2);										//等待清屏完成
	LCDSetXY(1, 1);
	LCDWriteString("消息数: ");
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

//设置每道时间显示方法
void SetMDSJXSFF()
{
	LCDWrite(WriteCode, LCD_CLEAR);					//清屏
	delayms(100);
	LCDSetXY(1, 1);
	LCDWriteString("请按数字选择方法");
	LCDSetXY(1, 2);
	LCDWriteString("当前:");

	if(Mdsjxsff == MDSJJSXS)
		LCDWriteString(" 即时显示");
	else
		LCDWriteString(" 滞后显示");

	LCDSetXY(1, 3);
	LCDWriteString("0 即时显示");
	LCDSetXY(1, 4);
	LCDWriteString("1 滞后显示");

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
	//0、定义相关变量
	uchar buf[3];
	
	//1、清屏，显示提示信息
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(3);
	LCDSetXY(1, 1);
	LCDWriteString("请输入运行代码：");	
	
	//2、接受按键
	LCDSetXY(1, 4);
	LCDPrintfByte(sizeof(FunctionTable) / sizeof(VoidFunctionPoint), 0);
	LCDWriteString(" Functions");	
//	NumberInput(buf, 2, 0, 1, 3);
	
	
	//3、校验按键的有效性
	if(NumberInput(buf, 2, 0, 1, 3) == 0)
		return;

	//4、再次检验函数是否合法
	buf[2] = CharToNum(buf);
	if(buf[2] < sizeof(FunctionTable)){	
		//4、运行相应程序
		FunctionTable[buf[2]]();
	}	
}

void MsgTest(){
	uchar LastMsg = 0, tmp;
	uint err_count = 0, count = 0;

	//显示提示信息
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

	//让从机选择工作模式
	SendMsg(MSGTEST);

	//循环比较
	/*
	这里有一个很奇怪的现象
	uchar a, b ,c;
	a = 0;
	b = 0xFF;
	if(a != b + 1) 为真

	uchar a, b, c;
	a = 0;
	b = 0xFF；
	c = b + 1;
	if(a != b) 为假
	*/
	while(1){
		//有消息就处理消息
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
		//消息少时(少于10条)就更新屏幕
		if((GetMsgCount() <= 10) && (GetMsgCount() != 0)){
			LCDSetXY(4, 2);
			LCDPrintfWord(count, 1);	
		}
		//有按键时退出
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

	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	delayms(2);
	LCDSetXY(1, 1);
	LCDWriteString("在线桩个数: ");
	onLine = GetOnLineIDs();
	LCDPrintfByte(onLine, 0);
	if(onLine > 1){
		LCDSetXY(1, 2);
		LCDWriteString("请只连一个传感器");
		delayms(1000);
		return;
	}
	
	LCDWrite(WriteCode, LCD_CLEAR);							//清屏
	delayms(2);
	for(id = 1; id <= 6; id++){
		//显示要设置的ID
		LCDSetXY(screen_addr[id - 1][0], screen_addr[id - 1][1]);
		LCDPrintfByte(id, 0);
		LCDWrite(WriteData, 'A');
		
		//倒计时5S
		i = 5;
		while(i){
			LCDSetXY(8, 4);
			LCDPrintfByte(i--, 0);
			delayms(1000);
		}
		LCDSetXY(8, 4);
		LCDPrintfByte(0, 0);

		//设置A桩ID
		{
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			SendMsg(MergeMsg(id, CHANGEID));
			delayms(200);
		}

		//读取ID
		myMsg = MergeMsg(id, COMMUNICATIONTEST);				//准备好要发的测试消息
		ClearMsg();
		SendMsg(myMsg);
		delayms(100);
		LCDSetXY(screen_addr[id - 1][0] + 1, screen_addr[id - 1][1]);

		//显示结果
		if((GetMsgCount() != 0) && (GetMsg() == myMsg)){
			LCDWrite(WriteData, '*');
			Beep(10);
		}else{
			LCDWrite(WriteData, '?');
			Beep(30);
		}

		//显示要设置的ID
		//显示要设置的ID
		LCDSetXY(screen_addr[id - 1][2], screen_addr[id - 1][3]);
		LCDPrintfByte(id, 0);
		LCDWrite(WriteData, 'B');

		//倒计时5S
		i = 5;
		while(i){
			LCDSetXY(8, 4);
			LCDPrintfByte(i--, 0);
			delayms(1000);
		}
		LCDSetXY(8, 4);
		LCDPrintfByte(0, 0);

		//设置B桩ID
		{
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			SendMsg(MergeMsg(GetReflectID(id), CHANGEID));
			delayms(200);
		}
		
		//读取ID
		myMsg = MergeMsg(GetReflectID(id), COMMUNICATIONTEST);				//准备好要发的测试消息
		ClearMsg();
		SendMsg(myMsg);
		delayms(100);
		LCDSetXY(screen_addr[id - 1][2] + 1, screen_addr[id - 1][3]);

		//显示结果
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
		//发送一个测试消息，看是否在线，对A桩
		myMsg = MergeMsg(id, COMMUNICATIONTEST);			//准备好要发的测试消息
		SendMsg(myMsg);										//给i号桩A发送一个COMMUNICATION_TEST信号
		delayms(10);										//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
		if(myMsg == GetMsg()){								//取消息
			//增加在线桩
			onLine++;
		}
		//发送一个测试消息，看是否在线，对B桩
		myMsg = MergeMsg(GetReflectID(id), COMMUNICATIONTEST);			//准备好要发的测试消息
		SendMsg(myMsg);										//给i号桩A发送一个COMMUNICATION_TEST信号
		delayms(10);										//等待消息,2ms收消息,2ms回消息,10ms足够i号桩发回的消息了
		if(myMsg == GetMsg()){								//取消息
			//增加在线桩
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
	LCDWriteString("桩自测正在进行..");
	ClearMsg();
	SendMsg(SELFTEST);

	//LED流水灯 tCount应当置为200ms的整数倍
	tCount = 30;
	LCDSetXY(1, 2);
	LCDWriteString("1 LED 灯");
	while(tCount--)
	{
		LCDSetXY(8, 2);
		LCDPrintfByte(tCount / 5, 0);
		delayms(200);
	}
	LCDSetXY(8, 2);
	LCDWriteString(" ");

	//红外接收测试
	tCount = 5 * 6;
	LCDSetXY(1, 3);
	LCDWriteString("2 红外管");
	while(tCount--)
	{
		LCDSetXY(8, 3);
		LCDPrintfByte(tCount / 5, 0);
		delayms(200);
	}
	LCDSetXY(8, 3);
	LCDWriteString(" ");

	//消息发射测试
	tCount = 5 * 2;
	LCDSetXY(1, 4);
	LCDWriteString("3 送消息");
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
		LCDWriteString("通过");	
	}
	else
	{
		LCDSetXY(7, 4);
		LCDWriteString("异常");	
	}

	//20140222删除下面这句,改为自动退回到菜单
	while(GetKey(0) == NULLKey);
	//delayms(1200);
}

void ZhuJiTest(){
	uchar i, myMsg;
	//清屏
	LCDWrite(WriteCode, LCD_CLEAR);
	delayms(10);

	//将语音全部读一次
	LCDSetXY(1, 1);
	LCDWriteString("声音检测：");
	for(i = 0; i <= 17; i++){
		LCDSetXY(6, 1);
		LCDPrintfByte(i, 1);
		SendThreeLines(i, WaitIfBusy);
		delayms(1000);
	}

	//检测蜂鸣器
	LCDSetXY(1, 2);
	LCDWriteString("蜂鸣检测: 请听！");
	Beep(20);

	//检测发消息和收消息
	LCDSetXY(1, 3);
	LCDWriteString("通讯检测请连从机");
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
		LCDWriteString("：通过  ");
	}else{
		LCDSetXY(5, 3);
		LCDWriteString("：未通过");
	}

	//检测按键
	LCDSetXY(1, 4);
	LCDWriteString("按键检测：");
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
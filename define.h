#include "stc12c5a60s2.h"

#ifndef MY_DEFINE

#define	MY_DEFINE

#define Val_TH0_Half			98						//38K计数器初始值
#define	N_38K					(10 * 2)				//每次发射或反射多少个脉冲
#define	N_38K_WAIT 				(3 * 2)					//反射桩在信号结束后等待多少个脉冲
//#define N_38K_SAMPLING	((16 + N_38K_WAIT + N_38K) * 2)	//取样多少个脉冲,第二版本是这个,发送完后立即取样
#define N_38K_SAMPLING			((16 +  N_38K) * 2)		//取样多少个脉冲,第三版本改的,发送完之后等待一个N_38K_WAIT再取样
														
#define IR_LED_OFF				1  						//关闭IR_LED的赋值
#define IR_LED_ON				0						//打开IR_LED的赋值

#define LEDON					0
#define LEDOFF					1

#define IR_REC_GET_SIGNAL		0						//红外有信号时电平值
#define IR_REC_NO_SIGNAL		1						//红外无信号时电平值

#define TEST_TIMES_100MS		10						//让桩工作10分之1秒，即100MS的时间的循环次数
#define	OPEN					1						//桩光线没有被遮断时函数返回值
#define	CLOSE					0						//桩光线被遮断时函数返回值
#define	INTERRUPT_BY_CODE		0xFF					//当StartWork被消息中断时的返回值

#define FreqOfUpdateDisplay		9						//FreqOfUpdateDisplay分之一秒更新一次显示

#define	StartBeep				1
#define StopBeep				0

#define NULLPoint				0xFF

#define	INIT_T0_FOR_CLOCK		(int)(((int)65536 - (int)10022))					//主机用到的0.01秒溢出时间的初值
#define	CLOCK_TH0			 	(uchar)((INIT_T0_FOR_CLOCK & (int)0xFF00) >> 8)		//对应的TH0的值
#define	CLOCK_TL0				(uchar)(INIT_T0_FOR_CLOCK & (int)0x00FF)			//对应的TL0的值

#define TimeOverMin				10
#define TimeOverMax				50
#define TimeOverDefault			50
#define WorkTimeDefault			(TimeOverDefault - SendMsgWaitTimeDefault / 10)
#define WorkTimeMin				(TimeOverMin - SendMsgWaitTimeDefault / 10)
#define WorkTimeMax				(TimeOverMax - SendMsgWaitTimeDefault / 10)
#define SendMsgWaitTimeDefault	70
#define ErrorTime				((uint)TimeOverMax *(uint)10 + (uint)50)
#define Every5MMinTime			90

//从机 用于存储ID在FLASH中
#define ID_Addr					0	

//主机 最多存多少数据，100个
#define NoOfMaxSaved			100
//主机 从机一次工作多长时间						1字节
#define TimeOverAddr			0				
//主机 主机测试次数设定							2字节
#define NoUsedAddr				1
//主机 测试成功次数设定							2字节
#define NoSuccessAddr			3
//主机 最好成绩									2字节
#define FastestRecAddr			5
//主机 到点提示时是用音箱还是蜂鸣器				1字节
#define BeepTypeAddr			7
//主机 不能复位的仪器使用次数					2字节
#define CanotResetNoAddr		8
//主机 主机工作模式 无限还是计次				1字节
#define WorkModeAddr			10
//主机	剩余次数								2字节
#define RemainTimesAddr			11
//主机 密码 共6字节，明文						6字节
#define PasswordAddr			13
//主机 用于测试电池工作时间的记录位			  共4字节
#define Min1Addr				19
#define Min2Addr				21
//主机 音量大小									1字节
#define YingLiangAddr			23
//主机 每道时间的显示方式，滞后还是即时			1字节
#define MdsjxsffAddr			24	
//是否第一次上电								1字节 'N' 表示不是第一次上电
#define FirstPowerOn			25
//主机 记录成绩的开始地址					  共100字节
#define RecordStartAddr			30

//密码长度
#define PasswordLen				6
//错误密码的返回值
#define WrongPassword			0
//正确密码的返回值
#define RightPassword			1
//没有设置密码的返回值
#define NULLPassword			2
//不可擦除的次数显示的最大值，到达这个值复位
#define MaxCanotResetNo			(50000)
//无限模式设置值
#define WORKMODE_NOLIMIT		0
//计次模式设置值
#define WORKMODE_COUNTTIMES		1
//音量中要用到的常数
#define YingLiangBase			0xE0
//最小音量
#define YingLiangMin			0
//最大音量
#define YingLiangMax			7

#define MDSJJSXS				1						//每道时间即时显示
#define MDSJZHXS				0						//每道时间滞后显示
#define BeepBuzzer				1						//蜂鸣器输出声音
#define BeepDAC					0						//音箱输出声音
#define BeepTimeStart			50						//准备完成时蜂鸣时长
#define BeepTimeTimeOver		30						//超时技术犯规蜂鸣时长
#define BeepTimeError			5						//错误时时长
#define BeepTimePointReached	10						//到点蜂鸣时长

//如果想只用一台主机调试机器,请打开这个选项
//#define SINGLEDEBUG

sbit	LED1 = P3^3;
sbit	LED2 = P3^4;
sbit	LED3 = P3^5;
sbit 	Speak = P3^4;

#define LCDBKLightON	0
#define LCDBKLightOFF	1

#define Min1ACount		50								//桩用到的,1A测试到连续多少上断才认为脚已经放稳了
#define WorkStatus 		LED1
#endif
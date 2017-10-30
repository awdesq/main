#include "STC12C5A60S2.h"
#include "KeyBoard.h"
#include "define.h"

#ifndef MYMENU
#define MYMENU

uint CharToNum(uchar* pData);
void Menu(void);
uchar NumberInput(uchar *pBuffer, uchar N, char IsPassword, uchar x, uchar y);
void SetIDs();
void SetTimeOver();
uchar Test();
uchar InfraredTest();
uchar CommunicationTest();
void FactorySet();
void DisplaySumary();
void ViewRecord();
void IRPositionTest();
void SetBeepType();
void SetWorkMode();
void SetRemainTimes();
void SetLiangDu();
void ViewMsg();
void ViewMsg2();
void SetYingLiang();
void SetMDSJXSFF();
void IRRecTimeLine();
void RunFunction();
void MsgTest();
void QuickSetIDs();
uchar GetOnLineIDs();
void ZhuangSelfTest();
void ZhuJiTest();
#endif
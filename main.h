#ifndef MAINDOTH
#define MAINDOTH

extern uchar WorkTime, RecordPoint, BeepType, WorkMode, Mdsjxsff, IsRoot;	//�����ļ�Ҫ�õ�
extern uint NoUsed, NoSuccess, FastestRec, CanotResetNo, RemainTimes; 		//�����ļ�Ҫ�õ�

void Beep(uchar Ten_ms);
void RestartSub();
void ErrorBeep();
void InitForBeep();
void InitForStartTest();
void InitGlobalVal();
void InitT0_For_CLOCK(void);
void printftime(uint time, uchar x, uchar y);
void printftime_IfNoMsg(uint time, uchar x, uchar y);
uint ReadWordEEPROM(uchar Addr);
uchar WriteWordEEPROM(uchar Addr, uint Data);
uchar ReadByteEEPROM(uchar Addr);
uchar WriteByteEEPROM(uchar Addr, uchar Data);
uchar GetANewAddrToSaveRecord();
uchar GetCurrentRecordAddr();
uchar GetPrevRecordAddr(uchar CurAddr);
void SpeakTime(uint time);
void SetPassword();
uchar CheckPassword();
void DisplayContentRealTime();
void TestBatteryTime();
void DisplayRemainTimesOverInfo();
void FirstPowerOntoResetValues();

#define SoundFunction(BeepTime, DACCode) {if(BeepType==BeepBuzzer){Beep(BeepTime);}else{SendThreeLines(DACCode,NotWait);}}

#endif
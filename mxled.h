#include "stc12c5a60s2.h"

#ifndef MXLED
#define MXLED

#define MXCodeDisplay								0xB4
#define MXCodeDisplayTimeAndTemp				 	0x01
#define MXCodeDisplaySpecialDay					 	0x02
#define MXCodeDisplayContent1					 	0x04
#define MXCodeDisplayContent2					 	0x08
#define MXCodeDisplayContent3					 	0x10
#define MXCodeSetLiangDu							0xB6
#define MXCodeLiangDuBase							0xA0  	
#define MXCodeLiangDuMin							0x00
#define	MXCodeLiangDuMax							0x0F
#define MXCodeContent1Move							0xB1
#define MXCodeContent2Move							0xB3
#define MXCodeContent3Move							0xB8
#define MXCodeLeftShift								0x00
#define MXCodeRealtimeLeftShift						0x05
#define MXCodeRealtimeStatic						0x07
#define MXCodeNULLContent							0xFF
#define MXCodeWAITOK								0x01
#define MXCodeNOTWAITOK								0x00
#define MXCodeRealtimeDisplay						0xBA

//void InitUartForMXLED(void);
void SendMXString(uchar *myStr, uchar where, uchar IsWaitOK);
void SendMXTime(uint t);
void SendMXCode(uchar myMXCode, uchar myMXContent, uchar IsWaitOK);
void WaitReturnOK();
#endif
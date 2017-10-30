#include "stc12c5a60s2.h"

#ifndef SOFTRESET
#define SOFTRESET

#define		SelfDefineISPDownloadCommand		0x22
#define 	UARTSBLens							45
#define		UARTRBLens							10

extern bit MXIsOK;
void SoftResetToISPMonitor();
void InitUART();
void SendOneByte(unsigned char c);
void TrigeUARTSend();
void WriteUARTSB(unsigned char c);
#endif


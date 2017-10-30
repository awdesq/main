#include "stc12c5a60s2.h"
#include "UseFlash.h"
#include <intrins.h>
#ifdef USEFLASH

void IAP_Idle(void)
{
	Set0(IAP_CONTR, 7);				/* disable Read/Program/Erase Sector */
	IAP_CMD = IAP_IDLE;
}

void IAP_Erase_Sector(uint addr)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				/* Enable IAP*/
	IAP_CMD = IAP_SECTOR_ERASE;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
}

void IAP_Program_Byte(uint addr, uchar dataByte)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				/* Enable IAP*/
	IAP_CMD = IAP_PROGRAM;
	IAP_ADDRH = addr >> 8;
	IAP_ADDRL = addr;
	IAP_DATA = dataByte;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
}

uchar IAP_Read_Byte(uint addr)
{
	IAP_CONTR = WAITTIME12M;
	Set1(IAP_CONTR, 7);				// Enable IAP
	IAP_CMD = IAP_READ;
	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
	_nop_();
	IAP_Idle();
	return IAP_DATA;		
}

#endif
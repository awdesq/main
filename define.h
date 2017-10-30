#include "stc12c5a60s2.h"

#ifndef MY_DEFINE

#define	MY_DEFINE

#define Val_TH0_Half			98						//38K��������ʼֵ
#define	N_38K					(10 * 2)				//ÿ�η��������ٸ�����
#define	N_38K_WAIT 				(3 * 2)					//����׮���źŽ�����ȴ����ٸ�����
//#define N_38K_SAMPLING	((16 + N_38K_WAIT + N_38K) * 2)	//ȡ�����ٸ�����,�ڶ��汾�����,�����������ȡ��
#define N_38K_SAMPLING			((16 +  N_38K) * 2)		//ȡ�����ٸ�����,�����汾�ĵ�,������֮��ȴ�һ��N_38K_WAIT��ȡ��
														
#define IR_LED_OFF				1  						//�ر�IR_LED�ĸ�ֵ
#define IR_LED_ON				0						//��IR_LED�ĸ�ֵ

#define LEDON					0
#define LEDOFF					1

#define IR_REC_GET_SIGNAL		0						//�������ź�ʱ��ƽֵ
#define IR_REC_NO_SIGNAL		1						//�������ź�ʱ��ƽֵ

#define TEST_TIMES_100MS		10						//��׮����10��֮1�룬��100MS��ʱ���ѭ������
#define	OPEN					1						//׮����û�б��ڶ�ʱ��������ֵ
#define	CLOSE					0						//׮���߱��ڶ�ʱ��������ֵ
#define	INTERRUPT_BY_CODE		0xFF					//��StartWork����Ϣ�ж�ʱ�ķ���ֵ

#define FreqOfUpdateDisplay		9						//FreqOfUpdateDisplay��֮һ�����һ����ʾ

#define	StartBeep				1
#define StopBeep				0

#define NULLPoint				0xFF

#define	INIT_T0_FOR_CLOCK		(int)(((int)65536 - (int)10022))					//�����õ���0.01�����ʱ��ĳ�ֵ
#define	CLOCK_TH0			 	(uchar)((INIT_T0_FOR_CLOCK & (int)0xFF00) >> 8)		//��Ӧ��TH0��ֵ
#define	CLOCK_TL0				(uchar)(INIT_T0_FOR_CLOCK & (int)0x00FF)			//��Ӧ��TL0��ֵ

#define TimeOverMin				10
#define TimeOverMax				50
#define TimeOverDefault			50
#define WorkTimeDefault			(TimeOverDefault - SendMsgWaitTimeDefault / 10)
#define WorkTimeMin				(TimeOverMin - SendMsgWaitTimeDefault / 10)
#define WorkTimeMax				(TimeOverMax - SendMsgWaitTimeDefault / 10)
#define SendMsgWaitTimeDefault	70
#define ErrorTime				((uint)TimeOverMax *(uint)10 + (uint)50)
#define Every5MMinTime			90

//�ӻ� ���ڴ洢ID��FLASH��
#define ID_Addr					0	

//���� ����������ݣ�100��
#define NoOfMaxSaved			100
//���� �ӻ�һ�ι����೤ʱ��						1�ֽ�
#define TimeOverAddr			0				
//���� �������Դ����趨							2�ֽ�
#define NoUsedAddr				1
//���� ���Գɹ������趨							2�ֽ�
#define NoSuccessAddr			3
//���� ��óɼ�									2�ֽ�
#define FastestRecAddr			5
//���� ������ʾʱ�������仹�Ƿ�����				1�ֽ�
#define BeepTypeAddr			7
//���� ���ܸ�λ������ʹ�ô���					2�ֽ�
#define CanotResetNoAddr		8
//���� ��������ģʽ ���޻��Ǽƴ�				1�ֽ�
#define WorkModeAddr			10
//����	ʣ�����								2�ֽ�
#define RemainTimesAddr			11
//���� ���� ��6�ֽڣ�����						6�ֽ�
#define PasswordAddr			13
//���� ���ڲ��Ե�ع���ʱ��ļ�¼λ			  ��4�ֽ�
#define Min1Addr				19
#define Min2Addr				21
//���� ������С									1�ֽ�
#define YingLiangAddr			23
//���� ÿ��ʱ�����ʾ��ʽ���ͺ��Ǽ�ʱ			1�ֽ�
#define MdsjxsffAddr			24	
//�Ƿ��һ���ϵ�								1�ֽ� 'N' ��ʾ���ǵ�һ���ϵ�
#define FirstPowerOn			25
//���� ��¼�ɼ��Ŀ�ʼ��ַ					  ��100�ֽ�
#define RecordStartAddr			30

//���볤��
#define PasswordLen				6
//��������ķ���ֵ
#define WrongPassword			0
//��ȷ����ķ���ֵ
#define RightPassword			1
//û����������ķ���ֵ
#define NULLPassword			2
//���ɲ����Ĵ�����ʾ�����ֵ���������ֵ��λ
#define MaxCanotResetNo			(50000)
//����ģʽ����ֵ
#define WORKMODE_NOLIMIT		0
//�ƴ�ģʽ����ֵ
#define WORKMODE_COUNTTIMES		1
//������Ҫ�õ��ĳ���
#define YingLiangBase			0xE0
//��С����
#define YingLiangMin			0
//�������
#define YingLiangMax			7

#define MDSJJSXS				1						//ÿ��ʱ�伴ʱ��ʾ
#define MDSJZHXS				0						//ÿ��ʱ���ͺ���ʾ
#define BeepBuzzer				1						//�������������
#define BeepDAC					0						//�����������
#define BeepTimeStart			50						//׼�����ʱ����ʱ��
#define BeepTimeTimeOver		30						//��ʱ�����������ʱ��
#define BeepTimeError			5						//����ʱʱ��
#define BeepTimePointReached	10						//�������ʱ��

//�����ֻ��һ̨�������Ի���,������ѡ��
//#define SINGLEDEBUG

sbit	LED1 = P3^3;
sbit	LED2 = P3^4;
sbit	LED3 = P3^5;
sbit 	Speak = P3^4;

#define LCDBKLightON	0
#define LCDBKLightOFF	1

#define Min1ACount		50								//׮�õ���,1A���Ե����������϶ϲ���Ϊ���Ѿ�������
#define WorkStatus 		LED1
#endif
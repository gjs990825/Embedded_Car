#ifndef __SYN7318_H
#define __SYN7318_H
#include "sys.h"

#define USART6_RX_LEN 100
#define USART6_TX_LEN 100

extern uint8_t USART6_RX_BUF[USART6_RX_LEN];
extern uint8_t USART6_TX_BUF[USART6_TX_LEN];
extern uint16_t USART6_RX_STA;

#define USART6_RxFlag ((USART6_RX_STA & 0xF000) == 0xF000)
#define USART6_RxLenth (USART6_RX_STA & 0x0FFF)

enum
{
    VoiceCmd_NotUsed = 0x01,         // δʹ��
    VoiceCmd_TurnRignt = 0x02,       // ����ת��
    VoiceCmd_NOTurnRight = 0x03,     // ��ֹ��ת
    VoiceCmd_DrvingToLeft = 0x04,    // �����ʻ
    VoiceCmd_NODrivingToLeft = 0x05, // ���б���
    VoiceCmd_TurnAround = 0x06       // ԭ�ص�ͷ
};

void USART6_Init(uint32_t baudrate);
void SYN7318_Init(void);
void USART6_SendChar(uint8_t ch);
void USART6_SendString(uint8_t *str, uint16_t len);
bool USART6_GetCmd(uint8_t *buf);
void USART6_print(char *str, ...);
bool SYN7318_Rst(void);
void SYN_TTS(uint8_t *str);
void SYN7318_Test(void);
bool VoiceComand_Process(uint8_t *cmd);
bool Start_VoiceCommandRecognition(uint8_t retryTimes);
	
#endif

#include "voice.h"
#include "delay.h"
#include <string.h>
#include "cba.h"
#include "infrared.h"
#include "data_base.h"
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"

#define _ENABLE_USART6_INFO_OUTPUT_ 1

#define SYN7318_RST_H GPIO_SetBits(GPIOB, GPIO_Pin_9)
#define SYN7318_RST_L GPIO_ResetBits(GPIOB, GPIO_Pin_9)

uint8_t USART6_RX_BUF[USART6_RX_LEN] = {0};
uint8_t USART6_TX_BUF[USART6_TX_LEN] = {0};
uint16_t USART6_RX_STA = 0;

// 唤醒
unsigned char Wake_Up[] = {0xfd, 0x00, 0x02, 0x51, 0x1F};
unsigned char Stop_Wake_Up[] = {0xFD, 0x00, 0x01, 0x52};
// 自动语音识别
unsigned char Start_ASR[] = {0xFD, 0x00, 0x02, 0x10, 0x05}; // 0x03
unsigned char Stop_ASR[] = {0xFD, 0x00, 0x01, 0x11};

// unsigned char Play_MP3[] = {0xFD, 0x00, 0x1E, 0x01, 0x01, 0xC6, 0xF4, 0xB6, 0xAF, 0xD3, 0xEF, 0xD2, 0xF4,
//                             0xBF, 0xD8, 0xD6, 0xC6, 0xBC, 0xDD, 0xCA, 0xBB, 0xA3, 0xAC, 0xC7, 0xEB,
//                             0xB7, 0xA2, 0xB3, 0xF6, 0xD6, 0xB8, 0xC1, 0xEE};

void USART6_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_TypeDefStructure;
    USART_InitTypeDef USART_TypeDefStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

    //PC6-Tx
    GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_AF;   //复用功能
    GPIO_TypeDefStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
    GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOC, &GPIO_TypeDefStructure);

    USART_TypeDefStructure.USART_BaudRate = baudrate;                                  //波特率
    USART_TypeDefStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件控制流
    USART_TypeDefStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 //接收与发送模式
    USART_TypeDefStructure.USART_Parity = USART_Parity_No;                             //无校验位
    USART_TypeDefStructure.USART_StopBits = USART_StopBits_1;                          //停止位1
    USART_TypeDefStructure.USART_WordLength = USART_WordLength_8b;                     //数据位8位
    USART_Init(USART6, &USART_TypeDefStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART6, ENABLE);
    USART_ClearFlag(USART6, USART_FLAG_TC);   //清除发送完成标志位
    USART_ClearFlag(USART6, USART_FLAG_RXNE); //清除接收完成标志位
}

void SYN7318_Init(void)
{
    USART6_Init(115200);

    GPIO_InitTypeDef GPIO_TypeDefStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //PB9 -- SYN7318_RESET
    GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_OUT;  //复用功能
    GPIO_TypeDefStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
    GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_TypeDefStructure);

    GPIO_SetBits(GPIOB, GPIO_Pin_9); //默认为高电平
}

void USART6_SendChar(uint8_t ch)
{
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
        ;
    USART_SendData(USART6, ch);
    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
        ;
}

void USART6_SendString(uint8_t *str, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        USART6_SendChar(str[i]);
    }
}

// 获取指令
bool USART6_GetCmd(uint8_t *buf)
{
    if (USART6_RxFlag)
    {
        memcpy(buf, USART6_RX_BUF, USART6_RxLenth);
        USART6_RX_STA = 0;
        return true;
    }
    else
    {
        buf[0] = 0;
        return false;
    }
}

void USART6_print(char *str, ...)
{
    uint16_t len;
    va_list ap;
    va_start(ap, str);
    vsprintf((char *)USART6_TX_BUF, str, ap);
    va_end(ap);
    len = strlen((char *)USART6_TX_BUF);

    for (uint16_t i = 0; i < len; i++)
    {
        USART6_SendChar(USART6_TX_BUF[i]);
    }
}

// uint16_t USART6_RX_STA = 0; XXXX 0000 0000 0000
void USART6_IRQHandler(void)
{
    static uint16_t cmdLenth = 0;
    if (USART_GetITStatus(USART6, USART_IT_RXNE) == SET)
    {
        uint8_t ch = USART_ReceiveData(USART6);

        if (ch == 0xFC) // 遇到FC重新接收
        {
            USART6_RX_STA = 0x8000;
            cmdLenth = 0;
        }
        else if (USART6_RX_STA == 0x8000) // 第一位长度
        {
            cmdLenth = ch;
            USART6_RX_STA = 0xC000;
        }
        else if (USART6_RX_STA == 0xC000) // 第二位长度
        {
            cmdLenth <<= 8;
            cmdLenth |= ch;
            USART6_RX_STA = 0xE000;
        }
        else if ((USART6_RX_STA & 0xF000) == 0xE000)
        {
            if (USART6_RxLenth < cmdLenth) // 小于指令长度
            {
                USART6_RX_BUF[USART6_RxLenth] = ch;
                USART6_RX_STA++;
                if (USART6_RxLenth >= cmdLenth)
                {
                    USART6_RX_STA |= 0xF000;

#if _ENABLE_USART6_INFO_OUTPUT_

                    print_info("SYN: ");
                    for (uint8_t i = 0; i < cmdLenth; i++)
                    {
                        print_info("%02X ", USART6_RX_BUF[i]);
                    }
                    print_info("\r\n");

#endif // _ENABLE_USART6_INFO_OUTPUT_
                }
            }
        }
    }
    USART_ClearITPendingBit(USART6, USART_IT_RXNE);
}

//语音模块复位
bool SYN7318_Rst(void)
{
    uint8_t buf[4];
    SYN7318_RST_H;
    delay_ms(10);
    SYN7318_RST_L;
    delay_ms(100);
    SYN7318_RST_H;

    WaitForFlagInMs(USART6_GetCmd(buf), true, 4000);

    return (buf[0] == 0x4A) ? true : false;
}

void SYN_TTS(uint8_t *str)
{
    uint8_t Length;
    uint8_t Frame[5]; //保存发送命令的数组
    uint8_t buf[4] = {0};

    Length = strlen((char *)str); // GAO edited 2019年3月7日
    Frame[0] = 0xFD;              //帧头
    Frame[1] = 0x00;
    Frame[2] = Length + 2;
    Frame[3] = 0x01; //语音合成播放命令
    Frame[4] = 0x00; //播放编码格式为“GB2312”

    USART6_SendString(Frame, 5);
    USART6_SendString(str, Length);

    WaitForFlagInMs(USART6_GetCmd(buf), true, 500);
    if (buf[0] != 0x41)
        return;

    WaitForFlagInMs(USART6_GetCmd(buf), true, Length * 350); //每个汉字为300ms 左右
    if (buf[0] != 0x4F)
        return;
}

// 查询状态
bool Status_Query(void)
{
    uint8_t Frame[4]; //保存发送命令的数组
    uint8_t buf[4] = {0};
    Frame[0] = 0xFD; //帧头
    Frame[1] = 0x00;
    Frame[2] = 0x01;
    Frame[3] = 0x21; //状态查询命令

    USART6_SendString(Frame, 4);
    WaitForFlagInMs(USART6_GetCmd(buf), true, 500);
    if (buf[0] != 0x41)
        return false;
    WaitForFlagInMs(USART6_GetCmd(buf), true, 500);
    return (buf[0] == 0x4F) ? true : false;
}

// 开启语音测试
void SYN7318_Test(void)
{
    uint8_t buf[6] = {0};

    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;

    SYN_TTS("请发唤醒词");
    LED1 = 1;
    delay_ms(300);
    if (Status_Query()) //模块空闲即开启唤醒
    {
        LED2 = 1;
        delay_ms(1);

        USART6_SendString(Wake_Up, 5); //发送唤醒指令
        WaitForFlagInMs(USART6_GetCmd(buf), true, 500);
        if (buf[0] == 0x41) // 唤醒开启成功
        {
            LED3 = 1;
            delay_ms(200);                  // 等待模块响应
            for (uint8_t i = 0; i < 4; i++) // 三次语音指令唤醒失败就放弃任务
            {
                Send_ZigBeeData(ZigBee_VoiceDriveAssistant); // 语音合成驾驶助手
                WaitForFlagInMs(USART6_GetCmd(buf), true, 3000);
                if (buf[0] == 0x21) // 唤醒成功
                {
                    LED4 = 1;
                    // USART6_SendString(Play_MP3, 33); //播放“我在这”
                    SYN_TTS("唤醒成功");
                    delay_ms(100);

                    Start_VoiceCommandRecognition(3);

                    break;
                }
            }
        }
        USART6_SendString(Stop_Wake_Up, 4); // 停止唤醒
        WaitForFlagInMs(USART6_GetCmd(buf), true, 5000);
    }
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
    LED4 = 0;
}

// 开始识别语音指令并判断
bool Start_VoiceCommandRecognition(uint8_t retryTimes)
{
    uint8_t buf[8] = {0};

    for (uint8_t i = 0; i < retryTimes; i++) // 三次识别失败退出
    {
        USART6_SendString(Start_ASR, 5);                 // 开始识别
        WaitForFlagInMs(USART6_GetCmd(buf), true, 500);  // 等待接收成功
        Send_ZigBeeData(ZigBee_VoiceRandom);             // 获取随机语音指令
        WaitForFlagInMs(USART6_GetCmd(buf), true, 4000); // 等待语音模块返回识别信息
        if ((buf[0] <= 0x06) && (buf[0] != 0x00))        // 返回了正确的命令字
        {
            if (VoiceComand_Process(buf) == false)
            {
                return true;
            }
        }
        USART6_SendString(Stop_ASR, 4);                 // 停止识别
        WaitForFlagInMs(USART6_GetCmd(buf), true, 500); // 等待接收成功
        delay_ms(100);
    }
    return false;
}

// 语音指令处理 返回是否需要再次识别
bool VoiceComand_Process(uint8_t *cmd)
{
    switch (cmd[0])
    {
    case 0x01: // 识别成功（带命令ID号）
    {
        switch (cmd[5])
        {
        case VoiceCmd_TurnRignt:
            SYN_TTS("向右转弯");
            break;
        case VoiceCmd_NOTurnRight:
            SYN_TTS("禁止右转");
            break;

        case VoiceCmd_DrvingToLeft:
            SYN_TTS("左侧行驶");
            break;

        case VoiceCmd_NODrivingToLeft:
            SYN_TTS("左行被禁");
            break;

        case VoiceCmd_TurnAround:
            SYN_TTS("原地掉头");
            break;
        default:
            break;
        }
        return false; // 识别完成，不需要再次识别
    }
	
    case 0x02: //识别成功（无命令ID号）
    {
        SYN_TTS("没有相应的ID");
        break;
    }
    case 0x03: //用户静音超时
    {
        SYN_TTS("静音超时，已进入休眠状态");
        break;
    }
    case 0x04: // 用户语音超时
    {
        SYN_TTS("语音超时");
        break;
    }
    case 0x05: // 识别据识
    {
        SYN_TTS("识别据识");
        break;
    }
    case 0x06: // 识别内部错误
    {
        SYN_TTS("识别内部错误");
        break;
    }
    case 0x07: // 识别拒识
    {
        SYN_TTS("识别拒识");
        break;
    }
    default:
    {
        SYN_TTS("错误");
        break;
    }
    }
    return true; // 识别未完成，需要再次识别
}

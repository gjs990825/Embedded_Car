#include "independent_task.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "bh1750.h"
#include "voice.h"
#include "roadway_check.h"
#include "tba.h"
#include "uart_a72.h"
#include "Rc522.h"
#include "malloc.h"
#include "debug.h"
#include "movement.h"
#include "route.h"
#include "my_lib.h"
#include "Timer.h"
#include "data_from_host.h"

// RFID相关 ↓

// 寻到白卡
uint8_t FOUND_RFID_CARD = false;
// 白卡路段
uint8_t RFID_RoadSection = false;
// 当前卡信息指针
RFID_Info_t *CurrentRFIDCard = NULL;
// 遇到白卡时的状态数据
struct StatusBeforeFoundRFID_Struct
{
    uint8_t stopFlag;
    uint8_t trackMode;
    int8_t currentSpeed;
    Moving_ByEncoder_t movingByencoder;
    uint16_t remainEncoderValue;
} StatusBeforeFoundRFID;

extern uint16_t Mp_Value;
// 保存遇到白卡时候的状态
void Save_StatusBeforeFoundRFID(void)
{
    extern uint16_t Mp_Value;
    extern int LSpeed, RSpeed;

    StatusBeforeFoundRFID.movingByencoder = Moving_ByEncoder;
    StatusBeforeFoundRFID.stopFlag = Stop_Flag;
    StatusBeforeFoundRFID.trackMode = Track_Mode;
    StatusBeforeFoundRFID.currentSpeed = (LSpeed + RSpeed) / 2;
    StatusBeforeFoundRFID.remainEncoderValue = temp_MP - Mp_Value;
}

// 恢复状态 encoderChangeValue: 前后设定码盘差值
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue)
{
    uint16_t Roadway_mp_Get(void);

    Roadway_mp_syn(); // 同步码盘
    Mp_Value = Roadway_mp_Get();

    Moving_ByEncoder = StatusBeforeFoundRFID.movingByencoder;
    Stop_Flag = StatusBeforeFoundRFID.stopFlag;
    Track_Mode = StatusBeforeFoundRFID.trackMode;
    // 循迹信息已清空，需要重新计算并减去执行中的行进值
    temp_MP = Mp_Value + StatusBeforeFoundRFID.remainEncoderValue;
    int8_t currentSpeed = StatusBeforeFoundRFID.currentSpeed;
    Update_MotorSpeed(currentSpeed, currentSpeed);
    Submit_SpeedChanges();
}

// 设定当前卡信息
void Set_CurrentCardInfo(RFID_Info_t *RFIDx)
{
    CurrentRFIDCard = RFIDx;
}

// 读卡
ErrorStatus Read_RFID(RFID_Info_t *RFIDx)
{
    ErrorStatus status = PICC_ReadBlock(RFIDx->dataBlockLocation, RFIDx->authMode, RFIDx->key, RFIDx->data);

    if (status == SUCCESS)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%02X ", RFIDx->data[i]);
            delay_ms(5);
        }
        print_info("\r\n");
    }
    else
    {
        print_info("READ CARD FAIL\r\n");
    }
    return status;
}

// RFID读卡任务，检测到白卡时执行
void RFID_Task(void)
{
    uint8_t i;

    // 当前卡信息未设定，跳出
    if (CurrentRFIDCard == NULL)
        return;

    // 记录位置信息
    CurrentRFIDCard->coordinate = NextStatus;
    print_info("Card At:(%d,%d)\r\n", CurrentRFIDCard->coordinate.x, CurrentRFIDCard->coordinate.y);

    MOVE(8);
    for (i = 0; i < 9; i++) // 读卡范围约 11.5-16.5，间隔一公分读取()
    {
        MOVE(1);
        if (Read_RFID(CurrentRFIDCard) == SUCCESS)
            break; // 读取成功，跳出
        delay_ms(500);
    }

    RFID_RoadSection = false; // 结束寻卡
    FOUND_RFID_CARD = false;  // 清空标志位
    TIM_Cmd(TIM5, DISABLE);   // 停止定时器
    MOVE(-(8 + i));           // 返回读卡前位置

    // 十字路口需要多退后一点，因为响应时间变长会多走一点
    if ((NextStatus.x % 2) && (NextStatus.y % 2) != 0)
    {
        MOVE(-2);
    }
}

// 配置调试卡使用的信息
uint8_t _testRFIDDataBlock = 5;
uint8_t _testRFIDKey[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t _testRFIDAuthMode = PICC_AUTHENT1A;

// RFID测试任务开始
void Task_RFIDTestStart(void)
{
    RFID_Info_t *rfid = mymalloc(SRAMIN, sizeof(RFID_Info_t));

    // 清空
    memset(rfid, 0, sizeof(RFID_Info_t));

    // 写入调试信息
    memcpy(rfid->key, _testRFIDKey, 6);
    rfid->authMode = _testRFIDAuthMode;
    rfid->dataBlockLocation = _testRFIDDataBlock;

    Set_CurrentCardInfo(rfid);
    RFID_RoadSection = true;
}

// RFID测试任务结束
void Task_RFIDTestEnd(void)
{
    myfree(SRAMIN, CurrentRFIDCard);
    CurrentRFIDCard = NULL;
    RFID_RoadSection = false;
}

// 使用设定key读某个扇区
void Test_RFID(uint8_t block)
{
    uint8_t buf[17];

    if (PICC_ReadBlock(block, _testRFIDAuthMode, _testRFIDKey, buf) == SUCCESS)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%02X ", buf[i]);
            delay_ms(5);
        }
        print_info("\r\n");
    }
    else
    {
        print_info("ERROR\r\n");
    }
}

// RFID相关 ↑

// 交通灯识别
void TrafficLight_Task(void)
{
    Send_ZigBeeDataNTimes(ZigBee_TrafficLightStartRecognition, 2, 200); // 开始识别交通灯
    delay_ms(700);
    Request_ToHost(RequestCmd_TrafficLight);
    WaitForFlagInMs(GetCmdFlag(FromHost_TrafficLight), SET, 13 * 1000); // 等待识别完成
}

// TFT图形图像识别
void TFT_Task(void)
{
    Request_ToHost(RequestCmd_TFTRecognition);                            // 请求识别TFT内容
    WaitForFlagInMs(GetCmdFlag(FromHost_TFTRecognition), SET, 37 * 1000); // 等待识别完成
}

// TFT显示部分
// TFT显示HEX
void TFT_Hex(uint8_t dat[3])
{
    uint8_t buf[8] = {0x55, 0x0b, 0x10, 0x02, 0x00, 0x00, 0x12, 0xbb};
    buf[2] = 0x40;
    memcpy(&buf[3], dat, 3);
    Send_ZigBeeData(buf);
}

// 立体显示部分

// 立体显示 显示车牌
void RotationLED_Plate(uint8_t plate[6], uint8_t coord[2])
{
    // Infrared_PlateData1[]
    memcpy(&Infrared_PlateData1[2], plate, 4);
    memcpy(&Infrared_PlateData2[2], &plate[4], 2);
    memcpy(&Infrared_PlateData2[4], coord, 2);
    Infrared_Send_A(Infrared_PlateData1);
    delay_ms(600);
    Infrared_Send_A(Infrared_PlateData2);
}

// 立体显示 显示距离
void RotationLED_Distance(uint16_t dis)
{
    uint8_t buf[6] = {0x00};

    buf[0] = 0xFF;
    buf[1] = 0x11;
    buf[2] = HEX2BCD((dis % 100) / 10);
    buf[2] = HEX2BCD(dis % 10);
    Infrared_Send_A(buf);
}

// 二维码识别
void QRCode_Task(uint8_t QRrequest)
{
    GetCmdFlag(FromHost_QRCodeRecognition) = RESET;
    Request_ToHost(QRrequest);
    WaitForFlagInMs(GetCmdFlag(FromHost_QRCodeRecognition), SET, 5 * 1000);
}

// 起始任务
void Start_Task(void)
{
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);

    Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStartTimer, 3, 20);
}

// 终止任务
void End_Task(void)
{
    Send_ZigBeeDataNTimes(ZigBee_LEDDisplayStopTimer, 3, 20);
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);
    delay_ms(500);
    delay_ms(500);
    delay_ms(500);
    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);
}

// led显示距离（输入距离）
void LEDDispaly_ShowDistance(uint16_t dis)
{
    ZigBee_LEDDisplayDistanceData[4] = HEX2BCD(dis / 100);
    ZigBee_LEDDisplayDistanceData[5] = HEX2BCD(dis % 100);
    Send_ZigBeeData(ZigBee_LEDDisplayDistanceData);
}

// 路灯档位调节，输入目标档位自动调整
void StreetLight_AdjustTo(uint8_t targetLevel)
{
    uint16_t temp_val[4], CurrentLightValue;
    int8_t errorValue, i;

    for (i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Beep(2);
        Infrared_Send_A(Infrared_LightAdd1);
        delay_ms(790);
        delay_ms(790);
    }
    CurrentLightValue = temp_val[0];

    bubble_sort(temp_val, 4); // 对获得数据排序算出初始档位
    for (i = 0; i < 4; i++)
    {
        if (CurrentLightValue == temp_val[i])
        {
            errorValue = (int8_t)targetLevel - (i + 1);
            break;
        }
    }

    if (errorValue >= 0) // 调整到目标档位
    {
        for (i = 0; i < errorValue; i++)
        {
            Infrared_Send_A(Infrared_LightAdd1);
            delay_ms(790);
            delay_ms(790);
        }
    }
    else
    {
        for (i = 0; i > errorValue; i--)
        {
            Infrared_Send_A(Infrared_LightAdd3);
            delay_ms(790);
            delay_ms(790);
        }
    }
}

// 道闸部分 ↓

// 道闸显示车牌
void BarrierGate_Plate(uint8_t plate[6])
{
    memcpy(&ZigBee_PlateBarrierGate_1[3], plate, 3);
    memcpy(&ZigBee_PlateBarrierGate_2[3], &plate[3], 3);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_1);
    delay_ms(790);
    Send_ZigBeeData(ZigBee_PlateBarrierGate_2);
    delay_ms(790);
}

// 道闸打开
void BarrierGate_OPEN(void)
{
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
}

// 道闸任务
void BarrierGate_Task(uint8_t plate[6])
{
    if (plate != NULL)
    {
        BarrierGate_Plate(plate);
    }
    BarrierGate_OPEN();
}

// 道闸部分 ↑

// 语音任务，错误重试2次
void Voice_Task(void)
{
    Start_VoiceCommandRecognition(3);
}

// ETC任务
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // 调整10次，不开直接走
    {
        // 六秒前的数据作废
        if ((ETC_Status.isSet == SET) && (!IsTimeOut(ETC_Status.timeStamp, 6 * 1000)))
            break;
        MOVE(7);  // 跟着节拍
        MOVE(-7); // 一起摇摆
    }
}

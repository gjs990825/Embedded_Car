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
#include "data_interaction.h"
#include "agv.h"
#include "canp_hostcom.h"

#define Send_ZigBeeData5Times(data) Send_ZigBeeDataNTimes(data, 5, 200)

// 暂存运行状态数据
struct RunningStatus_Struct
{
    uint8_t stopFlag;
    uint8_t trackMode;
    int8_t currentSpeed;
    Moving_ByEncoder_t movingByencoder;
    uint16_t remainEncoderValue;
} RunningStatus;

// 白卡的标志位和指针

// 寻到白卡
uint8_t FOUND_RFID_CARD = false;
// 白卡路段
uint8_t RFID_RoadSection = false;
// 当前卡信息指针
RFID_Info_t *CurrentRFIDCard = NULL;

// 特殊地形的标志位

// 特殊地形路段
uint8_t Special_RoadSection = false;
// 进入特殊地形
uint8_t ENTER_SPECIAL_ROAD = false;
// 特殊地形是否处理
uint8_t Special_Road_Processed = false;

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 通用运行中断和恢复 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 保存遇到白卡时候的状态
void Save_RunningStatus(void)
{
    extern uint16_t Mp_Value;
    extern int LSpeed, RSpeed;

    RunningStatus.movingByencoder = Moving_ByEncoder;
    RunningStatus.stopFlag = Stop_Flag;
    RunningStatus.trackMode = Track_Mode;
    RunningStatus.currentSpeed = (LSpeed + RSpeed) / 2;
    RunningStatus.remainEncoderValue = temp_MP - Mp_Value;
}

// 恢复状态遇到白卡前储存的状态
// encoderChangeValue: 前后设定码盘差值
void Resume_RunningStatus(uint16_t encoderChangeValue)
{
    extern uint16_t Mp_Value;
    uint16_t Roadway_mp_Get(void);

    Roadway_mp_syn(); // 同步码盘
    Mp_Value = Roadway_mp_Get();

    Moving_ByEncoder = RunningStatus.movingByencoder;
    Stop_Flag = RunningStatus.stopFlag;
    Track_Mode = RunningStatus.trackMode;
    // 循迹信息已清空，需要重新计算并减去执行中的行进值
    temp_MP = Mp_Value + RunningStatus.remainEncoderValue;
    int8_t currentSpeed = RunningStatus.currentSpeed;
    Update_MotorSpeed(currentSpeed, currentSpeed);
    Submit_SpeedChanges();
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ RFID处理部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 初始化默认KEY
void UseDefaultKey(uint8_t *key, uint8_t keyx)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        key[i] = keyx;
    }
}

// 块信息初始化
void Block_InfoInit(Block_Info_t *blockInfo, uint8_t blockNumber)
{
    memset(blockInfo, 0, sizeof(Block_Info_t) * blockNumber);

    for (uint8_t i = 0; i < blockNumber; i++)
    {
        blockInfo[i].authMode = PICC_AUTHENT1A;
    }
}

// 设定当前卡信息
void Set_CurrentCardInfo(RFID_Info_t *RFIDx)
{
    CurrentRFIDCard = RFIDx;
}

// 读取当前卡片信息
ErrorStatus Read_RFID(RFID_Info_t *RFIDx)
{
    ErrorStatus status = SUCCESS;
    ErrorStatus tmpStatus;

    for (uint8_t i = 0; i < RFIDx->blockNumber; i++)
    {
        tmpStatus = PICC_ReadBlock(&RFIDx->blockInfo[i]);

        if (tmpStatus)
        {
            print_info("Block%d success, ", RFIDx->blockInfo[i].block);
            print_info("data:%s\r\n", RFIDx->blockInfo[i].data);
        }
        else
        {
            print_info("Block%d fail\r\n", RFIDx->blockInfo[i].block);
        }
        // 添加结束符（RFID内一般为字符串）
        RFIDx->blockInfo[i].data[16] = '\0';
        status &= tmpStatus;
        delay(100);
    }

    print_info("RFID %s\r\n", status ? "all pass" : "something wrong");

    return status;
}

// RFID寻卡任务，检测到前方白卡时执行
void RFID_Task(void)
{
    uint8_t i;

    // 当前卡信息未设定，跳出
    if (CurrentRFIDCard == NULL)
        return;

    // 记录位置信息
    CurrentRFIDCard->coordinate = NextStatus;
    print_info("Card At:%s\r\n", ReCoordinate_Convert(NextStatus));

    MOVE(7);
    for (i = 0; i < 6; i++) // 间隔一公分
    {
        MOVE(1);
        delay_ms(500);
        if (Read_RFID(CurrentRFIDCard) == SUCCESS)
            break; // 读取成功，跳出
    }

    RFID_RoadSection = false; // 结束寻卡
    FOUND_RFID_CARD = false;  // 清空标志位
    CurrentRFIDCard = NULL;   // 清空指针
    MOVE(-(7 + i));           // 返回读卡前位置

    // 十字路口需要多退后一点，因为响应时间变长会多走一点
    // 另外防止过线的危险hhhhhhhh
    if ((NextStatus.x % 2) && (NextStatus.y % 2) != 0)
    {
        MOVE(-5);
    }
}

// RFIDx开始
void RFIDx_Begin(RFID_Info_t *RFIDx)
{
    Set_CurrentCardInfo(RFIDx);
    RFID_RoadSectionTrue();
}

// RFIDx结束
void RFIDx_End(void)
{
    RFID_RoadSectionFalse();
}

// 数据块定义

Block_Info_t RFID1_Block[] = {
    {.block = 4, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

Block_Info_t RFID2_Block[] = {
    {.block = 4, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {.block = 5, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

Block_Info_t RFID3_Block[] = {
    {.block = 5, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {.block = 6, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

// RFID卡片定义

#define DefineRFIDCard(RFIDx)                          \
    RFID_Info_t RFIDx## = {.blockInfo = RFIDx##_Block, \
                           .blockNumber = GET_ARRAY_LENGEH(RFIDx##_Block)}

DefineRFIDCard(RFID1);
DefineRFIDCard(RFID2);
DefineRFIDCard(RFID3);

// RFID测试用函数

void RFID1_Begin(void)
{
    RFIDx_Begin(&RFID1);
}

void RFID2_Begin(void)
{
    RFIDx_Begin(&RFID2);
}

void RFID3_Begin(void)
{
    RFIDx_Begin(&RFID3);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 特殊地形部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 特殊地形处理
void SpecialRoad_Task(void)
{
    Move_ByEncoder(30, 50);
    Submit_SpeedChanges();

    for (uint8_t i = 0; i < 5; i++)
    {
        delay(100);
        Get_Track();
    }

    // -2增加容错
    while (NumberOfWhite < (ALL_WHITE - 2))
    {
        Get_Track();
    }

    Special_RoadSection = false;
    ENTER_SPECIAL_ROAD = false;
    Special_Road_Processed = true;
}

// 特殊地形测试
// 标记下一个节点为特殊地形
void SpecialRoad_Test(void)
{
    Special_RoadSection = true;
}

// 蜂鸣两次
void BEEP_Twice(void)
{
    Beep(2);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 烽火台（报警台）部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 点亮报警台
void Alarm_ON(uint8_t code[6])
{
    Infrared_Send(code, 6);
}

// 报警台更改报警码
void Alarm_ChangeCode(uint8_t code[6])
{
    ZigBee_AlarmData[Pack_MainCmd] = Alarm_CodeFront3Bytes;
    memcpy(&ZigBee_AlarmData[Pack_SubCmd1], code, 3);
    Send_ZigBeeData5Times(ZigBee_AlarmData);

    ZigBee_AlarmData[Pack_MainCmd] = Alarm_CodeBack3Bytes;
    memcpy(&ZigBee_AlarmData[Pack_SubCmd1], &code[3], 3);
    Send_ZigBeeData(ZigBee_AlarmData);
    // warning：上面为了防止报警灯响起多次只发送一次，需要的话可发送多次
    // Send_ZigBeeDataNTimes(ZigBee_AlarmData, 2, 100);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 道闸部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 道闸显示车牌
void BarrierGate_Plate(uint8_t plate[6])
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_PlateFront3Bytes;
    memcpy(&ZigBee_BarrierGateData[Pack_SubCmd1], plate, 3);
    Send_ZigBeeData5Times(ZigBee_BarrierGateData);

    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_PlateBack3Bytes;
    memcpy(&ZigBee_BarrierGateData[Pack_SubCmd1], &plate[3], 3);
    Send_ZigBeeData5Times(ZigBee_BarrierGateData);
}

// 道闸控制
void BarrierGate_Control(bool status)
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_Control;
    ZigBee_BarrierGateData[Pack_SubCmd1] = status ? 0x01 : 0x02;
    Send_ZigBeeData(ZigBee_BarrierGateData);
}

// 道闸回传状态
void BarrierGate_ReturnStatus(void)
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_ReturnStatus;
    ZigBee_BarrierGateData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeData(ZigBee_BarrierGateData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ LED显示部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// LED显示标志物第一行显示数据
void LEDDisplay_DataToFistRow(uint8_t data[3])
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_DataToFirstRow;
    memcpy(&ZigBee_LEDDisplayData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}

// LED显示标志物第二行显示数据
void LEDDisplay_DataToSecondRow(uint8_t data[3])
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_DataToSecondRow;
    memcpy(&ZigBee_LEDDisplayData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}

// LED显示标志物进更改计时模式
void LEDDisplay_TimerMode(TimerMode_t mode)
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_TimerMode;
    ZigBee_LEDDisplayData[Pack_SubCmd1] = (uint8_t)mode;
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}

// LED显示标志物显示距离
void LEDDisplay_Distance(uint16_t dis)
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_ShowDistance;
    ZigBee_LEDDisplayData[Pack_SubCmd2] = HEX2BCD(dis / 100);
    ZigBee_LEDDisplayData[Pack_SubCmd3] = HEX2BCD(dis % 100);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体显示（旋转LED）部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 旋转LED显示车牌和坐标
void RotationLED_PlateAndCoord(uint8_t plate[6], RouteNode_t coord)
{
    uint8_t *stringCoord = ReCoordinate_Convert(coord);

    Infrared_RotationLEDData[1] = RotationLEDMode_PlateFront4BytesData;
    memcpy(&Infrared_RotationLEDData[2], plate, 4);
    Infrared_Send_A(Infrared_RotationLEDData);
    delay_ms(600);

    Infrared_RotationLEDData[1] = RotationLEDMode_PlateBack2BytesAndCoordInfo;
    memcpy(&Infrared_RotationLEDData[2], &plate[4], 2);
    memcpy(&Infrared_RotationLEDData[4], stringCoord, 2);
    Infrared_Send_A(Infrared_RotationLEDData);
}

// 旋转LED显示距离 warning: 只能显示两位数
void RotationLED_Distance(uint8_t dis)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Distance;
    Infrared_RotationLEDData[2] = ((dis % 100) / 10) + 0x30;
    Infrared_RotationLEDData[3] = (dis % 10) + 0x30;
    Infrared_Send_A(Infrared_RotationLEDData);
}

// 旋转LED显示图形
void RotationLED_Shape(Shape_t shape)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Shape;
    Infrared_RotationLEDData[2] = (uint8_t)shape;
    Infrared_Send_A(Infrared_RotationLEDData);
}

// 旋转LED显示颜色
void RotationLED_Color(Color_t color)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Color;
    Infrared_RotationLEDData[2] = (uint8_t)color;
    Infrared_Send_A(Infrared_RotationLEDData);
}

// 旋转LED显示路况
void RotationLED_RouteStatus(RouteStatus_t status)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_RouteStatus;
    Infrared_RotationLEDData[2] = (uint8_t)status;
    Infrared_Send_A(Infrared_RotationLEDData);
}

// 旋转LED显示默认数据
void RotationLED_Default(void)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Default;
    Infrared_Send_A(Infrared_RotationLEDData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ TFT显示屏部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// TFT显示编号图片
void TFT_ShowPicture(uint8_t TFTx, uint8_t picNumber)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    if (picNumber > 20 || picNumber < 1)
        return;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x00;
    ZigBee_TFTData[Pack_SubCmd2] = picNumber;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// TFT上一张图片
void TFT_PicturePrevious(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeDataNTimes(ZigBee_TFTData, 2, 200);
}

// TFT下一张图片
void TFT_PictureNext(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x02;
    Send_ZigBeeDataNTimes(ZigBee_TFTData, 2, 200);
}

// TFT图片自动翻页
void TFT_PictureAuto(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x03;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// TFT显示车牌
void TFT_Plate(uint8_t TFTx, uint8_t plate[6])
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_PlateDataA;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], plate, 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_PlateDataB;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], &plate[3], 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// TFT计时模式控制
void TFT_Timer(uint8_t TFTx, TimerMode_t mode)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Timer;
    ZigBee_TFTData[Pack_SubCmd1] = (uint8_t)mode;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// TFT六位数据显示模式（HEX）
void TFT_HexData(uint8_t TFTx, uint8_t data[3])
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Hex;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// TFT显示距离
void TFT_Distance(uint8_t TFTx, uint16_t dis)
{
    ZigBee_TFTData[Pack_Header2] = (TFTx == TFT_A) ? ZigBeeID_TFTA : ZigBeeID_TFTB;

    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Distance;
    ZigBee_TFTData[Pack_SubCmd2] = HEX2BCD(dis / 100);
    ZigBee_TFTData[Pack_SubCmd3] = HEX2BCD(dis % 100);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体车库部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 立体车库到达第X层
void StereoGarage_ToLayer(uint8_t garage_x, uint8_t layer)
{
    ZigBee_StereoGarageData[Pack_Header2] = (garage_x == StereoGarage_A) ? ZigBeeID_StereoGarage_A : ZigBeeID_StereoGarage_B;

    if (layer > 4 || layer < 1)
        return;

    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Control;
    ZigBee_StereoGarageData[Pack_SubCmd1] = layer;
    Send_ZigBeeData5Times(ZigBee_StereoGarageData);
}

// 立体车库返回位于第几层
void StereoGarage_ReturnLayer(uint8_t garage_x)
{
    ZigBee_StereoGarageData[Pack_Header2] = (garage_x == StereoGarage_A) ? ZigBeeID_StereoGarage_A : ZigBeeID_StereoGarage_B;

    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Return;
    ZigBee_StereoGarageData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeData(ZigBee_StereoGarageData);
}

// 立体车库返回前后红外信息
void StereoGarage_ReturnInfraredStatus(uint8_t garage_x)
{
    ZigBee_StereoGarageData[Pack_Header2] = (garage_x == StereoGarage_A) ? ZigBeeID_StereoGarage_A : ZigBeeID_StereoGarage_B;

    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Return;
    ZigBee_StereoGarageData[Pack_SubCmd1] = 0x02;
    Send_ZigBeeData(ZigBee_StereoGarageData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 交通灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 交通灯进入识别状态
void TrafficLight_RecognitionMode(uint8_t light_x)
{
    ZigBee_TrafficLightData[Pack_Header2] = (light_x == TrafficLight_A) ? ZigBeeID_TrafficLight_A : ZigBeeID_TrafficLight_B;

    ZigBee_TrafficLightData[Pack_MainCmd] = TrafficLight_Recognition;
    ZigBee_TrafficLightData[Pack_SubCmd1] = 0x00;
    Send_ZigBeeDataNTimes(ZigBee_TrafficLightData, 2, 150);
}

// 交通灯确认识别结果
void TrafficLight_ConfirmColor(uint8_t light_x, TrafficLightColor_t light)
{
    ZigBee_TrafficLightData[Pack_Header2] = (light_x == TrafficLight_A) ? ZigBeeID_TrafficLight_A : ZigBeeID_TrafficLight_B;

    ZigBee_TrafficLightData[Pack_MainCmd] = TrafficLight_Confirm;
    ZigBee_TrafficLightData[Pack_SubCmd1] = (uint8_t)light;
    Send_ZigBeeData5Times(ZigBee_TrafficLightData);
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 语音识别部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 语音播报特定编号语音
void VoiceBroadcast_Specific(uint8_t voiceID)
{
    if (voiceID > 6 || voiceID < 1)
        return;

    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_Specific;
    ZigBee_VoiceData[Pack_SubCmd1] = voiceID;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}

// 语音随机播报语音
void VoiceBroadcast_Radom(void)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_Random;
    ZigBee_VoiceData[Pack_SubCmd1] = 0x01;
    Send_ZigbeeData_To_Fifo(ZigBee_VoiceData, 8);
}

// 返回语音识别结果到自动评分系统
void VoiceRecognition_Return(uint8_t voiceID)
{
    ZigBee_VoiceReturnData[Pack_MainCmd] = voiceID;
    for (uint8_t i = 0; i < 3; i++)
    {
        Send_ZigbeeData_To_Fifo(ZigBee_VoiceReturnData, 8);
        delay_ms(100);
    }
    // 语音返回有固定码，不能校验
}

// 语音识别任务
void Voice_Recognition(void)
{
    VoiceRecognition_Return(Start_VoiceCommandRecognition(3));
}

// ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 智能路灯部分 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

// 路灯档位调节，输入目标档位自动调整，返回当前档位
uint8_t StreetLight_AdjustTo(uint8_t targetLevel)
{
    uint16_t temp_val[4];

    for (int8_t i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Beep(2);
        Infrared_Send_A(Infrared_LightAdd1);
        delay(1600);
    }

    uint16_t currentBrightness = temp_val[0];
    uint8_t currentLevel;

    // 对获得数据排序可算出当前档位
    bubble_sort(temp_val, 4);

    for (int8_t i = 0; i < 4; i++)
    {
        if (currentBrightness == temp_val[i])
        {
            currentLevel = i + 1;
            break;
        }
    }

    // 目标档位为0时不调整
    if (targetLevel != 0)
    {
        int8_t errorValue = (int8_t)targetLevel - currentLevel;
        // 调整到目标档位
        if (errorValue >= 0)
        {
            for (int8_t i = 0; i < errorValue; i++)
            {
                Infrared_Send_A(Infrared_LightAdd1);
                delay(1600);
            }
        }
        else
        {
            for (int8_t i = 0; i > errorValue; i--)
            {
                Infrared_Send_A(Infrared_LightAdd3);
                delay(1600);
            }
        }
    }
    return currentLevel;
}

////////////////////////////////////////////////////////////

// 双闪灯闪烁
void Emergency_Flasher(uint16_t time)
{
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);

    if (time > 0)
        delay(time);

    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);
}

// 通用起始任务
void Start_Task(void)
{
    Emergency_Flasher(1500);
    LEDDisplay_TimerMode(TimerMode_ON);
}

// 通用终止任务
void End_Task(void)
{
    LEDDisplay_TimerMode(TimerMode_OFF);
    Emergency_Flasher(1500);
}

// 道闸任务
// 显示车牌并打开道闸
void BarrierGate_Task(uint8_t plate[6])
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (plate != NULL)
        {
            BarrierGate_Plate(plate);
        }
        else
        {
            BarrierGate_Control(true);
        }
        delay(100);

        if (Get_BarrierGateStatus())
            break;
        else
            delay(100);
    }
}

// 交通灯识别
void TrafficLight_Task(uint8_t light_x)
{
    uint8_t requestID = (light_x == TrafficLight_A) ? RequestTask_TrafficLightA : RequestTask_TrafficLightB;

    TrafficLight_RecognitionMode(light_x); // 开始识别交通灯
    delay_ms(700);
    RequestToHost_Task(requestID);
    WaitForFlagInMs(GetCmdFlag(FromHost_TrafficLight), SET, 9 * 1000); // 等待识别完成
}

// TFT图形图像识别
void TFT_Task(uint8_t TFTx)
{
    uint8_t requestID = (TFTx == TFT_A) ? RequestTask_TFTRecognitionA : RequestTask_TFTRecognitionB;

    RequestToHost_Task(requestID);                                        // 请求识别TFT内容
    WaitForFlagInMs(GetCmdFlag(FromHost_TFTRecognition), SET, 60 * 1000); // 等待识别完成
}

// 二维码识别
void QRCode_Task(uint8_t QRCode_x)
{
    uint8_t QRrequest = (QRCode_x == QRCode_1) ? RequestTask_QRCode1 : RequestTask_QRCode2;
    GetCmdFlag(FromHost_QRCodeRecognition) = RESET;
    RequestToHost_Task(QRrequest);
    WaitForFlagInMs(GetCmdFlag(FromHost_QRCodeRecognition), SET, 15 * 1000);
}

// 语音任务
void Voice_Task(void)
{
    Voice_Recognition();
}

// ETC任务
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // 调整10次，不开直接走
    {
        // 六秒前的数据作废
        if ((ETC_Status.isSet == SET) && (!IsTimeOut(Get_ZigBeeReturnStamp(ETC), 6 * 1000)))
            break;
        MOVE(7);  // 跟着节拍
        MOVE(-7); // 一起摇摆
    }
}

// AGV任务
void AGV_Task(DataToAGV_t agvData)
{
    uint8_t agvRoute[20];

    // 坐标、起始点、坐标处理
    RouteString_Process(agvData.currentCoord, agvData.routeInfo, agvRoute);
    print_info("AGV_Route:%s\r\n", agvRoute);
    AGV_SetRoute(agvRoute);

    // 方向设定
    if (agvData.direction != DIR_NOTSET)
    {
        print_info("AGV_Dir:%d\r\n", agvData.direction);
        AGV_SetTowards(agvData.direction);
    }

    // 报警码
    if (agvData.alarmData != NULL)
    {
        Dump_Array("AGV_Alarm:\r\n", agvData.alarmData, 6);
        AGV_SendInfraredData(agvData.alarmData);
    }

    // 路灯档位
    if (agvData.streetLightLevel > 0 && agvData.streetLightLevel <= 4)
    {
        print_info("LightLevel:%d\r\n", agvData.streetLightLevel);
        AGV_SendData(AGVPresetData_StreetLight, &agvData.streetLightLevel, 1);
    }

    // 任务组
    if (agvData.taskNumber != 0)
    {
        for (uint8_t i = 0; i < agvData.taskNumber; i++)
        {
            uint8_t taskOrder = Get_TaskNumber(agvData.taskCoord[i].coord, agvRoute, 1);
            print_info("Task%d: %d\r\n", i, taskOrder);
            AGV_SetTaskID(taskOrder, agvData.taskCoord[i].taskID);
        }
    }

    // 是否需要避让
    bool needToAvoid = false;
    if (agvData.avoidGarage != NULL)
    {
        // 检测主车当前位置是否在从车路径点内
        if (Is_ContainCoordinate(agvRoute, ReCoordinate_Convert(CurrentStatus)) != -1)
        {
            needToAvoid = true;
        }
    }

    // 处理避让临时入库
    if (needToAvoid)
    {
        // 若首选库为从车入库点则选择备选库
        uint8_t *garage = NULL;
        if (Is_ContainCoordinate(agvRoute, agvData.avoidGarage) == -1)
        {
            garage = agvData.avoidGarage;
        }
        else
        {
            garage = agvData.avoidGarage2;
        }
        Auto_ReverseParcking(&CurrentStatus, garage, NULL);
    }

    // 启动
    AGV_Start();

    // 经过道闸处理
    if (agvData.barrierGateCoord != NULL)
    {
        // 判断是否经过道闸，经过则等待开启
        int8_t steps = Is_ContainCoordinate(agvRoute, agvData.barrierGateCoord);
        print_info("AGV_Steps:%d\r\n", steps);
        if (steps != -1)
        {
            delay(steps * 1700);
            BarrierGate_Task(NULL);
        }
    }

    // 等待任务执行完成
    WaitForFlagInMs(AGV_MissionComplete, SET, 20 * 1000);

    // 避让完成后出库
    if (needToAvoid)
    {
        MOVE(35);
    }
}

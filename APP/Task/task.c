#include "task.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "voice.h"
#include "power_check.h"
#include "can_user.h"
#include "data_base.h"
#include "roadway_check.h"
#include "tba.h"
#include "data_base.h"
#include "swopt_drv.h"
#include "uart_a72.h"
#include "Can_check.h"
#include "delay.h"
#include "can_user.h"
#include "Timer.h"
#include "Rc522.h"
#include "malloc.h"
#include "a_star.h"
#include "debug.h"
#include "movement.h"
#include "protocol.h"
#include "hardware.h"
#include "route.h"
#include "ultrasonic.h"
#include "my_lib.h"
#include "seven_seg.h"

// 寻到白卡
uint8_t FOUND_RFID_CARD = false;
// 白卡路段
uint8_t RFID_RoadSection = false;

struct StatusBeforeFoundRFID_Struct
{
    uint8_t stopFlag;
    uint16_t setEncoder;
    uint16_t currentEncoder;
    uint8_t trackMode;
    uint8_t currentSpeed;
    Moving_ByEncoder_t movingByencoder;
} StatusBeforeFoundRFID;

extern uint16_t Mp_Value;
// 保存遇到白卡时候的状态
void Save_StatusBeforeFoundRFID(void)
{
    StatusBeforeFoundRFID.movingByencoder = Moving_ByEncoder;
    StatusBeforeFoundRFID.currentEncoder = Mp_Value;
    StatusBeforeFoundRFID.stopFlag = Stop_Flag;
    StatusBeforeFoundRFID.trackMode = Track_Mode;
    StatusBeforeFoundRFID.setEncoder = temp_MP;
    StatusBeforeFoundRFID.currentSpeed = Car_Speed;
}

// 恢复状态 encoderChangeValue: 前后设定码盘差值
void Resume_StatusBeforeFoundRFID(uint16_t encoderChangeValue)
{
    Roadway_mp_syn(); // 同步码盘
    Moving_ByEncoder = StatusBeforeFoundRFID.movingByencoder;
    Stop_Flag = StatusBeforeFoundRFID.stopFlag;
    Track_Mode = StatusBeforeFoundRFID.trackMode;
    // 循迹信息已清空，需要重新计算并减去执行中的行进值
    temp_MP = StatusBeforeFoundRFID.setEncoder - StatusBeforeFoundRFID.currentEncoder - encoderChangeValue;
    Car_Speed = StatusBeforeFoundRFID.currentSpeed;
}

// 设定为白卡路段
void Task_RFID_RoadSectionTrue(void)
{
    RFID_RoadSection = true;
}

// 设定为非白卡路段
void Task_RFID_RoadSectionFalse(void)
{
    RFID_RoadSection = false;
}

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

// // 旋转led发送车牌
// void RotationLED_Task(void)
// {
//     Request_ToHost(RequestCmd_RotatingLED);
//     delay_ms(790);
//     Infrared_Send_A(Infrared_PlateData1);
//     delay_ms(600);
//     Infrared_Send_A(Infrared_PlateData2);
// }

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
void RotationLED_Distance(uint8_t dis)
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

    LED_TimerStart();
}

// 终止任务
void End_Task(void)
{
    LED_TimerStop();
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

// ETC任务
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // 摇摆10次，不开直接走
    {
        if ((ETC_Status.isSet == SET) && Check_IsTimeOut(ETC_Status.timeStamp, 6 * 1000)) // 六秒前的数据作废
            break;
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 7), Stop_Flag, FORBACKCOMPLETE); // 跟着节拍
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 7), Stop_Flag, FORBACKCOMPLETE); // 一起摇摆
    }
}

uint8_t RFID_x = 0, RFID_y = 0;
uint8_t RFID_DataBlockLoation = 5;
uint8_t RFID_DataBuffer[17] = {0};

// RFID读卡，检测到执行读卡
void RFID_Task(void)
{
    uint8_t i;
    RFID_x = NextStatus.x; // 获取RFID位置
    RFID_y = NextStatus.y;

    print_info("FOUND_RFID:%d,%d\r\n", RFID_x, RFID_y);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 8), Stop_Flag, FORBACKCOMPLETE);
    for (i = 0; i < 9; i++) // 读卡范围约 11.5-16.5，间隔读取()
    {
        if (Read_RFID_Block(RFID_DataBlockLoation, RFID_DataBuffer) == true)
            break; // 读取成功，跳出
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 1), Stop_Flag, FORBACKCOMPLETE);
        delay_ms(500);
    }

    RFID_RoadSection = false; // 结束寻卡
    FOUND_RFID_CARD = false;  // 清空标志位
    TIM_Cmd(TIM5, DISABLE);   // 停止定时器

    ExcuteAndWait(Back_Off(30, Centimeter_Value * (8 + i)), Stop_Flag, FORBACKCOMPLETE); // 返回读卡前位置
    // 十字路口需要多退后一点，因为响应时间变长会多走一点
    if (StatusBeforeFoundRFID.stopFlag == CROSSROAD)
    {
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 2), Stop_Flag, FORBACKCOMPLETE);
    }

    Control(Car_Speed, Car_Speed);
}

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

// 道闸任务
void BarrierGate_Task(uint8_t plate[6])
{
    if (plate != NULL)
    {
        BarrierGate_Plate(plate);
    }
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
    Send_ZigbeeData_To_Fifo(ZigBee_BarrierGateOPEN, 8);
    delay_ms(790);
}

void Voice_Task(void)
{
    Start_VoiceCommandRecognition(3);
}

void Test_RFID(uint8_t block)
{
    uint8_t buf[17];
    Read_RFID_Block(block, buf);
}

// 读扇区 使用KEYA，存在buf里
bool Read_RFID_Block(uint8_t block, uint8_t *buf)
{
    uint8_t key[8]; // 使用默认key

    for (uint8_t i = 0; i < 8; i++)
    {
        key[i] = 0xFF;
    }
    if (RFID_ReadBlock(block, key, buf) == MI_OK)
    {
        for (uint8_t i = 0; i < 16; i++)
        {
            print_info("%X ", buf[i]);
        }
        print_info("\r\n");
        return true;
    }
    else
    {
        print_info("ERROR\r\n");
        return false;
    }
}

// 下面是坐标点对应的任务集合，独立任务进入前需要保证位置距离朝向等准确无误
// 任务结束和开始车身方向不一样的需要手动设置 CurrentStaus.dir = DIR_XX;

void Task_5_0(void)
{
    Start_Task();
}

uint8_t *RFID_Key = NULL;
void Task_F6(void)
{
    TURN(-50);
    MOVE(15);

    delay_ms(700);
    QRCode_Task(RequestCmd_QRCode1);
    RFID_Key = Get_QRCode(DataRequest_QRCode1, 0x01); // 获取密钥

    MOVE(-15);
    TURN(-40);

    RFID_DataBlockLoation = Get_QRCode(DataRequest_QRCode1, 0x02)[0]; // 获取二维码信息中的RFID数据块信息
    print_info("RFID_Block:%d\r\n", RFID_DataBlockLoation);

    CurrentStaus.dir = DIR_LEFT;
}

void Task_3_1(void)
{
    TrafficLight_Task();
    RFID_RoadSection = true; // 白卡路段开始
}

uint16_t distanceMeasured = 0;

void Task_1_3(void)
{
    ExcuteAndWait(Turn_ByEncoder(-93), Stop_Flag, TURNCOMPLETE); // 修正值
    ExcuteAndWait(Back_Off(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);

    delay_ms(700);
    QRCode_Task(RequestCmd_QRCode2);
    AGV_SendInfraredData(Get_QRCode(DataRequest_QRCode2, 0x00)); // 发送红外数据到从车

    distanceMeasured = Ultrasonic_GetAverage(20);
    LEDDispaly_ShowDistance(distanceMeasured); // 发两次防止丢包
    delay_ms(700);
    LEDDispaly_ShowDistance(distanceMeasured);

    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(93), Stop_Flag, TURNCOMPLETE); // 修正值
}

void Task_1_5(void)
{
    RFID_RoadSection = false;

    ExcuteAndWait(Turn_ByEncoder(-40), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

    TFT_Task();

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(40), Stop_Flag, TURNCOMPLETE);

    TFT_Hex(Get_ShapeInfo());
    delay_ms(790);
    TFT_Hex(Get_ShapeInfo());
    delay_ms(790);

    AGV_SendInfraredData(Infrared_AlarmON); // 发送红外信息 // 需要注掉！！！
    delay_ms(700);
    AGV_SetTaskID(Get_TaskNumber("D4", RFID_DataBuffer), 0); // 设定任务点
    AGV_SetRoute(RFID_DataBuffer);                           // 发送从车路径信息
    delay_ms(700);
    AGV_SetTowards(DIR_LEFT); // 设定车头朝向
    delay_ms(700);            // 等待
    AGV_Start();              // 从车启动
    delay_ms(700);            // 等待
    AGV_Start();

    if (Get_TaskNumber("B1", RFID_DataBuffer) != -1) // 从车入库任务设定
    {
        AGV_SetTaskID(Get_TaskNumber("B1", RFID_DataBuffer), 1);
    }
    else if (Get_TaskNumber("B7", RFID_DataBuffer) != -1)
    {
        AGV_SetTaskID(Get_TaskNumber("B7", RFID_DataBuffer), 1);
    }

    if (Get_TaskNumber("B2", RFID_DataBuffer) != -1) // 经过主车的路径
    {
        ExcuteAndWait(Back_Off(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
    }

    AGVComplete_Status.isSet = RESET;
    WaitForFlagInMs(AGVComplete_Status.isSet, SET, 25 * 1000); // 等待从车执行入库完成

    if (Get_TaskNumber("B2", RFID_DataBuffer) != -1) // 经过主车的路径
    {
        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
    }

    // 计算到达道闸的时间(不过道闸)
    // int8_t taskNumber1 = Get_TaskNumber("F4", RFID_DataBuffer);
    // int8_t taskNumber2 = Get_TaskNumber("F2", RFID_DataBuffer);
    // int8_t taskNumber = (taskNumber1 < taskNumber2) ? taskNumber1 : taskNumber2;

    // if (taskNumber != -1) // 一个节点等待一秒
    // {
    //     for (int8_t i = 0; i < taskNumber; i++)
    //     {
    //         delay_ms(500);
    //         delay_ms(500);
    //     }
    //     BarrierGate_Task(NULL); // 为从车开启道闸
    // }

    ExcuteAndWait(Turn_ByEncoder(90 + 35), Stop_Flag, TURNCOMPLETE);

    RotationLED_Plate(Get_PlateNumber(), ReCoordinate_Covent(RFID_x, RFID_y));

    ExcuteAndWait(Turn_ByEncoder(-35), Stop_Flag, TURNCOMPLETE);

    CurrentStaus.dir = DIR_RIGHT;
}

void Task_3_5(void)
{
    uint8_t level = (Get_AllColorCount() * (distanceMeasured / 100)) % 4 + 1;

    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
    ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

    StreetLight_AdjustTo(level);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
    ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
}

void Task_5_5(void)
{
    BarrierGate_Task(NULL);
}

void Task_3_1_2(void)
{
    Voice_Task();
}

void Task_F6_2(void)
{
    ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);

    ExcuteAndWait(Back_Off(30, Centimeter_Value * 30), Stop_Flag, FORBACKCOMPLETE);

    Send_ZigBeeData(ZigBee_WirelessChargingON);
    Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 2, 700);
    End_Task();
}


// void Task_5_5(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(90 + 50), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
//     Beep(2);

//     delay_ms(700);
//     delay_ms(700); // 等待摄像头反应
//     TFT_Task();

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 17), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(-50 - 35), Stop_Flag, TURNCOMPLETE);

//     RotationLED_Task();

//     ExcuteAndWait(Turn_ByEncoder(35), Stop_Flag, TURNCOMPLETE);
//     CurrentStaus.dir = DIR_LEFT; // 与任务开始时方向不一致
// }

// void Task_3_5(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(22), Stop_Flag, TURNCOMPLETE);

//     TrafficLight_Task();

//     ExcuteAndWait(Turn_ByEncoder(-22), Stop_Flag, TURNCOMPLETE);

//     RFID_RoadSection = true; // 白卡路段开始
// }

// void Task_1_5(void)
// {
//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

//     delay_ms(700);
//     QRCode_Task();

//     uint16_t distanceMeasured = Ultrasonic_GetAverage(20);
//     LEDDispaly_ShowDistance(distanceMeasured); // 发两次防止丢包
//     delay_ms(700);
//     LEDDispaly_ShowDistance(distanceMeasured);

//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
// }

// void Task_1_3(void)
// {
//     RFID_RoadSection = false; // 白卡路段结束

//     // 路灯
//     ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

//     StreetLight_AdjustTo(3);

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
// }

// void Task_5_3(void)
// {
//     Voice_Task();

//     ExcuteAndWait(Turn_ByEncoder(90 + 45), Stop_Flag, TURNCOMPLETE);

//     Infrared_Send_A(Infrared_AlarmON);

//     ExcuteAndWait(Turn_ByEncoder(-45), Stop_Flag, TURNCOMPLETE);

//     AGV_SetTowards(DIR_DOWN);
//     delay_ms(700);
//     BarrierGate_Task("A12B34");
//     Send_ZigBeeData(ZigBee_AGVStart);

//     CurrentStaus.dir = DIR_DOWN;
// }

// void Task_F6(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);

//     ETC_Task();

//     CurrentStaus.dir = DIR_LEFT;
// }

// void Task_3_1(void)
// {
//     // 入库
//     delay_ms(500);
//     Send_ZigBeeData(ZigBee_WirelessChargingON);
//     delay_ms(700);
//     End_Task();
// }

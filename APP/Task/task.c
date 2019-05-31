#include "task.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "bh1750.h"
#include "voice.h"
#include "can_user.h"
#include "roadway_check.h"
#include "tba.h"
#include "uart_a72.h"
#include "Rc522.h"
#include "malloc.h"
#include "a_star.h"
#include "debug.h"
#include "movement.h"
#include "protocol.h"
#include "route.h"
#include "my_lib.h"
#include "Timer.h"
#include "agv.h"
#include "data_interaction.h"
#include "string.h"
#include "ctype.h"
#include "seven_segment_display.h"

uint16_t distanceMeasured = 0;
uint8_t lightLevel = 1;

void Task_A2(void)
{
    Start_Task();
}

void Task_B2(void)
{
    TURN_TO(DIR_UP);
    lightLevel = StreetLight_AdjustTo(0);

    TURN_TO(DIR_DOWN);
    TURN(45);

    MOVE(15);

    TFT_Task(TFT_A);

    MOVE(-15);

    uint8_t *plate = Get_PlateNumber(TFT_A);
    TFT_Plate(TFT_A, plate);

    TURN(-45);
}

void Task_B3(void)
{
    QRCode_Task(QRCode_1);

    RFID1.blockInfo[0].block = lightLevel * 4 + 0;
    RFID1.blockInfo[1].block = lightLevel * 4 + 1;

    RFID1_Begin();
}

void Task_B4(void)
{
    TURN_TO(DIR_RIGHT);

    TrafficLight_Task(TrafficLight_A);
}

void Task_D4(void)
{
    ETC_Task();
}

void Task_F4(void)
{
    QRCode_Task(QRCode_2);
    distanceMeasured = DistanceMeasure_Task();
    TFT_Distance(TFT_A, distanceMeasured);
}

void Task_F6(void)
{
    RFIDx_End();
    Send_RFIDData(1, RFID1.blockInfo[0].data, 16);
    delay(500);
    Send_RFIDData(2, RFID1.blockInfo[1].data, 16);
    delay(500);

    uint8_t *towards = Get_QRCode(QRCode_1, 1);
    uint8_t dir = Get_Towards("D5", towards);
    uint8_t *parking = Get_RFIDInfo(1);
    uint8_t light = Get_RFIDInfo(2)[0];

    AGV_SendData(1, parking, 2);
    AGV_SendData(2, &light, 1);

    taskCoord_t taskCoord[1];
    DataToAGV_t DataToAGV;

    taskCoord[0].coord = "F2";
    taskCoord[0].taskID = 0;

    DataToAGV.currentCoord = "D5";                      // 坐标点
    DataToAGV.direction = dir;                          // 车头朝向
    DataToAGV.routeInfo = "D5D6F6F4F2D2B2";             // 路径信息
    DataToAGV.alarmData = NULL;                         // 报警码
    DataToAGV.taskCoord = taskCoord;                    // 任务点和对应ID
    DataToAGV.taskNumber = GET_ARRAY_LENGEH(taskCoord); // 任务数量
    DataToAGV.barrierGateCoord = "F3";                  // 道闸位置
    DataToAGV.avoidGarage = "F7";                       // 主车避让车库
    DataToAGV.avoidGarage2 = "G6";                      // 主车备选避让车库
    DataToAGV.streetLightLevel = light;                 // 路灯档位
    AGV_Task(DataToAGV);

    Voice_Task();

    uint8_t *tftInfo = Get_TFTInfo(TFT_A);

    TURN_TO(DIR_LEFT);
    TURN(-45);
    RotationLED_PlateAndCoord(tftInfo, RFID1.coordinate);
    TURN(45);
}

void Task_D6(void)
{
    TURN(-45);
    Alarm_ON(Infrared_AlarmON);
    TURN(45);

    uint8_t *parking = Get_QRCode(1, 2);

    Auto_ReverseParcking(&CurrentStatus, parking, NULL);

    End_Task();

    SevenSegmentDisplay_Update(66);
    while (1)
    {
        SevenSegmentDisplay_Refresh();
        delay_ms(10);
    }
    
}

// int X, Y, N;

// void Task_F1(void)
// {
//     Start_Task();
// }

// void Task_F2(void)
// {
//     TURN_TO(DIR_LEFT);
//     TURN(45);

//     MOVE(15);
//     TFT_Task(TFT_A);
//     MOVE(-15);

//     uint8_t *plate = Get_PlateNumber(TFT_A);
//     uint8_t *tftInfo = Get_TFTInfo(TFT_A);

//     TFT_Plate(TFT_A, plate);

//     TURN(-45);
//     TURN(-45);

//     RotationLED_PlateAndCoord(tftInfo, Coordinate_Convert(&tftInfo[6]));

//     TURN(45);
// }

// void Task_D2(void)
// {
//     TrafficLight_Task(TrafficLight_A);

//     uint8_t sector = Get_ColorNumber(TFT_A, Color_Red);
//     uint8_t block = Get_ShapeNumber(TFT_A, Shape_Diamond);

//     RFID1.blockInfo->block = sector * 4 + block;

//     RFID1_Begin();
// }

// void Task_B2(void)
// {
//     QRCode_Task(QRCode_1);

//     distanceMeasured = DistanceMeasure_Task();
//     LEDDisplay_Distance(distanceMeasured + 1);
// }

// void Task_B4(void)
// {
//     // TURN_TO(DIR_LEFT);

//     TURN(90);

//     lightLevel = StreetLight_AdjustTo(0);
//     X = lightLevel;

//     TURN(180);

//     CurrentStatus.dir = DIR_RIGHT;

//     RFIDx_End();
//     Send_RFIDData(1, RFID1.blockInfo->data, 16);
//     SpecialRoad_Begin();
// }

// void Task_C4(void)
// {
// }

// void Task_D4(void)
// {
// }

// void Task_F4(void)
// {
//     Special_RoadSection = false;

//     uint8_t *towards = Get_RFIDInfo(1);
//     print_str(towards);
//     uint8_t dir = Get_Towards("D5", towards);
//     print_var(dir);

//     Y = Get_RFIDInfo(2)[0];

//     N = (((X * 3 - 1) * Y) % 4) + 1;

//     uint8_t *parking = NULL;

//     switch (N)
//     {
//     case 1:
//         parking = "B1";
//         break;
//     case 2:
//         parking = "D1";
//         break;
//     case 3:
//         parking = "F1";
//         break;
//     case 4:
//         parking = "G2";
//         break;

//     default:
//         parking = "G2";
//         break;
//     }

//     AGV_SendData(FromHost_AGVData1, parking, 3);
//     delay(500);

//     taskCoord_t taskCoord[3];
//     DataToAGV_t DataToAGV;

//     taskCoord[0].coord = "B5";
//     taskCoord[0].taskID = 0;
//     taskCoord[1].coord = "B4";
//     taskCoord[1].taskID = 1;
//     taskCoord[2].coord = "B6";
//     taskCoord[2].taskID = 2;

//     DataToAGV.currentCoord = "D5";                      // 坐标点
//     DataToAGV.direction = dir;                          // 车头朝向
//     DataToAGV.routeInfo = "D6B6B5B4";                     // 路径信息
//     DataToAGV.alarmData = NULL;                         // 报警码
//     DataToAGV.taskCoord = taskCoord;                    // 任务点和对应ID
//     DataToAGV.taskNumber = GET_ARRAY_LENGEH(taskCoord); // 任务数量
//     DataToAGV.barrierGateCoord = "C6";                  // 道闸位置
//     DataToAGV.avoidGarage = "G4";                       // 主车避让车库
//     DataToAGV.avoidGarage2 = "F7";                      // 主车备选避让车库
//     DataToAGV.streetLightLevel = 0;                     // 路灯档位
//     AGV_Task(DataToAGV);

//     Voice_Task();
// }

// void Task_F5(void)
// {
//     TURN(90);

//     uint8_t *alarm = Get_QRCode(QRCode_1, 1);

//     dump_array(alarm, 6);

//     Alarm_ON(alarm);

//     TURN(-90);
// }

// void Task_F6(void)
// {
//     TURN_TO(DIR_LEFT);
//     ETC_Task();
// }

// void Task_D6(void)
// {
//     Auto_ReverseParcking(&CurrentStatus, "D7", NULL);

//     StereoGarage_ToLayer(StereoGarage_A, 2);

//     End_Task();
//     SevenSegmentDisplay_Update(21);
//     while (1)
//     {
//         SevenSegmentDisplay_Refresh();
//         delay_ms(10);
//     }
// }

// void Task_D7(void)
// {
// }

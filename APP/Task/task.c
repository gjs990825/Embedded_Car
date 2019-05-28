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

void Task_F7(void)
{
    Start_Task();
}

void Task_F6(void)
{
    TURN_TO(DIR_LEFT);

    TrafficLight_Task(TrafficLight_A);
}

void Task_D6(void)
{
    QRCode_Task(QRCode_1);
}

void Task_B6(void)
{
    TFT_Task(TFT_A);
}

void Task_B4(void)
{
    TURN(-90);
    CurrentStatus.dir = DIR_LEFT;

    MOVE(-15);

    distanceMeasured = Ultrasonic_GetAverage(20);
    LEDDisplay_Distance(distanceMeasured);

    RFID1.blockInfo->block = (distanceMeasured / 100) % 3 + 4;

    uint8_t *key = Get_QRCode(QRCode_1, 1);

    dump_array(key, 6);
    // memcpy(RFID1.blockInfo->key, key, 6);

    MOVE(15);

    RFID1_Begin();
}

void Task_D4(void)
{
}

void Task_F4(void)
{
    RFIDx_End();

    TURN_TO(DIR_UP);

    TURN(-45);

    uint8_t *plate = Get_PlateNumber(TFT_A);
    RotationLED_PlateAndCoord(plate, RFID1.coordinate);

    TURN(45);

    BarrierGate_Task(plate);
}

void Task_F2(void)
{
    // Voice_Recognition();
    uint8_t id = Start_VoiceCommandRecognition(3);

    uint8_t data[3] = {0xDC, 0x01, id % 100};

    TFT_HexData(TFT_A, data);
}

void Task_D2(void)
{
    taskCoord_t taskCoord[1];
    DataToAGV_t DataToAGV;

    taskCoord[0].coord = "D4";
    taskCoord[0].taskID = 0;

    DataToAGV.currentCoord = "G2",                          // 坐标点
        DataToAGV.direction = DIR_LEFT,                     // 车头朝向
        DataToAGV.routeInfo = RFID1.blockInfo->data,        // 路径信息
        DataToAGV.alarmData = Infrared_AlarmON,             // 报警码
        DataToAGV.taskCoord = taskCoord,                    // 任务点和对应ID
        DataToAGV.taskNumber = GET_ARRAY_LENGEH(taskCoord), // 任务数量
        DataToAGV.barrierGateCoord = "F3",                  // 道闸位置
        DataToAGV.avoidGarage = "D1",                       // 主车避让车库
        DataToAGV.avoidGarage2 = "A2",                      // 主车备选避让车库
        DataToAGV.streetLightLevel = 0;                     // 路灯档位

    AGV_Task(DataToAGV);
}

void Task_B2(void)
{
    TURN_TO(DIR_UP);
    uint8_t level = Get_ShapeColorNumber(TFT_A, Shape_Rectangle, Color_Red);
    print_var(level);
    StreetLight_AdjustTo(level);

    Auto_ReverseParcking(&CurrentStatus, "A2", NULL);

    End_Task();

    SevenSegmentDisplay_Update(12);
    while (1)
    {
        SevenSegmentDisplay_Refresh();
        delay_ms(10);
    }
}

void Task_A2(void)
{
}

// void Task_F7(void)
// {
//     Start_Task();
// }

// void Task_F6(void)
// {
//     TURN(-50);
//     QRCode_Task(RequestTask_QRCode1);
//     TURN(-40);
//     CurrentStatus.dir = DIR_LEFT;
// }

// void Task_D6(void)
// {
//     TrafficLight_Task(TrafficLight_A);
// }

// void Task_B6(void)
// {
//     RFID1_Begin();

//     uint8_t *buf = Get_QRCode(DataRequest_QRCode1, 1);
//     dump_array(buf, 6);
//     memcpy(RFID1.blockInfo->key, buf, 6);
//     uint8_t block = (Get_QRCode(DataRequest_QRCode1, 2)[0] % 3) + 4;
//     print_var(block);
//     RFID1.blockInfo->block = block;
// }

// void Task_B4(void)
// {
//     TURN(-90);
//     CurrentStatus.dir = DIR_LEFT;
//     // TURN_TO(DIR_LEFT);
//     MOVE(-15);

//     QRCode_Task(RequestTask_QRCode2);
//     distanceMeasured = Ultrasonic_GetAverage(20);
//     LEDDisplay_Distance(distanceMeasured);

//     MOVE(15);
// }

// void Task_B2(void)
// {
//     RFIDx_End();

//     TURN(-35);
//     MOVE(10);

//     TFT_Task(TFT_A);

//     uint8_t buf[3];
//     buf[0] = Get_ShapeNumber(TFT_A, Shape_Pentagram) + 0xA0;
//     buf[1] = Get_ShapeNumber(TFT_A, Shape_Diamond) + 0xB0;
//     buf[2] = Get_ShapeNumber(TFT_A, Shape_Circle) + 0xE0;
//     TFT_HexData(TFT_A, buf);

//     MOVE(-10);
//     TURN(35);

//     DataToAGV_t DataToAGV;
//     uint8_t buffer[17];
//     RouteString_Process(NULL, RFID1.blockInfo->data, buffer);

//     taskCoord_t taskCoord[1];

//     taskCoord[0].coord = "D4";
//     taskCoord[0].taskID = 0;

//     DataToAGV.currentCoord = "G4";
//     DataToAGV.direction = DIR_LEFT;
//     DataToAGV.routeInfo = buffer;
//     DataToAGV.alarmData = Get_QRCode(DataRequest_QRCode2, 1);
//     DataToAGV.taskCoord = taskCoord;
//     DataToAGV.taskNumber = 1;
//     DataToAGV.barrierGateCoord = "F3";
//     DataToAGV.avoidGarage = "A1";
//     DataToAGV.avoidGarage2 = "A2";
//     DataToAGV.streetLightLevel = 0;
//     AGV_Task(DataToAGV);

//     TURN_TO(DIR_RIGHT);
//     TURN(40);

//     RotationLED_PlateAndCoord(Get_PlateNumber(TFT_A), RFID1.coordinate);

//     TURN(-40);
// }

// void Task_D2(void)
// {
//     TURN_TO(DIR_UP);

//     uint16_t n = Get_AllColorCount(TFT_A);
//     uint16_t m = distanceMeasured;

//     StreetLight_AdjustTo((n * (m / 100)) % 4 + 1);
// }

// void Task_F2(void)
// {
//     BarrierGate_Task("ABC233");
// }

// void Task_F4(void)
// {
// }

// void Task_D4(void)
// {

// }

// void Task_D6_2(void)
// {
//     Voice_Recognition();
// }

// void Task_F6_2(void)
// {
//     Reverse_Parcking(&CurrentStatus, "F7");
//     WirelessCharging_ON();
//     End_Task();
// }

// void Task_F7_2(void)
// {
// }

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

uint16_t distanceMeasured = 0;
RFID_Info_t RFID_1 = {.authMode = PICC_AUTHENT1A};

void Task_F7(void)
{
    RFID_Info_t *rfid = &RFID_1;

    memset(rfid, 0, sizeof(RFID_Info_t));
    for (uint8_t i = 0; i < 6; i++)
    {
        rfid->key[i] = 0x00;
    }
    rfid->authMode = PICC_AUTHENT1A;

    Start_Task();
}

void Task_F6(void)
{
    TURN_TO(DIR_LEFT);
    TrafficLight_Task();
}

void Task_D6(void)
{
    QRCode_Task(RequestTask_QRCode1);

    uint8_t *key = Get_QRCode(DataRequest_QRCode1, 1);
    dump_array(key, 6);
}

void Task_B6(void)
{
    TFT_Task();
}

void Task_B4(void)
{
    Set_CurrentCardInfo(&RFID_1);
    RFID_RoadSection = true;

    TURN(-90);
    CurrentStaus.dir = DIR_LEFT;

    MOVE(-15);

    QRCode_Task(RequestTask_QRCode2);

    distanceMeasured = Ultrasonic_GetAverage(10);
    RFID_1.dataBlockLocation = (distanceMeasured / 100) % 3 + 4;
    print_var(RFID_1.dataBlockLocation);

    LEDDisplay_Distance(distanceMeasured);

    MOVE(15);
    TURN_TO(DIR_RIGHT);
}

void Task_D4(void)
{
}

void Task_F4(void)
{
    TURN_TO(DIR_UP);

    TURN(-50);

    RotationLED_PlateAndCoord(Get_PlateNumber(), RFID_1.coordinate);

    TURN(50);

    BarrierGate_Task(NULL);
}

void Task_F2(void)
{
    uint8_t str[3];
    str[0] = 0xDC;
    str[1] = 0x01;
    str[2] = Start_VoiceCommandRecognition(3) % 100;
    TFT_HexData(str);
}

void Task_D2(void)
{
    DataToAGV_t agvData;

    agvData.alarmData = Get_QRCode(DataRequest_QRCode2, 1);
    agvData.barrierGateCoord = "F3";
    agvData.currentCoord = "G2";
    agvData.direction = DIR_LEFT;
    agvData.routeInfo = RFID_1.data;
    agvData.taskCoord = "D4";

    AGV_Task(agvData);
}

void Task_B2(void)
{
    TURN_TO(DIR_UP);

    uint8_t shapeNumber = Get_ShapeColorNumber(Shape_Rectangle, Color_Red);
    print_var(shapeNumber);
    StreetLight_AdjustTo(shapeNumber % 4 + 1);

    TURN_TO(DIR_RIGHT);
    MOVE(-30);

    WirelessCharging_ON();

    End_Task();
}

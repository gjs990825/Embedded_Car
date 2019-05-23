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

Block_Info_t RFID1_Block[2] = {
    {.block = 4, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {.block = 6, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

Block_Info_t RFID2_Block[2] = {
    {.block = 4, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {.block = 5, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

Block_Info_t RFID3_Block[2] = {
    {.block = 5, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
    {.block = 6, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};

RFID_Info_t RFID1 = {.blockInfo = RFID1_Block, .blockNumber = 2};
RFID_Info_t RFID2 = {.blockInfo = RFID2_Block, .blockNumber = 2};
RFID_Info_t RFID3 = {.blockInfo = RFID3_Block, .blockNumber = 2};

void RFID1_Begin(void)
{
    Set_CurrentCardInfo(&RFID1);
    RFID_RoadSectionTrue();
}

void RFID1_End(void)
{
    RFID_RoadSectionFalse();
}

void RFID2_Begin(void)
{
    Set_CurrentCardInfo(&RFID2);
    RFID_RoadSectionTrue();
}

void RFID2_End(void)
{
    RFID_RoadSectionFalse();
}

void RFID3_Begin(void)
{
    Set_CurrentCardInfo(&RFID3);
    RFID_RoadSectionTrue();
}

void RFID3_End(void)
{
    RFID_RoadSectionFalse();
}

uint8_t currentLightLevel;
// RFID_Info_t RFID1 = {.authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

uint8_t Calculate_N(uint8_t *str)
{
    uint8_t length = strlen((char *)str);
    uint8_t N = 0;

    for (uint8_t i = 0; i < length; i++)
    {
        if (isdigit(str[i]))
        {
            if ((str[i] - '0') % 2 != 0)
            {
                N += str[i] - '0';
            }
        }
    }
    return N;
}

uint8_t color_table[] = {0, Color_Red, Color_Yellow, Color_Blue, Color_Cyan, Color_PurpleMagenta, Color_Green};
uint8_t shape_table[] = {0, Shape_Pentagram, Shape_Diamond, Shape_Triangle, Shape_Rectangle, Shape_Circle};

uint8_t Calculate_M(uint8_t *str)
{
    uint8_t length = strlen((char *)str);
    uint8_t M = 0;

    uint8_t color1, shape1, color2, shape2;
    uint8_t times = 0;

    for (uint8_t i = 0; i < length; i++)
    {
        if (str[i] == 'C')
        {
            if (times == 0)
            {
                color1 = str[i + 1] - '0';
            }
            else
            {
                color2 = str[i + 1] - '0';
            }
        }
        if (str[i] == 'T')
        {
            if (times == 0)
            {
                shape1 = str[i + 1] - '0';
                times++;
            }
            else
            {
                shape2 = str[i + 1] - '0';
            }
        }
    }

    color1 = color_table[color1];
    shape1 = shape_table[shape1];
    color2 = color_table[color2];
    shape2 = shape_table[shape2];

    M += Get_ShapeColorNumber(TFT_A, shape1, color1);
    M += Get_ShapeColorNumber(TFT_A, shape2, color2);

    return M;
}

void TaskAfterParcking(void)
{
    while (1)
    {
        ;
    }
}

void Task_A2(void)
{
    Start_Task();
}

void Task_B2(void)
{
    // Auto_ReverseParcking(&CurrentStatus, "A4", TaskAfterParcking);
    // TURN_TO(DIR_UP);

    // currentLightLevel = StreetLight_AdjustTo(1);
    // RFID1.dataBlockLocation = currentLightLevel * 3 * 4;

    // TURN_TO(DIR_DOWN);

    // TURN(45);

    // // TFT_Task();

    // TURN(-45);
}

void Task_B3(void)
{
    TURN(-90);

    QRCode_Task(RequestTask_QRCode1);

    TURN(90);

    RFID_RoadSectionTrue();
    // Set_CurrentCardInfo(&RFID1);
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
    QRCode_Task(RequestTask_QRCode2);
}

void Task_F6(void)
{
    DataToAGV_t AGVData;
    taskCoord_t taskCoord[2];

    taskCoord[0].coord = "B2";
    taskCoord[0].taskID = AGVPresetTask_Streetlight;
    taskCoord[1].coord = "D6";
    taskCoord[1].taskID = 0;

    AGVData.alarmData = Infrared_AlarmON;
    AGVData.avoidGarage = "G6";
    AGVData.barrierGateCoord = "F3";
    AGVData.currentCoord = "D5";
    AGVData.direction = DIR_RIGHT;
    AGVData.routeInfo = "D5D6F6F2B2";
    AGVData.streetLightLevel = 3;
    AGVData.taskCoord = taskCoord;
    AGVData.tasknumber = 2;

    AGV_Task(AGVData);

    // AGVData.alarmData = 0;
    // AGVData.currentCoord = "D5";
    // AGVData.barrierGateCoord = "F3";
    // AGVData.direction = DIR_RIGHT;
    // AGVData.routeInfo = "D6F6F2B2";
    // AGVData.taskCoord = "B2";

    // uint8_t N = Calculate_N(RFID1.data2);
    // AGV_SendData(2, &N, 1);
    // uint8_t M = Calculate_M(RFID1.data);

    // uint8_t *garage = "A2";
    // garage = "A1";

    // AGV_SendData(1, &M, 1);

    AGV_Task(AGVData);

    Start_VoiceCommandRecognition(3);

    TURN_TO(DIR_LEFT);
    TURN(-40);

    // RotationLED_PlateAndCoord("AaBbCc", RFID1.coordinate);

    TURN(40);
}

void Task_D6(void)
{
    TURN(-40);

    uint8_t *alarmData = Get_QRCode(DataRequest_QRCode2, 1);
    Infrared_Send(alarmData, 6);

    TURN(40);

    Route_Task[8].node.x = 0;
    Route_Task[8].node.y = 1;
}

void Task_Test(void)
{
    Auto_ReverseParcking(&CurrentStatus, "F7", infinity_loop);
}

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
#include "data_from_host.h"

// void Task_5_0(void)
// {
//     Start_Task();
// }

// uint8_t *RFID_Key = NULL;
// void Task_F6(void)
// {
//     TURN(-50);
//     MOVE(15);

//     delay_ms(700);
//     QRCode_Task(RequestCmd_QRCode1);
//     RFID_Key = Get_QRCode(DataRequest_QRCode1, 0x01); // 获取密钥

//     MOVE(-15);
//     TURN(-40);

// //    RFID_DataBlockLoation = Get_QRCode(DataRequest_QRCode1, 0x02)[0]; // 获取二维码信息中的RFID数据块信息
// //    print_info("RFID_Block:%d\r\n", RFID_DataBlockLoation);

//     CurrentStaus.dir = DIR_LEFT;
// }

// void Task_3_1(void)
// {
//     TrafficLight_Task();
//     RFID_RoadSection = true; // 白卡路段开始
// }

// uint16_t distanceMeasured = 0;

// void Task_1_3(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(-93), Stop_Flag, TURNCOMPLETE); // 修正值
//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);

//     delay_ms(700);
//     QRCode_Task(RequestCmd_QRCode2);
//     AGV_SendInfraredData(Get_QRCode(DataRequest_QRCode2, 0x00)); // 发送红外数据到从车

//     distanceMeasured = Ultrasonic_GetAverage(20);
//     LEDDisplay_ShowDistance(distanceMeasured); // 发两次防止丢包
//     delay_ms(700);
//     LEDDisplay_ShowDistance(distanceMeasured);

//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 13), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(93), Stop_Flag, TURNCOMPLETE); // 修正值
// }

// void Task_1_5(void)
// {
//     RFID_RoadSection = false;

//     ExcuteAndWait(Turn_ByEncoder(-40), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);

//     TFT_Task();

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 15), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(40), Stop_Flag, TURNCOMPLETE);

//     TFT_HexData(Get_ShapeInfo());
//     delay_ms(790);
//     TFT_HexData(Get_ShapeInfo());
//     delay_ms(790);

//     AGV_SendInfraredData(Infrared_AlarmON); // 发送红外信息 // 需要注掉！！！
//     delay_ms(700);
// //    AGV_SetTaskID(Get_TaskNumber("D4", RFID_DataBuffer, 1), 0); // 设定任务点
// //    AGV_SetRoute(RFID_DataBuffer);                              // 发送从车路径信息
//     delay_ms(700);
//     AGV_SetTowards(DIR_LEFT); // 设定车头朝向
//     delay_ms(700);            // 等待
//     AGV_Start();              // 从车启动
//     delay_ms(700);            // 等待
//     AGV_Start();

// //    if (Get_TaskNumber("B1", RFID_DataBuffer, 1) != -1) // 从车入库任务设定
// //    {
// //        AGV_SetTaskID(Get_TaskNumber("B1", RFID_DataBuffer, 1), 1);
// //    }
// //    else if (Get_TaskNumber("B7", RFID_DataBuffer, 1) != -1)
// //    {
// //        AGV_SetTaskID(Get_TaskNumber("B7", RFID_DataBuffer, 1), 1);
// //    }

// //    if (Get_TaskNumber("B2", RFID_DataBuffer, 1) != -1) // 经过主车的路径
// //    {
// //        ExcuteAndWait(Back_Off(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
// //    }

// //    AGVComplete_Status.isSet = RESET;
// //    WaitForFlagInMs(AGVComplete_Status.isSet, SET, 25 * 1000); // 等待从车执行入库完成

// //    if (Get_TaskNumber("B2", RFID_DataBuffer, 1) != -1) // 经过主车的路径
// //    {
// //        ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 35), Stop_Flag, FORBACKCOMPLETE);
// //    }

//     ExcuteAndWait(Turn_ByEncoder(90 + 35), Stop_Flag, TURNCOMPLETE);

// //    RotationLED_PlateAndCoord(Get_PlateNumber(), ReCoordinate_Covent(RFID_x, RFID_y));

//     ExcuteAndWait(Turn_ByEncoder(-35), Stop_Flag, TURNCOMPLETE);

//     CurrentStaus.dir = DIR_RIGHT;
// }

// void Task_3_5(void)
// {
//     uint8_t level = (Get_AllColorCount() * (distanceMeasured / 100)) % 4 + 1;

//     ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);
//     ExcuteAndWait(Go_Ahead(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);

//     StreetLight_AdjustTo(level);

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 5), Stop_Flag, FORBACKCOMPLETE);
//     ExcuteAndWait(Turn_ByEncoder(90), Stop_Flag, TURNCOMPLETE);
// }

// void Task_5_5(void)
// {
//     BarrierGate_Task(NULL);
// }

// void Task_3_1_2(void)
// {
//     Voice_Task();
// }

// void Task_F6_2(void)
// {
//     ExcuteAndWait(Turn_ByEncoder(-90), Stop_Flag, TURNCOMPLETE);

//     ExcuteAndWait(Back_Off(30, Centimeter_Value * 30), Stop_Flag, FORBACKCOMPLETE);

//     Send_ZigBeeData(ZigBee_WirelessChargingON);
//     Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 2, 700);
//     End_Task();
// }

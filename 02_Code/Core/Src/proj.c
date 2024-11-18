#include "proj.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

st_route_info_t gstRouteInfo;

// ***********************************************************************
//	@ 函数名称： Proj_Hx711_SelfCheck
//	@ 函数功能： HX711的自检，确定基准值
//	@ 函数输入： 无
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Proj_Hx711_SelfCheck(void)
{
    u32 uiSum[2] = {0, 0};
    for (u8 i = 0; i < 16; i++)
    {
        uiSum[0] += Hx711_Read0();
        uiSum[1] += Hx711_Read1();
    }

    gstRouteInfo.uiHx711Ref[0] = uiSum[0] >> 4;
    gstRouteInfo.uiHx711Ref[1] = uiSum[1] >> 4;
}

// ***********************************************************************
//	@ 函数名称： Proj_Hx711_GetVal
//	@ 函数功能： 计算扭动的偏差值
//	@ 函数输入： 无
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Proj_Hx711_GetVal(void)
{
    gstRouteInfo.uiHx711Raw[0] = Hx711_Read0();
    gstRouteInfo.uiHx711Raw[1] = Hx711_Read1();

    if (gstRouteInfo.uiHx711Raw[0] < ((gstRouteInfo.uiHx711Ref[0] * 3) >> 2))
    {
        gstRouteInfo.uiHx711Raw[0] <<= 1;
        gstRouteInfo.uiHx711Diff[0] = ((s32)((s32)gstRouteInfo.uiHx711Raw[0] - gstRouteInfo.uiHx711Ref[0])) / 2;
    }
    else
    {
        gstRouteInfo.uiHx711Diff[0] = (s32)gstRouteInfo.uiHx711Raw[0] - gstRouteInfo.uiHx711Ref[0];
    }
    if (gstRouteInfo.uiHx711Raw[1] < ((gstRouteInfo.uiHx711Ref[1] * 3) >> 2))
    {
        gstRouteInfo.uiHx711Raw[1] <<= 1;
        gstRouteInfo.uiHx711Diff[1] = ((s32)gstRouteInfo.uiHx711Raw[1] - gstRouteInfo.uiHx711Ref[1]) >> 1;
    }
    else
    {

        gstRouteInfo.uiHx711Diff[1] = (s32)gstRouteInfo.uiHx711Raw[1] - gstRouteInfo.uiHx711Ref[1];
    }
}

// ***********************************************************************
//	@ 函数名称： Motor_SetDir
//	@ 函数功能： 设置两个电机的转向
//	@ 函数输入： EM_MOTOR_GO 或者 EM_MOTOR_BACK
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Motor_SetDir(u8 ubDirL, u8 ubDirR)
{
    HAL_GPIO_WritePin(MOTOR_L_DIR_GPIO_Port, MOTOR_L_DIR_Pin, EM_MOTOR_GO == ubDirL ? GPIO_PIN_RESET : GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOTOR_R_DIR_GPIO_Port, MOTOR_R_DIR_Pin, EM_MOTOR_GO == ubDirR ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// ***********************************************************************
//	@ 函数名称： Motor_Move
//	@ 函数功能： 电机的转动控制
//	@ 函数输入： ubMoveFlag：True则转动，否则是刹车，分别对应L和R
//              uwMoveSpeed: 当ubMoveFlag为TRUE时为前进速度，刹车是为刹车力度，取值范围:[0-4095]
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Motor_Move(u8 ubMoveFlagL, u16 uwMoveSpeedL, u8 ubMoveFlagR, u16 uwMoveSpeedR)
{
    if (ubMoveFlagL) // 左轮刹车
    {
        HAL_GPIO_WritePin(MOTOR_L_STOP_GPIO_Port, MOTOR_L_STOP_Pin, GPIO_PIN_SET); // 低电平为刹车
    }
    else
    {
        HAL_GPIO_WritePin(MOTOR_L_STOP_GPIO_Port, MOTOR_L_STOP_Pin, GPIO_PIN_RESET);
    }

    if (ubMoveFlagR) // 右轮刹车
    {
        HAL_GPIO_WritePin(MOTOR_R_STOP_GPIO_Port, MOTOR_R_STOP_Pin, GPIO_PIN_SET); // 低电平为刹车
    }
    else
    {
        HAL_GPIO_WritePin(MOTOR_R_STOP_GPIO_Port, MOTOR_R_STOP_Pin, GPIO_PIN_RESET);
    }

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uwMoveSpeedL > MOTOR_SPEED_MAX ? MOTOR_SPEED_MAX : uwMoveSpeedL);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, uwMoveSpeedR > MOTOR_SPEED_MAX ? MOTOR_SPEED_MAX : uwMoveSpeedR);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
}

em_sensor_state_t Proj_JudgeMode(s32 uiDiff)
{
    if ((uiDiff < SENSOR_THRED) && (SENSOR_THRED + SENSOR_THRED < 0))
    {
        return EM_SENSOR_STOP;
    }
    else if (uiDiff > SENSOR_THRED)
    {
        return EM_SENSOR_GO;
    }
    else
    {
        return EM_SENSOR_BACK;
    }
}

void Proj_GetAcc(void)
{
    static u8 ubAdcCnt = 0;
    static u16 uwAdcBuf[ADC_FILTER_FRAME];

    HAL_ADC_Start(&hadc1);                 // 启动ADC转换
    HAL_ADC_PollForConversion(&hadc1, 50); // 等待转换完成，50为最大等待时间，单位为ms

    if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC))
    {
        uwAdcBuf[ubAdcCnt] = HAL_ADC_GetValue(&hadc1); // 获取AD值
        // printf("Ad: %4d \r\n", uwAdcBuf[ubAdcCnt]);
    }
    ubAdcCnt += 1;
    if (ubAdcCnt >= ADC_FILTER_FRAME)
    {
        ubAdcCnt = 0;
    }

    u32 uiSum = 0;
    for (u8 i = 0; i < ADC_FILTER_FRAME; i++)
    {
        uiSum += uwAdcBuf[i];
    }
    gstRouteInfo.swAdcVal = uiSum / ADC_FILTER_FRAME;
    // printf("Ad: %4d ", gstRouteInfo.swAdcVal);
}

// ***********************************************************************
//	@ 函数名称： Proj_MotorCtl(void)
//	@ 函数功能： 电机的控制
//	@ 函数输入： 无
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Proj_MotorCtl(void)
{
    Proj_GetAcc();
    
    gstRouteInfo.uiDirDiff = ((s32)gstRouteInfo.uiHx711Diff[0] - (s32)gstRouteInfo.uiHx711Diff[1]) * 0.3; // 左转为正数

    printf("D: %5d ,", gstRouteInfo.uiDirDiff);
    if (gstRouteInfo.uiDirDiff < SENSOR_THRED && gstRouteInfo.uiDirDiff > -SENSOR_THRED)
    {
        gstRouteInfo.uiDirDiff = 0;
    }
    else if (gstRouteInfo.uiDirDiff >= SENSOR_THRED)
    {
        gstRouteInfo.uiDirDiff -= SENSOR_THRED;
    }
    else if (gstRouteInfo.uiDirDiff < -SENSOR_THRED)
    {
        gstRouteInfo.uiDirDiff += SENSOR_THRED;
    }
    printf("Diff: %5d ", gstRouteInfo.uiDirDiff);

    // 先给两个基础速度
    for (u8 i = 0; i < 2; i++)
    {
        if (gstRouteInfo.swAdcVal < STOP_THRED)
        {
            gstRouteInfo.uwTargetSpeed[i] = 0;
        }
        else
        {
            gstRouteInfo.uwTargetSpeed[i] = (gstRouteInfo.swAdcVal - STOP_THRED) / 2;
        }
    }

    u16 uwSpeed[2] = {gstRouteInfo.uwRealSpeed[0], gstRouteInfo.uwRealSpeed[1]};
    if (gstRouteInfo.uiDirDiff > 0)
    {
        if (gstRouteInfo.uwRealSpeed[0] > gstRouteInfo.uiDirDiff)
        {
            uwSpeed[0] = gstRouteInfo.uwRealSpeed[0] - gstRouteInfo.uiDirDiff;
        }
        else
        {
            uwSpeed[0] = 0;
        }
    }
    else
    {
        gstRouteInfo.uiDirDiff = AbsS32(gstRouteInfo.uiDirDiff);
        if (gstRouteInfo.uwRealSpeed[1] > gstRouteInfo.uiDirDiff)
        {
            uwSpeed[1] = gstRouteInfo.uwRealSpeed[1] - gstRouteInfo.uiDirDiff;
        }
        else
        {
            uwSpeed[1] = 0;
        }
    }
    printf("Speed: %d %d\r\n", uwSpeed[0], uwSpeed[1]);
    Motor_Move(0 == uwSpeed[0] ? 0 : 1, uwSpeed[0], 0 == uwSpeed[1] ? 0 : 1, uwSpeed[1]);
}

// ***********************************************************************
//	@ 函数名称： Proj_Init(void)
//	@ 函数功能： Proj的初始化，全部流程的初始化都在这里
//	@ 函数输入： 无
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Proj_Init(void)
{
    MemSetU8((u8 *)&gstRouteInfo, 0, sizeof(st_route_info_t));
    HAL_Delay(100);
    Motor_SetDir(EM_MOTOR_GO, EM_MOTOR_GO);
    Motor_Move(0, 0, 0, 0);

    MX_ADC1_Init();

    Proj_Hx711_SelfCheck();
}

// ***********************************************************************
//	@ 函数名称： Proj_Proc(void)
//	@ 函数功能： 运行的进程函数
//	@ 函数输入： 无
//	@ 函数输出： 无
//	@ 注意事项： 无
// ***********************************************************************
void Proj_Proc(void)
{
    Proj_Hx711_GetVal();
    Proj_MotorCtl();
    // printf("%06d %06d\r\n", gstRouteInfo.uiHx711Diff[0], gstRouteInfo.uiHx711Diff[1]);
    // OutPut_Data(gstRouteInfo.uiHx711Diff[0] / 20, gstRouteInfo.uiHx711Diff[1] / 20, gstRouteInfo.uiHx711Raw[0], gstRouteInfo.uiHx711Raw[1]);
}

void ProcTask(void const *argument)
{
    for (;;)
    {
        Proj_Proc();
        osDelay(5);
    }
}

void SpeedTask(void const *argument)
{
    u8 ubSensorSta[2];
    for (;;)
    {
        for (u8 i = 0; i < CONST_2; i++)
        {
            if (gstRouteInfo.uwRealSpeed[i] < gstRouteInfo.uwTargetSpeed[i])
            {
                gstRouteInfo.uwRealSpeed[i] += (0.12 * abs(gstRouteInfo.uiDirDiff) + SPEED_TASK_STEP);
            }
            else
            {
                if (gstRouteInfo.uwRealSpeed[i] > SPEED_TASK_STEP)
                {
                    gstRouteInfo.uwRealSpeed[i] -= (SPEED_TASK_STEP * 1.5);
                }
                else
                {
                    gstRouteInfo.uwRealSpeed[i] = 0;
                }
            }
        }

        // Motor_Move(1, 600, 1, 600);

        // ubSensorSta[0] = Proj_JudgeMode(gstRouteInfo.uiHx711Diff[0]);
        // ubSensorSta[1] = Proj_JudgeMode(gstRouteInfo.uiHx711Diff[1]);

        // if ((EM_SENSOR_STOP == ubSensorSta[0]) &&
        //     (EM_SENSOR_STOP == ubSensorSta[1]))
        // {
        //     Motor_Move(0 == gstRouteInfo.uwRealSpeed[0] ? 0 : 1, gstRouteInfo.uwRealSpeed[0], 0 == gstRouteInfo.uwRealSpeed[1] ? 0 : 1, gstRouteInfo.uwRealSpeed[1]);
        // }
        // else if ((EM_SENSOR_BACK == ubSensorSta[0]) &&
        //          (EM_SENSOR_BACK == ubSensorSta[1]))
        // {
        //     Motor_Move(0 == gstRouteInfo.uwRealSpeed[0] ? 0 : 1, gstRouteInfo.uwRealSpeed[0], 0 == gstRouteInfo.uwRealSpeed[1] ? 0 : 1, gstRouteInfo.uwRealSpeed[1]);
        // }
        // else
        // {
        //     s32 uiDiff = ((s32)gstRouteInfo.uiHx711Diff[0] - (s32)gstRouteInfo.uiHx711Diff[1]) * 0.15; // 左转为正数
        //     u16 uwSpeed[2];
        //     printf("Diff: %d\r\n", uiDiff);
        //     if (uiDiff > 0)
        //     {
        //         if (gstRouteInfo.uwRealSpeed[0] > uiDiff)
        //         {
        //             uwSpeed[0] = gstRouteInfo.uwRealSpeed[0] - uiDiff;
        //         }
        //         else
        //         {
        //             uwSpeed[0] = 0;
        //         }
        //     }
        //     else
        //     {
        //         uiDiff = AbsS32(uiDiff);
        //         if (gstRouteInfo.uwRealSpeed[1] > uiDiff)
        //         {
        //             uwSpeed[1] = gstRouteInfo.uwRealSpeed[1] - uiDiff;
        //         }
        //         else
        //         {
        //             uwSpeed[1] = 0;
        //         }
        //     }
        //     Motor_Move(0 == gstRouteInfo.uwRealSpeed[0] ? 0 : 1, uwSpeed[0], 0 == gstRouteInfo.uwRealSpeed[1] ? 0 : 1, uwSpeed[1]);
        // }
        osDelay(100);
    }
}

void StartTask03(void const *argument)
{
    for (;;)
    {
        osDelay(1000);
    }
}

void StartTask04(void const *argument)
{
    for (;;)
    {
        osDelay(1000);
    }
}

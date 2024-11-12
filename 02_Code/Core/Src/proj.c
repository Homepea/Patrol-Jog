#include "proj.h"

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

    gstRouteInfo.uiHx711Diff[0] = (s32)gstRouteInfo.uiHx711Ref[0] - gstRouteInfo.uiHx711Raw[0];
    gstRouteInfo.uiHx711Diff[1] = (s32)gstRouteInfo.uiHx711Ref[1] - gstRouteInfo.uiHx711Raw[1];
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
        HAL_GPIO_WritePin(MOTOR_L_STOP_GPIO_Port, MOTOR_L_STOP_Pin, GPIO_PIN_RESET); // 低电平为刹车
    }
    else
    {
        HAL_GPIO_WritePin(MOTOR_L_STOP_GPIO_Port, MOTOR_L_STOP_Pin, GPIO_PIN_SET);
    }

    if (ubMoveFlagR) // 右轮刹车
    {
        HAL_GPIO_WritePin(MOTOR_R_STOP_GPIO_Port, MOTOR_R_STOP_Pin, GPIO_PIN_RESET); // 低电平为刹车
    }
    else
    {
        HAL_GPIO_WritePin(MOTOR_R_STOP_GPIO_Port, MOTOR_R_STOP_Pin, GPIO_PIN_SET);
    }

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, uwMoveSpeedL);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, uwMoveSpeedR);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_2);
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
    OutPut_Data(gstRouteInfo.uiHx711Diff[0], gstRouteInfo.uiHx711Diff[1], gstRouteInfo.uiHx711Raw[0], gstRouteInfo.uiHx711Raw[1]);
}

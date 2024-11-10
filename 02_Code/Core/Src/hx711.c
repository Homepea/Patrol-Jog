#include "hx711.h"

u32 Hx711_Read0(void)
{
    u32 uiValue = 0;
    HX711_SCL_0_L(); // 使能,PD_SCK 置低
    __nop();
    uiValue = 0;
    while (HX711_SDA_0_R()) // AD转换未结束则等待，否则开始读取
    {
    };

    for (u8 i = 0; i < 24; i++)
    {
        HX711_SCL_0_H(); // PD_SCK 置高（发送脉冲）
        uiValue <<= 1;   // 下降沿来时变量uiValue左移一位，右侧补零
        HX711_SCL_0_L(); // PD_SCK 置低
        if (HX711_SDA_0_R())
            uiValue++;
        __nop();
        __nop();
    }
    HX711_SCL_0_H();
    uiValue  ^= 0x800000; // 第25个脉冲下降沿来时，转换数据
    HX711_SCL_0_L();
    return (uiValue);
}

u32 Hx711_Read1(void)
{
    u32 uiValue = 0;
    HX711_SCL_1_L(); // 使能,PD_SCK 置低
    __nop();
    uiValue = 0;
    while (HX711_SDA_1_R()) // AD转换未结束则等待，否则开始读取
    {
    };

    for (u8 i = 0; i < 24; i++)
    {
        HX711_SCL_1_H(); // PD_SCK 置高（发送脉冲）
        uiValue <<= 1;   // 下降沿来时变量uiValue左移一位，右侧补零
        HX711_SCL_1_L(); // PD_SCK 置低
        if (HX711_SDA_1_R())
            uiValue++;
        __nop();
        __nop();
    }
    HX711_SCL_1_H();
    uiValue ^= 0x800000; // 第25个脉冲下降沿来时，转换数据
    HX711_SCL_1_L();
    return (uiValue);
}

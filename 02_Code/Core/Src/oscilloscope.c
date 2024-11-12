#include "oscilloscope.h"
#include "proj.h"

//****************************************************************************
//  函数名：CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT)
//  功能：校验函数
//  说明：无
//*****************************************************************************/
unsigned short CRC_CHECK(unsigned char *Buf, unsigned char CRC_CNT)
{
    unsigned short CRC_Temp;
    unsigned char i, j;
    CRC_Temp = 0xffff;

    for (i = 0; i < CRC_CNT; i++)
    {
        CRC_Temp ^= Buf[i];
        for (j = 0; j < 8; j++)
        {
            if (CRC_Temp & 0x01)
                CRC_Temp = (CRC_Temp >> 1) ^ 0xa001;
            else
                CRC_Temp = CRC_Temp >> 1;
        }
    }
    return (CRC_Temp);
}

//****************************************************************************
//  函数名：OutPut_Data(float a,float b,float c,float d)
//  功能：发送数据函数
//  说明：无
//*****************************************************************************/
void OutPut_Data(float a, float b, float c, float d)
{
    float OutData[4] = {0};
    int temp[4] = {0};
    unsigned int temp1[4] = {0};
    unsigned char databuf[10] = {0};
    unsigned char i;
    unsigned short CRC16 = 0;
    OutData[0] = a;
    OutData[1] = b;
    OutData[2] = c;
    OutData[3] = d;
    for (i = 0; i < 4; i++)
    {
        temp[i] = (int)OutData[i];
        temp1[i] = (unsigned int)temp[i];
    }
    for (i = 0; i < 4; i++)
    {
        databuf[i * 2] = (unsigned char)(temp1[i] & 0xFF);
        databuf[i * 2 + 1] = (unsigned char)(temp1[i] >> 8);
    }

    CRC16 = CRC_CHECK(databuf, 8);
    databuf[8] = CRC16 & 0xFF;
    databuf[9] = CRC16 >> 8;

    for (i = 0; i < 10; i++)
    {
        HAL_UART_Transmit_DMA(&huart4, (uint8_t *)&databuf, 10);
    }
}

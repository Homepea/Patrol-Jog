#ifndef __PROJ_H__
#define __PROJ_H__

#include "common.h"
#include "stdio.h"
#include "adc.h"
#include "dac.h"
#include "usart.h"
#include "hx711.h"
#include "oscilloscope.h"


#define STOP_THRED          1600

typedef enum MOTOR_DIR
{
    EM_MOTOR_GO = 0,
    EM_MOTOR_BACK,
}em_motor_dir_t;

typedef enum SENSOR_STATE
{
    EM_SENSOR_STOP = 0,
    EM_SENSOR_GO,
    EM_SENSOR_BACK,
} em_sensor_state_t;


typedef struct ST_ROUTE_INFO_T
{
    u32 uiHx711Ref[2];
    u32 uiHx711Raw[2];
    s32 uiHx711Diff[2];

} st_route_info_t;

extern void Proj_Init(void);
extern void Proj_Proc(void);


#endif

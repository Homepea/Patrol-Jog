#ifndef __PROJ_H__
#define __PROJ_H__

#include "common.h"
#include "stdio.h"
#include "adc.h"
#include "dac.h"
#include "usart.h"
#include "hx711.h"
#include "oscilloscope.h"


extern void Proj_Init(void);
extern void Proj_Proc(void);


typedef struct ST_ROUTE_INFO_T
{
    u32 uiHx711Ref[2];
    u32 uiHx711Raw[2];
    s32 uiHx711Diff[2];

} st_route_info_t;




#endif

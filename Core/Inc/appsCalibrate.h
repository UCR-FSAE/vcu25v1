#ifndef __APPSCALIBRATE_H
#define __APPSCALIBRATE_H

#ifdef __cplusplus
extern "C" {
#endif

// user includes
#include "main.h"

// defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */

// function prototypes
void appsCalibrate();

extern ADC_HandleTypeDef hadc3;  /* ADC handle from main.c */

#ifdef __cplusplus
}
#endif


#endif

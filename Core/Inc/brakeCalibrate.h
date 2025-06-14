/*
 * brakeCalibrate.h
 *
 *  Created on: Jun 12, 2025
 *      Author: steve
 */

#ifndef __BRAKECALIBRATE_H_
#define __BRAKECALIBRATE_H_


#ifdef __cplusplus
extern "C" {
#endif

// user includes
#include "main.h"

// defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */

// function prototypes
void brakeCalibrate();

extern ADC_HandleTypeDef hadc1;  /* ADC handle from main.c */

#ifdef __cplusplus
}
#endif


#endif /* INC_BRAKECALIBRATE_H_ */

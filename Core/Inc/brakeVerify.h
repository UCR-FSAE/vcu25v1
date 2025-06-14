/*
 * brakeVerify.h
 *
 *  Created on: Jun 13, 2025
 *      Author: Jyro Jimenez
 */

#ifndef __BRAKEVERIFY_H_
#define __BRAKEVERIFY_H_


#ifdef __cplusplus
extern "C" {
#endif

// user includes
#include "main.h"

// defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */

// function prototypes
void brakeVerifyProcess();

extern ADC_HandleTypeDef hadc1;  /* ADC handle from main.c */
extern osMessageQueueId_t appsQueueHandle;

#ifdef __cplusplus
}
#endif


#endif /* INC_BRAKECALIBRATE_H_ */

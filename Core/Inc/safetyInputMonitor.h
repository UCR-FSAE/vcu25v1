/**
  ******************************************************************************
  * @file    safetyInputMonitor.h
  * @brief   FSAE Safety Input Monitor header file
  *          Handles RTD Switch (PE6), RTM Active (PE4), Brake Sensor (PF9/ADC3)
  *          and implements FSAE Ready to Drive logic with buzzer control (PE5)
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SAFETY_INPUT_MONITOR_H
#define __SAFETY_INPUT_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void safetyInputMonitorProcess(void);
bool getRtdButtonState(void);
bool getRtmActiveState(void);
bool getBrakePressedState(void);
bool getReadyToDriveState(void);
void controlBuzzer(bool enable);

/* Private defines -----------------------------------------------------------*/
/* ADC threshold for brake sensor detection (12-bit ADC: 0-4095) */
#define BRAKE_THRESHOLD_PRESSED    2000   /* Above this = brake pressed */

/* FSAE Ready to Drive timing */
#define BUZZER_DURATION_MS         3000   /* 3 seconds buzzer duration */
#define SIGNAL_DEBOUNCE_COUNT      3      /* Number of consistent readings required */

/* GPIO pin states (active-low with pull-up) */
#define GPIO_ACTIVE_LOW_PRESSED    GPIO_PIN_RESET  /* Button pressed = low */
#define GPIO_ACTIVE_LOW_RELEASED   GPIO_PIN_SET    /* Button released = high */

#ifdef __cplusplus
}
#endif

#endif /* __SAFETY_INPUT_MONITOR_H */ 
/**
  ******************************************************************************
  * @file    vcu.h
  * @brief   Vehicle Control Unit header file
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VCU_H
#define __VCU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void VCU_Init(void);
void VCU_Process(void);
void VCU_EnableInverter(void);
void VCU_DisableInverter(void);

/* Private defines -----------------------------------------------------------*/
/* CAN message IDs */
#define VCU_INVERTER_COMMAND_ID 0x0C0

/* Control bytes */
#define VCU_DIRECTION_FORWARD 0x00
#define VCU_DIRECTION_REVERSE 0x01
#define VCU_INVERTER_ENABLE  0x01
#define VCU_INVERTER_DISABLE 0x00

#ifdef __cplusplus
}
#endif

#endif /* __VCU_H */

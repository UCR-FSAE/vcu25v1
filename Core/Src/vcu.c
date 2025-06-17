/**
  ******************************************************************************
  * @file    vcu.c
  * @brief   Vehicle Control Unit implementation
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "vcu.h"
#include <string.h>

// EXTERN DECLARATIONS FOR GLOBAL VARIABLES
extern volatile float global_torque_command;

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define TORQUE_MAX_VALUE  32767   /* 16-bit max torque value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */
#define BRAKE_THRESHOLD   100     /* Threshold to detect brake press */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint16_t acceleratorRaw = 0;
static uint16_t brakeRaw = 0;
static uint16_t torqueCommand = 0;
static uint8_t vcuActive = 0;

/* External variables --------------------------------------------------------*/
// hadc1 = apps pot 1
extern ADC_HandleTypeDef hadc3;  /* ADC handle from main.c */
// hcan1 = high priority can line
extern CAN_HandleTypeDef hcan1;  /* CAN handle from main.c */
extern CAN_HandleTypeDef hcan2;

extern osMessageQueueId_t torqueQueueHandle;


// pedal input
extern uint32_t appsConverted;
extern bool inverterFault;

/* Private function prototypes -----------------------------------------------*/
static void VCU_ProcessAnalogInputs(void);
static void VCU_TransmitCANMessage(uint16_t torque, uint8_t direction, uint8_t inverterEnable);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  Initialize the Vehicle Control Unit
  * @retval None
  */
void VCU_Init(void)
{
  /* Set initial state */
  vcuActive = 0;
  acceleratorRaw = 0;
  brakeRaw = 0;
  torqueCommand = 0;

  /* Configure ADC for accelerator pedal reading */
  /* Note: Main ADC initialization happens in main.c */

  /* Send initial disable message to ensure inverter is off */
//  HAL_ADC_Start(&hadc3);
  VCU_DisableInverter();
  VCU_ClearInverterFaults();
//  HAL_Delay(1000);
  VCU_EnableInverter();
//  HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
//  HAL_Delay(1000);
//  HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
}

/**
  * @brief  Process VCU main functionality (to be called periodically)
  * @retval None
  */
void VCU_Process(void)
{
  /* Read accelerator pedal position from ADC */
//  HAL_ADC_Start(&hadc3);
//  if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK)
//  {
//	acceleratorRaw = HAL_ADC_GetValue(&hadc3);
//  }


  /* In a real system, we would read brake position from another ADC channel */
  /* For now, we simulate no brake press */
  brakeRaw = 0;

//  VCU_EnableInverter();
  /* Process analog inputs and update commands */
  VCU_ProcessAnalogInputs();
}

/**
  * @brief  Process analog inputs to derive control commands
  * @retval None
  */
static void VCU_ProcessAnalogInputs(void)
{
  /* Only process if VCU is active */
  if (!vcuActive)
  {
	return;
  }

  /* Check brake pedal status - if pressed, set torque to zero */
  if (brakeRaw > BRAKE_THRESHOLD)
  {
	torqueCommand = 0;
	VCU_TransmitCANMessage(0, VCU_DIRECTION_FORWARD, VCU_INVERTER_ENABLE);
	return;
  }

//  /* Filter out noise at very low values */
//  if (acceleratorRaw < ADC_THRESHOLD)
//  {
//	torqueCommand = 0;
//  }
//  else
//  {
	/* Convert 12-bit ADC value (0-4095) to torque value (0-32767) */
	/* Apply basic linear mapping for now */

//	torqueCommand = (uint16_t)(((uint32_t)acceleratorRaw * TORQUE_MAX_VALUE) / ADC_MAX_VALUE);
//	  torqueCommand = appsConverted
//  }

//  torqueCommand = 75;
  
  // Read from global variable instead of queue
   float received_torque = global_torque_command;
   static uint32_t no_msg_count = 0;
  
  // Convert float torque (N⋅m) to uint16_t for CAN transmission
  // Scale appropriately for your inverter protocol
  // Limit to reasonable range (0-100 Nm max)
   if (received_torque < 0.0f) received_torque = 0.0f;
   if (received_torque > 100.0f) received_torque = 100.0f;
  
  // Scale to 0-10000 range (instead of 0-3276700 which was causing issues)
   torqueCommand = (uint16_t)(received_torque);  // 0-10000 range
  // no_msg_count = 0;

  
  if (inverterFault == 1) { VCU_TransmitCANMessage(0, VCU_DIRECTION_FORWARD, VCU_INVERTER_DISABLE); }
  else { VCU_TransmitCANMessage(torqueCommand, VCU_DIRECTION_FORWARD, VCU_INVERTER_ENABLE); }
  /* Send CAN message with torque command */

}

/**
  * @brief  Transmit CAN message to inverter
  * @param  torque: Torque command value (0-32767)
  * @param  direction: Direction command (0=forward, 1=reverse)
  * @param  inverterEnable: Inverter enable state (0=disable, 1=enable)
  * @retval None
  */
static void VCU_TransmitCANMessage(uint16_t torque, uint8_t direction, uint8_t inverterEnable)
{
  CAN_TxHeaderTypeDef txHeader;
  uint8_t txData[8];
  uint32_t txMailbox;
  HAL_StatusTypeDef status;

  /* Configure transmission */
  txHeader.StdId = VCU_INVERTER_COMMAND_ID;
  txHeader.ExtId = 0;
  txHeader.IDE = CAN_ID_STD;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.DLC = 8;
  txHeader.TransmitGlobalTime = DISABLE;

  /* Pack torque command (little-endian) */
  txData[0] = (uint8_t)(torque & 0xFF);
  txData[1] = (uint8_t)((torque >> 8) & 0xFF);

  /* Speed command (0 for torque control mode) */
  txData[2] = 0;
  txData[3] = 0;

  /* Direction and inverter control */
  txData[4] = direction;
  txData[5] = inverterEnable;

  /* Torque limits (using default) */
  txData[6] = 0;
  txData[7] = 0;

  /* Check if mailboxes are available */
  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
    // No free mailboxes - abort oldest message and try again
    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX0);
    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX1);
    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX2);
  }

  /* Send CAN message */
  status = HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);
  
  if (status != HAL_OK) {
    // CAN transmission failed
    uint32_t errorCode = HAL_CAN_GetError(&hcan1);
    
    
    // Try to abort and retry once
    if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) {
      Error_Handler();
    }
  }
  else {
     HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
     HAL_Delay(10);
     HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
     HAL_Delay(10);
  }
}

/**
  * @brief  Enable inverter
  * @retval None
  */
void VCU_EnableInverter(void)
{
  /* Set system to active */
  vcuActive = 1;
  /* Send enable message with zero torque command */
  VCU_TransmitCANMessage(0, VCU_DIRECTION_FORWARD, VCU_INVERTER_ENABLE);

  /* Visual indication - could toggle an LED here */
  // DISABLED: LED debugging moved to ADC verify task only
  // HAL_GPIO_TogglePin(GPIOB, 7);
  // HAL_Delay(10);
  // HAL_GPIO_TogglePin(GPIOB, 7);
  // HAL_Delay(10);
}

/**
  * @brief  Disable inverter
  * @retval None
  */
void VCU_DisableInverter(void)
{
  /* Set system to inactive */
  vcuActive = 0;

  /* Send disable message with zero torque command */
  VCU_TransmitCANMessage(0, VCU_DIRECTION_FORWARD, VCU_INVERTER_DISABLE);

  /* Visual indication - could toggle an LED here */
}


void VCU_ClearInverterFaults(void)
{
	  CAN_TxHeaderTypeDef txHeader;
	  uint8_t txData[8];
	  uint32_t txMailbox;
	  HAL_StatusTypeDef status;

	  /* Configure transmission */
	  txHeader.StdId = VCU_INVERTER_CLEAR_ID;
	  txHeader.ExtId = 0;
	  txHeader.IDE = CAN_ID_STD;
	  txHeader.RTR = CAN_RTR_DATA;
	  txHeader.DLC = 8;
	  txHeader.TransmitGlobalTime = DISABLE;

	  txData[0] = (uint8_t) 0x14;
	  txData[1] = 0;

	  txData[2] = 1;
	  txData[3] = 0;

	  /* Direction and inverter control */
	  txData[4] = 0;
	  txData[5] = 0;

	  /* Torque limits (using default) */
	  txData[6] = 0;
	  txData[7] = 0;

	  /* Check if mailboxes are available */
	  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0) {
	    // No free mailboxes - abort oldest message and try again
	    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX0);
	    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX1);
	    HAL_CAN_AbortTxRequest(&hcan1, CAN_TX_MAILBOX2);
	  }

	  /* Send CAN message */
	  status = HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox);

	  if (status != HAL_OK) {
	    // CAN transmission failed
	    uint32_t errorCode = HAL_CAN_GetError(&hcan1);

	    // Try to abort and retry once
	    if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) {
	      Error_Handler();
	    }
	  }
}

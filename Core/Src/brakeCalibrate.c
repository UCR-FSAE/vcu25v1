/*
 * brakeCalibrate.c
 *
 *  Created on: Jun 12, 2025
 *      Author: steve
 */

#include "brakeCalibrate.h"

// min and max pot values for calibration
extern uint32_t brakesRaw1Min;
extern uint32_t brakesRaw1Max;

extern CAN_HandleTypeDef hcan1;

int vb1;

void brakeCalibrate(void) {

	// set up CAN filters
	CAN_TxHeaderTypeDef txHeader;
	uint8_t txData[8];
	uint32_t txMailbox;

	/* Configure transmission */
	txHeader.StdId = 0x01;
	txHeader.ExtId = 0;
	txHeader.IDE = CAN_ID_STD;
	txHeader.RTR = CAN_RTR_DATA;
	txHeader.DLC = 3;
	txHeader.TransmitGlobalTime = DISABLE;

	txData[0] = 0;
	txData[1] = 0x01;
	txData[2] = 0;
	txData[3] = 0;
	txData[4] = 0;
	txData[5] = 0;
	txData[6] = 0;
	txData[7] = 0;


	brakesRaw1Min = 0xFFFF;
	brakesRaw1Max = 0;


	HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);

	// send begin max calibration
	// sends a calibration message to the dashboard for screen toggle
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}
//	else {
//		HAL_GPIO_TogglePin(GPIOB, 14);
//		HAL_Delay(10);
//		HAL_GPIO_TogglePin(GPIOB, 14);
//	}

    HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_SET);

	uint32_t t0 = HAL_GetTick(); // ms since power-up
	// for 3000 ms window
	while (HAL_GetTick() - t0 < 3000) {
		HAL_ADC_Stop(&hadc3);
		if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

		if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
			vb1 = HAL_ADC_GetValue(&hadc3);
			if (vb1 > brakesRaw1Max) brakesRaw1Max = vb1;
		}
		else {
			vb1 = 0; // Indicate error
			HAL_ADC_Stop(&hadc3);
		}
		HAL_Delay(10);
	}

	// send begin min calibraion
    HAL_GPIO_WritePin(GPIOB, LD2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_SET);

	txData[1] = 0;
	txData[2] = 1;
	// sends a calibration message to the dashboard for screen toggle
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}

	t0 = HAL_GetTick();
	while (HAL_GetTick() - t0 < 3000) {
		HAL_ADC_Stop(&hadc3); // Stop any ongoing conversion before starting a new sequence

		if (HAL_ADC_Start(&hadc3) != HAL_OK) {
			Error_Handler();
			break; // Exit calibration loop on error
		}

		if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
			vb1 = HAL_ADC_GetValue(&hadc3); // Get value for Rank 1 (Channel ?)

			// Update min values (CORRECTED LOGIC)
			if (vb1 < brakesRaw1Min) brakesRaw1Min = vb1;
		}
		else {
			vb1 = 0; // Indicate error
			HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
			// Optionally, break the loop:
			// break;
		}
		HAL_Delay(10); // Adjust as needed

	}


    HAL_GPIO_WritePin(GPIOB, LD3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LD1_Pin, GPIO_PIN_RESET);

	txData[0] = 1;
	txData[1] = 0;
	txData[2] = 0;
	// sends a calibration message to the dashboard for screen toggle, return to main gauge cluster
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}

	return;
}

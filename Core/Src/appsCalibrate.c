
#include "appsCalibrate.h"

// min and max pot values for calibration
extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;
extern CAN_HandleTypeDef hcan1;

int v1;
int v2;

void appsCalibrate(void) {

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

	// read accelerator pedal press
	HAL_GPIO_TogglePin(GPIOB, 0);
	//	HAL_Delay(100);

	appsRaw1Min = 0xFFFF;
	appsRaw2Min = 0xFFFF;

	appsRaw1Max = 0;
	appsRaw2Max = 0;


	// send begin max calibration
	// sends a calibration message to the dashboard for screen toggle
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}
	else {
		HAL_GPIO_TogglePin(GPIOB, 14);
		HAL_Delay(10);
		HAL_GPIO_TogglePin(GPIOB, 14);
	}

	HAL_GPIO_TogglePin(GPIOB, LD2_Pin);

	uint32_t t0 = HAL_GetTick(); // ms since power-up
	// for 3000 ms window
	while (HAL_GetTick() - t0 < 3000) {
	    ADC_ChannelConfTypeDef sConfig = {0};
	    uint16_t adc_value = 0;

	    // pot 1
	    HAL_ADC_Stop(&hadc3);

	    sConfig.Channel = ADC_CHANNEL_5;
	    sConfig.Rank = ADC_REGULAR_RANK_1; // Always Rank 1 for a single conversion
	    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; // Adjust based on your sensor and clock
	    sConfig.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) { Error_Handler(); }

	    // Start the ADC conversion
	    if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

	    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) { adc_value = HAL_ADC_GetValue(&hadc3); }
	    else {
	        v1 = 0; // Or some error value
	        HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
	    }
	    v1 = adc_value;
	    if (v1 > appsRaw1Max) appsRaw1Max = v1;

	    // pot 2
	    HAL_ADC_Stop(&hadc3);

	    sConfig.Channel = ADC_CHANNEL_7;
	    sConfig.Rank = ADC_REGULAR_RANK_2; // Always Rank 1 for a single conversion
	    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; // Adjust based on your sensor and clock
	    sConfig.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) { Error_Handler(); }

	     // Start the ADC conversion
	     if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

	     if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) { adc_value = HAL_ADC_GetValue(&hadc3); }
	     else {
	         v2 = 0; // Or some error value
	         HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
	     }
	    v2 = adc_value;
	    if (v2 > appsRaw2Max) appsRaw2Max = v2;
	    HAL_ADC_Stop(&hadc3);
	}

	// send begin min calibraion
	HAL_GPIO_TogglePin(GPIOB, LD2_Pin);


	txData[1] = 0;
	txData[2] = 1;
	// sends a calibration message to the dashboard for screen toggle
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}
	else {
		HAL_GPIO_TogglePin(GPIOB, 14);
		HAL_Delay(10);
		HAL_GPIO_TogglePin(GPIOB, 14);
	}

	HAL_GPIO_TogglePin(GPIOB, LD3_Pin);

	t0 = HAL_GetTick();
	while (HAL_GetTick() - t0 < 3000) {
	    ADC_ChannelConfTypeDef sConfig = {0};
	    uint16_t adc_value = 0;

	    // pot 1
	    HAL_ADC_Stop(&hadc3);

	    sConfig.Channel = ADC_CHANNEL_5;
	    sConfig.Rank = ADC_REGULAR_RANK_1; // Always Rank 1 for a single conversion
	    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; // Adjust based on your sensor and clock
	    sConfig.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) { Error_Handler(); }

	    // Start the ADC conversion
	    if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

	    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) { adc_value = HAL_ADC_GetValue(&hadc3); }
	    else {
	        v1 = 0; // Or some error value
	        HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
	    }
	    v1 = adc_value;
	    if (v1 < appsRaw1Min) appsRaw1Min = v1;

	    // pot 2
	    HAL_ADC_Stop(&hadc3);

	    sConfig.Channel = ADC_CHANNEL_7;
	    sConfig.Rank = ADC_REGULAR_RANK_2; // Always Rank 1 for a single conversion
	    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; // Adjust based on your sensor and clock
	    sConfig.Offset = 0;

	    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) { Error_Handler(); }

	     // Start the ADC conversion
	     if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

	     if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) { adc_value = HAL_ADC_GetValue(&hadc3); }
	     else {
	         v2 = 0; // Or some error value
	         HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
	     }
	    v2 = adc_value;
	    if (v2 < appsRaw2Min) appsRaw2Min = v2;
	    HAL_ADC_Stop(&hadc3);
	}

	HAL_GPIO_TogglePin(GPIOB, LD3_Pin);

	txData[0] = 1;
	txData[1] = 0;
	txData[2] = 0;
	// sends a calibration message to the dashboard for screen toggle, return to main gauge cluster
	if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK) {
		if (HAL_CAN_AbortTxRequest(&hcan1, txMailbox) != HAL_OK) { Error_Handler(); }
	}
	else {
		HAL_GPIO_TogglePin(GPIOB, 14);
		HAL_Delay(10);
		HAL_GPIO_TogglePin(GPIOB, 14);
	}



	//	HAL_GPIO_TogglePin(GPIOB, 0);
	//	HAL_Delay(100);

	return;
}


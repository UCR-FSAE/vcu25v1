
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

		// It's good practice to stop if coming from a different mode or if errors occurred previously,
		// but if ContinuousConvMode is DISABLED, the ADC will stop after one sequence.
		// So, this HAL_ADC_Stop() might be redundant but is harmless.
		HAL_ADC_Stop(&hadc3); // Stop any ongoing conversion before starting a new sequence

		// Start the ADC conversion. This will trigger a conversion for all channels
		// configured in the sequence (ADC_CHANNEL_5 then ADC_CHANNEL_7).
		if (HAL_ADC_Start(&hadc3) != HAL_OK) {
			// Handle error - e.g., call Error_Handler(), perhaps break the loop
			Error_Handler();
			break; // Exit calibration loop on error
		}

		// Poll for conversion completion of the entire sequence of NbrOfConversion channels.
		if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) { // 10ms timeout should be ample
			// Get the converted values.
			// HAL_ADC_GetValue() will return the channels in the order they were ranked
			// during ADC initialization (Rank 1 then Rank 2).
			v1 = HAL_ADC_GetValue(&hadc3); // Get value for Rank 1 (ADC_CHANNEL_5)
			v2 = HAL_ADC_GetValue(&hadc3); // Get value for Rank 2 (ADC_CHANNEL_7)

			// Update maximum values
			if (v1 > appsRaw1Max) appsRaw1Max = v1;
			if (v2 > appsRaw2Max) appsRaw2Max = v2;
		}
		else {
			// Conversion not complete within timeout or error occurred
			// You might want to indicate an error or retry.
			// For calibration, returning 0 or a special value might be useful.
			v1 = 0; // Indicate error
			v2 = 0; // Indicate error
			HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out or had an error
			// Optionally, break the loop:
			// break;
		}

		// If ContinuousConvMode is DISABLED, the ADC stops itself after completing the sequence.
		// No need for an explicit HAL_ADC_Stop(&hadc3) here after the poll, as the next loop iteration
		// will call HAL_ADC_Stop() again before starting.

		// Add a small delay to avoid busy-waiting too aggressively and allow other
		// tasks (if any) or system functions to run, without overly slowing down calibration.
		HAL_Delay(10); // Adjust this delay to control the sampling rate during calibration.
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
		HAL_ADC_Stop(&hadc3); // Stop any ongoing conversion before starting a new sequence

		if (HAL_ADC_Start(&hadc3) != HAL_OK) {
			Error_Handler();
			break; // Exit calibration loop on error
		}

		if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
			v1 = HAL_ADC_GetValue(&hadc3); // Get value for Rank 1 (Channel 5)
			v2 = HAL_ADC_GetValue(&hadc3); // Get value for Rank 2 (Channel 7)

			// Update min values (CORRECTED LOGIC)
			if (v1 < appsRaw1Min) appsRaw1Min = v1;
			if (v2 < appsRaw2Min) appsRaw2Min = v2;
		}
		else {
			v1 = 0; // Indicate error
			v2 = 0; // Indicate error
			HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
			// Optionally, break the loop:
			// break;
		}
		HAL_Delay(10); // Adjust as needed

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

	return;
}


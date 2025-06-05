
#include "appsCalibrate.h"

extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;


void appsCalibrate(void) {
	// read accelerator pedal press
	HAL_GPIO_TogglePin(GPIOB, 0);
//	HAL_Delay(100);

	appsRaw1Min = 0xFFFF;
	appsRaw2Min = 0xFFFF;


	uint32_t t0 = HAL_GetTick();               // ms since power-up
	while (HAL_GetTick() - t0 < 3000)          // 3000 ms window
	{
	    HAL_ADC_Start(&hadc3);                 // start single conversion
	    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK)
	    {
	        uint16_t v1 = HAL_ADC_GetValue(&hadc3);     // ch-1
	        if (v1 < appsRaw1Min) appsRaw1Min = v1;
	    }

	    /* Re-configure hadc3 to second channel if not in scan mode */
	    //HAL_ADC_ConfigChannel(&hadc3,&sConfigCh2);    // optional

	    HAL_ADC_Start(&hadc3);
	    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK)
	    {
	        uint16_t v2 = HAL_ADC_GetValue(&hadc3);     // ch-2
	        if (v2 < appsRaw2Min) appsRaw2Min = v2;
	    }
	}

	appsRaw1Max = 0;
	appsRaw2Max = 0;

	t0 = HAL_GetTick();
	while (HAL_GetTick() - t0 < 3000)
	{
	    HAL_ADC_Start(&hadc3);
	    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK)
	    {
	        uint16_t v1 = HAL_ADC_GetValue(&hadc3);
	        if (v1 > appsRaw1Max) appsRaw1Max = v1;
	    }

	    /* second channel (if needed) */
	    HAL_ADC_Start(&hadc3);
	    if (HAL_ADC_PollForConversion(&hadc3, 5) == HAL_OK)
	    {
	        uint16_t v2 = HAL_ADC_GetValue(&hadc3);
	        if (v2 > appsRaw2Max) appsRaw2Max = v2;
	    }
	}
//	HAL_GPIO_TogglePin(GPIOB, 0);
//	HAL_Delay(100);



	return;
}


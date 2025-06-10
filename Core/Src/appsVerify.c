#include "appsVerify.h"


// user defines
#define ADC_MAX_VALUE     4095    /* 12-bit ADC max value */
#define TORQUE_MAX_VALUE  32767   /* 16-bit max torque value */
#define ADC_THRESHOLD     100     /* Noise threshold for ADC */
#define BRAKE_THRESHOLD   100     /* Threshold to detect brake press */

#define TASK_PERIOD_MS   5u
#define ADC_NUM_CHANNELS 2

// EXTERN DECLARATIONS FOR GLOBAL VARIABLES
extern volatile float global_accel_position;
extern volatile bool global_data_updated;
extern osSemaphoreId_t ADCDataReadyHandle;

extern uint32_t appsRaw1Min;
extern uint32_t appsRaw1Max;
extern uint32_t appsRaw2Min;
extern uint32_t appsRaw2Max;
extern float appsConverted;

extern bool pedalFault;
extern uint32_t adcResults[ADC_NUM_CHANNELS];


static uint32_t appsRaw1 = 0;
static uint32_t appsRaw2 = 0;
static float appsConverted1;
static float appsConverted2;

void appsVerifyProcess(void) {
	static bool offsetFlag = false;
	static TickType_t mismatchStart = 0;
    static uint32_t debug_counter = 0;
    debug_counter++;

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
        adc_value = 0; // Or some error value
        HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
    }
    appsRaw1 = adc_value;

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
         adc_value = 0; // Or some error value
         HAL_ADC_Stop(&hadc3); // Stop the ADC if it timed out
     }
    appsRaw2 = adc_value;
    HAL_ADC_Stop(&hadc3);

    // FIXED: Convert raw values to 0.0-1.0 range properly
    appsConverted1 = (float)(appsRaw1 - appsRaw1Min) / (float)(appsRaw1Max - appsRaw1Min);
    appsConverted2 = (float)(appsRaw2 - appsRaw2Min) / (float)(appsRaw2Max - appsRaw2Min);

    // Clamp to valid range (safety check)
    if (appsConverted1 < 0.0f) appsConverted1 = 0.0f;
    if (appsConverted1 > 1.0f) appsConverted1 = 1.0f;
    if (appsConverted2 < 0.0f) appsConverted2 = 0.0f;
    if (appsConverted2 > 1.0f) appsConverted2 = 1.0f;

    // FIXED: Use float math for sensor agreement check
    float diff = (appsConverted1 > appsConverted2) ?
                 (appsConverted1 - appsConverted2) :
                 (appsConverted2 - appsConverted1);

    const float SENSOR_TOLERANCE = 0.10f;  // 10% tolerance

    if (diff > SENSOR_TOLERANCE && (appsConverted1 > 0.05f || appsConverted2 > 0.05f)) {
        if (!offsetFlag) {
            offsetFlag = true;
            mismatchStart = osKernelGetTickCount();  // Use CMSIS-RTOS
        }
        else if ((osKernelGetTickCount() - mismatchStart) >= 100) { // 100ms in ticks
//            pedalFault = true;
        }
    }
    else {
        offsetFlag = false;
    }

    // FIXED: Calculate the average and convert to proper integer range (0-1000)
    float average = (appsConverted1 + appsConverted2) / 2.0f;
    // FIXED: Store normalized value in global variable (already in 0.0-1.0 range)
    global_accel_position = average;  // Already in 0.0-1.0 range
    global_data_updated = true;

}


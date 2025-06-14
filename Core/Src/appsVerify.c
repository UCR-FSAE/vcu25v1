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
extern volatile bool global_accel_data_updated;
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

    if (HAL_ADC_Start(&hadc3) != HAL_OK) { Error_Handler(); }

    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK) {
        appsRaw1 = HAL_ADC_GetValue(&hadc3);
        appsRaw2 = HAL_ADC_GetValue(&hadc3);
    }
    else {
        appsRaw1 = 0;
        appsRaw2 = 0;
        HAL_ADC_Stop(&hadc3); // stop the ADC if it timed out due to error
    }

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
        	HAL_GPIO_TogglePin(GPIOB, LD1_Pin);
        }
    }
    else {
        offsetFlag = false;
    }

    float average = (appsConverted1 + appsConverted2) / 2.0f;
    global_accel_position = average;  // 0.0-1.0 range
    global_accel_data_updated = true;

}


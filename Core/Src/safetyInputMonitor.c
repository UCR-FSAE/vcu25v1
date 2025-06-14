/**
  ******************************************************************************
  * @file    safetyInputMonitor.c
  * @brief   FSAE Safety Input Monitor implementation
  *          Monitors RTD Switch (PE6), RTM Active (PE4), Brake Sensor (PF9/ADC3)
  *          and implements FSAE Ready to Drive system with buzzer control (PE5)
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "safetyInputMonitor.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {
    RTD_STATE_IDLE,
    RTD_STATE_CHECKING_CONDITIONS,
    RTD_STATE_BUZZER_ACTIVE,
    RTD_STATE_READY_TO_DRIVE
} RTD_StateMachine_t;

/* Private define ------------------------------------------------------------*/
#define TASK_PERIOD_MS              10    /* Task execution period in milliseconds */
#define SIGNAL_DEBOUNCE_COUNT       3     /* Number of consistent readings required */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static bool rtdButtonState = false;
static bool rtmActiveState = false;
static bool brakeState = false;
static bool readyToDriveState = false;

static uint32_t rtdButtonDebounceCount = 0;
static uint32_t rtmActiveDebounceCount = 0;
static uint32_t brakeDebounceCount = 0;

static RTD_StateMachine_t rtdStateMachine = RTD_STATE_IDLE;
static uint32_t buzzerStartTime = 0;
static bool buzzerActive = false;

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc3;  /* ADC3 handle from main.c */

/* Global variables for inter-task communication */
extern volatile bool global_rtd_button_state;
extern volatile bool global_rtm_active_state;
extern volatile bool global_brake_pressed;
extern volatile bool global_ready_to_drive;

/* Private function prototypes -----------------------------------------------*/
static bool readGpioInput(GPIO_TypeDef* port, uint16_t pin);
static bool readBrakeSensor(void);
static void debounceSignal(bool currentReading, bool* signalState, uint32_t* debounceCount);
static void processReadyToDriveStateMachine(void);
static uint32_t getCurrentTimeMs(void);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  Main Safety Input monitor process function (called from FreeRTOS task)
  * @retval None
  */
void safetyInputMonitorProcess(void)
{
    /* Read RTD Switch (PE6) - active low */
    bool rtdButtonReading = (readGpioInput(RTD_BUTTON_GPIO_Port, RTD_BUTTON_Pin) == GPIO_ACTIVE_LOW_PRESSED);
    debounceSignal(rtdButtonReading, &rtdButtonState, &rtdButtonDebounceCount);
    
    /* Read RTM Active (PE4) - active low */
    bool rtmActiveReading = (readGpioInput(RTM_ACTIVE_GPIO_Port, RTM_ACTIVE_Pin) == GPIO_ACTIVE_LOW_PRESSED);
    debounceSignal(rtmActiveReading, &rtmActiveState, &rtmActiveDebounceCount);
    
    /* Read Brake Sensor (PF9/ADC3) */
    bool brakeReading = readBrakeSensor();
    debounceSignal(brakeReading, &brakeState, &brakeDebounceCount);
    
    /* Process Ready to Drive state machine */
    processReadyToDriveStateMachine();
    
    /* Update global variables for other tasks to access */
    global_rtd_button_state = rtdButtonState;
    global_rtm_active_state = rtmActiveState;
    global_brake_pressed = brakeState;
    global_ready_to_drive = readyToDriveState;
}

/**
  * @brief  Get current RTD Button state
  * @retval bool: true if RTD button is pressed, false otherwise
  */
bool getRtdButtonState(void)
{
    return rtdButtonState;
}

/**
  * @brief  Get current RTM Active state  
  * @retval bool: true if RTM is active, false otherwise
  */
bool getRtmActiveState(void)
{
    return rtmActiveState;
}

/**
  * @brief  Get current Brake Pressed state
  * @retval bool: true if brake is pressed, false otherwise
  */
bool getBrakePressedState(void)
{
    return brakeState;
}

/**
  * @brief  Get current Ready to Drive state
  * @retval bool: true if Ready to Drive is active, false otherwise
  */
bool getReadyToDriveState(void)
{
    return readyToDriveState;
}

/**
  * @brief  Control the buzzer (PE5)
  * @param  enable: true to turn on buzzer, false to turn off
  * @retval None
  */
void controlBuzzer(bool enable)
{
    if (enable) {
        HAL_GPIO_WritePin(BUZZER_ACTIVE_GPIO_Port, BUZZER_ACTIVE_Pin, GPIO_PIN_SET);
        buzzerActive = true;
    } else {
        HAL_GPIO_WritePin(BUZZER_ACTIVE_GPIO_Port, BUZZER_ACTIVE_Pin, GPIO_PIN_RESET);
        buzzerActive = false;
    }
}

/**
  * @brief  Read a GPIO input pin state
  * @param  port: GPIO port (e.g., GPIOE)
  * @param  pin: GPIO pin (e.g., GPIO_PIN_6)
  * @retval GPIO_PinState: GPIO_PIN_SET or GPIO_PIN_RESET
  */
static bool readGpioInput(GPIO_TypeDef* port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET);
}

/**
  * @brief  Read brake sensor from ADC3 (PF9)
  * @retval bool: true if brake is pressed (above threshold), false otherwise
  */
static bool readBrakeSensor(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adcValue = 0;
    bool brakePressed = false;
    
    /* Configure ADC channel for brake sensor (PF9 = ADC3_IN7) */
    sConfig.Channel = BRAKE_SENSOR_ADC_CHANNEL;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
    
    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
    {
        /* Configuration error - return safe default */
        return false;
    }
    
    /* Start ADC conversion */
    if (HAL_ADC_Start(&hadc3) != HAL_OK)
    {
        /* Start error - return safe default */
        return false;
    }
    
    /* Wait for conversion to complete */
    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK)
    {
        /* Get ADC value */
        adcValue = HAL_ADC_GetValue(&hadc3);
        
        /* Determine brake state based on threshold */
        brakePressed = (adcValue > BRAKE_THRESHOLD_PRESSED);
    }
    else
    {
        /* Conversion timeout - return safe default */
        brakePressed = false;
    }
    
    /* Stop ADC */
    HAL_ADC_Stop(&hadc3);
    
    return brakePressed;
}

/**
  * @brief  Debounce a digital signal to prevent false triggering
  * @param  currentReading: Current signal reading (true/false)
  * @param  signalState: Pointer to the debounced signal state
  * @param  debounceCount: Pointer to the debounce counter
  * @retval None
  */
static void debounceSignal(bool currentReading, bool* signalState, uint32_t* debounceCount)
{
    if (currentReading == *signalState)
    {
        /* Signal is stable - reset debounce counter */
        *debounceCount = 0;
    }
    else
    {
        /* Signal changed - increment debounce counter */
        (*debounceCount)++;
        
        /* If we have enough consistent readings, update the state */
        if (*debounceCount >= SIGNAL_DEBOUNCE_COUNT)
        {
            *signalState = currentReading;
            *debounceCount = 0;
        }
    }
}

/**
  * @brief  Process the FSAE Ready to Drive state machine
  * @retval None
  */
static void processReadyToDriveStateMachine(void)
{
    uint32_t currentTime = getCurrentTimeMs();
    
    switch (rtdStateMachine) {
        case RTD_STATE_IDLE:
            /* Check if all conditions are met for Ready to Drive */
            if (rtmActiveState && brakeState && rtdButtonState) {
                /* All conditions met - start buzzer and transition */
                controlBuzzer(true);
                buzzerStartTime = currentTime;
                rtdStateMachine = RTD_STATE_BUZZER_ACTIVE;
            }
            readyToDriveState = false;
            break;
            
        case RTD_STATE_BUZZER_ACTIVE:
            /* Check if buzzer duration has elapsed */
            if ((currentTime - buzzerStartTime) >= BUZZER_DURATION_MS) {
                /* Buzzer finished - turn off and set Ready to Drive */
                controlBuzzer(false);
                readyToDriveState = true;
                rtdStateMachine = RTD_STATE_READY_TO_DRIVE;
            }
            /* Check if any condition is no longer met */
            else if (!rtmActiveState || !brakeState || !rtdButtonState) {
                /* Conditions lost - abort sequence */
                controlBuzzer(false);
                readyToDriveState = false;
                rtdStateMachine = RTD_STATE_IDLE;
            }
            break;
            
        case RTD_STATE_READY_TO_DRIVE:
            /* Check if RTM Active is lost (safety requirement) */
            if (!rtmActiveState) {
                /* RTM lost - disable Ready to Drive */
                readyToDriveState = false;
                rtdStateMachine = RTD_STATE_IDLE;
            }
            /* Check if RTD switch is turned OFF (allows driver to disable system) */
            else if (!rtdButtonState) {
                /* RTD switch turned OFF - disable Ready to Drive */
                readyToDriveState = false;
                rtdStateMachine = RTD_STATE_IDLE;
            }
            /* Ready to Drive remains active until RTM is lost or RTD switch turned OFF */
            break;
            
        default:
            /* Invalid state - reset to idle */
            controlBuzzer(false);
            readyToDriveState = false;
            rtdStateMachine = RTD_STATE_IDLE;
            break;
    }
}

/**
  * @brief  Get current system time in milliseconds
  * @retval uint32_t: Current time in milliseconds
  */
static uint32_t getCurrentTimeMs(void)
{
    return HAL_GetTick();
} 
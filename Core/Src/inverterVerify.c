/*
 * inverterVerify.c
 *
 *  Created on: Jun 19, 2025
 *      Author: Justin Im
 */

#include "inverterVerify.h"

void inverterVerification() {
    CAN_RxHeaderTypeDef rxHeader;
    uint8_t rxData[8];
    HAL_StatusTypeDef status;

    // Check FIFO0 for messages
    while (HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) > 0) {
        status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rxHeader, rxData);

        if (status == HAL_OK && rxHeader.DLC >= 8) {
            // Current is in bytes 6 and 7, little endian
            uint16_t current = (uint16_t)(rxData[7] << 8) | rxData[6];
            if (current >= 40) {
                currentFault = true;
            }
        }
    }
}




/*
 * inverterVerify.h
 *
 *  Created on: Jun 19, 2025
 *      Author: Justin Im
 */

#ifndef INC_INVERTERVERIFY_H_
#define INC_INVERTERVERIFY_H_

#include "main.h"

extern bool currentFault;
extern CAN_HandleTypeDef hcan1;

void inverterVerification(void);



#endif /* INC_INVERTERVERIFY_H_ */

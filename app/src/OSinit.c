﻿/*!****************************************************************************
 * @file		OSinit.c
 * @author		d_el - Storozhenko Roman
 * @version		V1.0
 * @copyright	GNU Lesser General Public License v3
 * @brief		Create semaphore, mutex, nitialization operational system
 */

/*!****************************************************************************
 * Include
 */
#include "OSinit.h"
#include "stm32f4x7_eth.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "systemTSK.h"
#include "assert.h"

/*!****************************************************************************
 * @brief Init operating system
 */
void OSinit(void){
	BaseType_t Result = pdTRUE;

	Result &= xTaskCreate(systemTSK, "systemTSK", SYSTEM_TSK_SZ_STACK, NULL, SYSTEM_TSK_PRIO, NULL);
	assert(Result == pdTRUE);
	vTaskStartScheduler();
}

/*************** LGPL ************** END OF FILE *********** D_EL ************/
﻿/*!****************************************************************************
 * @file		regulatorConnTSK.c
 * @author		d_el
 * @version		V1.1
 * @date		13.12.2017
 * @copyright	The MIT License (MIT). Copyright (c) 2017 Storozhenko Roman
 * @brief		connect interface with regulator
 */

/*!****************************************************************************
 * Include
 */
#include "string.h"
#include "assert.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "crc.h"
#include "uart.h"
#include "systemTSK.h"
#include "regulatorConnTSK.h"

/*!****************************************************************************
 * MEMORY
 */
static QueueHandle_t 		queueCommand;
static SemaphoreHandle_t 	regulatorConnUartRxSem;
uartTsk_type 				uartTsk = { .state = uartUndef };

/******************************************************************************
 * Local function declaration
 */
static void uartTskHook(uart_type *puart);

/*!****************************************************************************
 * @brief
 */
void regulatorConnTSK(void *pPrm){
	(void)pPrm;
	TickType_t	xLastWakeTime = xTaskGetTickCount();
	BaseType_t 	res;
	uint16_t 	crc;
	uint16_t 	errPrev = 0;
	uint16_t 	noAnswerPrev = 0;

	vTaskDelay(1000);

	// Create a queue
	queueCommand = xQueueCreate(UART_TSK_QUEUE_COMMAND_LEN, sizeof(request_type));
	assert(queueCommand != NULL);

	// Create Semaphore for UART
	vSemaphoreCreateBinary(regulatorConnUartRxSem);
	assert(regulatorConnUartRxSem != NULL);
	xSemaphoreTake(regulatorConnUartRxSem, portMAX_DELAY);

	uart_setCallback(uartTskUse, (uartCallback_type)NULL, uartTskHook);

	while(1){
		uartTsk.queueLen = uxQueueMessagesWaiting(queueCommand);

		crc = crc16Calc(&crcModBus, &fp.tf.task, sizeof(task_type));
		memcpy(uartTskUse->pTxBff, &fp.tf.task, sizeof(task_type));
		*(uint16_t*) (uartTskUse->pTxBff + sizeof(task_type)) = crc;
		uart_write(uartTskUse, uartTskUse->pTxBff, sizeof(task_type) + sizeof(uint16_t));

		uart_read(uartTskUse, uartTskUse->pRxBff, sizeof(psState_type) + sizeof(meas_type) + sizeof(uint16_t));
		res = xSemaphoreTake(regulatorConnUartRxSem, pdMS_TO_TICKS(UART_TSK_MAX_WAIT_ms));
		if(res == pdTRUE){
			// Receive answer
			crc = crc16Calc(&crcModBus, uartTskUse->pRxBff, sizeof(psState_type) + sizeof(meas_type) + sizeof(uint16_t));
			if(crc == 0){
				// Queue command
				request_type request;
				res = xQueueReceive(queueCommand, &request, 0);
				if(res == pdPASS){
					fp.tf.task.request = request;
				}else{
					fp.tf.task.request = setNone;
				}
				memcpy(&fp.tf.state, uartTskUse->pRxBff, sizeof(psState_type) + sizeof(meas_type));
				uartTsk.normAnswer++;
				errPrev = uartTsk.errorAnswer;
				noAnswerPrev = uartTsk.noAnswer;
				uartTsk.state = uartConnect;
			}else{
				uartTsk.errorAnswer++;
				if((uartTsk.errorAnswer - errPrev) > UART_TSK_MAX_ERR){
					uartTsk.state = uartNoConnect;
				}
			}
		}else{
			// Timeout
			uartTsk.noAnswer++;
			if((uartTsk.noAnswer - noAnswerPrev) > UART_TSK_MAX_ERR){
				uartTsk.state = uartNoConnect;
			}
		}

		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(UART_TSK_PERIOD));
	}
}

/*!****************************************************************************
 * @brief
 */
uint8_t sendCommand(request_type command){
	BaseType_t res;

	if(queueCommand == NULL){
		return 1;
	}

	res = xQueueSend(queueCommand, (void*)&command, 0);
	if(res == pdPASS){
		return 0;
	}else{
		return 1;
	}
}

/*!****************************************************************************
 * @brief
 */
uint8_t waitForTf(void){
	uint32_t l_normAnswer = uartTsk.normAnswer;
	uint32_t l_noAnswer = uartTsk.noAnswer;
	uint32_t l_errorAnswer = uartTsk.errorAnswer;
	uint32_t cnt = 3;		// 3 attempt for failure connect

	while(cnt != 0){
		if(l_normAnswer < uartTsk.normAnswer){
			return 0;
		}
		if(l_noAnswer != uartTsk.noAnswer){
			l_noAnswer = uartTsk.noAnswer;
			cnt--;
		}
		if(l_errorAnswer != uartTsk.errorAnswer){
			l_errorAnswer = uartTsk.errorAnswer;
			cnt--;
		}
	}
	return 1;
}

/*!****************************************************************************
 * @brief	uart RX & TX callback
 */
static void uartTskHook(uart_type *puart){
	(void)puart;
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(regulatorConnUartRxSem, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/******************************** END OF FILE ********************************/

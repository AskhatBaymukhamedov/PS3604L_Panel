﻿/*!****************************************************************************
 * @file		uart.h
 * @author		d_el - Storozhenko Roman
 * @version		V1.5
 * @date    	09.01.2016
 * @copyright	GNU Lesser General Public License v3
 * @brief		Driver for uart STM32L4 MCUs
 *
 * @history 26.03.2016 - remade for new gpio driver
 * @history 24.09.2016 - rx isr, uart write
 * @history 05.08.2017 - make callback from pointers
 */
#ifndef UART_H
#define UART_H

/*!****************************************************************************
 * Include
 */
#include "stm32f4xx.h"
#include "gpio.h"
#include "stdint.h"
#include "stdio.h"

/*!****************************************************************************
 * Define
 */
//UART1
#define     UART1_USE                   (1)
#define     UART1_TxBffSz               (64)
#define     UART1_RxBffSz               (64)
#define     UART1_RxDmaInterruptPrior   (15)
#define     UART1_TXIRQPrior            (15)
#define     UART1_PINAFTX               (7)
#define     UART1_PINAFRX               (7)
#define     UART1_HALFDUPLEX            (0)
#define     UART1_RX_IDLE_LINE_MODE     (1)

//UART3
#define     UART3_USE                   (1)
#define     UART3_TxBffSz               (64)
#define     UART3_RxBffSz               (64)
#define     UART3_RxDmaInterruptPrior   (15)
#define     UART3_TXIRQPrior            (15)
#define     UART3_PINAFTX               (7)
#define     UART3_PINAFRX               (7)
#define     UART3_HALFDUPLEX            (0)
#define     UART3_RX_IDLE_LINE_MODE     (1)

//UART4
#define     UART4_USE                   (1)
#define     UART4_TxBffSz               (64)
#define     UART4_RxBffSz               (64)
#define     UART4_RxDmaInterruptPrior   (15)
#define     UART4_TXIRQPrior            (15)
#define     UART4_PINAFTX               (8)
#define     UART4_PINAFRX               (8)
#define     UART4_HALFDUPLEX            (0)
#define     UART4_RX_IDLE_LINE_MODE     (1)

/*!****************************************************************************
 * Enumeration
 */

/******************************************************************************
 * Typedef
 */
typedef enum {
	BR9600,
	BR38400,
	BR57600,
	BR115200,
	BR_NUMBER
} uartBaudRate_type;

typedef enum {
	uartTxFree,
	uartTxRun,
	uartTxSuccess,
	uartTxErr
} uartTxState_type;

typedef enum {
	uartRxFree,
	uartRxRun,
	uartRxSuccess,
	uartRxStop,
	uartRxErr
} uartRxState_type;

typedef struct uartStruct{
	USART_TypeDef 				*pUart;
	uint8_t 					*pTxBff;
	uint8_t 					*pRxBff;
	DMA_Stream_TypeDef 		    *pDmaStreamTx;		///< DMA Controller Tx
	DMA_Stream_TypeDef 		    *pDmaStreamRx;		///< DMA Controller Rx
	volatile uint32_t 			*dmaIfcrTx;  		///< DMA interrupt flag clear register Tx
	volatile uint32_t 			*dmaIfcrRx;  		///< DMA interrupt flag clear register Rx
	uint32_t 					dmaIfcrMaskTx;  	///< DMA interrupt flag clear register mask Tx
	uint32_t 					dmaIfcrMaskRx;  	///< DMA interrupt flag clear register mask Rx
	void (*txHoock)(struct uartStruct *uart);
	void (*rxHoock)(struct uartStruct *uart);
	volatile uartTxState_type 	txState :8;
	volatile uartRxState_type 	rxState :8;
	uartTxState_type 			baudRate :4;
	uint8_t 					halfDuplex :1;
	uint8_t 					rxIdleLineMode :1;
	volatile uint16_t 			txCnt;
	volatile uint16_t 			rxCnt;
} uart_type;

typedef void (*uartCallback_type)(uart_type *uart);

/*!****************************************************************************
 * Exported variables
 */
#if (UART1_USE == 1)
extern uart_type *uart1;
#endif //UART1_USE

#if (UART3_USE == 1)
extern uart_type *uart3;
#endif //UART3_USE

#if (UART4_USE == 1)
extern uart_type *uart4;
#endif //UART4_USE

/*!****************************************************************************
 * Macro functions
 */
#define uartGetRemainTx(uartx)      (uartx->pDmaStreamTx->NDTR)
#define uartGetRemainRx(uartx)      (uartx->pDmaStreamRx->NDTR)
#define UART_FREQ 					(42000000U)		///< [Hz]
#define uartMakeMantissa(baud)		(UART_FREQ / 16 / (baud))
#define uartMakeFraction(baud)		(((UART_FREQ + (baud) / 2)  / (baud)) - (uartMakeMantissa(baud) * 16))
#define uartMakeBrr(baud)			(uartMakeMantissa(baud) << USART_BRR_DIV_Mantissa_Pos | uartMakeFraction(baud))


/*!****************************************************************************
 * Prototypes for the functions
 */
void uart_init(uart_type *uartx, uartBaudRate_type baudRate);
void uart_deinit(uart_type *uartx);
void uart_setBaud(uart_type *uartx, uartBaudRate_type baudRate);
void uart_setCallback(uart_type *uartx, uartCallback_type txHoock, uartCallback_type rxHoock);
void uart_write(uart_type *uartx, void *src, uint16_t len);
void uart_read(uart_type *uartx, void *dst, uint16_t len);
void uart_stopRead(uart_type *uartStruct);

#endif //UART_H
/*************** LGPL ************** END OF FILE *********** D_EL ************/

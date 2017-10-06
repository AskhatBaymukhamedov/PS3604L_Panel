﻿/*!****************************************************************************
 * @file		drivers.c
 * @author		d_el - Storozhenko Roman
 * @version		V1.0
 * @date		05-07-2013
 * @copyright	GNU Lesser General Public License v3
 * @brief		MCU peripheral initialization
 */

/*!****************************************************************************
 * Include
 */
#include "drivers.h"

#include "uniqueDeviceId.h"
uint8_t macAddress[6];
void macAddressSet(void){
	uint32_t mach = makeID(getDid());
	macAddress[0] = 0x28;
	macAddress[1] = 0x05;
	macAddress[2] = ( mach >> 0)  & 0xFF;
	macAddress[3] = ( mach >> 8)  & 0xFF;
	macAddress[4] = ( mach >> 16) & 0xFF;
	macAddress[5] = ( mach >> 24) & 0xFF;
}

/*!****************************************************************************
 * MCU peripheral initialization
 */
void hardInit(void){
	clock_init();
	gpio_init();

	ETH_BSP_Config();	//configure Ethernet (GPIOs, clocks, MAC, DMA)
	sysTimeMeasEnable();
	enco_init();
	//beep_init();
	//ledPwm_init();
	spfd_init();
	//ssd_init();
	rtcStatus_type rtcStatus = rtc_init();
	if((rtcStatus == rtc_Ok)&&(rtcStatus == rtc_wasOn)){
		fp.state.rtcOscillatorError = 0;
	}else{
		fp.state.rtcOscillatorError = 1;
	}
	//i2c_init(i2c2);
	uart_init(uart1, BR38400);
	uart_init(uart3, BR57600);
	uart_init(uart4, BR115200);
	//pvd_init();
	macAddressSet();
}

/*************** LGPL ************** END OF FILE *********** D_EL ************/
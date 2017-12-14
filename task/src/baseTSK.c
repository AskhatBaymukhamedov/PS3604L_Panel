﻿/*!****************************************************************************
 * @file		base.h
 * @author		d_el - Storozhenko Roman
 * @version		V1.0
 * @date		01.01.2015
 * @copyright	GNU Lesser General Public License v3
 * @brief		This task is base GUI
 */
#include "baseTSK.h"

/******************************************************************************
 * Memory
 */
base_type	bs;
uint32_t 	timebs_us __attribute((used));

/******************************************************************************
 * Base task
 */
void baseTSK(void *pPrm){
	TickType_t 				xLastWakeTime = xTaskGetTickCount();
	TickType_t 				IdleTime = xTaskGetTickCount();
	const prmHandle_type 	*sHandle = &prmh[NbsSet0u];
	const prmHandle_type 	*pHandle = &prmh[NbsSet0u];
	baseVar_type 			varParam = VAR_VOLT;
	uint32_t 				measV;	//[uV]
	uint32_t 				measI;	//[uA]
	uint8_t 				bigstepUp = 0;
	uint8_t 				bigstepDown = 0;
	uint8_t 				setDef = 0;
	enStatus_type 			enstatus;
	char 					str[30];

	disp_setColor(black, red);
	disp_fillScreen(black);
	//Печать статических символов
	grf_line(0, 107, 159, 107, halfLightGray);
	ksSet(30, 10, kUp | kDown);
	enSetNtic(2);

	while(1){
		gppin_set(GP_LED3);
		sysTimeMeasStart(sysTimeBs);

		/**************************************
		 * Обработка кнопок
		 */
		if(keyProc() != 0){
			BeepTime(ui.beep.key.time, ui.beep.key.freq);
			IdleTime = xTaskGetTickCount();

			if(keyState(kNext)){
				varParam++;
				if(varParam >= 3){
					varParam = VAR_VOLT;
				}
			}else if(keyState(kMode)){
				if(fp.tf.state.bit.switchIsON == 0){
					selWindow(chargerWindow);
				}else{
					BeepTime(ui.beep.error.time, ui.beep.error.freq);
				}
			}else if(keyState(kView)){                        	//Переключаем набор параметров
				if(fp.tf.state.bit.switchIsON == 0){    //Если выключен выход
					bs.curPreSet++;
					if(bs.curPreSet >= NPRESET){
						bs.curPreSet = 0;
					}
				}else{      //Если включен выход
					BeepTime(ui.beep.error.time, ui.beep.error.freq);
				}
			}else if(keyState(kOnOff)){
				uint8_t result;
				if(fp.tf.state.bit.switchIsON == 0){
					result = sendCommand(setSwitchOn);
				}else{
					result = sendCommand(setSwitchOff);
				}
				if(result != 0){
					disp_putStr(0, 0, &arial, 0, "Error Connect");
					vTaskDelay(1000);
				}
			}else if(keyState(kUp)){
				bigstepUp = 1;
			}else if(keyState(kDown)){
				bigstepDown = 1;
			}else if(keyState(kZero)){
				setDef = 1;
			}
		}

		/***************************************
		 * Вынимаем значение с энкодера
		 */
		sHandle = pHandle + (bs.curPreSet * 3) + varParam;
		if(bigstepUp != 0){
			enstatus = enBigStepUp(sHandle);
			bigstepUp = 0;
		}else if(bigstepDown != 0){
			enstatus = enBigStepDown(sHandle);
			bigstepDown = 0;
		}else if(setDef != 0){
			enWriteVal(sHandle, &(sHandle)->def);
			enstatus = enCharge;
			setDef = 0;
		}else{
			enstatus = enUpDate(sHandle);
		}
		if((enstatus == enLimDown) || (enstatus == enLimUp)){
			BeepTime(ui.beep.encoLim.time, ui.beep.encoLim.freq);
		}else if((enstatus == enTransitionDown) || (enstatus == enTransitionUp)){
			BeepTime(ui.beep.encoTransition.time, ui.beep.encoTransition.freq);
		}

		/***************************************
		 * Задание регулятора
		 */
		fp.tf.task.u = bs.set[bs.curPreSet].u * 1000;
		switch(bs.set[bs.curPreSet].mode){
			case (baseImax): {
				fp.tf.task.mode = mode_overcurrentShutdown;
				fp.tf.task.i = bs.set[bs.curPreSet].i * 1000;
			}
				break;
			case (baseILimitation): {
				fp.tf.task.mode = mode_limitation;
				fp.tf.task.i = bs.set[bs.curPreSet].i * 1000;
			}
				break;
			case (baseUnprotected): {
				fp.tf.task.mode = mode_limitation;
				fp.tf.task.i = I_SHORT_CIRCUIT;
			}
				break;
		}

		/**************************************
		 * Перекладываем измеренные данные
		 */
		measV = fp.tf.meas.u;
		if(measV > 99999999){
			measV = 99999999;
		}
		measI = fp.tf.meas.i;
		if(measI > 9999999){
			measI = 9999999;
		}

		/**************************************
		 * Запуск заставки
		 */
		if(fp.tf.state.bit.switchIsON != 0){
			IdleTime = xTaskGetTickCount();
		}
		if((xTaskGetTickCount() - IdleTime) >= IDLE_TIME){
			if((xTaskGetTickCount() % 2) != 0){
				selWindow(cube3dWindow);
			}else{
				selWindow(bubblesWindow);
			}
		}

		/**************************************
		 * Вывод на дисплей
		 */
		//Печать значения напряжения
		if(fp.tf.state.bit.switchIsON != 0){
			sprintf(str, "%02u.%03u", measV / 1000000, (measV / 1000) % 1000);
		}else{
			sprintf(str, "%02u.%03u", bs.set[bs.curPreSet].u / 1000, bs.set[bs.curPreSet].u % 1000);
		}
		if(varParam == VAR_VOLT){
			disp_setColor(black, ui.color.cursor);
		}else{
			disp_setColor(black, ui.color.voltage);
		}
		disp_putStr(16, 0, &dSegBold, 6, str);
		disp_putChar(150, 18, &font8x12, 'V');

		//Печать значения тока
		if(varParam == VAR_CURR){
			disp_setColor(black, ui.color.cursor);
		}else{
			disp_setColor(black, ui.color.current);
		}

		if(fp.tf.state.bit.switchIsON != 0){
			if(measI < 99000){
				sprintf(str, "%2u.%03u", measI / 1000, measI % 1000);
				disp_putChar(150, 36, &font8x12, 'm');
				disp_putChar(150, 49, &font8x12, 'A');
			}else{
				sprintf(str, "%2u.%03u", measI / 1000000, (measI / 1000) % 1000);
				disp_putChar(150, 36, &font8x12, ' ');
				disp_putChar(150, 49, &font8x12, 'A');
			}

		}else{
			strcpy(str, "--.---");
			disp_putChar(150, 36, &font8x12, ' ');
			disp_putChar(150, 49, &font8x12, ' ');
		}
		disp_putStr(16, 36, &dSegBold, 6, str);

		//Печать текущий набор настроек
		switch(bs.curPreSet){
			case 0:
				grf_fillRect(0, 104, 53, 3, white);
				grf_fillRect(105, 104, 55, 3, black);
				break;
			case 1:
				grf_fillRect(53, 104, 53, 3, white);
				grf_fillRect(0, 104, 53, 3, black);
				break;
			case 2:
				grf_fillRect(105, 104, 55, 3, white);
				grf_fillRect(53, 104, 53, 3, black);
				break;
		}

		//Печать значения Imax
		if(varParam == VAR_CURR){
			disp_setColor(black, ui.color.cursor);
		}else{
			disp_setColor(black, ui.color.imax);
		}
		sprintf(str, "Lim = %2u.%03u A ", bs.set[bs.curPreSet].i / 1000, bs.set[bs.curPreSet].i % 1000);
		disp_putStr(16, 70, &arial, 0, str);

		//Печать режима по току
		if(varParam == VAR_MODE){
			disp_setColor(black, ui.color.cursor);
		}else{
			disp_setColor(black, ui.color.mode);
		}
		if(bs.set[bs.curPreSet].mode == baseImax){
			disp_putStr(16, 88, &arial, 0, "I max            ");
		}
		if(bs.set[bs.curPreSet].mode == baseILimitation){
			disp_putStr(16, 88, &arial, 0, "Limitation     ");
		}
		if(bs.set[bs.curPreSet].mode == baseUnprotected){
			disp_putStr(16, 88, &arial, 0, "Unprotected");
		}

		//Параметры нагрузки, время
		printStatusBar();

		//Измерение времени выполнения одной итерации задачи
		sysTimeMeasStop(sysTimeBs);
		timebs_us = sysTimeMeasGet_us(sysTimeBs);
		gppin_reset(GP_LED3);

		/*************************************/
		//vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BASE_TSK_PERIOD));                       //Запускать задачу каждые 30ms
		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

/*!****************************************************************************
 *
 */
void printStatusBar(void){
	static uint8_t	errPrev = 0;
	static uint8_t	modeIlimPrev = 0;
	static uint8_t	ovfCurrent = 0;
	char 			str[30];

	if(modeIlimPrev != fp.tf.state.bit.modeIlim){
		if(fp.tf.state.bit.modeIlim != 0){
			BeepTime(ui.beep.cvToCc.time, ui.beep.cvToCc.freq);
		}else{
			BeepTime(ui.beep.ccToCv.time, ui.beep.ccToCv.freq);
		}
		modeIlimPrev = fp.tf.state.bit.modeIlim;
	}

	if((fp.tf.state.bit.ovfCurrent != 0) && (ovfCurrent == 0)){
		BeepTime(ui.beep.ovfCurrent.time, ui.beep.ovfCurrent.freq);
	}
	ovfCurrent = fp.tf.state.bit.ovfCurrent;

	if((fp.tf.state.bit.errorLinearRegTemperSens != 0) || (fp.tf.state.bit.ovfLinearRegTemper != 0) || (fp.tf.state.bit.reverseVoltage != 0)
			|| (uartTsk.state == uartNoConnect)){
		BeepTime(ui.beep.error.time, ui.beep.error.freq);
		disp_setColor(black, white);
		if(errPrev == 0){
			grf_fillRect(0, 108, 160, 19, black);
			errPrev = 0;
		}

		if(fp.tf.state.bit.errorLinearRegTemperSens != 0){
			disp_putStr(16, 112, &arial, 0, "Err Temp Sensor");
		}else if(fp.tf.state.bit.ovfLinearRegTemper != 0){
			disp_putStr(16, 112, &arial, 0, "Overflow Temp");
		}else if(fp.tf.state.bit.reverseVoltage != 0){
			disp_putStr(16, 112, &arial, 0, "Reverse Voltage");
		}else if(uartTsk.state == uartNoConnect){
			disp_putStr(35, 112, &arial, 0, "No Connect");
		}

		errPrev = 1;
	}else{  //Аварий нету
		if(errPrev != 0){
			grf_fillRect(0, 108, 160, 19, black);
			errPrev = 0;
		}
		disp_setColor(black, white);

		//Выходная мощность
		sprintf(str, "%02u.%03u W", fp.tf.meas.power / 1000, fp.tf.meas.power % 1000);
		disp_putStr(0, 110, &font6x8, 0, str);

		//Сопротивление нагузки
		if(fp.tf.meas.resistens != 99999){
			sprintf(str, "%05u  \xB1", fp.tf.meas.resistens);
			disp_putStr(0, 120, &font6x8, 0, str);
		}else{
			disp_putStr(0, 120, &font6x8, 0, " ---   \xB1");
		}

		//Печать температуры
		sprintf(str, "%02u.%u \xB0 C", fp.tf.meas.temperatureLin / 10, fp.tf.meas.temperatureLin % 10);
		disp_putStr(60, 120, &font6x8, 0, str);

		//Печать времени
		time_t unixTime = time(NULL);
		unixTime = unixTime + fp.fpSet.timezone * 60 * 60;

		struct tm tmUtc;
		gmtime_r(&unixTime, &tmUtc);

		struct tm tmLocal;
		localtime_r(&unixTime, &tmLocal);

		//println("tmUtc.tm_hour %u", tmUtc.tm_hour);
		//println("tmLocal.tm_hour %u", tmLocal.tm_hour);

		strftime(str, sizeof(str), "%H:%M:%S", &tmUtc);
		disp_putStr(110, 110, &font6x8, 0, str);
		strftime(str, sizeof(str), "%d.%m.%y", &tmUtc);
		disp_putStr(110, 120, &font6x8, 0, str);

		//LAN
		static TickType_t xTime;
		static uint8_t ledon = 0;
		if(fp.state.lanLink != 0){
			if(fp.state.lanActive != 0){
				xTime = xTaskGetTickCount();
				ledon = 1;
				fp.state.lanActive = 0;
			}
			if((ledon != 0)&&((xTaskGetTickCount() - xTime) >= 200)){
				ledon = 0;
			}

			if(ledon == 0){
				disp_setColor(black, white);
			}else{
				disp_setColor(black, red);
			}

			sprintf(str, "LAN");
			disp_putStr(60, 110, &font6x8, 0, str);
		}
		else{
			sprintf(str, "    ");
			disp_putStr(60, 110, &font6x8, 0, str);
		}

		errPrev = 0;
	}
}




/**************************************
 * Настройка клавиатуры
 */
/*if(fvarParamNew != 0){
 if(varParam == VAR_VOLT){
 kc.AutoPress = AutoPressON;                                         //Разрешить автонажатие
 }
 if(varParam == VAR_CURR){
 kc.AutoPress = AutoPressON;
 }
 if(varParam == VAR_MODE){
 kc.AutoPress = AutoPressOFF;
 }
 }*/

/*************** LGPL ************** END OF FILE *********** D_EL ************/

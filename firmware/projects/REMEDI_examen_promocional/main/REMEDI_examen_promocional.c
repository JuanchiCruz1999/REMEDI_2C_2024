/*! @mainpage REMEDI EXAMEN PROMOCIONAL
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Juan Cruz REMEDI (juan.remedi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/

#define PERIODO_TITILEO_LUZ 500 * 1000
#define PERIODO_ALARMA_PRECAUCION 1000 * 1000
#define PERIODO_ALARMA_PELIGRO 5000 * 1000
#define PELIGRO
#define PRECAUCION
/*==================[internal data definition]===============================*/

uint16_t distancia = 0;

#define GPIO_ALARMA_PELIGRO
#define GPIO_ALARMA_PRECAUCION
/*==================[internal functions declaration]=========================*/

static void MedirDistancia(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            distancia = HcSr04ReadDistanceInCentimeters();
			if(distancia<500 && distancia >300)
            UartSendString(UART_PC, "PRECAUCION, VEHICULO CERCA.");
			else if (distancia < 300)
			UartSendString(UART_PC, "PELIGRO, VEHICULO CERCA.");
			
        }
    }
}

static void PrenderLEDS(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (distancia >= 500)
		{
			LedOn(LED_1);
		}
		else if (distancia < 500 && distancia >= 300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
		}
		else if (distancia < 300)
		{
			LedOn(LED_1);
			LedOn(LED_2);
			LedOn(LED_3);
		}

		//        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
	}
}

void sonar_buzzer(void *pvParameter)
{
	if (PRECAUCION)
	{
		GPIOOn(GPIO_ALARMA_PRECAUCION);
		vTaskDelay(PERIODO_ALARMA_PRECAUCION / portTICK_PERIOD_MS);
	}
	else if (PELIGRO)
	{
		GPIOOn(GPIO_ALARMA_PRECAUCION);
		vTaskDelay(PERIODO_ALARMA_PELIGRO / portTICK_PERIOD_MS);
	}
}

void detectar_caida(void *pvParameter){

	uint8_t lecturaG_ejeX,lecturaG_ejeY,lecturaG_ejeZ
	while(true){
        AnalogInputReadSingle(CH1,&lecturaG_ejeX);
        AnalogInputReadSingle(CH2,&lecturaG_ejeY);
        AnalogInputReadSingle(CH3,&lecturaG_ejeZ);

		SumatoriaG=lectura_ejeX+lectura_ejeY+lecturaG_ejeZ;
		if(sumatoriaG>4)




        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{


}
/*==================[end of file]============================================*/
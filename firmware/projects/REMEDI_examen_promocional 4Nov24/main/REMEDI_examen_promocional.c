/*! @mainpage REMEDI EXAMEN FINAL PROGRAMABLE
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * HC SR04
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * °   HC SR 04     | GPIO 20
 * 
 *    CANAL X   CH1
 * 	  CANAL Y   CH2
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/11/2024  | Document creation		                         |
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
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/

/**
 * @def PERIODO_DELAY
 * @brief define cada cuanto se lee la distancia y se encienden/apagan los leds.
 */
#define PERIODO_DELAY 500

/**
 * @def PERIODO_ALARMA_PRECAUCION
 * @brief define cada cuanto suena el buzzer cuando se está en situacion de "advertencia".
 */
#define PERIODO_ALARMA_PRECAUCION 500*1000

/**
 * @def PERIODO_ALARMA_PELIGRO
 * @brief define cada cuanto suena el buzzer cuando se está en situación de peligro
 */
#define PERIODO_ALARMA_PELIGRO 250*1000

/**
 * @def GPIO_ALARMA_SONORA
 * @brief define el puerto GPIO al cual se conectará el buzzer
 */
#define GPIO_ALARMA_SONORA GPIO_20

/*==================[internal data definition]===============================*/

/**
 * @brief variable que almacenará el dato de distancia.
 */
uint16_t distancia = 0;

/**
 * @brief variable que almacenará la suma algebraica de la medición de los acelerometros en Volts.
 */
uint16_t SumatoriaG_Volts = 0;

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de medir distancia.
 */
TaskHandle_t MedirDistancia_task_handle

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de mostrar advertencias por UART
 */
TaskHandle_t Advertencias_UART_task_handle

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de prender y apagar los LEDs
 */
TaskHandle_t PrenderLEDS_task_handle

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de hacer sonar el buzzer.
 */
TaskHandle_t sonar_buzzer_task_handle

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de detectar una caida.
 */
TaskHandle_t detectar_caida


/*==================[internal functions declaration]=========================*/

/**
 * @brief mide la distancia de los vehiculos hacia el sensor HC_SR04.
 * @param pvParameter parametro sin usar.
 */
static void MedirDistancia(void *pvParameter)
{
	while (true)
	{
		distancia = HcSr04ReadDistanceInCentimeters();
		vTaskDelay(PERIODO_DELAY / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Manda advertencia por UART acorde a las mediciones del HC_SR04 y del acelerometro.
 * El dato de 2,85 es la suma algebraica de los voltajes medidos por los tres acelerometros, uno en cada eje:
 * V(G)=sensibilidad * G + V(G=0).
 * @param pvParameter parametro sin usar.
 */
static void Advertencias_UART(void *pvParameter)
{
	while (true)
	{
		if (distancia < 500 && distancia > 300)
			UartSendString(UART_PC, "PRECAUCION, VEHICULO CERCA.");
		else if (distancia < 300)
			UartSendString(UART_PC, "PELIGRO, VEHICULO CERCA.");

		if (sumatoriaG_Volts > 2.85)
			UartSendString(UART_PC, "CAÍDA DETECTADA.");
	}
}

/**
 * @brief Prende los LEDs acorde a la distancia medida por el sensor de ultrasonido.
 * @param pvParameter parametro sin usar.
 */
static void PrenderLEDS(void *pvParameter)
{
	while (true)
	{
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
		else
			
		vTaskDelay(PERIODO_DELAY / portTICK_PERIOD_MS);
	}
}

/**
 * @brief hace sonar al buzzer con distintas frecuencias.
 * @param pvParameter parametro sin usar.
 */
void sonar_buzzer(void *pvParameter)
{
	while (true)
	{
		if (PRECAUCION)
		{
			GPIOOn(GPIO_ALARMA_SONORA);
			vTaskDelay(PERIODO_ALARMA_PRECAUCION / portTICK_PERIOD_MS);
			GPIOOff(GPIO_ALARMA_SONORA);
		}

		else if (PELIGRO)
		{
			GPIOOn(GPIO_ALARMA_SONORA);
			vTaskDelay(PERIODO_ALARMA_PELIGRO / portTICK_PERIOD_MS);
			GPIOOff(GPIO_ALARMA_SONORA);
		}

	}
}

/**
 * @brief Mide la variación de la posición.
 * @param pvParameter parametro sin usar.
 */
void detectar_caida(void *pvParameter)
{
	uint8_t lecturaG_ejeX_Volts, lecturaG_ejeY_Volts, lecturaG_ejeZ_Volts;

	while (true)
	{
		AnalogInputReadSingle(CH1, &lecturaG_ejeX_Volts);
		AnalogInputReadSingle(CH2, &lecturaG_ejeY_Volts);
		AnalogInputReadSingle(CH3, &lecturaG_ejeZ_Volts);

		SumatoriaG_Volts = lectura_ejeX_Volts + lectura_ejeY_Volts + lecturaG_ejeZ_Volts;
	}
}

/**
 * @brief Funcion main del programa
 */
/*==================[external functions definition]==========================*/
void app_main(void)
{
	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();

	serial_config_t pantalla =
		{
			.port = UART_PC,
			.baud_rate = 115200,
			.func_p = Advertencias_UART,
			.param_p = NULL,
		};

	analog_input_config_t config = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};

	xTaskCreate(&MedirDistancia, "MEDIR", 512, NULL, 5, &MedirDistancia_task_handle);
	xTaskCreate(&Advertencias_UART, "ADVERTIR", 512, NULL, 5, &Advertencias_UART_task_handle);
	xTaskCreate(&PrenderLEDS, "PRENDER", 512, NULL, 5, &PrenderLEDS_task_handle);
	xTaskCreate(&sonar_buzzer, "SONAR", 512, NULL, 5, &sonar_buzzer_task_handle);
	xTaskCreate(&detectar_caida, "CAIDA", 512, NULL, 5, &detectar_caida_task_handle);
}
/*==================[end of file]============================================*/
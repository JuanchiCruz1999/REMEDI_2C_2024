/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa permite manipular cualquiera de los LED de la placa ESP32
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
 * | 15/08/2024 | Document creation		                         |
 *
 * @author Juan Cruz REMEDI (juan.remedi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD
 * @brief Período de tiempo para la funcion de retardo vTaskDelay();
 */
#define CONFIG_BLINK_PERIOD 100
/** @def ON
 * @brief Modo "ENCENDIDO" para el LED seleccionado
 */
#define ON 1
/** @def OFF
 * @brief Modo "APAGADO" para el LED seleccionado
 */
#define OFF 2
/** @def TOOGLE
 * @brief Modo "PARPADEO" para el LED seleccionado
 */
#define TOGGLE 3

/*==================[internal data definition]===============================*/
/**
 * @brief Guarda la información de los parámetros del LED a controlar
 */
struct leds
{
      uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;   // indica el tiempo de cada ciclo
} miled;

/*==================[internal functions declaration]=========================*/
/**
 * @brief Function to configure LED based on the given parameters
 * @param Ledcito Pointer to the LED configuration structure
 */
void ConfigurarLed(struct leds *Ledcito)
{

	switch (Ledcito->mode)
	{
	case ON:
		if (Ledcito->n_led == 1)
			LedOn(LED_1);
		else if (Ledcito->n_led == 2)
			LedOn(LED_2);
		else if (Ledcito->n_led == 3)
			LedOn(LED_3);
		break;

	case OFF:
		if (Ledcito->n_led == 1)
			LedOff(LED_1);
		else if (Ledcito->n_led == 2)
			LedOff(LED_2);
		else if (Ledcito->n_led == 3)
			LedOff(LED_3);
		break;

	case TOGGLE:
		for (uint8_t i = 0; i < Ledcito->n_ciclos; i++)
		{
			if (Ledcito->n_led == 1)
			{
				LedToggle(LED_1);
			}
			else if (Ledcito->n_led == 2)
			{
				LedToggle(LED_2);
			}
			else if (Ledcito->n_led == 3)
			{
				LedToggle(LED_3);
			}
			for (uint8_t j = 0; j < Ledcito->periodo / CONFIG_BLINK_PERIOD; j++)
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

		}
		break;
		default:
			break;
		
	}
}

/*==================[external functions definition]==========================*/
/**
 * @brief Función "main" de la aplicación, donde se definen los valores de los parametros manualmente.
 */
void app_main(void){
	
	LedsInit();
	miled.mode =TOGGLE;
	miled.n_ciclos=10;
	miled.periodo=500;
	miled.n_led=1;
	ConfigurarLed(&miled);

}

/*==================[end of file]============================================*/

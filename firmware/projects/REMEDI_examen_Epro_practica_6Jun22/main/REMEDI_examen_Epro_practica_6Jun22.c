/*! @mainpage Template
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
 * | 	ECHO	 	| 	GPIO 3		|
 * | 	TRIGGER	 	| 	GPIO 2		|
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND 		|
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
 *
 * @author REMEDI, Juan (juan.remedi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <led.h>
#include <uart_mcu.h>
#include <hc_sr04.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"

/*==================[macros and definitions]=================================*/

#define PERIODO_LECTURA_ACELEROMETRO 2
#define PERIODO_LECTURA_DISTANCIA 2
#define PERIODO_DELAY 100
/*==================[internal data definition]===============================*/

bool En_Movimiento = false;
uint16_t distancia = 0;
uint16_t Promedio_medidas_en_G = 0;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Mide la variación de la posición.
 * @param pvParameter parametro sin usar.
 */
void sensar_movimiento_por_acelerometro(void *pvParameter)
{
	uint8_t lectura_analogica_acelerometro;
	uint8_t medida_en_G;
	uint8_t Suma_medidas_acelerometro_en_G;

	while (true)
	{
		for (unsigned int i = 0; i < 10; i++)
		{

			AnalogInputReadSingle(CH1, &lectura_analogica_acelerometro);
			medida_en_G = lectura_analogica_acelerometro * (5.5 / 3.3);
			Suma_medidas_acelerometro_en_G += medida_en_G;
		}

		Promedio_medidas_en_G = Suma_medidas_acelerometro_en_G / 10;
		if (Promedio_medidas_en_G >= 2)
			En_Movimiento = true;
		else{
			En_Movimiento = false;
			Promedio_medidas_en_G = 0;
		}

		vTaskDelay(PERIODO_LECTURA_ACELEROMETRO / portTICK_PERIOD_MS);
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
		if (distancia >= 100)
		{
			LedsOffAll();
		}
		else if (distancia < 100 && distancia >= 50)
		{
			LedOn(LED_1);
		}
		else if (distancia < 50 && distancia >= 20)
		{
			LedOn(LED_2);
		}
		else if (distancia < 20)
		{

			LedOn(LED_3);
		}
		vTaskDelay(PERIODO_LECTURA_ACELEROMETRO / portTICK_PERIOD_MS);
	}
}

/**
 * @brief mide la distancia de los vehiculos hacia el sensor HC_SR04.
 * @param pvParameter parametro sin usar.
 */
static void MedirDistancia(void *pvParameter)
{
	while (true)
	{
		if (En_Movimiento)
			distancia = HcSr04ReadDistanceInCentimeters();
		else if (!En_Movimiento)
			distancia = 0;
		vTaskDelay(PERIODO_LECTURA_DISTANCIA / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Esta funcion chequea la entrada por UART y enciende un led RGB AZUL (nose como)
 * @param pvParameter parametro sin usar.
 */
static void chequearTecla(void *pvParameter)
{
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);

	while (true)
	{
		    if (tecla == 'a')
    {
        LedOn(LED_2);//Simular led azul ??
		LedOn(LED_3);
		if (Promedio_medidas_en_G>2)
		LedsOffAll();
    }
	vTaskDelay(PERIODO_DELAY / portTICK_PERIOD_MS);
	}
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();
	HcSr04Init(GPIO_3,GPIO_2);

	GPIOInit(GPIO_3,GPIO_2);

	serial_config_t MODULO_BLUETOOTH =
		{
			.port = UART_CONNECTOR,
			.baud_rate = 115200,
			.func_p = chequearTecla,
			.param_p = NULL,
		};

		analog_input_config_t config_canal = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0};


}
/*==================[end of file]============================================*/
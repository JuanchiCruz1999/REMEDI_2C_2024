/*! @mainpage Proyecto 2 Ejercicio 1
 *
 * \section genDesc General Description
 * Este proyecto permite realizar mediciones de distancia de objetos empleando un sensor de ultrasonido,
 * además de LEDs para complementar la medicion y una pantalla LCD para mostrar la lectura.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/09/2024 | Document creation		                         |
 *
 * @author Juan Cruz REMEDI (juan.remedi@ingenieria.uner.edu.ar)
 *
 * @section Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO 3		|
 * | 	TRIGGER	 	| 	GPIO 2		|
 * | 	+5V 	 	| 	+5V     	|
 * | 	GND 	 	| 	GND 		|

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "lcditse0803.h"

/*==================[macros and definitions]=================================*/

/** @def TIEMPO_TECLAS
 * @brief tiempo en el cual se actualiza la lectura de las teclas
 */
#define TIEMPO_TECLAS 100

/** @def TIEMPO_DELAY
 * @brief tiempo que demora la acción de una tarea dada
 */
#define TIEMPO_DELAY 1000

/*==================[internal data definition]===============================*/

/**
 * @brief variable de tipo task handle encargada de manejar la tarea "MedirDistancia"
 */
TaskHandle_t MedirDistancia_task_handle = NULL;

/**
 * @brief variable de tipo task handle encargada de manejar la tarea "LeerTeclas"
 */
TaskHandle_t LeerTeclas_task_handle = NULL;

/**
 * @brief variable de tipo task handle encargada de manejar la tarea "MostrarPorPantalla"
 */
TaskHandle_t MostrarPorPantalla_task_handle = NULL;

/**
 * @brief variable de tipo task handle encargada de manejar la tarea "PrenderLEDS"
 */
TaskHandle_t PrenderLEDS_task_handle = NULL;

/**
 * @brief Variable booleana que controla cuando medir distancia con el sensor HC_SR04
 */
bool ON = false;

/**
 * @brief Variable booleana que permite mantener una medición de distancia fija en la pantalla
 */
bool HOLD = false;

/**
 * @brief
 */
/*variable que almacena un dato de distancia medido*/
uint16_t distancia = 0;

/*==================[internal functions declaration]=========================*/

/**
 * @brief mide la medida de distancia entre un objeto y el sensor HC-SR04 y
 * la almacena en una variable
 * @param pvParameter parametro sin usar
 */
static void MedirDistancia(void *pvParameter)
{
    while (true)
    {
        if (ON)
            distancia = HcSr04ReadDistanceInCentimeters();
        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
    }
}

/**
 * @brief detecta la tecla que se presionó y cambia de estado las variables
 * booleanas ON y HOLD
 * @param pvParameter parametro sin usar
 */
static void LeerTeclas(void *pvParameter)
{
    uint8_t teclas;
    // LedsInit();
    while (1)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            ON = !ON;
            break;
        case SWITCH_2:
            HOLD = !HOLD;
            break;
        }
        // LedToggle(LED_3);
        vTaskDelay(TIEMPO_TECLAS / portTICK_PERIOD_MS);
    }
}

/**
 * @brief muestra por display la medida de distancia obtenida
 * @param pvParameter parametro sin usar
 */
static void MostrarPorPantalla(void *pvParameter)
{
    while (true)
    {
        if (ON)
        {
            if (!HOLD)
                LcdItsE0803Write(distancia);
        }
        else
        {
            LcdItsE0803Off();
        }
        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
    }
}

/**
 * @brief prende los distintos LEDS acorde al dato de distancia obtenido
 * @param pvParameter parametro sin usar
 */
static void PrenderLEDS(void *pvParameter)
{
    while (true)
    {
        if (ON)
        {
            if (distancia < 10)
            {
                LedsOffAll();
            }
            else if (distancia < 20 && distancia >= 10)
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if (distancia < 30 && distancia >= 20)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else if (distancia >= 30)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
        }
        else if (!ON)
        {
            LedsOffAll();
        }
        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief funcion "main" del programa
 */
void app_main(void)
{

    LedsInit();
    SwitchesInit();
    LcdItsE0803Init();
    HcSr04Init(GPIO_3, GPIO_2);

    xTaskCreate(&MedirDistancia, "MEDIR", 512, NULL, 5, &MedirDistancia_task_handle);
    xTaskCreate(&LeerTeclas, "LEER", 512, NULL, 5, &LeerTeclas_task_handle);
    xTaskCreate(&MostrarPorPantalla, "MOSTRAR", 512, NULL, 5, &MostrarPorPantalla_task_handle);
    xTaskCreate(&PrenderLEDS, "PRENDER", 512, NULL, 5, &PrenderLEDS_task_handle);
}

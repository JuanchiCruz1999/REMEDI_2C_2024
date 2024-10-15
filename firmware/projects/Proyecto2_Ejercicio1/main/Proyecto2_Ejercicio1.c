/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 4/09/2024 | Document creation		                         |
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

/* variables que manejan y coordinan la acción de sus tareas correspondientes*/
TaskHandle_t MedirDistancia_task_handle = NULL;
TaskHandle_t LeerTeclas_task_handle = NULL;
TaskHandle_t MostrarPorPantalla_task_handle = NULL;
TaskHandle_t PrenderLEDS_task_handle = NULL;

/*variables booleanas que se encargan de:
ON: medir distancia por sensor de ultrasonido y mostrar por pantalla al accionar la tecla 1
HOLD: mantener una medicion dada en el display al accionar la tecla 2*/
bool ON = false;
bool HOLD = false;

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
        else if (!ON)
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
        if(ON)
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
        else if(!ON)
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

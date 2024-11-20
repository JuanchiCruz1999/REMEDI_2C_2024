/*! @mainpage Proyecto 2 Ejercicio 2
 *
 * \section genDesc General Description
 *
 * En este proyecto se modifica la actividad del punto 1
 * de manera de utilizar interrupciones para el control de las teclas y el control de tiempos (Timers).
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 18/09/2024 | Document creation		                         |
 *
 * @author REMEDI Juan Cruz (juan.remedi@ingenieria.uner.edu.ar)
 *
 * @section hardConn Hardware Connection
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO 3		|
 * | 	TRIGGER	 	| 	GPIO 2		|
 * | 	+5V 	 	| 	+5V     	|
 * | 	GND 	 	| 	GND 		|

 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "led.h"
#include "switch.h"
#include "hc_sr04.h"
#include "lcditse0803.h"

/*==================[macros and definitions]=================================*/

/** @def CONFIG_BLINK_PERIOD_TIMER_B
 * @brief tiempo de accion del timer B
 */
#define CONFIG_BLINK_PERIOD_TIMER_B 1300 * 1000
/*==================[internal data definition]===============================*/

/**
 * @brief variable de tipo task handle encargada de manejar la tarea "MedirDistancia"
 */
TaskHandle_t MedirDistancia_task_handle = NULL;

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

/*variable que almacena el dato de distancia medido*/
uint16_t distancia = 0;

/*==================[internal functions declaration]=========================*/

/**
 * @brief mide la distancia entre un objeto y el sensor HC-SR04 y la almacena en una variable
 * @param pvParameter parametro sin usar
 */
static void MedirDistancia(void *pvParameter)
{

    while (true)
    {
        if (ON)
            distancia = HcSr04ReadDistanceInCentimeters();
        // vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

/**
 * @brief muestra por display la medida de distancia obtenida en un momento dado
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
        //        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}
/**
 * @brief cambia de estado la variable booleana ON
 * @param pvParameter parametro sin usar
 */
static void Interrumpir_tecla1(void *pvParameter)
{
    ON = !ON;
}

/**
 * @brief cambia de estado la variable booleana HOLD
 * @param pvParameter parametro sin usar
 */
static void Interrumpir_tecla2(void *pvParameter)
{
    HOLD = !HOLD;
}

/**
 * @brief prende los distintos LEDS acorde a la medición de distancia
 * @param pvParameter parametro sin usar
 */
static void PrenderLEDS(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
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
        //        vTaskDelay(TIEMPO_DELAY / portTICK_PERIOD_MS);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

/**
 * @brief envia notificaciones a las tareas asociadas con el timer B
 * @param param parametro sin usar
 */
void FuncTimer_TimerB(void *param)
{
    vTaskNotifyGiveFromISR(MostrarPorPantalla_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(MedirDistancia_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(PrenderLEDS_task_handle, pdFALSE); /* Envía una notificación a la tarea asociada al LED_2 */
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

    /* Inicialización de timers */
    timer_config_t Func_timerB = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimer_TimerB,
        .param_p = NULL};
    TimerInit(&Func_timerB);

    /* Activación de interrupciones */
    SwitchActivInt(SWITCH_1, Interrumpir_tecla1, NULL);
    SwitchActivInt(SWITCH_2, Interrumpir_tecla2, NULL);

    /* Creación de tareas */
    xTaskCreate(&MedirDistancia, "MEDIR", 512, NULL, 5, &MedirDistancia_task_handle);
    //    xTaskCreate(&LeerTeclas, "LEER", 512, NULL, 5, &LeerTeclas_task_handle);
    xTaskCreate(&MostrarPorPantalla, "MOSTRAR", 512, NULL, 5, &MostrarPorPantalla_task_handle);
    xTaskCreate(&PrenderLEDS, "PRENDER", 512, NULL, 5, &PrenderLEDS_task_handle);
    /* Inicialización del conteo de timers */
    TimerStart(Func_timerB.timer);
    //    TimerStart(Func_TimerB.timer);
}

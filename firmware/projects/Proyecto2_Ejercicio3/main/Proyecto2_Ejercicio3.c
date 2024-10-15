/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink at different rates, using FreeRTOS tasks and timer interrupts.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/09/2024 | Document creation		                         |
 *
 * @author REMEDI Juan Cruz (juan.remedi@ingenieria.uner.edu.ar)
 *
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/

/**
 * @def CONFIG_BLINK_PERIOD_TIMER_B
 * @brief tiempo de acción del timer B
 */
#define CONFIG_BLINK_PERIOD_TIMER_B 1300 * 1000

/*==================[internal data definition]===============================*/
// TaskHandle_t led1_task_handle = NULL;
// TaskHandle_t led2_task_handle = NULL;

/* maneja la tarea asociada a la medición de distancia por sensor de ultrasonido*/
TaskHandle_t MedirDistancia_task_handle = NULL;

/* maneja la tarea asociada a mostrar por display la medida de distancia realidaza*/
TaskHandle_t MostrarPorPantalla_task_handle = NULL;

/* maneja la tarea asociada a prender y apagar los LEDS*/
TaskHandle_t PrenderLEDS_task_handle = NULL;

/* variable booleana que coordina las funciones de medir y mostrar por display*/
bool ON = false;
/* variable booleana que coordina la funcion de mantener en el display una medicion dada */
bool HOLD = false;

/* variable que guarda una medición de distancia realizada*/
uint16_t distancia = 0;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Esta función se encarga de medir la distancia entre un objeto y el sensor HC-SR04
 * @param pvParameter parametro sin usar
 */
static void MedirDistancia(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ON)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
            UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
            UartSendString(UART_PC, "cm\n\r");
        }
    }
}

/**
 * @brief Esta funcion es responsable de mostrar por el display la medida obtenida.
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
 * @brief Esta funcion chequea la entrada por UART y actualiza los estados de las variables
 * booleanas ON y HOLD
 */
static void chequearTecla()
{
    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);

    if (tecla == 'O')
    {
        ON = !ON;
    }
    else if (tecla == 'H')
    {
        HOLD = !HOLD;
    }
}

/**
 * @brief cambia de estado la variable booleanada ON
 * @param pvParameter parametro sin usar
 */
static void Interrumpir_tecla1(void *pvParameter)
{
    ON = !ON;
}

/**
 * @brief cambia de estado la variable booleanada HOLD
 * @param pvParameter parametro sin usar
 */
static void Interrumpir_tecla2(void *pvParameter)
{
    HOLD = !HOLD;
}

/**
 * @brief Esta funcion recibe la notificacion correspondiente y prende el led acorde a la 
 * lectura de distancia del sensor HC-SR04
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
 * @brief acciona el timer B y envía una notificacion a cada tarea asociada
 * @param param parametro sin usar
 */
void FuncTimer_TimerB(void *param)
{
    vTaskNotifyGiveFromISR(MostrarPorPantalla_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(MedirDistancia_task_handle, pdFALSE);
    vTaskNotifyGiveFromISR(PrenderLEDS_task_handle, pdFALSE);
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
    serial_config_t serial_global = {
        .port = UART_PC,
        .baud_rate = 9600,
        .func_p = chequearTecla,
        .param_p = NULL};
    UartInit(&serial_global);

    timer_config_t Func_timerB = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_TIMER_B,
        .func_p = FuncTimer_TimerB,
        .param_p = NULL};
    TimerInit(&Func_timerB);

    SwitchActivInt(SWITCH_1, Interrumpir_tecla1, NULL);
    SwitchActivInt(SWITCH_2, Interrumpir_tecla2, NULL);

    /* Creación de tareas */
    xTaskCreate(&MedirDistancia, "MEDIR", 512, NULL, 5, &MedirDistancia_task_handle);
    //    xTaskCreate(&LeerTeclas, "LEER", 512, NULL, 5, &LeerTeclas_task_handle);
    xTaskCreate(&MostrarPorPantalla, "MOSTRAR", 512, NULL, 5, &MostrarPorPantalla_task_handle);
    xTaskCreate(&PrenderLEDS, "PRENDER", 512, NULL, 5, &PrenderLEDS_task_handle);

    /* Inicialización del conteo de timers */
    TimerStart(Func_timerB.timer);
}

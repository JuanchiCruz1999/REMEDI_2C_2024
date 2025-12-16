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
 * |    Peripheral    |   ESP32   	            |
 * |:----------------:|:------------------------|
 * |HcSr04(tanque H20)| GPIO_20 y GPIO_21		|
 * | Electroválvula   |          GPIO_8	    	|
 * |Balanza analógica |          GPIO_9 	   	|
 * |Señal Balanza     |           CH1 	   	    |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 16/12/2025 | Document creation		                         |
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
#include "timer_mcu.h"
#include "hc_sr04.h" 
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "led.h"
#include "switch.h"
#include "math.h"
/*==================[macros and definitions]=================================*/

/** @def GPIO_ELECTROVALVULA
 * @brief GPIO que enciende/apaga la electrovalvula con una señal en alto/bajo respectivamente.
 */
#define GPIO_ELECTROVALVULA GPIO_8

/** @def GPIO_COMPUERTA_ALIMENTO
 * @brief GPIO que abre/cierra la compuerta que deja pasar alimento con una señal en alto/bajo respectivamente.
 */
#define GPIO_COMPUERTA_ALIMENTO GPIO_9

/** @def SENIAL_BALANZA
 * @brief Canal a través del cual se leen los valores en voltaje de la señal analógica de la balanza.
 */
#define SENIAL_BALANZA CH1

/** @def PERIODO_TIEMPO_1
 * @brief Tiempo entre dos distintas lecturas de volumen y peso y dos mensajes distintos por UART.
 */
#define PERIODO_TIEMPO_1 5000000 //5 segundos en microsegundos para el timer

/** @def PERIODO_TIEMPO_1
 * @brief Periodo al que quiero que lea lo que le mando por las teclas.
 */
#define CONFIG_BLINK_PERIOD_TECLAS 200

/*==================[internal data definition]===============================*/

int32_t volumen_agua_tanque = 0;

int32_t peso_recipiente_alimento = 0;

/*guarda el valor de la tecla que se presiona */
uint8_t teclas;

bool ENCENDIDO= false;

/**
 * @brief Variable encargada de manejar la tarea relacionada con medir el volumen de agua
 */
TaskHandle_t task_handle_agua = NULL;

/**
 * @brief Variable encargada de manejar la tarea relacionada con medir el peso de alimento
 */
TaskHandle_t task_handle_alimento = NULL;

/**
 * @brief Variable encargada de manejar la tarea relacionada con el display por UART
 */
TaskHandle_t task_handle_UART = NULL;

/*==================[internal functions declaration]=========================*/

/** @fn  Timer_1_Func
 * @brief  tarea que mide la altura del agua cada 5 seg y calcula el volumen de agua en el tanque
 * @param param parámetro sin usar.
 */
void Timer_1_Func(void *param){
	vTaskNotifyGiveFromISR(task_handle_agua, pdFALSE);
	vTaskNotifyGiveFromISR(task_handle_alimento, pdFALSE);	
	vTaskNotifyGiveFromISR(task_handle_UART, pdFALSE);	

}

/** @fn  medir_nivel_agua_task
 * @brief  tarea que mide la altura del agua cada 5 seg y calcula el volumen de agua en el tanque
 */
void medir_nivel_agua_task(void *pvParameter){
	while(1){
		
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		
		float altura_agua;
		altura_agua=30-HcSr04ReadDistanceInCentimeters();//el sensor está en la parte de arriba del tanque de 30 cm de altura
		volumen_agua_tanque=3.14*((20/2)^2)*altura_agua;// V=PI*r^2*h -> d=r/2 y altura h en [cm], volumen en cm^3
	}
}

/** @fn  manipular_electrovalvula
 * @brief  enciende/apaga la electrovalvula acorde al volumen de agua que hay en el tanque
 */
void manipular_electrovalvula(){
	if(volumen_agua_tanque>=2500){
		GPIOOff(GPIO_ELECTROVALVULA);
	}
	else if(volumen_agua_tanque<=500){
		GPIOOn(GPIO_ELECTROVALVULA);
	}
}

/** @fn  medir_y_convertir_a_gramos_Task
 * @brief  toma la medida de la balanza en volts y la convierte a gramos
 * @param param parámetro sin usar.
 */
void medir_y_convertir_a_gramos_Task(void *param){
	while(1){

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		int32_t Amplitud_senial_analogica;
		AnalogInputReadSingle(SENIAL_BALANZA,&Amplitud_senial_analogica);
		peso_recipiente_alimento=0+((1000-0)/(3.3-0))*(Amplitud_senial_analogica-0);// aplicando Ec. recta con dos puntos
	}
}

/** @fn  accionar_mecanismo_alimento
 * @brief  Abre o cierra la compuerta que sirve alimento dependiendo del peso del recipiente.
 */
void accionar_mecanismo_alimento(){
	if(peso_recipiente_alimento>=1000){
		GPIOOff(GPIO_COMPUERTA_ALIMENTO);
	}
	else if (peso_recipiente_alimento<=50){
		GPIOOn(GPIO_COMPUERTA_ALIMENTO);
	}
}

/** @fn  informar_por_UART_Task
 * @brief  Manda mensajes por puerto serie sobre las medidas tomadas de volumen y peso.
 * @param param parámetro sin usar.
 */
void informar_por_UART_Task(void *param){
	while(1){

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		UartSendString(UART_PC, "Agua: " );
		UartSendString(UART_PC, (char*)UartItoa(volumen_agua_tanque,10));
		UartSendString(UART_PC, " [cm^3] , Alimento: " );
		UartSendString(UART_PC, (char*)UartItoa(peso_recipiente_alimento,10));
		UartSendString(UART_PC, " [gr]\r\n");
	}
}

/** @fn  EncenderYDetenerPorTecla()
 * @brief  Al presionar la tecla 1, se prende/apaga el led y se enciende/detiene el sistema.
 */
void EncenderYDetenerPorTecla()
{
	while(1)
	{
     	teclas  = SwitchesRead();
     	
		if(teclas == SWITCH_1 && ENCENDIDO){
			ENCENDIDO=!ENCENDIDO;//lo pone en FALSE
			LedOff(LED_1);
			GPIOOff(GPIO_ELECTROVALVULA);
			GPIOOff(GPIO_COMPUERTA_ALIMENTO);

		}

		else if(teclas == SWITCH_1 && !ENCENDIDO){
			ENCENDIDO=ENCENDIDO;//lo pone en TRUE
			LedOn(LED_1);
			GPIOOn(GPIO_ELECTROVALVULA);
			GPIOOn(GPIO_COMPUERTA_ALIMENTO);
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_TECLAS / portTICK_PERIOD_MS);
	}
}


/*==================[external functions definition]==========================*/
/**
 * @brief funcion "main" del programa.
 */
void app_main(void){

	//Inicializo GPIOs
    GPIOInit(GPIO_ELECTROVALVULA, GPIO_OUTPUT);
    GPIOInit(GPIO_COMPUERTA_ALIMENTO, GPIO_OUTPUT);

	//Inicializo Sensor de ultrasonido
	HcSr04Init(GPIO_20, GPIO_21);

	// configuracion para entrada analogica
    analog_input_config_t analogInput1 = {
        .input = CH1,       // Se configura para leer del canal 1 del conversor analógico-digital (ADC)
        .mode = ADC_SINGLE, // Se configura para realizar una única lectura analógica
    };
    AnalogInputInit(&analogInput1); // Inicializa

    // Para el puerto serie
    serial_config_t my_uart = {
        .port = UART_PC,
        .baud_rate = 115200, /*!< baudrate (bits per second) */
        .func_p = NULL,      /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
        .param_p = NULL      /*!< Pointer to callback function parameters */
    };
    UartInit(&my_uart);

	//Para la tecla
    SwitchesInit();
	LedsInit();
    SwitchActivInt(SWITCH_1, &EncenderYDetenerPorTecla, NULL);

	// configuro timer1 para:
	// 1) control de volumen de agua en los tanques
	// 2) medir peso de alimento con la balanza
	// 3) informar por UART las medidas tomadas en 1) y 2)
	// Las tres tareas se hacen cada 5 segundos
    timer_config_t timer_1 = {
        .timer = TIMER_A,
        .period = PERIODO_TIEMPO_1,
        .func_p = Timer_1_Func,
        .param_p = NULL
	};

    TimerInit(&timer_1); // inicializo timer 1
    TimerStart(timer_1.timer); // para que comience el timer 1

	//creo tareas
    xTaskCreate(&medir_nivel_agua_task, "medir volumen de agua", 512, NULL, 5, &task_handle_agua);
    xTaskCreate(&medir_y_convertir_a_gramos_Task, "pesar alimento", 512, NULL, 5, &task_handle_alimento);
    xTaskCreate(&informar_por_UART_Task, "mandar mensaje por UART", 512, NULL, 5, &task_handle_UART);
}
/*==================[end of file]============================================*/
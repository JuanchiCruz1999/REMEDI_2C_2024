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
 * CONEXION PUERTOS HC_SR04
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	ECHO	 	| 	GPIO3		|
 * | 	TRIGGER	 	| 	GPIO2		|
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND 		|
 *
 * CONEXION BARRERA
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	BARRERA	 	| 	GPIO19	|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/11/2024 | Document creation		                         |
 *
 * @author REMEDI, Juan Cruz (juan.remedi@ingenieria.uner.edu.ar)
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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/** @def PERIODO_LECTURA_DISTANCIA
 * @brief Tiempo entre lecturas de distancia
*/
#define PERIODO_LECTURA_DISTANCIA 100

/** @def PERIODO_LECTURA_VELOCIDAD
 * @brief Tiempo entre lecturas de velocidad
*/
#define PERIODO_LECTURA_VELOCIDAD 100

/** @def UMBRAL_DISTANCIA
 * @brief Valor de distancia en metros a partir del cual se arranca a medir velocidad de vehiculo
*/
#define UMBRAL_DISTANCIA 10

/** @def PERIODO_TOGGLE_LEDS
 * @brief Tiempo en el que se actualizan los leds
*/
#define PERIODO_TOGGLE_LEDS 1000

/** @def PERIODO_LECTURA_PESO
 * @brief Tamaño del buffer para los valores del ECG
*/

#define PERIODO_LECTURA_PESO 5

/** @def GPIO_BARRERA
 * @brief gpio a usar para controlar la barrera
*/
#define GPIO_BARRERA GPIO_19

/*==================[internal data definition]===============================*/

/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de medir distancia
 */
TaskHandle_t medir_distancia_task_handle_task_handle;
/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de medir velocidad
 */
TaskHandle_t medir_velocidad_task_handle;
/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de prender y apagar los LEDs
 */
TaskHandle_t PrenderLEDS_task_handle;
/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de pesar vehiculos
 */
TaskHandle_t Pesar_task_handle;
/**
 * @brief variable de tipo taskHandle encargada de manipular la tarea de informar peso y velocidad por uart.
 */
TaskHandle_t InformarporUART_task_handle;

/**
 * @brief iterador del arreglo de datos de distancia
 */
uint16_t posicion = 0;

/**
 * @brief arreglo de datos de distancia
 */
uint16_t arreglo_distancia[]={0,0,0,0,0,0,0,0,0,0};
/**
 * @brief idato de velocidad
 */
uint16_t velocidad = 0;
/**
 * @brief dato de velocidad maxima
 */
uint16_t velocidad_maxima = 0;
/**
 * @brief dato de peso
 */
uint16_t peso = 0;
/**
 * @brief controla si arrancar o no a medir velocidad
 */

bool vehiculo_en_marcha = false;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Prende los LEDs acorde a la distancia medida por el sensor de ultrasonido.
 * @param pvParameter parametro sin usar.
 */
void medir_distancia(void *pvparameter){

	while(true){

		if(posicion<10){
			arreglo_distancia[posicion]=HcSr04ReadDistanceInCentimeters()/100;//pasar el valor a metros
			posicion++;
		}

		else if(posicion==10){
			posicion=0;
			for(unsigned int i=0;i<9;i++){
				arreglo_distancia[i]=0;//una vez lleno,vacío los datos del arreglo para poder volver a llenarlo con medida
			}
		}
			
		if(arreglo_distancia[posicion]<UMBRAL_DISTANCIA){
			vehiculo_en_marcha=true;
		}

		else if(arreglo_distancia[i]>=UMBRAL_DISTANCIA)
			vehiculo_en_marcha=false;
		}

	vTaskDelay(PERIODO_LECTURA_DISTANCIA/portTICK_PERIOD_MS);
}


/**
 * @brief mide la velocidad una vez se registra un vehiculo a menos de 10 metros y la almacena en una variable
 * tambien guarda el dato de la velocidad maxima alcanzada
 * @param pvParameter parametro sin usar.
 */
void medir_velocidad(void *pvparameter){

	while(true){

		if(posicion > 0 && vehiculo_en_marcha==true){
			//DISTANCIA RECORRIDA EN UNA CENTÉSIMA DE SEGUNDO
			velocidad=(arreglo_distancia[posicion]-arreglo_distancia[posicion-1])/0.1;//velocidad=DELTA_DISTANCIA/DELTA_TIEMPO

		if(velocidad > velocidad_maxima){
			velocidad_maxima = velocidad;
			}
		}
	}
	vTaskDelay(PERIODO_LECTURA_DISTANCIA/portTICK_PERIOD_MS);
	for(unsigned int i=0;i<9;i++){
		arreglo_distancia[i]=0;
	}

	vTaskDelay(PERIODO_LECTURA_DISTANCIA/portTICK_PERIOD_MS);

}


/**
 * @brief Prende los LEDs acorde a la distancia entre el vehículo y el sensor de ultrasonido.
 * @param pvParameter parametro sin usar.
 */
static void PrenderLEDS(void *pvParameter)
{
	while (true)
	{
		if (velocidad > 8)
		{
			LedOn(LED_3);
		}
		else if (velocidad <= 8 && velocidad > 0)
		{
			LedOn(LED_2);
		}
		else if (velocidad == 0)
		{
			LedOn(LED_1);
		}
		else{
			LedsOffAll();
			VEHICULO_DETENIDO=true;
		}
		vTaskDelay(PERIODO_DELAY / portTICK_PERIOD_MS);
	}
}

/**
 * @brief Otorga un valor de peso del vehículo a partir de la lectura de las galgas.
 * A su vez, informa el peso y la velocidad máxima del vehículo medidos.
 * @param pvParameter parametro sin usar.
 */

static void Pesar(void *pvParameter){
	
	while (true){

		uint16_t lecturas_an_peso_GALGA1,lectura_an_peso_GALGA2;
		uint16_t suma_lecturas_GALGA1, suma_lecturas_GALGA2;
		
		AnalogInputReadSingle(CH1,&lecturas_an_peso_GALGA1);
		suma_lecturas_GALGA1+=lecturas_an_peso_GALGA1;

		AnalogInputReadSingle(CH2,&lecturas_an_peso_GALGA2);
		suma_lecturas_GALGA2+=lecturas_an_peso_GALGA2;


		peso = (suma_lecturas_GALGA1/50)+(suma_lecturas_GALGA2/50);

		UartSendString(UART_PC, "Peso: "); 
		UartSendString(UART_PC, (char*)UartItoa(peso, 10));
		UartSendString(UART_PC, "kg \n\r");

		UartSendString(UART_PC, "Velocidad máxima: "); 
		UartSendString(UART_PC, (char*)UartItoa(velocidad_maxima, 10));
		UartSendString(UART_PC, "m/s \n\r"); 

	}
	vTaskDelay(PERIODO_LECTURA_PESO/portTICK_PERIOD_MS);
}

static void InformarporUART(void *pvParameter){

    while (true){
    
		UartSendString(UART_PC, "Peso: "); 
		UartSendString(UART_PC, (char*)UartItoa(peso, 10));
		UartSendString(UART_PC, "kg \n\r");

		UartSendString(UART_PC, "Velocidad máxima: "); 
		UartSendString(UART_PC, (char*)UartItoa(velocidad_maxima, 10));
		UartSendString(UART_PC, "m/s \n\r"); 
	}
}

/**
 * @brief Controla por UART y teclas la apertura 
 * @param pvParameter parametro sin usar.
 */
static void controlar_barrera(){

    uint8_t tecla;
    UartReadByte(UART_PC, &tecla);

    if (tecla == 'O')
    {
        GPIOOn(GPIO_BARRERA); //se abre la barrera
    }
    else if (tecla == 'H')
    {
        GPIOOff(GPIO_BARRERA);
	}
}


/*==================[external functions definition]==========================*/

/**
 * @brief Aplicación principal del programa.
 * @param pvParameter parametro sin usar.
 */
void app_main(void){

	LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);

xTaskCreate(&medir_distancia, "OSCILOSCOPIO", 2048, NULL, 5, &medir_distancia_task_handle);
xTaskCreate(&medir_velocidad, "OSCILOSCOPIO", 2048, NULL, 5, &medir_velocidad_task_handle);
xTaskCreate(&PrenderLEDS, "OSCILOSCOPIO", 2048, NULL, 5, &PrenderLEDS_task_handle);
xTaskCreate(&Pesar, "OSCILOSCOPIO", 2048, NULL, 5, &UART_task_handle);
xTaskCreate(&InformarporUART, "OSCILOSCOPIO", 2048, NULL, 5, &InformarporUART_task_handle);

}
/*==================[end of file]============================================*/
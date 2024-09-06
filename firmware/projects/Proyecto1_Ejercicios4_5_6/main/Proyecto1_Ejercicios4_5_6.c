/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Este programa permite mostrar por un display de 7 segmentos un numero de
 * 3 cifras a eleccion del usuario.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 * | 	PIN_25	 	| 	GPIO_20		|
 * | 	PIN_26	 	| 	GPIO_21		|
 * | 	PIN_27	 	| 	GPIO_22		|
 * | 	PIN_28	 	| 	GPIO_23		|
 * | 	PIN_22	 	| 	GPIO_9		|
 * | 	PIN_23	 	| 	GPIO_18		|
 * | 	PIN_24	 	| 	GPIO_19		|
 * 
 * 
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 14/08/2024 | Document creation		                         |
 *
 * @author Juan Cruz REMEDI (juan.remedi@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"


/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
/**
 * @brief estructura para manipular variables de tipo GPIO
 * 
 */
typedef struct
{
	gpio_t pin;			/*!< GPIO Numero de pin */
	io_t dir;			/*!< GPIO direcciones: '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t arreglo[4];

/*==================[internal functions declaration]=========================*/
/**
 * @brief Función para convertir un numero de 32 bits a una BCD (en forma de arreglo).
 * 
 * @param dato_32_bits dato de 32 bits a convertir.
 * @param cantidad_digitos numero de digitos del arreglo de BCD.
 * @param numero_BCD arreglo que almacena los digitos del numero en BCD.
 * @return 0
 */

int8_t convertToBcdArray(uint32_t dato_32_bits,uint8_t cantidad_digitos,uint8_t *numero_BCD){
	for(uint8_t i=0;i<cantidad_digitos;i++){
		numero_BCD[i]=dato_32_bits%10;
		dato_32_bits=dato_32_bits/10;
	}
	return 0;
}

/**
 * @brief Función para "mapear" un numero a los pins del GPIO).
 * 
 * @param arreglo el arreglo de las configuraciones guardadas del GPIO.
 * @param Digito el numero a "mapear" a los pins del GIPIO.
 */

void mapearBits(gpioConf_t *arreglo, uint8_t Digito){

	for(uint8_t k=0;k<4;k++){
		if(Digito&1<<k)
			GPIOOn(arreglo[k].pin);
		else
			GPIOOff(arreglo[k].pin);
	}
}
/**
 * @brief Funcion para mostrar por pantalla el numero de 32 bits.
 * 
 * @param dato32bits el numero a mostrar.
 * @param CantDigitosSalida el numero de digitos a mostrar.
 * @param arreglo_1 el arreglo de las configuraciones de GPIO para los digitos
 * @param arreglo_2 el arreglo de las configuraciones de GPIO para los selectores.
 */

void mostrar_por_display(uint32_t dato32bits, uint8_t CantDigitosSalida, gpioConf_t* arreglo_1, gpioConf_t *arreglo_2){

	uint8_t array_auxiliar[CantDigitosSalida];
	convertToBcdArray(dato32bits,CantDigitosSalida,array_auxiliar);
	for(uint8_t i=0;i<CantDigitosSalida;i++){
		mapearBits(arreglo_1,array_auxiliar[i]);
		GPIOOn(arreglo_2[i].pin);
		GPIOOff(arreglo_2[i].pin);
	}
}

/*==================[external functions definition]==========================*/

/**
 * @brief función "main" del programa.
 */

void app_main(void){
	
	uint32_t dato=210;
	uint8_t digitos=3;
	uint8_t num_BCD[digitos];
	//uint8_t numero=5;
	convertToBcdArray(dato,digitos,num_BCD);

	for(uint8_t j=0;j<digitos;j++){
		printf(" Digito %u\n", num_BCD[j]);
	}

	gpioConf_t 	arreglo[]=
{
	{GPIO_20,GPIO_OUTPUT},
	{GPIO_21,GPIO_OUTPUT},
	{GPIO_22,GPIO_OUTPUT},
	{GPIO_23,GPIO_OUTPUT},
};

	for(uint8_t k=0;k<4;k++){
		GPIOInit(arreglo[k].pin,arreglo[k].dir);
	}

	//mapearBits(arreglo,numero);

	gpioConf_t Pin_Selector[]=
	{
	{GPIO_9,GPIO_OUTPUT},
	{GPIO_18,GPIO_OUTPUT},
	{GPIO_19,GPIO_OUTPUT},
	};

	for(uint8_t i=0;i<3;i++){
		GPIOInit(Pin_Selector[i].pin,Pin_Selector[i].dir);
	}

	mostrar_por_display(dato,digitos,arreglo,Pin_Selector);
	//for(uint8_t j=0;j<4;j++){
	//	printf(" Digito Mapeado %u\n", num_BCD[j]);
	//}
}
/*==================[end of file]============================================*/
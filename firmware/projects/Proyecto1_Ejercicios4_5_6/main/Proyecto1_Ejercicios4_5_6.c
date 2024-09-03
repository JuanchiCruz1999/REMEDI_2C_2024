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
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"


/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

gpioConf_t arreglo[4];

/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t dato_32_bits,uint8_t cantidad_digitos,uint8_t *numero_BCD){
	for(uint8_t i=0;i<cantidad_digitos;i++){
		numero_BCD[i]=dato_32_bits%10;
		dato_32_bits=dato_32_bits/10;
	}
	return 0;
}

void mapearBits(gpioConf_t *arreglo, uint8_t Digito){

	for(uint8_t k=0;k<4;k++){
		if(Digito&1<<k)
			GPIOOn(arreglo[k].pin);
		else
			GPIOOff(arreglo[k].pin);
	}
}
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
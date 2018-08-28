//----------------------------------------------------------------------------------------------------------
//     /$$$$$  /$$$$$$  /$$    /$$  /$$$$$$        /$$$$$$       /$$       /$$$$$$$$ /$$   /$$  /$$$$$$   //
//    |__  $$ /$$__  $$| $$   | $$ /$$__  $$      |_  $$_/      | $$      | $$_____/| $$  /$$/ /$$__  $$  //
//       | $$| $$  \ $$| $$   | $$| $$  \ $$        | $$        | $$      | $$      | $$ /$$/ | $$  \ $$  //
//       | $$| $$  | $$|  $$ / $$/| $$$$$$$$        | $$        | $$      | $$$$$   | $$$$$/  | $$$$$$$$  //
//  /$$  | $$| $$  | $$ \  $$ $$/ | $$__  $$        | $$        | $$      | $$__/   | $$  $$  | $$__  $$  //
// | $$  | $$| $$  | $$  \  $$$/  | $$  | $$        | $$        | $$      | $$      | $$\  $$ | $$  | $$  //
// |  $$$$$$/|  $$$$$$/   \  $/   | $$  | $$       /$$$$$$      | $$$$$$$$| $$$$$$$$| $$ \  $$| $$  | $$  //
//  \______/  \______/     \_/    |__/  |__/      |______/      |________/|________/|__/  \__/|__/  |__/  //
//----------------------------------------------------------------------------------------------------------


// ----- Includes -------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <USART_Handler.h>
#include "diag/Trace.h"

#include "Timer.h"
#include "BlinkLed.h"
#include "stm32f4xx_hal_uart.h"
#include "SPI_Handler.h"
#include "I2C_Handler.h"
#include "Interrupt_Handler.h"

// ----- Timing definitions ---------------------------------------------------

// Keep the LED on for 2/3 of a second.
#define BLINK_ON_TICKS  (TIMER_FREQUENCY_HZ * 3 / 4)
#define BLINK_OFF_TICKS (TIMER_FREQUENCY_HZ - BLINK_ON_TICKS)

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

// ----- EXTERN VARIABLES ----------------------------------------------------

extern UART_HandleTypeDef UartHandle;
// extern SPI_HandleTypeDef SpiHandle;
// extern UART_HandleTypeDef UartHandle2;  // Handle used for UART2 peripherals
extern I2C_HandleTypeDef I2cHandle;

// ----- Variables To Be Exported --------------------------------------------

int ledPin = 0;

// ------ Main() program -----------------------------------------------------

int main(int argc, char* argv[])
{
	/* ---------- Initialization phase of the program, used to initialize system stuff ---------- */
	HAL_Init();
	SystemClock_Config();
	timer_start(); 		// timer for led blinking test


	/* ---------- Initialization phase of the program, used to initialize peripherals ---------- */
	ButtonInit();
	blink_led_init();
	InitUSART3();
	// InitUSART2();
	InitSPI();
	InitI2C();




	/* ---------- Initialization phase of the program, used to initialize interrupts ---------- */
	EXTI15_10_IRQHandler_Config();



	/* Variable space for the main program*/

	// ----- UART variables -----
	char uart_input_buffer[5];

	// ----- SPI variables -----
	char spi_master_receive_buffer[5];
	char spi_slave_receive_buffer[5];
	char spi_slave_transmit_buffer[] = "_ACK_"; 	// acknowledge message for successful SPI communication

	// ----- I2C variables -----
	char i2c_master_transmit_buffer[] = "TEST I2C KOMUNIKACIJA!";
	char i2c_slave_receive_buffer[sizeof(i2c_master_transmit_buffer)];
	char i2c_uart_master_input[10];
	char i2c_uart_slave_output[10];

	// ----- Timer variables -----
	uint32_t seconds = 0;
	uint32_t count = 0;


	/* Executing part of the program */
	while (1)
	{
		/* ----------------------------------------------
		 * --- Testing the I2CwithUART communication ----
		 * ----------------------------------------------
#ifdef MASTER_BOARD_I2C
		GPIO_PinState pin_state = GPIO_PIN_SET; 	//HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13); // Used for reading on board button state
		if(pin_state == GPIO_PIN_SET) {
			HAL_UART_Receive(&UartHandle, &i2c_uart_master_input, (uint16_t)sizeof(i2c_uart_master_input), HAL_MAX_DELAY);
			HAL_StatusTypeDef return_value = HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)I2C_ADDRESS, (uint8_t*)i2c_uart_master_input, sizeof(i2c_uart_master_input), 5000u);
			if(return_value == HAL_OK) {
				ledPin = 0;
				char i2c_master_receive_buffer[5];
				timer_sleep(100u);
				HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)I2C_ADDRESS, (uint8_t*)i2c_master_receive_buffer, sizeof(i2c_master_receive_buffer), 5000u);
				HAL_UART_Transmit(&UartHandle, i2c_master_receive_buffer, sizeof(i2c_master_receive_buffer), HAL_MAX_DELAY);
			} else if(return_value == HAL_TIMEOUT) {
				ledPin = 7;
			} else if(return_value == HAL_ERROR) {
				ledPin = 14;
			}
			blink_led_on();
			timer_sleep(seconds == 0 ? 1000u : 1000u);
			blink_led_off();
		}
#else
		HAL_StatusTypeDef return_value = HAL_I2C_Slave_Receive(&I2cHandle, (uint8_t*)i2c_uart_slave_output, sizeof(i2c_uart_slave_output), 5000u);
		if(return_value == HAL_OK) { // !strcmp(i2c_uart_slave_output, "TEST I2C KOMUNIKACIJA!") // for second task
			HAL_UART_Transmit(&UartHandle, i2c_uart_slave_output, sizeof(i2c_uart_slave_output), HAL_MAX_DELAY);
			ledPin = 0;
			char i2c_slave_transmit_buffer[] = "_ACK_";
			HAL_I2C_Slave_Transmit(&I2cHandle, (uint8_t*)i2c_slave_transmit_buffer, sizeof(i2c_slave_transmit_buffer), 5000u);
		} else if(return_value == HAL_TIMEOUT) {
			HAL_UART_Transmit(&UartHandle, "HAL_TIMEOUT_UART_RECEIVE", sizeof("HAL_TIMEOUT_UART_RECEIVE"), HAL_MAX_DELAY);
			ledPin = 7;
		} else if(return_value == HAL_ERROR) {
			HAL_UART_Transmit(&UartHandle, "HAL_ERROR_UART_RECEIVE", sizeof("HAL_ERROR_UART_RECEIVE"), HAL_MAX_DELAY);
			ledPin = 14;
		}
		blink_led_on();
		timer_sleep(seconds == 0 ? 1000u : 1000u);
		blink_led_off();
#endif
		End of testing I2CwithUART communication ----- */




		/* ----------------------------------------------
		 * --- testing the SPIwithUART communication ----
		 * ----------------------------------------------
		// GPIO_PinState pin_State = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13); //used for reading onboard button state
#ifdef MASTER_BOARD_SPI
		HAL_UART_Receive(&UartHandle, &uart_input_buffer, (uint16_t)sizeof(uart_input_buffer), HAL_MAX_DELAY);

		HAL_StatusTypeDef returnValue = HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*)uart_input_buffer, (uint8_t*)spi_master_receive_buffer, sizeof(uart_input_buffer), 5000 );
		if(returnValue == HAL_OK) {
			HAL_UART_Transmit(&UartHandle, spi_master_receive_buffer, sizeof(spi_master_receive_buffer), HAL_MAX_DELAY);
			ledPin = 0;
		} else if(returnValue == HAL_TIMEOUT) {
			ledPin = 7;
		} else if(returnValue == HAL_ERROR) {
			ledPin = 14;
		}
		blink_led_on();
		timer_sleep(seconds == 0 ? 2000u : 2000u);
		blink_led_off();

#else
		HAL_StatusTypeDef returnValue = HAL_SPI_TransmitReceive(&SpiHandle, (uint8_t*)spi_slave_transmit_buffer, (uint8_t*)spi_slave_receive_buffer, sizeof(spi_slave_receive_buffer), 5000);
		if(returnValue == HAL_OK) {
			HAL_UART_Transmit(&UartHandle, spi_slave_receive_buffer, sizeof(spi_slave_receive_buffer), HAL_MAX_DELAY);
			ledPin = 0;
		} else if(returnValue == HAL_TIMEOUT) {
			HAL_UART_Transmit(&UartHandle, "HAL_TIMEOUT", sizeof("HAL_TIMEOUT"), HAL_MAX_DELAY);
			ledPin = 7;
		} else if(returnValue == HAL_ERROR) {
			HAL_UART_Transmit(&UartHandle, "HAL_ERROR", sizeof("HAL_ERROR"), HAL_MAX_DELAY);
			ledPin = 14;
		}
		blink_led_on();
		timer_sleep(seconds == 0 ? 2000u : 2000u);
		blink_led_off();
#endif
		---------- End of testing SPIwithUART communication ----- */



		/* ----------------------------------------------
		 * ----- testing the UART communication ---------
		 * ----------------------------------------------
		while(1){
			HAL_UART_Receive(&UartHandle, &buffer_receive, sizeof(buffer_receive), HAL_MAX_DELAY);
			if (buffer_receive[0] == 13 || count == 5) {
				count = 0;
				break;
			}
			buffer[count++] = buffer_receive[0];
		}
		if(!strncmp(buffer, "green", 5)){	// Zelena led dioda
			ledPin = 0;
			HAL_UART_Transmit(&UartHandle, "zelena ", 7, HAL_MAX_DELAY);
		}
		else if(!strncmp(buffer, "blue", 4)){	// Plava led dioda
			ledPin = 7;
			HAL_UART_Transmit(&UartHandle, "plava ", 6, HAL_MAX_DELAY);
		}
		else if(!strncmp(buffer, "red", 3)){	// Crvena led dioda
			ledPin = 14;
			HAL_UART_Transmit(&UartHandle, "crvena ", 7, HAL_MAX_DELAY);
		}
		---------- End of testing UART communication */



		/* ----------------------------------------------
		 * ----- Changing the LED color ---------
		 * ----------------------------------------------
		if(buffer[0] == '1')	// Zelena led dioda
			ledPin = 0;
		else if(buffer[0] == '2')	// Plava led dioda
			ledPin = 7;
		else if(buffer[0] == '3')	// Crvena led dioda
			ledPin = 14;
		---------- End of testing LED color -------- */

		/* ----------------------------------------------
		 * -------- LED blinking testing ------------
		 * ----------------------------------------------
		blink_led_on();
		timer_sleep(seconds == 0 ? TIMER_FREQUENCY_HZ : BLINK_ON_TICKS);
		blink_led_off();
		timer_sleep(BLINK_OFF_TICKS);
		++seconds;
		---------- End of LED blinking testing ------------*/
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

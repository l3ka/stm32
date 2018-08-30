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
#include "MPU_Handler.h"
#include "Timer_Handler.h"
#include "MadgwickAHRS.h"

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

// --------------------- MadgwickAHRS -------------------

extern float realGyroX;
extern float realGyroY;
extern float realGyroZ;
extern float realAccelX;
extern float realAccelY;
extern float realAccelZ;
extern float realMagX;
extern float realMagY;
extern float realMagZ;

extern volatile float q0;
extern volatile float q1;
extern volatile float q2;
extern volatile float q3;

// ----- Variables To Be Exported --------------------------------------------

int ledPin = 0;

// ------ Main() program -----------------------------------------------------

int main(int argc, char* argv[])
{
	/* ---------- Initialization phase of the program, used to initialize system stuff ----------
	HAL_Init();
	SystemClock_Config();
	timer_start(); 		// timer for led blinking test
	---------------------------- End ---------------------------------------------------------- */

	/* ---------- Initialization phase of the program, used to initialize peripherals ---------- */
	ButtonInit();
	blink_led_init();
	InitUSART3();
	// InitUSART2();
	InitSPI();
	InitI2C();
	// InitTimer();

	/* ---------- Initialization phase of the program, used to initialize interrupts ---------- */
	// EXTI15_10_IRQHandler_Config();



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

	// ----- MPU variables -----
	HAL_StatusTypeDef status = HAL_ERROR;

	char i2c_mpu_write_buffer[30] = { 0 };
	float realTemp = 0.0f;

	char i2c_mpu_temp_buffer[2] = { 0 };
	char i2c_mpu_gyro_buffer[6] = { 0 };
	char gyro_config = 0;
	char accel_config = 0;
	char i2c_mpu_gyro_write_buffer[100] = { 0 };
	char i2c_mpu_accel_buffer[6] = { 0 };
	char i2c_mpu_accel_write_buffer[100] = { 0 };

	char mag_config = 111;
	char i2c_mpu_mag_buffer[7] = { 0 };
	char i2c_mpu_mag_write_buffer[100] = { 0 };
	char magnetic_status_1 = 0;

	char i2c_mpu_quaternion_data[100] = { 0 };

	uint16_t intgerTemperature = 0;
	uint16_t mask = 0;

	// ----- Timer variables -----
	uint32_t seconds = 0;
	uint32_t count = 0;

	/* ----- Configuring the bypass mode -------------------------------------*/
	HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x37, 1, &mag_config, 1, HAL_MAX_DELAY);
	mag_config &= 0xFD;
	mag_config |= 0x02;
	HAL_I2C_Mem_Write(&I2cHandle, 0xD0u, 0x37u, 1, &mag_config, 1, HAL_MAX_DELAY);
	/*--------------------End of config the bypass mode ------------------------------------------*/


	/* ----- Configuring the gyroscope ---------------
	 * GYRO_SENSITIVITY_FS_0 */
	HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x1B, 1, &gyro_config, 1, HAL_MAX_DELAY);
	gyro_config &= 0xEF;
	gyro_config |= 0x08;
	HAL_I2C_Mem_Write(&I2cHandle, 0xD0u, 0x1Bu, 1, &gyro_config, 1, HAL_MAX_DELAY);

	/* * GYRO_SENSITIVITY_FS_0
	HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x1A, 1, &gyro_config, 1, HAL_MAX_DELAY);
	gyro_config &= 0xF8;
	gyro_config |= 0x03;
	HAL_I2C_Mem_Write(&I2cHandle, 0xD0u, 0x1A, 1, &gyro_config, 1, HAL_MAX_DELAY);
	*/

	/* ----- Configuring the accelerometer -------------------------------------
	 * ACCEL_SENSITIVITY_FS_1 */
	HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x1C, 1, &accel_config, 1, HAL_MAX_DELAY);
	accel_config &= 0xEF;
	accel_config |= 0x08;
	HAL_I2C_Mem_Write(&I2cHandle, 0xD0u, 0x1C, 1, &accel_config, 1, HAL_MAX_DELAY);
	/* --------------------End of config ------------------------------------------*/

	/* ----- Configuring the magnetometer ------------------------------------- */
	HAL_I2C_Mem_Read(&I2cHandle, 0x18u, 0x0A, 1, &mag_config, 1, HAL_MAX_DELAY);
	mag_config &= 0xE0;
	mag_config |= 0x16;
	HAL_I2C_Mem_Write(&I2cHandle, 0x18u, 0x0A, 1, &mag_config, 1, HAL_MAX_DELAY);
	/* -------------------- End of config magnetometer ------------------------------------------*/




	/* ----- Executing part of the program */
	while (1)
	{
		// Read temperature from MPU-9250
		/// HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x41, 1, (uint8_t*)i2c_mpu_temp_buffer, 2, HAL_MAX_DELAY);

		// Read gyroscope from MPU-9250
		HAL_StatusTypeDef status_gyro = HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x43, 1, (uint8_t*)i2c_mpu_gyro_buffer, 6, HAL_MAX_DELAY);

		// Read accelerometer from MPU-9250
		HAL_StatusTypeDef status_accel = HAL_I2C_Mem_Read(&I2cHandle, 0xD0u, 0x3B, 1, (uint8_t*)i2c_mpu_accel_buffer, 6, HAL_MAX_DELAY);

		// Read magnetometer from MPU-9250
		// resetMagnetometer();
		HAL_StatusTypeDef status_mag = HAL_I2C_Mem_Read(&I2cHandle, 0x18u, 0x02, 1, &magnetic_status_1, 1, HAL_MAX_DELAY);

		if((magnetic_status_1 & 0x01) == 1){
			status = HAL_I2C_Mem_Read(&I2cHandle, 0x18u, 0x03, 1, (uint8_t*)i2c_mpu_mag_buffer, 7, HAL_MAX_DELAY);
		}

		if(status_gyro == HAL_OK && status_accel == HAL_OK && status_mag == HAL_OK) {
			ledPin = 0;

			/* -----------------------------------------------------------------------------------
			 * ----------------- parsing the temperature through the function ----------------------
			 * -----------------------------------------------------------------------------------
			readingTheTemperature(i2c_mpu_temp_buffer, i2c_mpu_write_buffer);
			HAL_UART_Transmit(&UartHandle, (uint8_t*)i2c_mpu_write_buffer, sizeof(i2c_mpu_write_buffer), HAL_MAX_DELAY);
			 ----------------- End of parsing the temperature through the function------------------*/

			/* -----------------------------------------------------------------------------------
			 * ----------------- parsing the gyroscope through the function ----------------------
			 * ----------------------------------------------------------------------------------- */
			readingTheGyroscope(i2c_mpu_gyro_buffer, i2c_mpu_gyro_write_buffer);
			// HAL_UART_Transmit(&UartHandle, (uint8_t*)i2c_mpu_gyro_write_buffer, sizeof(i2c_mpu_gyro_write_buffer), HAL_MAX_DELAY);
			/* ---------------- End of parsing the gyroscope through the function ------------- */

			/* -----------------------------------------------------------------------------------
			 * ----------------- parsing the accelerometer through the function ----------------------
			 * ----------------------------------------------------------------------------------- */
			readingTheAccel(i2c_mpu_accel_buffer, i2c_mpu_accel_write_buffer);
			// HAL_UART_Transmit(&UartHandle, (uint8_t*)i2c_mpu_accel_write_buffer, sizeof(i2c_mpu_accel_write_buffer), HAL_MAX_DELAY);
			/*  ----------------- End of parsing the accelerometer through the function------------------*/


			/* -----------------------------------------------------------------------------------
			 * ----------------- parsing the magnetometer through the function ----------------------
			 * ----------------------------------------------------------------------------------- */
			readingTheMagnetometer(i2c_mpu_mag_buffer, i2c_mpu_mag_write_buffer);
			// HAL_UART_Transmit(&UartHandle, (uint8_t*)i2c_mpu_mag_write_buffer, sizeof(i2c_mpu_mag_write_buffer), HAL_MAX_DELAY);
			/* ----------------- End of parsing the magnetometer through the function------------------*/


			/* -----------------------------------------------------------------------------------
			 * ----------------- parsing the MadgwickAHRS through the function ----------------------
			 * ----------------------------------------------------------------------------------- */
			MadgwickAHRSupdate(realGyroX, realGyroY, realGyroZ, realAccelX, realAccelY, realAccelZ, realMagX, realMagY, realMagZ);
			for (int i = 0; i < sizeof(i2c_mpu_quaternion_data); ++i)
				i2c_mpu_quaternion_data[i] = 0;
			//snprintf(i2c_mpu_quaternion_data, 100, "Q1: %.4f Q2: %.4f Q3: %.4f Q4: %.4f\n", q0, q1, q2, q3);
			snprintf(i2c_mpu_quaternion_data, 100, "%.4f %.4f %.4f %.4f\n", q0, q1, q2, q3);
			HAL_UART_Transmit(&UartHandle, (uint8_t*)i2c_mpu_quaternion_data, sizeof(i2c_mpu_quaternion_data), HAL_MAX_DELAY);
			/* ---------------- End parsing the MadgwickAHRS through the function -----------------------*/
		} else if(status == HAL_TIMEOUT) {
			ledPin = 7;
		} else if(status == HAL_ERROR) {
			ledPin = 14;
		} else if(status == HAL_BUSY) {
			ledPin = 7;
			blink_led_on();
			ledPin = 14;
			blink_led_on();
			timer_sleep(2000u);
			blink_led_off();
			ledPin = 7;
			blink_led_off();
		}
		blink_led_on();
		timer_sleep(100u);
		blink_led_off();


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

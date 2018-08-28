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

#include "Interrupt_Handler.h"

// ----- Variables To Be Exported --------------------------------------------


int LED_FLAG = 0;
volatile int UART_TRANSMIT_IT_FLAG = 0;
volatile int UART_RECEIVE_IT_FLAG = 0;
char uart_transmit_it_buffer[] = "BUTTON PRESSED!\n";
char uart_receive_it_buffer[1];


// ----- EXTERN VARIABLES -----------------------------------------------------

extern int ledPin;
extern UART_HandleTypeDef UartHandle;


// ----- Function Definitions -------------------------------------------------

 void EXTI15_10_IRQHandler_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;

	/* Enable GPIOC clock */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* Configure PC.13 pin as input floating */
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_13;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Enable and set EXTI line 15_10 Interrupt to the lowest priority */
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
 {
	 static uint32_t last_interrupt_time = 0;
	 uint32_t current_interrupt_time = HAL_GetTick();
	 if(current_interrupt_time - last_interrupt_time > 150u) {
		 if (GPIO_Pin == GPIO_PIN_13) {
			 /* ----- LED toggle with UART interrupt ------- */
			 HAL_UART_Transmit_IT(&UartHandle, (uint8_t*)uart_transmit_it_buffer, sizeof(uart_transmit_it_buffer));
			 while(!UART_TRANSMIT_IT_FLAG);
			 UART_TRANSMIT_IT_FLAG = 0;

			 HAL_UART_Receive_IT(&UartHandle, (uint8_t*)uart_receive_it_buffer, sizeof(uart_receive_it_buffer));
			 while(!UART_RECEIVE_IT_FLAG);
			 UART_RECEIVE_IT_FLAG = 0;

			 blink_led_off();

			 if(uart_receive_it_buffer[0] == '1') {
				 ledPin = 0;
			 }
			 else if(uart_receive_it_buffer[0] == '2') {
				 ledPin = 7;
			 }
			 else if(uart_receive_it_buffer[0] == '3') {
				 ledPin = 14;
			 }

			 blink_led_on();
			 /* ----- End of LED toggle with UART interrupt ------- */



			 /* ---------- LED toggle with interrupt ------------
			 ledPin = 7;
		 	 if(LED_FLAG == 0) {
		 	 	 blink_led_on();
		 		 LED_FLAG = 1;
		 	 }
		 	 else {
		 		 blink_led_off();
		 		 LED_FLAG = 0;
		 	 }
		 	 ------- End of LED toggle with interrupt ---------- */
		 }
	 }
	 last_interrupt_time = current_interrupt_time;

 }

 void EXTI15_10_IRQHandler(void)
 {
	 HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
 }

 void USART3_IRQHandler(void)
 {
	 HAL_UART_IRQHandler(&UartHandle);
 }

 void HAL_UART_RxCpltCallback(UART_HandleTypeDef* UartHandle)
 {
	 UART_RECEIVE_IT_FLAG = 1;
 }

 void HAL_UART_TxCpltCallback(UART_HandleTypeDef* UartHandle)
 {
	 UART_TRANSMIT_IT_FLAG = 1;
 }

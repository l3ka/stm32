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

#include "Timer_Handler.h"

// ----- Definitions ----------------------------------------------------------


// ----- Variables To Be Exported ---------------------------------------------

TIM_HandleTypeDef TimHandle;
uint16_t uwPrescalerValue = 0;

// ----- Function Definitions -------------------------------------------------

void InitTimer()
{
	/* Compute the prescaler value to have TIM3 counter clock equal to 10 KHz */
	uwPrescalerValue = (uint32_t) ((SystemCoreClock /2) / 10000) - 1;

	/* Set TIMx instance */
	TimHandle.Instance = TIM3;

	/* Initialize TIM3 peripheral as follows:
	   + Period = 10000 - 1
	   + Prescaler = ((SystemCoreClock/2)/10000) - 1
	   + ClockDivision = 0
	   + Counter direction = Up
	*/
	TimHandle.Init.Period = 50 - 1;
	TimHandle.Init.Prescaler = uwPrescalerValue;
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;


	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM3_CLK_ENABLE();

	/*##-2- Configure the NVIC for TIMx ########################################*/
	/* Set the TIMx priority */
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM3_IRQn);


	HAL_TIM_Base_Init(&TimHandle);

	HAL_TIM_Base_Start_IT(&TimHandle);

}



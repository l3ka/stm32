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

#include "SPI_Handler.h"

// ----- Variables To Be Exported ---------------------------------------------

SPI_HandleTypeDef SpiHandle;

// ----- Function Definitions -------------------------------------------------

void InitSPI(void){
	SpiHandle.Instance               = SPI1;
	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
	SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle.Init.CRCPolynomial     = 7;
	SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;

#ifdef MASTER_BOARD
	SpiHandle.Init.Mode = SPI_MODE_MASTER;
#else
	SpiHandle.Init.Mode = SPI_MODE_SLAVE;
#endif /* MASTER_BOARD */

	HAL_SPI_Init(&SpiHandle);
}


void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable GPIO TX/RX clock */
	SPIx_SCK_GPIO_CLK_ENABLE();
	SPIx_MISO_GPIO_CLK_ENABLE();
	SPIx_MOSI_GPIO_CLK_ENABLE();
	/* Enable SPI clock */
	SPIx_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/* SPI SCK GPIO pin configuration  */
	GPIO_InitStruct.Pin       = GPIO_PIN_5;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* SPI MISO GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* SPI MOSI GPIO pin configuration  */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

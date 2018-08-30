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

#include "I2C_Handler.h"

// ----- Variables To be Exported -----------------------------

I2C_HandleTypeDef I2cHandle;

// ---- Function Definitions ----------------------------------

void InitI2C(){
	I2cHandle.Instance             = I2C1;
	I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
	I2cHandle.Init.ClockSpeed      = 100000;
	I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
	I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
	I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
	I2cHandle.Init.OwnAddress2     = 0xFE; 		// Used for dual addressing mode
	HAL_I2C_Init(&I2cHandle);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable GPIO Clocks #################################################*/
  /* Enable GPIO SCL/SDA clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE(); // Useful if pins are on different ports

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* I2C SCL GPIO pin configuration  */
  GPIO_InitStruct.Pin       = GPIO_PIN_8;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* I2C SDA GPIO pin configuration  */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*##-3- Enable I2C peripheral Clock ########################################*/
  /* Enable I2C1 clock */
  __HAL_RCC_I2C1_CLK_ENABLE();
}

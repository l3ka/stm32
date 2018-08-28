################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/BlinkLed.c \
../src/I2C_Handler.c \
../src/Interrupt_Handler.c \
../src/SPI_Handler.c \
../src/Timer.c \
../src/USART_Handler.c \
../src/_initialize_hardware.c \
../src/_write.c \
../src/main.c \
../src/stm32f4xx_hal_msp.c \
../src/stm32f4xx_hal_uart.c 

OBJS += \
./src/BlinkLed.o \
./src/I2C_Handler.o \
./src/Interrupt_Handler.o \
./src/SPI_Handler.o \
./src/Timer.o \
./src/USART_Handler.o \
./src/_initialize_hardware.o \
./src/_write.o \
./src/main.o \
./src/stm32f4xx_hal_msp.o \
./src/stm32f4xx_hal_uart.o 

C_DEPS += \
./src/BlinkLed.d \
./src/I2C_Handler.d \
./src/Interrupt_Handler.d \
./src/SPI_Handler.d \
./src/Timer.d \
./src/USART_Handler.d \
./src/_initialize_hardware.d \
./src/_write.d \
./src/main.d \
./src/stm32f4xx_hal_msp.d \
./src/stm32f4xx_hal_uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F446xx -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/stm32f4xx_hal_msp.o: ../src/stm32f4xx_hal_msp.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F446xx -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f4-hal" -std=gnu11 -Wno-missing-prototypes -Wno-missing-declarations -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/stm32f4xx_hal_msp.d" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



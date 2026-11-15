################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/USART_Debug/usart2.c 

OBJS += \
./Core/USART_Debug/usart2.o 

C_DEPS += \
./Core/USART_Debug/usart2.d 


# Each subdirectory must supply rules for building sources it contributes
Core/USART_Debug/%.o Core/USART_Debug/%.su Core/USART_Debug/%.cyclo: ../Core/USART_Debug/%.c Core/USART_Debug/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/Embedded_system/STM32/projects/STM32F411_F330_Flight_Controller/STM32F411_F330_Flight_Controller/Core/MPU6050" -I"D:/Embedded_system/STM32/projects/STM32F411_F330_Flight_Controller/STM32F411_F330_Flight_Controller/Core/USART_Debug" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-USART_Debug

clean-Core-2f-USART_Debug:
	-$(RM) ./Core/USART_Debug/usart2.cyclo ./Core/USART_Debug/usart2.d ./Core/USART_Debug/usart2.o ./Core/USART_Debug/usart2.su

.PHONY: clean-Core-2f-USART_Debug


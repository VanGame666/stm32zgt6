################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Device/AT24C02.c \
../Device/ProtocolProcessing.c \
../Device/SoftwareCRC.c 

OBJS += \
./Device/AT24C02.o \
./Device/ProtocolProcessing.o \
./Device/SoftwareCRC.o 

C_DEPS += \
./Device/AT24C02.d \
./Device/ProtocolProcessing.d \
./Device/SoftwareCRC.d 


# Each subdirectory must supply rules for building sources it contributes
Device/%.o Device/%.su Device/%.cyclo: ../Device/%.c Device/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"E:/工作/STM32F407ZGT6_My/Device" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Device

clean-Device:
	-$(RM) ./Device/AT24C02.cyclo ./Device/AT24C02.d ./Device/AT24C02.o ./Device/AT24C02.su ./Device/ProtocolProcessing.cyclo ./Device/ProtocolProcessing.d ./Device/ProtocolProcessing.o ./Device/ProtocolProcessing.su ./Device/SoftwareCRC.cyclo ./Device/SoftwareCRC.d ./Device/SoftwareCRC.o ./Device/SoftwareCRC.su

.PHONY: clean-Device


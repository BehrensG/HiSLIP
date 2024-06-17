################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/HiSLIP/Src/HiSLIP_Either.c \
../Core/HiSLIP/Src/HiSLIP_Support.c \
../Core/HiSLIP/Src/HiSLIP_Sync.c \
../Core/HiSLIP/Src/HiSLIP_Task.c \
../Core/HiSLIP/Src/HiSLIP_aSync.c 

OBJS += \
./Core/HiSLIP/Src/HiSLIP_Either.o \
./Core/HiSLIP/Src/HiSLIP_Support.o \
./Core/HiSLIP/Src/HiSLIP_Sync.o \
./Core/HiSLIP/Src/HiSLIP_Task.o \
./Core/HiSLIP/Src/HiSLIP_aSync.o 

C_DEPS += \
./Core/HiSLIP/Src/HiSLIP_Either.d \
./Core/HiSLIP/Src/HiSLIP_Support.d \
./Core/HiSLIP/Src/HiSLIP_Sync.d \
./Core/HiSLIP/Src/HiSLIP_Task.d \
./Core/HiSLIP/Src/HiSLIP_aSync.d 


# Each subdirectory must supply rules for building sources it contributes
Core/HiSLIP/Src/%.o Core/HiSLIP/Src/%.su Core/HiSLIP/Src/%.cyclo: ../Core/HiSLIP/Src/%.c Core/HiSLIP/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -DUSE_HAL_DRIVER -DDATA_IN_D2_SRAM -DSTM32H743xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I"D:/tmp/HiSLIP/SOFTWARE/Core/SCPI" -I"D:/tmp/HiSLIP/SOFTWARE/Core/SCPI/libscpi/inc/scpi" -I"D:/tmp/HiSLIP/SOFTWARE/Core/SCPI/libscpi/inc" -I"D:/tmp/HiSLIP/SOFTWARE/Core/HiSLIP/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Middlewares/Third_Party/LwIP/src/apps/http -O1 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-HiSLIP-2f-Src

clean-Core-2f-HiSLIP-2f-Src:
	-$(RM) ./Core/HiSLIP/Src/HiSLIP_Either.cyclo ./Core/HiSLIP/Src/HiSLIP_Either.d ./Core/HiSLIP/Src/HiSLIP_Either.o ./Core/HiSLIP/Src/HiSLIP_Either.su ./Core/HiSLIP/Src/HiSLIP_Support.cyclo ./Core/HiSLIP/Src/HiSLIP_Support.d ./Core/HiSLIP/Src/HiSLIP_Support.o ./Core/HiSLIP/Src/HiSLIP_Support.su ./Core/HiSLIP/Src/HiSLIP_Sync.cyclo ./Core/HiSLIP/Src/HiSLIP_Sync.d ./Core/HiSLIP/Src/HiSLIP_Sync.o ./Core/HiSLIP/Src/HiSLIP_Sync.su ./Core/HiSLIP/Src/HiSLIP_Task.cyclo ./Core/HiSLIP/Src/HiSLIP_Task.d ./Core/HiSLIP/Src/HiSLIP_Task.o ./Core/HiSLIP/Src/HiSLIP_Task.su ./Core/HiSLIP/Src/HiSLIP_aSync.cyclo ./Core/HiSLIP/Src/HiSLIP_aSync.d ./Core/HiSLIP/Src/HiSLIP_aSync.o ./Core/HiSLIP/Src/HiSLIP_aSync.su

.PHONY: clean-Core-2f-HiSLIP-2f-Src


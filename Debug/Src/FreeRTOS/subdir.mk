################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/FreeRTOS/croutine.c \
../Src/FreeRTOS/event_groups.c \
../Src/FreeRTOS/list.c \
../Src/FreeRTOS/queue.c \
../Src/FreeRTOS/tasks.c \
../Src/FreeRTOS/timers.c 

C_DEPS += \
./Src/FreeRTOS/croutine.d \
./Src/FreeRTOS/event_groups.d \
./Src/FreeRTOS/list.d \
./Src/FreeRTOS/queue.d \
./Src/FreeRTOS/tasks.d \
./Src/FreeRTOS/timers.d 

OBJS += \
./Src/FreeRTOS/croutine.o \
./Src/FreeRTOS/event_groups.o \
./Src/FreeRTOS/list.o \
./Src/FreeRTOS/queue.o \
./Src/FreeRTOS/tasks.o \
./Src/FreeRTOS/timers.o 


# Each subdirectory must supply rules for building sources it contributes
Src/FreeRTOS/%.o Src/FreeRTOS/%.su: ../Src/FreeRTOS/%.c Src/FreeRTOS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32F103RCTx -DSTM32 -DSTM32F1 -DUSE_STDPERIPH_DRIVER -DSTM32F10X_HD -c -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/FreeRTOS/portable/GCC/ARM_CM3" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/BSP" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/BSP/CMSIS/CM3/DeviceSupport/ST/STM32F10x" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/BSP/CMSIS/CM3/CoreSupport" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/EKF" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/FatFs" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/FreeRTOS/include" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/System" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/STM32F10x_StdPeriph_Driver/inc" -I"D:/ProgramData/STM32CubeIDE/workspace_1.10.1/Tank_Dual_Pro/Src/User" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src-2f-FreeRTOS

clean-Src-2f-FreeRTOS:
	-$(RM) ./Src/FreeRTOS/croutine.d ./Src/FreeRTOS/croutine.o ./Src/FreeRTOS/croutine.su ./Src/FreeRTOS/event_groups.d ./Src/FreeRTOS/event_groups.o ./Src/FreeRTOS/event_groups.su ./Src/FreeRTOS/list.d ./Src/FreeRTOS/list.o ./Src/FreeRTOS/list.su ./Src/FreeRTOS/queue.d ./Src/FreeRTOS/queue.o ./Src/FreeRTOS/queue.su ./Src/FreeRTOS/tasks.d ./Src/FreeRTOS/tasks.o ./Src/FreeRTOS/tasks.su ./Src/FreeRTOS/timers.d ./Src/FreeRTOS/timers.o ./Src/FreeRTOS/timers.su

.PHONY: clean-Src-2f-FreeRTOS

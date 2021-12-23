################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/logs.c \
../src/main.c \
../src/manejo.c \
../src/mensajes.c 

OBJS += \
./src/logs.o \
./src/main.o \
./src/manejo.o \
./src/mensajes.o 

C_DEPS += \
./src/logs.d \
./src/main.d \
./src/manejo.d \
./src/mensajes.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-2c-cuatri-ganado/biblioteca_compartida/src" -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



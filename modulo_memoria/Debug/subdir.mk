################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../client.c \
../comunicacion.c \
../dumps.c \
../init_memoria.c \
../logs.c \
../paginacion.c \
../utils.c 

OBJS += \
./client.o \
./comunicacion.o \
./dumps.o \
./init_memoria.o \
./logs.o \
./paginacion.o \
./utils.o 

C_DEPS += \
./client.d \
./comunicacion.d \
./dumps.d \
./init_memoria.d \
./logs.d \
./paginacion.d \
./utils.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-2c-cuatri-ganado/biblioteca_compartida/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



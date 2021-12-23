################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/client.c \
../src/comunicacion.c \
../src/init_memoria.c \
../src/paginacion.c \
../src/utils.c 

OBJS += \
./src/client.o \
./src/comunicacion.o \
./src/init_memoria.o \
./src/paginacion.o \
./src/utils.o 

C_DEPS += \
./src/client.d \
./src/comunicacion.d \
./src/init_memoria.d \
./src/paginacion.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-2c-cuatri-ganado/biblioteca_compartida/src" -O2 -g -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/comunicacion.o: ../src/comunicacion.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-2021-2c-cuatri-ganado/biblioteca_compartida/src" -O2 -g -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/comunicacion.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



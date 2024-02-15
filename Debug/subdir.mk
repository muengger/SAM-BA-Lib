################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Helpers.cpp \
../IntelHexBin.cpp \
../SAMBALib.cpp 

OBJS += \
./Helpers.o \
./IntelHexBin.o \
./SAMBALib.o 

CPP_DEPS += \
./Helpers.d \
./IntelHexBin.d \
./SAMBALib.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	/home/sedo/Arch_Toolchain/BuildDir/host/usr/bin/i686-buildroot-linux-gnu-g++ -std=c++0x -I/home/sedo/workspace/LIB_RFID_NFC_SEDO -I/home/sedo/Arch_Toolchain/BuildDir/host/usr/i686-buildroot-linux-gnu/sysroot/usr/include/PCSC -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



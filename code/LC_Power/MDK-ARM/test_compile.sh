#!/bin/bash
echo "Testing bsp_gpio.c compilation..."
arm-none-eabi-gcc -c ../Layer/Bsp/bsp_gpio.c \
  -I../Layer/Bsp \
  -I../Core/Inc \
  -I../Drivers/STM32F3xx_HAL_Driver/Inc \
  -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include \
  -I../Drivers/CMSIS/Include \
  -DSTM32F334x8 \
  -mcpu=cortex-m4 \
  -mthumb \
  -o bsp_gpio.o 2>&1

/**
 * @file ads1115_interface.c
 * @brief ADS1115 I2C read/write and delay callback function implementations
 * @author Şükrü Can Kılıç
 * @date 19.01.2026
 */

#include "ads1115_interface.h"

uint8_t ads1115_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    /* Write your platform code for I2C write function (STM32, ESP32, ARDUINO etc.) */

    return 0;
}

uint8_t ads1115_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    /* Write your platform code for I2C read function (STM32, ESP32, ARDUINO etc.) */

    return 0;
}

void ads1115_delay_ms(uint32_t milliseconds)
{
    /* Write your platform code for delay in milliseconds function (STM32, ESP32, ARDUINO etc.) */
}
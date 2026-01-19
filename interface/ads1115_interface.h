/**
 * @file ads1115_interface.h
 * @brief ADS1115 I2C read/write and delay callback function declarations
 * @author Şükrü Can Kılıç
 * @date 19.01.2026
 */

#include "ads1115.h"

/**
 * @brief ADS1115 I2C write function (ads1115_i2c_write_t callback)
 * 
 * @param i2c_addr ADS1115 I2C address
 * @param reg_addr ADS1115 register address
 * @param data Pointer to register value
 * @param len Length of register value
 * @return Error code
 */
uint8_t ads1115_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief ADS1115 I2C read function (ads1115_i2c_read_t callback)
 * 
 * @param i2c_addr ADS1115 I2C address
 * @param reg_addr ADS1115 register address
 * @param data Pointer to register value
 * @param len Length of register value
 * @return Error code
 */
uint8_t ads1115_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
 * @brief ADS1115 delay_ms function (ads1115_delay_ms_t callback)
 * 
 * @param milliseconds Delay time in milliseconds
 */
void ads1115_delay_ms(uint32_t milliseconds);
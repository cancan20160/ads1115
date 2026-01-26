/**
 * @file ads1115_interface.h
 * @brief ADS1115 Hardware Abstraction Layer - Interface Declarations
 * @version 1.0.0
 * @author Şükrü Can Kılıç
 * @date 26-01-2026
 *
 * @details This file defines the hardware-specific callback implementations.
 * These functions bridge the ADS1115 generic driver with the specific MCU
 * I2C and Delay peripherals (e.g., STM32 HAL, ESP-IDF, Arduino).
 */

#ifndef ADS1115_INTERFACE_H
#define ADS1115_INTERFACE_H

#include "ads1115.h"

/**
 * @defgroup ADS1115_Interface Hardware Interface
 * @ingroup ADS1115_Driver
 * @brief Platform-specific implementation of I/O operations.
 * @{
 */

/*===========================================================================*/
/* PUBLIC INTERFACE FUNCTIONS                                                */
/*===========================================================================*/

/**
 * @brief Platform-specific I2C write implementation.
 * @details This function must be assigned to @ref ads1115_i2c_write_t in the handle.
 * It follows the standard I2C write protocol:
 * 1. Start condition
 * 2. Send Slave Address (Write bit)
 * 3. Send Register Address
 * 4. Send Data bytes (MSB first)
 * 5. Stop condition
 *
 * @param i2c_addr  ADS1115 7-bit I2C slave address.
 * @param reg_addr  Target register address inside the ADS1115.
 * @param data      Pointer to the buffer containing data to write.
 * @param len       Number of bytes to be written (usually 2 for 16-bit registers).
 * @return true if write was successful, false otherwise.
 */
bool ads1115_i2c_write(uint8_t i2c_addr, uint8_t reg_addr, const uint8_t *data, uint8_t len);

/**
 * @brief Platform-specific I2C read implementation.
 * @details This function must be assigned to @ref ads1115_i2c_read_t in the handle.
 * It typically follows the "Restart" or "Combined" I2C message format:
 * 1. Write the register address to the device.
 * 2. Repeated start.
 * 3. Read the requested number of bytes.
 *
 * @param i2c_addr  ADS1115 7-bit I2C slave address.
 * @param reg_addr  Target register address to read from.
 * @param[out] data Pointer to the buffer where received data will be stored.
 * @param len       Number of bytes to read (usually 2).
 * @return true if read was successful, false otherwise.
 */
bool ads1115_i2c_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);

/**
 * @brief Platform-specific millisecond delay implementation.
 * @details This function must be assigned to @ref ads1115_delay_ms_t in the handle.
 * It is used for timing during single-shot conversions.
 *
 * @param milliseconds Time to wait in milliseconds.
 */
void ads1115_delay_ms(uint32_t milliseconds);

/** @} */ // End of ADS1115_Interface

#endif /* ADS1115_INTERFACE_H */
/**
 * @file ads1115.c
 * @brief ADS1115 16-Bit ADC Driver - Implementation File
 * @version 1.0.0
 * @author Şükrü Can Kılıç
 * @date 26-01-2026
 */

#include "ads1115.h"
#include <stddef.h>

/**
 * @addtogroup ADS1115_Driver
 * @{
 */

/*===========================================================================*/
/* PRIVATE REGISTER DEFINITIONS                                              */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Registers Internal Registers
 * @brief Direct register addresses as defined in the datasheet.
 * @{
 */
#define ADS1115_REG_CONVERSION 0x00 /**< ADC result register (Contains the last conversion result) */
#define ADS1115_REG_CONFIG 0x01     /**< Configuration register (Used to control operating mode, data rate, etc.) */
#define ADS1115_REG_LO_THRESH 0x02  /**< Lower threshold for comparator */
#define ADS1115_REG_HI_THRESH 0x03  /**< Upper threshold for comparator */
/** @} */

/*===========================================================================*/
/* PRIVATE BIT MASKS AND SHIFTS                                              */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Bitmasks Register Bitmasks
 * @brief Bit positions and masks for the Configuration Register.
 * @{
 */
#define ADS1115_OS_MASK 0x8000
#define ADS1115_OS_SHIFT 15
#define ADS1115_OS_START_SINGLE 0x8000

#define ADS1115_MUX_MASK 0x7000
#define ADS1115_MUX_SHIFT 12

#define ADS1115_PGA_MASK 0x0E00
#define ADS1115_PGA_SHIFT 9

#define ADS1115_MODE_MASK 0x0100
#define ADS1115_MODE_SHIFT 8

#define ADS1115_DR_MASK 0x00E0
#define ADS1115_DR_SHIFT 5

#define ADS1115_COMP_MODE_MASK 0x0010
#define ADS1115_COMP_MODE_SHIFT 4

#define ADS1115_COMP_POL_MASK 0x0008
#define ADS1115_COMP_POL_SHIFT 3

#define ADS1115_COMP_LAT_MASK 0x0004
#define ADS1115_COMP_LAT_SHIFT 2

#define ADS1115_COMP_QUE_MASK 0x0003
#define ADS1115_COMP_QUE_SHIFT 0
/** @} */

/*===========================================================================*/
/* PRIVATE CONSTANTS                                                         */
/*===========================================================================*/

/** @brief Full Scale Range values in millivolts corresponding to @ref ads1115_range_t */
static const uint16_t ADS1115_FSR_VALUES[] = {6144, 4096, 2048, 1024, 512, 256};

/** @brief Nominal conversion time in microseconds for each @ref ads1115_data_rate_t */
static const uint32_t ADS1115_CONV_TIME_US[] = {
    125000, 62500, 31250, 15625, 7813, 4000, 2106, 1163};

/*===========================================================================*/
/* PRIVATE FUNCTIONS                                                         */
/*===========================================================================*/

/**
 * @brief Writes a 16-bit value to an ADS1115 register.
 * @details Handles Big-Endian conversion required by the ADS1115 hardware.
 * @param handle Device handle.
 * @param reg_addr Target register address.
 * @param value 16-bit value to write.
 * @return ads1115_error_t
 */
static ads1115_error_t write_register(ads1115_handle_t *handle, uint8_t reg_addr, uint16_t value)
{
    uint8_t data[2];
    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)(value & 0xFF);

    return handle->i2c_write((uint8_t)handle->i2c_addr, reg_addr, data, 2) ? ADS1115_OK : ADS1115_ERROR_I2C_WRITE;
}

/**
 * @brief Reads a 16-bit value from an ADS1115 register.
 * @param handle Device handle.
 * @param reg_addr Register address to read.
 * @param[out] value Pointer to store the result.
 * @return ads1115_error_t
 */
static ads1115_error_t read_register(ads1115_handle_t *handle, uint8_t reg_addr, uint16_t *value)
{
    uint8_t data[2];
    if (!handle->i2c_read((uint8_t)handle->i2c_addr, reg_addr, data, 2))
    {
        return ADS1115_ERROR_I2C_READ;
    }
    *value = ((uint16_t)data[0] << 8) | data[1];
    return ADS1115_OK;
}

/**
 * @brief Builds the 16-bit configuration word from the handle structure.
 * @param config Pointer to configuration struct.
 * @return uint16_t Calculated register value.
 */
static uint16_t build_config_register(const ads1115_config_t *config)
{
    uint16_t reg = 0;
    reg |= ((uint16_t)config->mux << ADS1115_MUX_SHIFT) & ADS1115_MUX_MASK;
    reg |= ((uint16_t)config->range << ADS1115_PGA_SHIFT) & ADS1115_PGA_MASK;
    reg |= ((uint16_t)config->mode << ADS1115_MODE_SHIFT) & ADS1115_MODE_MASK;
    reg |= ((uint16_t)config->data_rate << ADS1115_DR_SHIFT) & ADS1115_DR_MASK;
    reg |= ((uint16_t)config->comp_mode << ADS1115_COMP_MODE_SHIFT) & ADS1115_COMP_MODE_MASK;
    reg |= ((uint16_t)config->comp_pol << ADS1115_COMP_POL_SHIFT) & ADS1115_COMP_POL_MASK;
    reg |= ((uint16_t)config->comp_latch << ADS1115_COMP_LAT_SHIFT) & ADS1115_COMP_LAT_MASK;
    reg |= ((uint16_t)config->comp_queue << ADS1115_COMP_QUE_SHIFT) & ADS1115_COMP_QUE_MASK;
    return reg;
}

/**
 * @brief Synchronizes the local handle configuration with the physical device.
 */
static ads1115_error_t update_config_register(ads1115_handle_t *handle)
{
    return write_register(handle, ADS1115_REG_CONFIG, build_config_register(&handle->config));
}

/**
 * @brief Converts raw ADC bits to voltage based on the selected FSR.
 * @details Implements the transfer function:
 * $$V = \frac{\text{raw} \times \text{FSR}}{2^{15}}$$
 */
static float raw_to_voltage(ads1115_range_t range, int16_t raw_value)
{
    if (range > ADS1115_RANGE_0V256)
        range = ADS1115_RANGE_2V048;
    return (raw_value * (float)ADS1115_FSR_VALUES[range]) / 32768.0f;
}

/*===========================================================================*/
/* PUBLIC API IMPLEMENTATIONS                                                */
/*===========================================================================*/

/** @addtogroup ADS1115_Functions
 * @{
 */

ads1115_error_t ads1115_init(ads1115_handle_t *handle)
{
    if (handle == NULL)
        return ADS1115_ERROR_NULL_POINTER;
    if (!handle->i2c_read || !handle->i2c_write || !handle->delay_ms)
        return ADS1115_ERROR_INVALID_PARAM;

    ads1115_error_t err;
    if ((err = update_config_register(handle)) != ADS1115_OK)
        return err;
    if ((err = write_register(handle, ADS1115_REG_LO_THRESH, (uint16_t)handle->config.low_threshold)) != ADS1115_OK)
        return err;
    if ((err = write_register(handle, ADS1115_REG_HI_THRESH, (uint16_t)handle->config.high_threshold)) != ADS1115_OK)
        return err;

    handle->is_initialized = true;
    return ADS1115_OK;
}

ads1115_error_t ads1115_deinit(ads1115_handle_t *handle)
{
    if (handle == NULL)
        return ADS1115_ERROR_NULL_POINTER;
    handle->is_initialized = false;
    return ADS1115_OK;
}

/* PGA/Range functions */
ads1115_error_t ads1115_set_range(ads1115_handle_t *handle, ads1115_range_t range)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (range > ADS1115_RANGE_0V256)
        return ADS1115_ERROR_INVALID_PARAM;
    handle->config.range = range;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_range(ads1115_handle_t *handle, ads1115_range_t *range)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (range == NULL)
        return ADS1115_ERROR_NULL_POINTER;
    uint16_t reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &reg);
    if (err == ADS1115_OK)
        *range = (ads1115_range_t)((reg & ADS1115_PGA_MASK) >> ADS1115_PGA_SHIFT);
    return err;
}

/* Data Rate functions */
ads1115_error_t ads1115_set_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t data_rate)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (data_rate > ADS1115_DR_860_SPS)
        return ADS1115_ERROR_INVALID_PARAM;
    handle->config.data_rate = data_rate;
    return update_config_register(handle);
}

/* Conversion Control */
ads1115_error_t ads1115_continuous_conversion_start(ads1115_handle_t *handle)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    handle->config.mode = ADS1115_MODE_CONTINUOUS;
    return update_config_register(handle);
}

ads1115_error_t ads1115_continuous_conversion_stop(ads1115_handle_t *handle)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    handle->config.mode = ADS1115_MODE_SINGLE_SHOT;
    return update_config_register(handle);
}

ads1115_error_t ads1115_continuous_conversion_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (!adc_raw || !voltage)
        return ADS1115_ERROR_NULL_POINTER;
    uint16_t raw;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONVERSION, &raw);
    if (err == ADS1115_OK)
    {
        *adc_raw = (int16_t)raw;
        *voltage = raw_to_voltage(handle->config.range, *adc_raw);
    }
    return err;
}

ads1115_error_t ads1115_single_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (!adc_raw || !voltage)
        return ADS1115_ERROR_NULL_POINTER;

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
        return err;

    config_reg |= ADS1115_OS_START_SINGLE;
    if ((err = write_register(handle, ADS1115_REG_CONFIG, config_reg)) != ADS1115_OK)
        return err;

    /* Timing Calculation */
    uint32_t wait_us = ADS1115_CONV_TIME_US[handle->config.data_rate];
    handle->delay_ms((wait_us / 1000) + 1);

    uint16_t raw;
    if ((err = read_register(handle, ADS1115_REG_CONVERSION, &raw)) == ADS1115_OK)
    {
        *adc_raw = (int16_t)raw;
        *voltage = raw_to_voltage(handle->config.range, *adc_raw);
    }
    return err;
}

/* Threshold & Ready Control */
ads1115_error_t ads1115_set_compare_threshold(ads1115_handle_t *handle, int16_t low, int16_t high)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    ads1115_error_t err;
    if ((err = write_register(handle, ADS1115_REG_LO_THRESH, (uint16_t)low)) != ADS1115_OK)
        return err;
    if ((err = write_register(handle, ADS1115_REG_HI_THRESH, (uint16_t)high)) != ADS1115_OK)
        return err;
    handle->config.low_threshold = low;
    handle->config.high_threshold = high;
    return ADS1115_OK;
}

ads1115_error_t ads1115_is_ready(ads1115_handle_t *handle, bool *flag)
{
    if (!handle->is_initialized)
        return ADS1115_ERROR_NOT_INITIALIZED;
    if (!flag)
        return ADS1115_ERROR_NULL_POINTER;
    uint16_t reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &reg);
    if (err == ADS1115_OK)
        *flag = (reg & ADS1115_OS_MASK) != 0;
    return err;
}

/** @} */ // End of ADS1115_Functions
/** @} */ // End of ADS1115_Driver
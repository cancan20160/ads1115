/**
 * @file ads1115.c
 * @brief ADS1115 16-Bit ADC Driver Implementation
 * @author Şükrü Can Kılıç
 * @date 19.01.2026
 */

#include "ads1115.h"
#include <stddef.h>

/*===========================================================================*/
/* PRIVATE REGISTER DEFINITION                                               */
/*===========================================================================*/

/**
 * @brief ADS1115 Register addresses
 */
#define ADS1115_REG_CONVERSION 0x00 /**< ADC result register (read-only) */
#define ADS1115_REG_CONFIG 0x01     /**< Chip congifuration register (read/write) */
#define ADS1115_REG_LO_THRESH 0x02  /**< Interrupt low threshold register (read/write) */
#define ADS1115_REG_HI_THRESH 0x03  /**< Interrupt high threshold register (read/write) */

/*===========================================================================*/
/* PRIVATE BIT MASKS AND SHIFTS                                              */
/*===========================================================================*/

/* Operational Status (Bit 15) */
#define ADS1115_OS_MASK 0x8000
#define ADS1115_OS_SHIFT 15
#define ADS1115_OS_IDLE 0x8000
#define ADS1115_OS_START_SINGLE 0x8000

/* Input Multiplexer (Bits 14:12) */
#define ADS1115_MUX_MASK 0x7000
#define ADS1115_MUX_SHIFT 12

/* Programmable Gain (Bits 11:9) */
#define ADS1115_PGA_MASK 0x0E00
#define ADS1115_PGA_SHIFT 9

/* Operating Mode (Bit 8) */
#define ADS1115_MODE_MASK 0x0100
#define ADS1115_MODE_SHIFT 8

/* Data Rate (Bits 7:5) */
#define ADS1115_DR_MASK 0x00E0
#define ADS1115_DR_SHIFT 5

/* Comparator Mode (Bit 4) */
#define ADS1115_COMP_MODE_MASK 0x0010
#define ADS1115_COMP_MODE_SHIFT 4

/* Comparator Polarity (Bit 3) */
#define ADS1115_COMP_POL_MASK 0x0008
#define ADS1115_COMP_POL_SHIFT 3

/* Latching Comparator (Bit 2) */
#define ADS1115_COMP_LAT_MASK 0x0004
#define ADS1115_COMP_LAT_SHIFT 2

/* Comparator Queue (Bits 1:0) */
#define ADS1115_COMP_QUE_MASK 0x0003
#define ADS1115_COMP_QUE_SHIFT 0

/*===========================================================================*/
/* PRIVATE CONSTANTS                                                         */
/*===========================================================================*/

#define ADS1115_MAX_VALUE 32767
#define ADS1115_MIN_VALUE -32768

/* Full Scale Range values in millivolts */
static const uint16_t ADS1115_FSR_VALUES[] = {
    6144, /* ADS1115_RANGE_6_144V */
    4096, /* ADS1115_RANGE_4_096V */
    2048, /* ADS1115_RANGE_2_048V */
    1024, /* ADS1115_RANGE_1_024V */
    512,  /* ADS1115_RANGE_0_512V */
    256   /* ADS1115_RANGE_0_256V */
};

/* Conversion time in microseconds */
static const uint32_t ADS1115_CONV_TIME_US[] = {
    125000, /* ADS1115_DR_8_SPS */
    62500,  /* ADS1115_DR_16_SPS */
    31250,  /* ADS1115_DR_32_SPS */
    15625,  /* ADS1115_DR_64_SPS */
    7813,   /* ADS1115_DR_128_SPS */
    4000,   /* ADS1115_DR_250_SPS */
    2106,   /* ADS1115_DR_475_SPS */
    1163    /* ADS1115_DR_860_SPS */
};

/*===========================================================================*/
/* PRIVATE FUNCTIONS                                                         */
/*===========================================================================*/

static ads1115_error_t write_register(ads1115_handle_t *handle, uint8_t reg_addr, uint16_t value)
{
    uint8_t data[2];

    /* ADS1115 uses big-endian (MSB first) */
    data[0] = (uint8_t)((value >> 8) & 0xFF);
    data[1] = (uint8_t)(value & 0xFF);

    if (!handle->i2c_write((uint8_t)handle->i2c_addr, reg_addr, data, 2))
    {
        return ADS1115_ERROR_I2C_WRITE;
    }

    return ADS1115_OK;
}

static ads1115_error_t read_register(ads1115_handle_t *handle, uint8_t reg_addr, uint16_t *value)
{
    uint8_t data[2];

    if (!handle->i2c_read((uint8_t)handle->i2c_addr, reg_addr, data, 2))
    {
        return ADS1115_ERROR_I2C_READ;
    }

    /* ADS1115 uses big-endian (MSB first) */
    *value = ((uint16_t)data[0] << 8) | data[1];

    return ADS1115_OK;
}

static uint16_t build_config_register(const ads1115_config_t *config)
{
    uint16_t reg_value = 0;

    reg_value |= ((uint16_t)config->mux << ADS1115_MUX_SHIFT) & ADS1115_MUX_MASK;
    reg_value |= ((uint16_t)config->range << ADS1115_PGA_SHIFT) & ADS1115_PGA_MASK;
    reg_value |= ((uint16_t)config->mode << ADS1115_MODE_SHIFT) & ADS1115_MODE_MASK;
    reg_value |= ((uint16_t)config->data_rate << ADS1115_DR_SHIFT) & ADS1115_DR_MASK;
    reg_value |= ((uint16_t)config->comp_mode << ADS1115_COMP_MODE_SHIFT) & ADS1115_COMP_MODE_MASK;
    reg_value |= ((uint16_t)config->comp_pol << ADS1115_COMP_POL_SHIFT) & ADS1115_COMP_POL_MASK;
    reg_value |= ((uint16_t)config->comp_latch << ADS1115_COMP_LAT_SHIFT) & ADS1115_COMP_LAT_MASK;
    reg_value |= ((uint16_t)config->comp_queue << ADS1115_COMP_QUE_SHIFT) & ADS1115_COMP_QUE_MASK;

    return reg_value;
}

static ads1115_error_t update_config_register(ads1115_handle_t *handle)
{
    uint16_t config_reg = build_config_register(&handle->config);
    return write_register(handle, ADS1115_REG_CONFIG, config_reg);
}

static uint32_t get_conversion_time_us(ads1115_data_rate_t data_rate)
{
    if (data_rate > ADS1115_DR_860_SPS)
    {
        return ADS1115_CONV_TIME_US[ADS1115_DR_128_SPS];
    }
    return ADS1115_CONV_TIME_US[data_rate];
}

static float raw_to_voltage(ads1115_range_t range, int16_t raw_value)
{
    if (range > ADS1115_RANGE_0V256)
    {
        range = ADS1115_RANGE_2V048;
    }

    float fsr = (float)ADS1115_FSR_VALUES[range];
    return (raw_value * fsr) / 32768.0f;
}

/*===========================================================================*/
/* PUBLIC API IMPLEMENTATIONS                                                */
/*===========================================================================*/

ads1115_error_t ads1115_init(ads1115_handle_t *handle)
{
    if (handle == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    if ((handle->i2c_read == NULL) || (handle->i2c_write == NULL) || (handle->delay_ms == NULL))
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    /* Write default configuration */
    ads1115_error_t err = update_config_register(handle);
    if (err != ADS1115_OK)
    {
        return err;
    }

    /* Write default thresholds */
    err = write_register(handle, ADS1115_REG_LO_THRESH, (uint16_t)handle->config.low_threshold);
    if (err != ADS1115_OK)
    {
        return err;
    }

    err = write_register(handle, ADS1115_REG_HI_THRESH, (uint16_t)handle->config.high_threshold);
    if (err != ADS1115_OK)
    {
        return err;
    }

    handle->is_initialized = true;
    return ADS1115_OK;
}

ads1115_error_t ads1115_deinit(ads1115_handle_t *handle)
{
    if (handle == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    handle->is_initialized = false;
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_range(ads1115_handle_t *handle, ads1115_range_t range)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (range > ADS1115_RANGE_0V256)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.range = range;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_range(ads1115_handle_t *handle, ads1115_range_t *range)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (range == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *range = (ads1115_range_t)((config_reg & ADS1115_PGA_MASK) >> ADS1115_PGA_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t data_rate)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (data_rate > ADS1115_DR_860_SPS)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.data_rate = data_rate;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t *data_rate)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (data_rate == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *data_rate = (ads1115_data_rate_t)((config_reg & ADS1115_DR_MASK) >> ADS1115_DR_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_continuous_conversion_start(ads1115_handle_t *handle)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    handle->config.mode = ADS1115_MODE_CONTINUOUS;
    return update_config_register(handle);
}

ads1115_error_t ads1115_continuous_conversion_stop(ads1115_handle_t *handle)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    handle->config.mode = ADS1115_MODE_SINGLE_SHOT;
    return update_config_register(handle);
}

ads1115_error_t ads1115_continuous_conversion_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if ((adc_raw == NULL) || (voltage == NULL))
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t raw_data;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONVERSION, &raw_data);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *adc_raw = (int16_t)raw_data;
    *voltage = raw_to_voltage(handle->config.range, *adc_raw);

    return ADS1115_OK;
}

ads1115_error_t ads1115_single_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if ((adc_raw == NULL) || (voltage == NULL))
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    /* Read current config */
    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    /* Start single conversion */
    config_reg |= ADS1115_OS_START_SINGLE;
    err = write_register(handle, ADS1115_REG_CONFIG, config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    /* Wait for conversion */
    uint32_t conv_time_us = get_conversion_time_us(handle->config.data_rate);
    handle->delay_ms((conv_time_us / 1000) + 1);

    /* Read conversion result */
    uint16_t raw_data;
    err = read_register(handle, ADS1115_REG_CONVERSION, &raw_data);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *adc_raw = (int16_t)raw_data;
    *voltage = raw_to_voltage(handle->config.range, *adc_raw);

    return ADS1115_OK;
}

ads1115_error_t ads1115_set_channel(ads1115_handle_t *handle, ads1115_mux_t channel)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (channel > ADS1115_MUX_AIN3_GND)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.mux = channel;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_channel(ads1115_handle_t *handle, ads1115_mux_t *channel)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (channel == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *channel = (ads1115_mux_t)((config_reg & ADS1115_MUX_MASK) >> ADS1115_MUX_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t compare)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (compare > ADS1115_COMP_MODE_WINDOW)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.comp_mode = compare;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t *compare)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (compare == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *compare = (ads1115_comp_mode_t)((config_reg & ADS1115_COMP_MODE_MASK) >> ADS1115_COMP_MODE_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t comp_queue)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (comp_queue > ADS1115_COMP_QUE_DISABLE)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.comp_queue = comp_queue;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t *comp_queue)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (comp_queue == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *comp_queue = (ads1115_comp_queue_t)((config_reg & ADS1115_COMP_QUE_MASK) >> ADS1115_COMP_QUE_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t latch)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (latch > ADS1115_COMP_LAT_LATCHING)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.comp_latch = latch;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t *latch)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (latch == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *latch = (ads1115_comp_latch_t)((config_reg & ADS1115_COMP_LAT_MASK) >> ADS1115_COMP_LAT_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t polarity)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (polarity > ADS1115_COMP_POL_ACTIVE_HIGH)
    {
        return ADS1115_ERROR_INVALID_PARAM;
    }

    handle->config.comp_pol = polarity;
    return update_config_register(handle);
}

ads1115_error_t ads1115_get_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t *polarity)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (polarity == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *polarity = (ads1115_comp_polarity_t)((config_reg & ADS1115_COMP_POL_MASK) >> ADS1115_COMP_POL_SHIFT);
    return ADS1115_OK;
}

ads1115_error_t ads1115_set_compare_threshold(ads1115_handle_t *handle, int16_t low_threshold, int16_t high_threshold)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    handle->config.low_threshold = low_threshold;
    handle->config.high_threshold = high_threshold;

    ads1115_error_t err = write_register(handle, ADS1115_REG_LO_THRESH, (uint16_t)low_threshold);
    if (err != ADS1115_OK)
    {
        return err;
    }

    return write_register(handle, ADS1115_REG_HI_THRESH, (uint16_t)high_threshold);
}

ads1115_error_t ads1115_get_compare_threshold(ads1115_handle_t *handle, int16_t *low_threshold, int16_t *high_threshold)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if ((low_threshold == NULL) || (high_threshold == NULL))
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t low_reg, high_reg;

    ads1115_error_t err = read_register(handle, ADS1115_REG_LO_THRESH, &low_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    err = read_register(handle, ADS1115_REG_HI_THRESH, &high_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *low_threshold = (int16_t)low_reg;
    *high_threshold = (int16_t)high_reg;

    return ADS1115_OK;
}

ads1115_error_t ads1115_is_ready(ads1115_handle_t *handle, bool *flag)
{
    if (!handle->is_initialized)
    {
        return ADS1115_ERROR_NOT_INITIALIZED;
    }

    if (flag == NULL)
    {
        return ADS1115_ERROR_NULL_POINTER;
    }

    uint16_t config_reg;
    ads1115_error_t err = read_register(handle, ADS1115_REG_CONFIG, &config_reg);
    if (err != ADS1115_OK)
    {
        return err;
    }

    *flag = (config_reg & ADS1115_OS_MASK) ? true : false;
    return ADS1115_OK;
}

/**
 * @file ads1115.h
 * @brief ADS1115 16-Bit ADC Driver - Header File
 * @version 1.0.0
 * @author Şükrü Can Kılıç
 * @date 26-01-2026
 *
 * @details This driver provides a complete interface for the Texas Instruments ADS1115.
 * It supports single-ended and differential measurements, programmable gain,
 * and comparator functions.
 * * Datasheet: https://www.ti.com/lit/ds/symlink/ads1115.pdf
 */

#ifndef ADS1115_H
#define ADS1115_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup ADS1115_Driver ADS1115 ADC Driver
 * @brief Main group for ADS1115 driver modules.
 * @{
 */

/*===========================================================================*/
/* TYPE DEFINITIONS                                                          */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Enums Enumerations
 * @brief Critical constants for device configuration.
 * @{
 */

/**
 * @brief Error codes for ADS1115 operations
 */
typedef enum
{
    ADS1115_OK = 0,                /**< Operation successful */
    ADS1115_ERROR_INVALID_PARAM,   /**< Invalid parameter passed */
    ADS1115_ERROR_I2C_WRITE,       /**< I2C write operation failed */
    ADS1115_ERROR_I2C_READ,        /**< I2C read operation failed */
    ADS1115_ERROR_TIMEOUT,         /**< Operation timeout */
    ADS1115_ERROR_NOT_INITIALIZED, /**< Device not initialized */
    ADS1115_ERROR_CONVERSION_BUSY, /**< Conversion in progress */
    ADS1115_ERROR_NULL_POINTER     /**< Null pointer passed */
} ads1115_error_t;

/**
 * @brief I2C address selection based on ADDR pin connection
 */
typedef enum
{
    ADS1115_ADDR_GND = 0x48, /**< ADDR pin connected to GND */
    ADS1115_ADDR_VDD = 0x49, /**< ADDR pin connected to VDD */
    ADS1115_ADDR_SDA = 0x4A, /**< ADDR pin connected to SDA */
    ADS1115_ADDR_SCL = 0x4B, /**< ADDR pin connected to SCL */
} ads1115_i2c_addr_t;

/**
 * @brief Input multiplexer configuration (channel selection)
 */
typedef enum
{
    ADS1115_MUX_AIN0_AIN1 = 0, /**< Differential: $V_{IN} = AIN_0 - AIN_1$ */
    ADS1115_MUX_AIN0_AIN3 = 1, /**< Differential: $V_{IN} = AIN_0 - AIN_3$ */
    ADS1115_MUX_AIN1_AIN3 = 2, /**< Differential: $V_{IN} = AIN_1 - AIN_3$ */
    ADS1115_MUX_AIN2_AIN3 = 3, /**< Differential: $V_{IN} = AIN_2 - AIN_3$ */
    ADS1115_MUX_AIN0_GND = 4,  /**< Single-ended: $AIN_0$ vs GND */
    ADS1115_MUX_AIN1_GND = 5,  /**< Single-ended: $AIN_1$ vs GND */
    ADS1115_MUX_AIN2_GND = 6,  /**< Single-ended: $AIN_2$ vs GND */
    ADS1115_MUX_AIN3_GND = 7,  /**< Single-ended: $AIN_3$ vs GND */
} ads1115_mux_t;

/**
 * @brief Programmable Gain Amplifier (PGA) and Full-Scale Range (FSR)
 */
typedef enum
{
    ADS1115_RANGE_6V144 = 0, /**< $\pm 6.144V$ range (FSR = 6.144V) */
    ADS1115_RANGE_4V096 = 1, /**< $\pm 4.096V$ range (FSR = 4.096V) */
    ADS1115_RANGE_2V048 = 2, /**< $\pm 2.048V$ range (FSR = 2.048V) - Default */
    ADS1115_RANGE_1V024 = 3, /**< $\pm 1.024V$ range (FSR = 1.024V) */
    ADS1115_RANGE_0V512 = 4, /**< $\pm 0.512V$ range (FSR = 0.512V) */
    ADS1115_RANGE_0V256 = 5, /**< $\pm 0.256V$ range (FSR = 0.256V) */
} ads1115_range_t;

/**
 * @brief Device operating mode
 */
typedef enum
{
    ADS1115_MODE_CONTINUOUS = 0,  /**< Continuous conversion mode */
    ADS1115_MODE_SINGLE_SHOT = 1, /**< Single-shot conversion mode (Power-down) */
} ads1115_mode_t;

/**
 * @brief Data rate (samples per second)
 */
typedef enum
{
    ADS1115_DR_8_SPS = 0,   /**< 8 samples per second */
    ADS1115_DR_16_SPS = 1,  /**< 16 samples per second */
    ADS1115_DR_32_SPS = 2,  /**< 32 samples per second */
    ADS1115_DR_64_SPS = 3,  /**< 64 samples per second */
    ADS1115_DR_128_SPS = 4, /**< 128 samples per second - Default */
    ADS1115_DR_250_SPS = 5, /**< 250 samples per second */
    ADS1115_DR_475_SPS = 6, /**< 475 samples per second */
    ADS1115_DR_860_SPS = 7, /**< 860 samples per second */
} ads1115_data_rate_t;

/**
 * @brief Comparator operational mode
 */
typedef enum
{
    ADS1115_COMP_MODE_TRADITIONAL = 0, /**< Traditional comparator with hysteresis */
    ADS1115_COMP_MODE_WINDOW = 1,      /**< Window comparator */
} ads1115_comp_mode_t;

/**
 * @brief ALERT/RDY pin polarity
 */
typedef enum
{
    ADS1115_COMP_POL_ACTIVE_LOW = 0,  /**< Pin is low when active - Default */
    ADS1115_COMP_POL_ACTIVE_HIGH = 1, /**< Pin is high when active */
} ads1115_comp_polarity_t;

/**
 * @brief Comparator latching behavior
 */
typedef enum
{
    ADS1115_COMP_LAT_NON_LATCHING = 0, /**< ALERT/RDY clears when data is in range */
    ADS1115_COMP_LAT_LATCHING = 1,     /**< ALERT/RDY latches until data is read */
} ads1115_comp_latch_t;

/**
 * @brief Comparator queue and disable control
 */
typedef enum
{
    ADS1115_COMP_QUE_1_CONV = 0,  /**< Assert after one conversion over threshold */
    ADS1115_COMP_QUE_2_CONV = 1,  /**< Assert after two conversions over threshold */
    ADS1115_COMP_QUE_4_CONV = 2,  /**< Assert after four conversions over threshold */
    ADS1115_COMP_QUE_DISABLE = 3, /**< Disable comparator (Hi-Z state) - Default */
} ads1115_comp_queue_t;

/** @} */ // End of ADS1115_Enums

/*===========================================================================*/
/* CALLBACK FUNCTION TYPES                                                   */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Callbacks Function Pointers
 * @brief Platform-specific I/O abstractions.
 * @{
 */

typedef bool (*ads1115_i2c_write_t)(uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, uint8_t length);
typedef bool (*ads1115_i2c_read_t)(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t length);
typedef void (*ads1115_delay_ms_t)(uint32_t milliseconds);

/** @} */

/*===========================================================================*/
/* CONFIGURATION STRUCTURES                                                  */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Structs Structures
 * @brief Device handle and configuration definitions.
 * @{
 */

/**
 * @brief ADS1115 full configuration parameters
 */
typedef struct
{
    ads1115_mux_t mux;                /**< Multiplexer selection */
    ads1115_range_t range;            /**< Full-scale range selection */
    ads1115_mode_t mode;              /**< Conversion mode */
    ads1115_data_rate_t data_rate;    /**< SPS setting */
    ads1115_comp_mode_t comp_mode;    /**< Comparator mode */
    ads1115_comp_polarity_t comp_pol; /**< ALERT/RDY polarity */
    ads1115_comp_latch_t comp_latch;  /**< Latching setting */
    ads1115_comp_queue_t comp_queue;  /**< Comparator queueing */
    int16_t low_threshold;            /**< Low threshold register value */
    int16_t high_threshold;           /**< High threshold register value */
} ads1115_config_t;

/**
 * @brief ADS1115 device handle
 */
typedef struct
{
    ads1115_i2c_addr_t i2c_addr;   /**< I2C 7-bit slave address */
    ads1115_config_t config;       /**< Device settings */
    ads1115_i2c_write_t i2c_write; /**< Hardware write function */
    ads1115_i2c_read_t i2c_read;   /**< Hardware read function */
    ads1115_delay_ms_t delay_ms;   /**< Hardware delay function */
    bool is_initialized;           /**< Internal state flag */
} ads1115_handle_t;

/** @} */

/*===========================================================================*/
/* DEFAULT CONFIGURATION                                                     */
/*===========================================================================*/

/**
 * @brief Default power-up configuration macro.
 */
#define ADS1115_DEFAULT_CONFIGURATION {          \
    .mux = ADS1115_MUX_AIN0_AIN1,                \
    .range = ADS1115_RANGE_2V048,                \
    .mode = ADS1115_MODE_SINGLE_SHOT,            \
    .data_rate = ADS1115_DR_128_SPS,             \
    .comp_mode = ADS1115_COMP_MODE_TRADITIONAL,  \
    .comp_pol = ADS1115_COMP_POL_ACTIVE_LOW,     \
    .comp_latch = ADS1115_COMP_LAT_NON_LATCHING, \
    .comp_queue = ADS1115_COMP_QUE_DISABLE,      \
    .low_threshold = 0x8000,                     \
    .high_threshold = 0x7FFF}

/*===========================================================================*/
/* PUBLIC API FUNCTIONS                                                      */
/*===========================================================================*/

/**
 * @defgroup ADS1115_Functions API Functions
 * @brief Public interface for interacting with the ADS1115.
 * @{
 */

/**
 * @brief Initializes the ADS1115 with the provided handle settings.
 * @param handle Pointer to the device handle structure.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_init(ads1115_handle_t *handle);

/**
 * @brief Soft de-initialization and cleanup.
 * @param handle Pointer to the device handle structure.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_deinit(ads1115_handle_t *handle);

/**
 * @brief Sets the Programmable Gain Amplifier (PGA) range.
 * @param handle Pointer to the device handle structure.
 * @param range The desired full-scale range.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_range(ads1115_handle_t *handle, ads1115_range_t range);

/**
 * @brief Reads the current PGA range from the handle.
 * @param handle Pointer to the device handle structure.
 * @param[out] range Pointer to store the current range.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_range(ads1115_handle_t *handle, ads1115_range_t *range);

/**
 * @brief Configures the conversion data rate.
 * @param handle Pointer to the device handle structure.
 * @param data_rate Samples per second selection.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t data_rate);

/**
 * @brief Retrieves the current data rate.
 * @param handle Pointer to the device handle structure.
 * @param[out] data_rate Pointer to store the data rate.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t *data_rate);

/**
 * @brief Starts continuous conversion mode.
 * @param handle Pointer to the device handle structure.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_continuous_conversion_start(ads1115_handle_t *handle);

/**
 * @brief Stops continuous conversion and enters power-down mode.
 * @param handle Pointer to the device handle structure.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_continuous_conversion_stop(ads1115_handle_t *handle);

/**
 * @brief Reads the latest conversion result in continuous mode.
 * * @details The voltage is calculated using:
 * $$V_{out} = \text{adc\_raw} \times \frac{\text{FSR}}{2^{15}}$$
 * * @param handle Pointer to the device handle structure.
 * @param[out] adc_raw Raw 16-bit signed integer output.
 * @param[out] voltage Calculated voltage in Volts.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_continuous_conversion_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage);

/**
 * @brief Performs a single-shot conversion and returns the result.
 * @param handle Pointer to the device handle structure.
 * @param[out] adc_raw Raw 16-bit signed integer output.
 * @param[out] voltage Calculated voltage in Volts.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_single_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage);

/**
 * @brief Selects the input channel(s) via the multiplexer.
 * @param handle Pointer to the device handle structure.
 * @param channel The MUX configuration to apply.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_channel(ads1115_handle_t *handle, ads1115_mux_t channel);

/**
 * @brief Gets the current MUX configuration.
 * @param handle Pointer to the device handle structure.
 * @param[out] channel Pointer to store the MUX setting.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_channel(ads1115_handle_t *handle, ads1115_mux_t *channel);

/**
 * @brief Sets the comparator operation mode.
 * @param handle Pointer to the device handle structure.
 * @param compare Traditional or Window mode.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t compare);

/**
 * @brief Gets the current comparator mode.
 * @param handle Pointer to the device handle structure.
 * @param[out] compare Pointer to store the mode.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t *compare);

/**
 * @brief Sets the number of conversions before ALERT/RDY pin assertion.
 * @param handle Pointer to the device handle structure.
 * @param comp_queue Number of samples or disable.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t comp_queue);

/**
 * @brief Gets the current comparator queue setting.
 * @param handle Pointer to the device handle structure.
 * @param[out] comp_queue Pointer to store the setting.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t *comp_queue);

/**
 * @brief Configures ALERT/RDY pin polarity.
 * @param handle Pointer to the device handle structure.
 * @param polarity ACTIVE_LOW or ACTIVE_HIGH.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t polarity);

/**
 * @brief Gets ALERT/RDY pin polarity.
 * @param handle Pointer to the device handle structure.
 * @param[out] polarity Pointer to store the polarity.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t *polarity);

/**
 * @brief Sets the high and low thresholds for the comparator.
 * @param handle Pointer to the device handle structure.
 * @param low_threshold Lower bound value.
 * @param high_threshold Upper bound value.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_compare_threshold(ads1115_handle_t *handle, int16_t low_threshold, int16_t high_threshold);

/**
 * @brief Gets the current high and low threshold values.
 * @param handle Pointer to the device handle structure.
 * @param[out] low_threshold Pointer to store lower bound.
 * @param[out] high_threshold Pointer to store upper bound.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_compare_threshold(ads1115_handle_t *handle, int16_t *low_threshold, int16_t *high_threshold);

/**
 * @brief Enables or disables comparator latching.
 * @param handle Pointer to the device handle structure.
 * @param latch Latching or non-latching mode.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_set_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t latch);

/**
 * @brief Gets the current latching setting.
 * @param handle Pointer to the device handle structure.
 * @param[out] latch Pointer to store the setting.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_get_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t *latch);

/**
 * @brief Polls the device to check if a conversion is complete.
 * @param handle Pointer to the device handle structure.
 * @param[out] flag Set to true if data is ready.
 * @return @ref ads1115_error_t result.
 */
ads1115_error_t ads1115_is_ready(ads1115_handle_t *handle, bool *flag);

/** @} */ // End of ADS1115_Functions
/** @} */ // End of ADS1115_Driver

#ifdef __cplusplus
}
#endif

#endif /* ADS1115_H */
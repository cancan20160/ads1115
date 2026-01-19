#ifndef ADS1115_H
#define ADS1115_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

/*===========================================================================*/
/* TYPE DEFINITIONS                                                          */
/*===========================================================================*/

/**
 * @brief Error codes for ADS1115 operations
 */
typedef enum {
    ADS1115_OK = 0,                     /**< Operation successful */
    ADS1115_ERROR_INVALID_PARAM,        /**< Invalid parameter passed */
    ADS1115_ERROR_I2C_WRITE,            /**< I2C write operation failed */
    ADS1115_ERROR_I2C_READ,             /**< I2C read operation failed */
    ADS1115_ERROR_TIMEOUT,              /**< Operation timeout */
    ADS1115_ERROR_NOT_INITIALIZED,      /**< Device not initialized */
    ADS1115_ERROR_CONVERSION_BUSY,      /**< Conversion in progress */
    ADS1115_ERROR_NULL_POINTER          /**< Null pointer passed */
} ads1115_error_t;

/**
 * @brief I2C address selection based on ADDR pin connection
 */
typedef enum {
    ADS1115_ADDR_GND = 0x48,        /**< ADDR pin connected to GND */
    ADS1115_ADDR_VDD = 0x49,        /**< ADDR pin connected to VDD */
    ADS1115_ADDR_SDA = 0x4A,        /**< ADDR pin connected to SDA */
    ADS1115_ADDR_SCL = 0x4B,        /**< ADDR pin connected to SCL */
} ads1115_i2c_addr_t;

/**
 * @brief Input multiplexer configuration (channel selection)
 */
typedef enum {
    ADS1115_MUX_AIN0_AIN1 = 0,    /**< Differantial: AIN0 - AIN1 */
    ADS1115_MUX_AIN0_AIN3 = 1,    /**< Differantial: AIN0 - AIN3 */
    ADS1115_MUX_AIN1_AIN3 = 2,    /**< Differantial: AIN1 - AIN3 */
    ADS1115_MUX_AIN2_AIN3 = 3,    /**< Differantial: AIN2 - AIN3 */
    ADS1115_MUX_AIN0_GND  = 4,    /**< Single-ended: AIN0 */
    ADS1115_MUX_AIN1_GND  = 5,    /**< Single-ended: AIN1 */
    ADS1115_MUX_AIN2_GND  = 6,    /**< Single-ended: AIN2 */
    ADS1115_MUX_AIN3_GND  = 7,    /**< Single-ended: AIN3 */
} ads1115_mux_t;

/**
 * @brief Programmable gain amplifier configuration
 */
typedef enum {
    ADS1115_RANGE_6V144 = 0,        /**< +/- 6.144V range (FSR = 6.144V) */
    ADS1115_RANGE_4V096 = 1,        /**< +/- 4.096V range (FSR = 4.096V) */
    ADS1115_RANGE_2V048 = 2,        /**< +/- 2.048V range (FSR = 2.048V) - Default */
    ADS1115_RANGE_1V024 = 3,        /**< +/- 1.024V range (FSR = 1.024V) */
    ADS1115_RANGE_0V512 = 4,        /**< +/- 0.512V range (FSR = 0.512V) */
    ADS1115_RANGE_0V256 = 5,        /**< +/- 0.256V range (FSR = 0.256V) */
} ads1115_range_t;

/**
 * @brief Device operating mode
 */
typedef enum {
    ADS1115_MODE_CONTINUOUS  = 0,   /**< Continuous conversion mode */
    ADS1115_MODE_SINGLE_SHOT = 1,   /**< Single-shot conversion mode */
} ads1115_mode_t;

/**
 * @brief Data rate (samples per second)
 */
typedef enum {
    ADS1115_DR_8_SPS   = 0,         /**< 8 samples per second */
    ADS1115_DR_16_SPS  = 1,         /**< 16 samples per second */
    ADS1115_DR_32_SPS  = 2,         /**< 32 samples per second */
    ADS1115_DR_64_SPS  = 3,         /**< 64 samples per second */
    ADS1115_DR_128_SPS = 4,         /**< 128 samples per second - Default */
    ADS1115_DR_250_SPS = 5,         /**< 250 samples per second */
    ADS1115_DR_475_SPS = 6,         /**< 475 samples per second */
    ADS1115_DR_860_SPS = 7,         /**< 860 samples per second */
} ads1115_data_rate_t;

/**
 * @brief Comperator mode
 */
typedef enum {
    ADS1115_COMP_MODE_TRADITIONAL = 0,  /**< Traditional comparator */
    ADS1115_COMP_MODE_WINDOW = 1,       /**< Window comparator */
} ads1115_comp_mode_t;

/**
 * @brief Comparator polarity
 */
typedef enum {
    ADS1115_COMP_POL_ACTIVE_LOW = 0,    /**< Active low - Default */
    ADS1115_COMP_POL_ACTIVE_HIGH = 1,   /**< Active high */
} ads1115_comp_polarity_t;

/**
 * @brief Latching comparator
 */
typedef enum {
    ADS1115_COMP_LAT_NON_LATCHING = 0,  /**< Nonlatching comparator - Default */
    ADS1115_COMP_LAT_LATCHING = 1,      /**< Latching comparator */
} ads1115_comp_latch_t;

/**
 * @brief Comparator queue and disable
 */
typedef enum {
    ADS1115_COMP_QUE_1_CONV  = 0,       /**< Assert after one conversion */
    ADS1115_COMP_QUE_2_CONV  = 1,       /**< Assert after two conversions */
    ADS1115_COMP_QUE_4_CONV  = 2,       /**< Assert after four conversions */
    ADS1115_COMP_QUE_DISABLE = 3,       /**< Disable comparator and set ALERT/RDY pin to high-impedance - Default */
} ads1115_comp_queue_t;

/*===========================================================================*/
/* CALLBACK FUNCTION TYPES                                                   */
/*===========================================================================*/

/**
 * @brief I2C write callback function type
 * 
 * @param device_addr I2C device address (7-bit)
 * @param reg_addr Register address to write to
 * @param data Pointer to data buffer to write
 * @param length Number of bytes to write
 * @return true if write successful, false otherwise
 */
typedef bool (*ads1115_i2c_write_t)(uint8_t device_addr, uint8_t reg_addr, const uint8_t *data, uint8_t length);

/**
 * @brief I2C read callback function type
 * 
 * @param device_addr I2C device address (7-bit)
 * @param reg_addr Register address to read from
 * @param data Pointer to buffer to store read data
 * @param length Number of bytes to read
 * @return true if read successful, false otherwise
 */
typedef bool (*ads1115_i2c_read_t)(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, uint8_t length);

/**
 * @brief Delay callback function type
 * 
 * @param milliseconds Delay duration in milliseconds
 */
typedef void (*ads1115_delay_ms_t)(uint32_t milliseconds);

/*===========================================================================*/
/* CONFIGURATION STRUCTURES                                                  */
/*===========================================================================*/

/**
 * @brief ADS1115 configuration structure
 */
typedef struct {
    ads1115_mux_t mux;                  /**< Input multiplexer configuration */
    ads1115_range_t range;              /**< Programmable gain amplifier */
    ads1115_mode_t mode;                /**< Operating mode */
    ads1115_data_rate_t data_rate;      /**< Data rate (samples per second) */
    ads1115_comp_mode_t comp_mode;      /**< Comparator mode */
    ads1115_comp_polarity_t comp_pol;   /**< Comparator polarity */
    ads1115_comp_latch_t comp_latch;    /**< Comparator latching */
    ads1115_comp_queue_t comp_queue;    /**< Comparator queue */
    int16_t low_threshold;              /**< Low threshold value */
    int16_t high_threshold;             /**< High threshold value */
} ads1115_config_t;

/**
 * @brief ADS1115 device handle structure
 */
typedef struct {
    ads1115_i2c_addr_t i2c_addr;        /**< I2C device address */
    ads1115_config_t config;            /**< Current device configuration */
    ads1115_i2c_write_t i2c_write;      /**< I2C write callback */
    ads1115_i2c_read_t i2c_read;        /**< I2C read callback */
    ads1115_delay_ms_t delay_ms;        /**< Delay callback */
    bool is_initialized;                /**< Initialization status */
} ads1115_handle_t;

/*===========================================================================*/
/* DEFAULT CONFIGURATION                                                     */
/*===========================================================================*/

/**
 * @brief Default configuration values at compile time
 * 
 * These values match the ADS1115 power-up defaults as per datasheet.
 * Users can modify these at runtime using setter functions.
 */
#define ADS1115_DEFAULT_CONFIGURATION {         \
    .mux = ADS1115_MUX_AIN0_AIN1,               \
    .range = ADS1115_RANGE_2V048,               \
    .mode = ADS1115_MODE_SINGLE_SHOT,           \
    .data_rate = ADS1115_DR_128_SPS,            \
    .comp_mode = ADS1115_COMP_MODE_TRADITIONAL, \
    .comp_pol = ADS1115_COMP_POL_ACTIVE_LOW,    \
    .comp_latch = ADS1115_COMP_LAT_NON_LATCHING,\
    .comp_queue = ADS1115_COMP_QUE_DISABLE,     \
    .low_threshold = 0x8000,                    \
    .high_threshold = 0x7FFF                    \
}

/*===========================================================================*/
/* PUBLIC API FUNCTIONS                                                      */
/*===========================================================================*/

/**
 * @brief Initialize ADS1115 device
 * 
 * @param handle Pointer to device handle
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_init(ads1115_handle_t *handle);

/**
 * @brief De-initialize ADS1115 device
 * 
 * @param handle Pointer to device handle
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_deinit(ads1115_handle_t *handle);

/**
 * @brief Set range of adc
 * 
 * @param handle Pointer to device handle
 * @param range Maximum voltage range
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_range(ads1115_handle_t *handle, ads1115_range_t range);

/**
 * @brief Get range of adc
 * 
 * @param handle Pointer to device handle
 * @param range Maximum voltage range
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_range(ads1115_handle_t *handle, ads1115_range_t range);

/**
 * @brief Set adc data sample rate
 * 
 * @param handle Pointer to device handle
 * @param data_rate Data rate setting
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t data_rate);

/**
 * @brief Get adc data sample rate
 * 
 * @param handle Pointer to device handle
 * @param data_rate Data rate setting
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_data_rate(ads1115_handle_t *handle, ads1115_data_rate_t data_rate);

/**
 * @brief Start adc continuous conversion mode
 * 
 * @param handle Pointer to device handle
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_continuous_conversion_start(ads1115_handle_t *handle);

/**
 * @brief Stop adc continuous conversion mode
 * 
 * @param handle Pointer to device handle
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_continuous_conversion_stop(ads1115_handle_t *handle);

/**
 * @brief Read the result of adc continuously
 * 
 * @param handle Pointer to device handle
 * @param adc_raw Pointer to raw adc result
 * @param voltage Pointer to converted adc result
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_continuous_conversion_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage);

/**
 * @brief Read adc result once
 * 
 * @param handle Pointer to device handle
 * @param adc_raw Pointer to raw adc result
 * @param voltage Pointer to converted adc result
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_single_read(ads1115_handle_t *handle, int16_t *adc_raw, float *voltage);

/**
 * @brief Set adc input channel from mux
 * 
 * @param handle Pointer to device handle
 * @param channel Input adc channel from mux
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_channel(ads1115_handle_t *handle, ads1115_mux_t channel);

/**
 * @brief Set adc input channel from mux
 * 
 * @param handle Pointer to device handle
 * @param channel Pointer to input adc channel from mux
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_channel(ads1115_handle_t *handle, ads1115_mux_t *channel);

/**
 * @brief Set comparator mode
 * 
 * @param handle Pointer to device handle
 * @param compare Interrupt compare mode
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t compare);

/**
 * @brief Get current comparator mode from device
 * 
 * @param handle Pointer to device handle
 * @param compare Pointer to interrupt compare mode
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_compare_mode(ads1115_handle_t *handle, ads1115_comp_mode_t *compare);

/**
 * @brief Set comparatore queue
 * 
 * @param handle Pointer to device handle
 * @param comp_queue Comparator queue
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t comp_queue);

/**
 * @brief Get comparatore queue
 * 
 * @param handle Pointer to device handle
 * @param comp_queue Pointer to comparator queue
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_compare_queue(ads1115_handle_t *handle, ads1115_comp_queue_t *comp_queue);

/**
 * @brief Set the alert pin polatiry mode (ACTIVE_HIGH or ACTIVE_LOW)
 * 
 * @param handle Pointer to device handle
 * @param polarity Alert pin polarity
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t polarity);

/**
 * @brief Get the alert pin polatiry (ACTIVE_HIGH or ACTIVE_LOW)
 * 
 * @param handle Pointer to device handle
 * @param polarity Pointer to alert pin polarity
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_compare_alert(ads1115_handle_t *handle, ads1115_comp_polarity_t *polarity);

/**
 * @brief Set the comparator low and high threshold
 * 
 * @param handle Pointer to device handle
 * @param low_threshold Compare low threshold
 * @param high_threshold Compare high threshold
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_compare_threshold(ads1115_handle_t *handle, int16_t low_threshold, int16_t high_threshold);

/**
 * @brief Get the comparator low and high threshold
 * 
 * @param handle Pointer to device handle
 * @param low_threshold Pointer to compare low threshold
 * @param high_threshold Pointer to compare high threshold
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_compare_threshold(ads1115_handle_t *handle, int16_t *low_threshold, int16_t *high_threshold);

/**
 * @brief Set comparator latching mode
 * 
 * @param handle Pointer to device handle
 * @param latch Comparator latch
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_set_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t latch);

/**
 * @brief Get comparator latching mode
 * 
 * @param handle Pointer to device handle
 * @param latch Pointer to comparator latch
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_get_compare_latch(ads1115_handle_t *handle, ads1115_comp_latch_t *latch);

/**
 * @brief Check if conversion is ready
 * 
 * @param handle Pointer to device handle
 * @param flag Pointer to store flag status
 * @return ads1115_error_t Error code
 */
ads1115_error_t ads1115_is_ready(ads1115_handle_t *handle, bool *flag);

#ifdef __cplusplus
}
#endif

#endif /* ADS1115_H*/
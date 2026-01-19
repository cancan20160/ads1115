# ADS1115 16-Bit ADC Driver
Platform-independent driver for Texas Instruments ADS1115 4-Channel 16-bit ADC with I2C Interface.

## Features
- Platform-independent (works on STM32, ESP32, Linux, Arduino, etc.)
- HAL abstraction via callback functions
- Single-shot and continuous conversion modes
- Programmable gain amplifier (6 different voltage ranges)
- Programmable sample rate (8-860 SPS)
- Comparator with threshold and alert functionality
- Fully document with Doxygen

## Documentation
Full API documentation is available in `latex/refman.pdf` and `docs/html/index.html` (generate with Doxygen).

## Quick Start

### 1. Include Header
```c
#include "ads1115.h"
```

### 2. Implement Platform Callbacks

Example for STM32 HAL:
```c
void ads1115_delay_ms(uint32_t milliseconds)
{
    HAL_Delay(milliseconds);
}
```

### 3. Initialize Device
```c
ads1115_handle_t handle = {
    .i2c_addr = ADS1115_ADDR_GND,
    .i2c_write = ads1115_i2c_write,
    .i2c_read = ads1115_i2c_read,
    .delay_ms = ads1115_delay_ms,
    .config = ADS1115_DEFAULT_CONFIGURATION,
    .is_initialized = false
};

if (ads1115_init(&handle) != ADS1115_OK)
{
    // Handle error
}
```

### 4. Read ADC Value

**Single-shot mode:**
```c
int16_t adc_raw;
float voltage;

ads1115_set_channel(&handle, ADS1115_MUX_AIN0_GND);
ads1115_set_range(&handle, ADS1115_RANGE_4V096);

if (ads1115_single_read(&handle, &adc_raw, &voltage) == ADS1115_OK)
{
    printf("ADC: %d, Voltage: %.3f mV\n", adc_raw, voltage);
}
```

**Continuous mode:**
```c
int16_t adc_raw;
float voltage;

ads1115_set_channel(&handle, ADS1115_MUX_AIN0_GND);
ads1115_continuous_conversion_start(&handle);

while (1)
{
    if (ads1115_continuous_conversion_read(&adc_handle, &adc_raw, &voltage) == ADS1115_OK)
    {
        printf("ADC: %d, Voltage: %.3f mV\n", adc_raw, voltage);
    }
    
    HAL_Delay(100);
}
```

## API Reference

### Initialization

- `ads1115_init()` - Initialize device
- `ads1115_deinit()` - De-initialize device

### Configuration

- `ads1115_set_range()` / `ads1115_get_range()` - Set/get voltage range
- `ads1115_set_data_rate()` / `ads1115_get_data_rate()` - Set/get sample rate
- `ads1115_set_channel()` / `ads1115_get_channel()` - Set/get input channel

### Reading

- `ads1115_single_read()` - Single-shot conversion
- `ads1115_continuous_conversion_start()` - Start continuous mode
- `ads1115_continuous_conversion_read()` - Read in continuous mode
- `ads1115_continuous_conversion_stop()` - Stop continuous mode
- `ads1115_is_ready()` - Check if conversion is ready

### Comparator

- `ads1115_set_compare_mode()` / `ads1115_get_compare_mode()` - Comparator mode
- `ads1115_set_compare_threshold()` / `ads1115_get_compare_threshold()` - Thresholds
- `ads1115_set_compare_queue()` / `ads1115_get_compare_queue()` - Queue setting
- `ads1115_set_compare_latch()` / `ads1115_get_compare_latch()` - Latch mode
- `ads1115_set_compare_alert()` / `ads1115_get_compare_alert()` - Alert polarity

## Error Handling

All functions return `ads1115_error_t`:
```c
typedef enum {
    ADS1115_OK = 0,
    ADS1115_ERROR_INVALID_PARAM,
    ADS1115_ERROR_I2C_WRITE,
    ADS1115_ERROR_I2C_READ,
    ADS1115_ERROR_TIMEOUT,
    ADS1115_ERROR_NOT_INITIALIZED,
    ADS1115_ERROR_CONVERSION_BUSY,
    ADS1115_ERROR_NULL_POINTER
} ads1115_error_t;
```

## License

MIT License - see LICENSE file

## Author

Şükrü Can Kılıç (sukrucankilic23@gmail.com)

## Datasheet

[ADS1115 Datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf)

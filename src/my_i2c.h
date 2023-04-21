#pragma once

#include <stdint.h>
#include <stdbool.h>

//! @defgroup master_i2c Master I2C driver
//!
//! @brief Thread-safe Master I2C driver
//!
//! This driver uses the I2C_NUM_1 peripheral on the ESP32. Functions are multi-
//! thread safe, protected by an internal semaphore.
//! @{

//! @brief Initialize the I2C bus in master mode
//!
void my_i2c_init(uint8_t addr);

//! @brief Make a write transaction on the I2C bus
//!
//! @param[in] address Address of I2C bus to write to (7-bit address, right-aligned)
//! @param[in] reg I2C register to write to
//! @param[in] data Pointer to memory location to read data from
//! @param[in] size Number of bytes to write
bool my_i2c_write(uint8_t reg, uint8_t data);

//! @brief Make a read transaction on the I2C bus
//!
//! @param[in] address Address of I2C bus to read from(7-bit address, right-aligned)
//! @param[in] reg I2C register to read from
//! @param[out] data Pointer to memory location to write data to
//! @param[in] size Number of bytes to read
bool my_i2c_read(uint8_t reg, uint8_t *val);
bool I2C_Test();
//! @}

#ifndef _AT24C02_H
#define _AT24C02_H

#include "i2c.h"

// EEPROM
class AT24C02
{
public:
    /// @param timeout ms
    constexpr AT24C02(I2C *i2c, uint8_t A = 0b000, uint32_t timeout = HAL_MAX_DELAY);

    /// @brief ping AT24C02
    /// @return HAL_OK if success else HAL_ERROR
    HAL_StatusTypeDef ping();

    HAL_StatusTypeDef write(uint8_t mem_addr, uint8_t byte);
    HAL_StatusTypeDef write(uint8_t mem_addr, const void *p, uint16_t size);
    HAL_StatusTypeDef read(uint8_t mem_addr, void *p);
    HAL_StatusTypeDef read(uint8_t mem_addr, void *p, uint16_t size);

private:
    I2C *i2c;
    uint8_t dev_addr;
    uint32_t timeout; // ms
};

#pragma region detail

constexpr uint8_t AT24C02_DevAddr = 0xa0;

constexpr AT24C02::AT24C02(I2C *i2c, uint8_t A, uint32_t timeout)
    : i2c(i2c), dev_addr(AT24C02_DevAddr | ((A & 0b111) << 1)), timeout(timeout)
{
}

#pragma endregion

#endif // _AT24C02_H

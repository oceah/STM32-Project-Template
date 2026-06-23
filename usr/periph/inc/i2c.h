#ifndef I2C_H
#define I2C_H

#include "pin.h"

using namespace hal;

class I2C
{
public:
    constexpr I2C(const Pin &SCL, const Pin &SDA, uint32_t speed = 100000) noexcept;

    void init();

    // Check whether a device ACKs dev_addr. dev_addr is the 7-bit address shifted left by 1.
    HAL_StatusTypeDef ping(uint8_t dev_addr);
    // Read bytes from a device register.
    HAL_StatusTypeDef read(uint8_t dev_addr, uint8_t mem_addr, void *data, uint32_t size);
    // Read bytes directly from a device.
    HAL_StatusTypeDef read(uint8_t dev_addr, void *data, uint32_t size);
    // Write bytes to a device register.
    HAL_StatusTypeDef write(uint8_t dev_addr, uint8_t mem_addr, const void *data, uint32_t size);
    // Write bytes directly to a device.
    HAL_StatusTypeDef write(uint8_t dev_addr, const void *data, uint32_t size);

private:
    Pin SCL;
    Pin SDA;
    uint32_t clk; // us

    void start();
    void stop();

    void tx_byte(uint8_t byte);
    uint8_t rx_byte();
    void tx_ack(uint8_t bit);
    uint8_t rx_ack();

    HAL_StatusTypeDef tx_checked(uint8_t byte);
    HAL_StatusTypeDef read_bytes(uint8_t *data, uint32_t size);
};

constexpr I2C::I2C(const Pin &SCL, const Pin &SDA, uint32_t speed) noexcept
    : SCL(SCL), SDA(SDA), clk(1000000 / speed)
{
}

#endif // I2C_H

#include "i2c.h"

void I2C::init()
{
    SCL = 1;
    SDA = 1;
}

HAL_StatusTypeDef I2C::ping(uint8_t dev_addr)
{
    start();
    auto status = tx_checked(dev_addr);
    stop();
    return status;
}

HAL_StatusTypeDef I2C::read(uint8_t dev_addr, uint8_t mem_addr, void *p_, uint32_t size)
{
    auto p = static_cast<uint8_t *>(p_);
    if (size == 0)
        return HAL_OK;
    if (p == nullptr)
        return HAL_ERROR;

    start();
    if (tx_checked(dev_addr) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }
    if (tx_checked(mem_addr) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }

    start();
    if (tx_checked(dev_addr | 0x01) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }

    auto status = read_bytes(p, size);
    stop();
    return status;
}

HAL_StatusTypeDef I2C::read(uint8_t dev_addr, void *p_, uint32_t size)
{
    auto p = static_cast<uint8_t *>(p_);
    if (size == 0)
        return HAL_OK;
    if (p == nullptr)
        return HAL_ERROR;

    start();
    if (tx_checked(dev_addr | 0x01) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }

    auto status = read_bytes(p, size);
    stop();
    return status;
}

HAL_StatusTypeDef I2C::write(uint8_t dev_addr, uint8_t mem_addr, const void *p_, uint32_t size)
{
    auto p = static_cast<const uint8_t *>(p_);
    if (size > 0 && p == nullptr)
        return HAL_ERROR;

    start();
    if (tx_checked(dev_addr) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }
    if (tx_checked(mem_addr) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }

    while (size--) {
        if (tx_checked(*p++) != HAL_OK) {
            stop();
            return HAL_ERROR;
        }
    }

    stop();
    return HAL_OK;
}

HAL_StatusTypeDef I2C::write(uint8_t dev_addr, const void *p_, uint32_t size)
{
    auto p = static_cast<const uint8_t *>(p_);
    if (size == 0)
        return HAL_OK;
    if (p == nullptr)
        return HAL_ERROR;

    start();
    if (tx_checked(dev_addr) != HAL_OK) {
        stop();
        return HAL_ERROR;
    }

    while (size--) {
        if (tx_checked(*p++) != HAL_OK) {
            stop();
            return HAL_ERROR;
        }
    }

    stop();
    return HAL_OK;
}

void I2C::start()
{
    SDA = 1;
    SCL = 1;
    delay_us(clk);
    SDA = 0;
    delay_us(clk);
    SCL = 0;
}

void I2C::stop()
{
    SDA = 0;
    SCL = 1;
    delay_us(clk);
    SDA = 1;
    delay_us(clk);
}

void I2C::tx_byte(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; i++) {
        SDA = byte >> 7;
        delay_us(clk);
        SCL = 1;
        delay_us(clk);
        SCL = 0;
        delay_us(clk);
        byte <<= 1;
    }
}

uint8_t I2C::rx_byte()
{
    uint8_t byte = 0x00;
    SDA          = 1;
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        SCL = 1;
        delay_us(clk);
        if (SDA.read())
            byte |= 0x01;
        SCL = 0;
        delay_us(clk);
    }
    return byte;
}

void I2C::tx_ack(uint8_t bit)
{
    SDA = bit;
    delay_us(clk);
    SCL = 1;
    delay_us(clk);
    SCL = 0;
    delay_us(clk);
}

uint8_t I2C::rx_ack()
{
    SDA = 1;
    SCL = 1;
    delay_us(clk);
    uint8_t ack = SDA.read();
    SCL         = 0;
    delay_us(clk);
    return ack;
}

HAL_StatusTypeDef I2C::tx_checked(uint8_t byte)
{
    tx_byte(byte);
    return rx_ack() ? HAL_ERROR : HAL_OK;
}

HAL_StatusTypeDef I2C::read_bytes(uint8_t *data, uint32_t size)
{
    while (size > 1) {
        *data++ = rx_byte();
        tx_ack(0);
        --size;
    }

    *data = rx_byte();
    tx_ack(1);
    return HAL_OK;
}

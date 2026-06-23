#include "AT24C02.h"

#include <algorithm>

HAL_StatusTypeDef AT24C02::ping()
{
    return i2c->ping(dev_addr);
}

HAL_StatusTypeDef AT24C02::write(uint8_t mem_addr, uint8_t byte)
{
    return i2c->write(dev_addr, mem_addr, &byte, 1);
}

HAL_StatusTypeDef AT24C02::write(uint8_t mem_addr, const void *p_, uint16_t size)
{
    auto p = static_cast<const uint8_t *>(p_);
    size   = std::min<uint16_t>(size, 0x100 - mem_addr);
    if (size && (mem_addr & 7)) {
        auto status = ping();
        if (status != HAL_OK)
            return status;

        auto sz = std::min<uint16_t>(8 - (mem_addr & 7), size);
        status  = i2c->write(dev_addr, mem_addr, p, sz);
        if (status != HAL_OK)
            return status;
        mem_addr += sz;
        p += sz;
        size -= sz;
    }
    while (size) {
        auto status = ping();
        if (status != HAL_OK)
            return status;

        auto sz = std::min<uint16_t>(8, size);
        status  = i2c->write(dev_addr, mem_addr, p, sz);
        if (status != HAL_OK)
            return status;
        mem_addr += sz;
        p += sz;
        size -= sz;
    }
    return HAL_OK;
}

HAL_StatusTypeDef AT24C02::read(uint8_t mem_addr, void *p)
{
    return i2c->read(dev_addr, mem_addr, p, 1);
}

HAL_StatusTypeDef AT24C02::read(uint8_t mem_addr, void *p, uint16_t size)
{
    auto status = ping();
    if (status != HAL_OK)
        return status;
    size = std::min<uint16_t>(size, 0x100 - mem_addr);
    return i2c->read(dev_addr, mem_addr, p, size);
}

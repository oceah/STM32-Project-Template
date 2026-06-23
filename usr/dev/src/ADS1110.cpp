#include "ADS1110.h"

namespace
{
    static constexpr uint8_t ADS1110_ADDR = 0x90;
}

HAL_StatusTypeDef ADS1110::ping()
{
    return i2c->ping(ADS1110_ADDR);
}

float ADS1110::get()
{
    uint8_t buf[3];
    if (i2c->read(ADS1110_ADDR, buf, 3) != HAL_OK)
        return 0xffff;
    int16_t raw = (int16_t)((buf[0] << 8) | buf[1]);
    return raw * 2.048f / 32768.0f / pgaf;
}

HAL_StatusTypeDef ADS1110::set(DR data_rate, PGA pga)
{
    uint8_t cfg = (uint8_t)data_rate | (uint8_t)pga;
    pgaf        = 1 << (uint8_t)pga;
    return i2c->write(ADS1110_ADDR, &cfg, 1);
}

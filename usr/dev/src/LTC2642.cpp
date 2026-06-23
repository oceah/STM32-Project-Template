#include "LTC2642.h"

#ifdef HAL_SPI_MODULE_ENABLED

void LTC2642::init()
{
    CS = 1;
}

void LTC2642::set(float v)
{
    if (v > vref)
        v = vref;
    if (v < -vref)
        v = -vref;

    float code_f = 32768.0f * (v / vref) + 32768.0f;
    if (code_f > 65535.0f)
        code_f = 65535.0f;
    if (code_f < 0.0f)
        code_f = 0.0f;
    auto code = static_cast<uint16_t>(code_f);

    uint8_t tx[2] = {
        static_cast<uint8_t>(code >> 8),
        static_cast<uint8_t>(code & 0xff),
    };

    CS = 0;
    HAL_SPI_Transmit(hspi, tx, 2, HAL_MAX_DELAY);
    CS = 1;
}

#endif // HAL_SPI_MODULE_ENABLED

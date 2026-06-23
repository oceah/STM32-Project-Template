#ifndef ADS1110_H
#define ADS1110_H

#include "i2c.h"

// ADC
class ADS1110
{
public:
    constexpr ADS1110(I2C *i2c);

    HAL_StatusTypeDef ping();
    float get();

    enum class DR : uint8_t {
        SPS240 = 0 << 2,
        SPS60  = 1 << 2,
        SPS30  = 2 << 2,
        SPS15  = 3 << 2,
    };

    enum class PGA : uint8_t {
        _1 = 0,
        _2 = 1,
        _4 = 2,
        _8 = 3,
    };

    /**
     * @param data_rate 240|60|30|15
     * @param PGA 1|2|4|8
     */
    HAL_StatusTypeDef set(DR data_rate = DR::SPS15, PGA pga = PGA::_1);

private:
    I2C *i2c;
    float pgaf;
};

constexpr ADS1110::ADS1110(I2C *i2c)
    : i2c(i2c), pgaf(1 << (uint8_t)PGA::_1)
{
}

#endif // ADS1110_H

#ifndef AD5420_H
#define AD5420_H

#include "sys.h"

#ifdef HAL_SPI_MODULE_ENABLED

#include "pin.h"

using namespace hal;

// DAC
class AD5420
{
public:
    enum class Range : uint16_t {
        _4_20mA = 0b101,
        _0_20mA = 0b110,
        _0_24mA = 0b111,
    };

    enum class RSET : uint16_t {
        Internal = 0,
        External = 1,
    };

    struct Config {
        Range range       = Range::_4_20mA;
        RSET rset         = RSET::Internal;
        bool outputEnable = true;
        bool daisyChain   = false; // 菊花链
    };

    constexpr AD5420(SPI_HandleTypeDef *hspi, const Pin &LATCH);

    void init(const Config &config = default_config);
    void set(float mA);

private:
    SPI_HandleTypeDef *hspi;
    Pin LATCH;
    Config config;

    constexpr static Config default_config{
        .range        = Range::_4_20mA,
        .rset         = RSET::Internal,
        .outputEnable = true,
        .daisyChain   = false,
    };

    void write(uint8_t addr, uint16_t data);
};

constexpr AD5420::AD5420(SPI_HandleTypeDef *hspi, const Pin &LATCH)
    : hspi(hspi), LATCH(LATCH)
{
}

#endif // HAL_SPI_MODULE_ENABLED

#endif // AD5420_H

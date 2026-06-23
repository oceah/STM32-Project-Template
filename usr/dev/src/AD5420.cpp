#include "AD5420.h"

#ifdef HAL_SPI_MODULE_ENABLED

namespace
{
    static constexpr uint8_t REG_DATA  = 0x01;
    static constexpr uint8_t REG_CTRL  = 0x55;
    static constexpr uint8_t REG_RESET = 0x56;
}

void AD5420::init(const Config &config)
{
    LATCH = 0;
    write(REG_RESET, 0x0001);
    delay_ms(1);
    uint16_t ctrl_v =
        (static_cast<uint16_t>(config.rset) << 13) |
        (static_cast<uint16_t>(config.outputEnable) << 12) |
        (static_cast<uint16_t>(config.daisyChain) << 4) |
        static_cast<uint16_t>(config.range);
    write(REG_CTRL, ctrl_v);
    set(config.range == Range::_4_20mA ? 4.0f : 0.0f);
    this->config = config;
}

void AD5420::set(float mA)
{
    float min_mA = config.range == Range::_4_20mA ? 4.0f : 0.0f;
    float max_mA = config.range == Range::_0_24mA ? 24.0f : 20.0f;
    if (mA < min_mA) {
        mA = min_mA;
    } else if (mA > max_mA) {
        mA = max_mA;
    }
    uint16_t code = static_cast<uint16_t>(
        (mA - min_mA) * 65535.0f / (max_mA - min_mA) + 0.5f);
    write(REG_DATA, code);
}

void AD5420::write(uint8_t addr, uint16_t data)
{
    uint8_t tx[3] = {
        addr,
        static_cast<uint8_t>(data >> 8),
        static_cast<uint8_t>(data & 0xFF)};
    LATCH = 0;
    HAL_SPI_Transmit(hspi, tx, 3, HAL_MAX_DELAY);
    LATCH = 1;
    LATCH = 0;
}

#endif // HAL_SPI_MODULE_ENABLED

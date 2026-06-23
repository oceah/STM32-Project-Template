#ifndef ADS1246_H
#define ADS1246_H

#include "sys.h"

#ifdef HAL_SPI_MODULE_ENABLED

#include "pin.h"

using namespace hal;

// ADC
class ADS1246
{
public:
    constexpr ADS1246(SPI_HandleTypeDef *hspi, Pin CS, float vref = 2.048f);

    void init();
    float get();
    uint8_t get_id();

private:
    SPI_HandleTypeDef *hspi;
    Pin CS;
    float vref;

    uint8_t tx_buf[3];
    uint8_t rx_buf[3];

    void write_cmd(uint8_t cmd);
    uint8_t read_reg(uint8_t reg);
    void write_reg(uint8_t reg, uint8_t data);
};

constexpr ADS1246::ADS1246(SPI_HandleTypeDef *hspi, Pin CS, float vref)
    : hspi(hspi), CS(CS), vref(vref),
      tx_buf{}, rx_buf{}
{
}

#endif // HAL_SPI_MODULE_ENABLED

#endif // ADS1246_H

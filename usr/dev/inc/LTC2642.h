#ifndef LTC2642_H
#define LTC2642_H

#include "sys.h"

#ifdef HAL_SPI_MODULE_ENABLED

#include "pin.h"

using namespace hal;

class LTC2642
{
public:
    constexpr LTC2642(SPI_HandleTypeDef *hspi, Pin CS, float vref = 2.048f);

    void init();
    void set(float v);

private:
    SPI_HandleTypeDef *hspi;
    Pin CS;
    float vref;
};

constexpr LTC2642::LTC2642(SPI_HandleTypeDef *hspi, Pin CS, float vref)
    : hspi(hspi), CS(CS), vref(vref)
{
}

#endif // HAL_SPI_MODULE_ENABLED

#endif // LTC2642_H

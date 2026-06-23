#ifndef _OLED12864I2C_H
#define _OLED12864I2C_H

#include "i2c.h"

#include <utility>

class OLED12864I2C
{
public:
    constexpr OLED12864I2C(I2C *i2c);

    void init();

    HAL_StatusTypeDef ping() const;
    void clear();

    /// @brief set insert position
    /// @param r row position 0-3
    /// @param c col position 0-15
    void seek(uint8_t r, uint8_t c);

    void putchar(char chr);
    void print(const char *str);

    /**
     * img[0] ... img[127]
     * ...
     * img[7*128] ... img[7*128+127]
     * byte = [bit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0]
     *         down <---                       ---> up
     */
    void putimg(const void *img);

private:
    I2C *i2c;
    uint8_t r, c;

    void set_cursor(uint8_t x, uint8_t y);
};

constexpr OLED12864I2C::OLED12864I2C(I2C *i2c)
    : i2c(i2c), r(0), c(0)
{
}

#endif // _OLED12864I2C_H

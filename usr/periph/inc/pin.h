#ifndef PIN_H
#define PIN_H

#include "sys.h"

namespace hal
{

    struct Pin {
        GPIO_TypeDef *port;
        uint16_t pin;

        constexpr Pin() noexcept;
        constexpr Pin(GPIO_TypeDef *port, uint16_t pin) noexcept;

        bool read() const;
        void write(bool high);
        void toggle();

        void operator=(bool high);
    };

    constexpr Pin::Pin() noexcept
        : port(nullptr), pin(0)
    {
    }

    constexpr Pin::Pin(GPIO_TypeDef *port, uint16_t pin) noexcept
        : port(port), pin(pin)
    {
    }

} // namespace hal

#endif // PIN_H

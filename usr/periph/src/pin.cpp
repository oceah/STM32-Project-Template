#include "pin.h"

namespace hal
{

    bool Pin::read() const
    {
        return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
    }

    void Pin::write(bool high)
    {
        HAL_GPIO_WritePin(port, pin, high ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    void Pin::toggle()
    {
        HAL_GPIO_TogglePin(port, pin);
    }

    void Pin::operator=(bool high)
    {
        write(high);
    }

} // namespace hal

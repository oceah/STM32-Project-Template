#include "edge_trigger.h"

#ifdef HAL_EXTI_MODULE_ENABLED

namespace hal
{

    namespace
    {

        ISR exti_isr[16];

        bool valid_exti_pin(uint16_t GPIO_Pin)
        {
            return GPIO_Pin != 0 && (GPIO_Pin & (GPIO_Pin - 1U)) == 0;
        }

    } // namespace

    ISR *edge_trigger_get_isr(uint16_t GPIO_Pin)
    {
        if (!valid_exti_pin(GPIO_Pin))
            return nullptr;

        return &exti_isr[__builtin_ctz(GPIO_Pin)];
    }

    extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
    {
        auto p = edge_trigger_get_isr(GPIO_Pin);
        if (p != nullptr) {
            p->on_interrupt();
        }
    }

} // namespace hal

#endif // HAL_EXTI_MODULE_ENABLED

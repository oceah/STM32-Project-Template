#include "pwm.h"

#ifdef HAL_TIM_MODULE_ENABLED

#include <limits> // nan

namespace hal
{

    HAL_StatusTypeDef PWM::start()
    {
        if (htim == nullptr)
            return HAL_ERROR;

        return HAL_TIM_PWM_Start(htim, channel);
    }

    HAL_StatusTypeDef PWM::stop()
    {
        if (htim == nullptr)
            return HAL_ERROR;

        return HAL_TIM_PWM_Stop(htim, channel);
    }

    float PWM::get() const
    {
        if (htim == nullptr || htim->Init.Period == 0)
            return std::numeric_limits<float>::quiet_NaN();

        float compare = __HAL_TIM_GetCompare(htim, channel);
        float period  = static_cast<float>(htim->Init.Period);
        return compare / period;
    }

    void PWM::set(float duty)
    {
        if (htim == nullptr)
            return;

        uint32_t compare;
        if (duty <= 0.0f) {
            compare = 0;
        } else if (duty >= 1.0f) {
            compare = htim->Init.Period;
        } else {
            compare = htim->Init.Period * duty;
        }
        __HAL_TIM_SetCompare(htim, channel, compare);
    }

} // namespace hal

#endif // HAL_TIM_MODULE_ENABLED

#ifndef PWM_H
#define PWM_H

#include "sys.h"

#ifdef HAL_TIM_MODULE_ENABLED

#include "tim.h"

namespace hal
{

    // TIM PWM channel wrapper. The HAL timer handle is not owned by this class.
    struct PWM {
        TIM_HandleTypeDef *htim;
        uint32_t channel;

        constexpr PWM() noexcept;
        constexpr PWM(TIM_HandleTypeDef *htim, uint32_t channel) noexcept;

        HAL_StatusTypeDef start();
        HAL_StatusTypeDef stop();

        float get() const;
        void set(float duty);
    };

#pragma region detail

    constexpr PWM::PWM() noexcept
        : htim(nullptr), channel(0)
    {
    }

    constexpr PWM::PWM(TIM_HandleTypeDef *htim, uint32_t channel) noexcept
        : htim(htim), channel(channel)
    {
    }

#pragma endregion

} // namespace hal

#endif // HAL_TIM_MODULE_ENABLED

#endif // PWM_H

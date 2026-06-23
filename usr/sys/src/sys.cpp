#include "sys.h"
#include <cmath>

namespace
{

    void delay_systick(uint32_t counts)
    {
        const uint32_t reload = SysTick->LOAD + 1U;
        uint32_t previous     = SysTick->VAL;
        uint32_t elapsed      = 0U;
        while (elapsed < counts) {
            const uint32_t current = SysTick->VAL;
            const uint32_t step    = previous >= current ? previous - current : previous + (reload - current);
            if (step >= counts - elapsed) {
                return;
            }
            elapsed += step;
            previous = current;
        }
    }

} // namespace

void delay(float s)
{
    if (!std::isfinite(s) || s <= 0.0f) {
        return;
    }

    double remain = static_cast<double>(s);
    while (remain >= static_cast<double>(UINT32_MAX)) {
        delay_s(UINT32_MAX);
        const double next = remain - static_cast<double>(UINT32_MAX);
        if (next >= remain) {
            return;
        }
        remain = next;
    }

    const uint32_t whole_seconds = static_cast<uint32_t>(remain);
    if (whole_seconds > 0U) {
        delay_s(whole_seconds);
        remain -= static_cast<double>(whole_seconds);
    }

    const uint32_t whole_ms = static_cast<uint32_t>(remain * 1000.0);
    if (whole_ms > 0U) {
        delay_ms(whole_ms);
        remain -= static_cast<double>(whole_ms) / 1000.0;
    }

    const uint32_t whole_us = static_cast<uint32_t>(std::ceil(remain * 1000000.0));
    if (whole_us > 0U) {
        delay_us(whole_us);
    }
}

void delay_us(uint32_t us)
{
    const uint32_t counts_per_us = HAL_RCC_GetSysClockFreq() / 1000000U;
    if (us == 0U || counts_per_us == 0U) {
        return;
    }
    const uint32_t max_us_per_chunk = UINT32_MAX / counts_per_us;
    while (us > max_us_per_chunk) {
        delay_systick(max_us_per_chunk * counts_per_us);
        us -= max_us_per_chunk;
    }
    if (us > 0U) {
        delay_systick(us * counts_per_us);
    }
}

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

void delay_s(uint32_t s)
{
    constexpr uint32_t max_seconds_per_call = UINT32_MAX / 1000;
    while (s >= max_seconds_per_call) {
        delay_ms(max_seconds_per_call * 1000);
        s -= max_seconds_per_call;
    }
    if (s > 0) {
        delay_ms(s * 1000);
    }
}

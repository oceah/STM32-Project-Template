#include "HC_SR04.h"

#if defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

float HC_SR04::get() const
{
    uint32_t magic_v = magic.load(std::memory_order_relaxed);
    return std::bit_cast<float>(magic_v);
}

void HC_SR04::update()
{
    bool expected = false;
    if (!active.compare_exchange_strong(
            expected, true,
            std::memory_order_acquire,
            std::memory_order_relaxed))
        return;
    // reset
    tick.store(0, std::memory_order_relaxed);
    rising_t = 0;
    // start
    trigger.start();
    scheduled_task.start();
    // trig
    TRIG = 1;
    delay_us(10);
    TRIG = 0;
}

void HC_SR04::TIMHandler::operator()()
{
    self->tim_isr();
}

void HC_SR04::EXTIHandler::operator()()
{
    self->exti_isr();
}

void HC_SR04::stop()
{
    trigger.stop();
    scheduled_task.stop();
    active.store(false, std::memory_order_release);
}

void HC_SR04::tim_isr()
{
    auto tick_v = tick.fetch_add(1, std::memory_order_relaxed) + 1;
    // 超时
    if (tick_v >= max_tick) {
        // 写入无效值
        float invalid = std::numeric_limits<float>::quiet_NaN();
        magic.store(std::bit_cast<uint32_t>(invalid), std::memory_order_relaxed);

        stop();
    }
}

void HC_SR04::exti_isr()
{
    if (ECHO.read())
        rising_t = tick.load(std::memory_order_relaxed);
    else {
        uint32_t falling_t = tick.load(std::memory_order_relaxed);
        float magic_v      = (float)(falling_t - rising_t) * (1.0f / 18000.0f * 340.f / 2.0f);
        magic.store(std::bit_cast<uint32_t>(magic_v), std::memory_order_relaxed);

        stop();
    }
}

#endif // defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

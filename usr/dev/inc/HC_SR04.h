#ifndef HC_SR04_H
#define HC_SR04_H

#include "pin.h"
#include "edge_trigger.h"
#include "scheduled_task.h"

#if defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

using namespace hal;

#include <bit>
#include <atomic>
#include <limits>
#include <cstdint>

// 超声波测距模块
class HC_SR04
{
    static_assert(
        std::atomic<bool>::is_always_lock_free,
        "HC_SR04 requires lock-free atomic<bool>");

    static_assert(
        std::atomic<uint32_t>::is_always_lock_free,
        "HC_SR04 requires lock-free atomic<uint32_t>");

public:
    /// @param _18k clock source in 18000 Hz
    /// @param timeout timeout in ms
    constexpr HC_SR04(const Pin &TRIG, const Pin &ECHO, const ClockSource &_18k, uint32_t timeout = 30);

    float get() const; // 读取距离(m)

    void update();

private:
    struct TIMHandler {
        HC_SR04 *self;
        void operator()();
    };

    struct EXTIHandler {
        HC_SR04 *self;
        void operator()();
    };

    Pin TRIG, ECHO;
    const uint32_t max_tick;

    ScheduledTask<TIMHandler> scheduled_task;
    EdgeTrigger<EXTIHandler> trigger;

    std::atomic<uint32_t> magic = std::bit_cast<uint32_t>(std::numeric_limits<float>::quiet_NaN());
    std::atomic<bool> active    = false;
    std::atomic<uint32_t> tick  = 0;

    uint32_t rising_t = 0;

    void stop();
    void tim_isr();
    void exti_isr();
};

constexpr HC_SR04::HC_SR04(const Pin &TRIG, const Pin &ECHO, const ClockSource &_18k, uint32_t timeout)
    : TRIG(TRIG), ECHO(ECHO),
      max_tick(timeout * 18),
      scheduled_task(_18k, TIMHandler{this}),
      trigger(ECHO, EXTIHandler{this})
{
}

#endif // defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

#endif // HC_SR04_H

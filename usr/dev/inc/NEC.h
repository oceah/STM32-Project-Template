#ifndef NEC_H
#define NEC_H

#include "pin.h"

using namespace hal;

// NEC 红外发送协议
class NEC_TX
{
public:
    constexpr NEC_TX(Pin IR_TX);

    void init();
    void send(uint16_t data); // 红外发送数据

private:
    Pin IR_TX;

    void emit_enable(int32_t us);
};

#include "edge_trigger.h"
#include "scheduled_task.h"

#if defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

using namespace hal;

#include <atomic>
#include <optional>

// NEC 红外接收协议
class NEC_RX
{
public:
    constexpr NEC_RX(const Pin &IR_RX, const ClockSource &_38k4);

    /// @brief 读取红外接收数据
    /// @return 8位地址+8位数据
    std::optional<uint16_t> try_get();

    void start();
    void stop();

    NEC_RX(const NEC_RX &)            = delete;
    NEC_RX &operator=(const NEC_RX &) = delete;
    NEC_RX(NEC_RX &&)                 = delete;
    NEC_RX &operator=(NEC_RX &&)      = delete;

private:
    struct Handler {
        NEC_RX *self;
        void operator()();
    };

    Pin IR_RX;
    Clock clock;
    EdgeTrigger<Handler> trigger;

    std::atomic<uint32_t> magic = 0; // bit16 done, bit15~8 addr, bit7~0 data

    bool started  = false;
    uint8_t index = 0;
    uint8_t buf[4];

    void isr();
};

#endif // defined(HAL_EXTI_MODULE_ENABLED)  && defined(HAL_TIM_MODULE_ENABLED)

#pragma region detail

constexpr NEC_TX::NEC_TX(Pin IR_TX)
    : IR_TX(IR_TX)
{
}

#if defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)
constexpr NEC_RX::NEC_RX(const Pin &IR_RX, const ClockSource &_38k4)
    : IR_RX(IR_RX), clock(_38k4), trigger(IR_RX, Handler(this))
{
}
#endif

#pragma endregion

#endif // NEC_H

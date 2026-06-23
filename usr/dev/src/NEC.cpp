#include "NEC.h"

namespace
{
    inline void emit_disable(uint32_t us)
    {
        delay_us(us);
    }
}

void NEC_TX::init()
{
    IR_TX = 1;
}

void NEC_TX::send(uint16_t data)
{
    // 发送引导码
    emit_enable(9000);  // emit 9ms
    emit_disable(4500); // delay 4.5ms
    // 发送数据位
    uint8_t buf[4];
    buf[0] = data >> 8;
    buf[1] = ~buf[0];
    buf[2] = data & 0xff;
    buf[3] = ~buf[2];
    for (uint8_t i = 0; i < 4; i++)
        for (uint8_t j = 0; j < 8; j++) {
            emit_enable(560); // emit 0.56ms
            emit_disable(buf[i] & 0x01 ? 1680 : 560);
            buf[i] >>= 1;
        }
    // 发送停止位
    emit_enable(600); // emit 600us
}

void NEC_TX::emit_enable(int32_t us)
{
    while (us > 0) {
        IR_TX = 0;
        delay_us(13);
        IR_TX = 1;
        delay_us(13);
        us -= 26;
    }
}

#if defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

std::optional<uint16_t> NEC_RX::try_get()
{
    uint32_t magic_v = magic.exchange(0, std::memory_order_acquire);
    if (magic_v & 0x10000)
        return static_cast<uint16_t>(magic_v);
    return std::nullopt;
}

void NEC_RX::start()
{
    started = false;
    index   = 0;
    clock.reset();
    trigger.start();
}

void NEC_RX::stop()
{
    trigger.stop();
    clock.stop();
    started = false;
    index   = 0;
}

void NEC_RX::Handler::operator()()
{
    self->isr();
}

void NEC_RX::isr()
{
    auto tick = clock.now();

    if (!started) {
        clock.reset();
        clock.start();
        started = true;
        index   = 0;
        return;
    }

    // 普通 NEC start: 9ms + 4.5ms ≈ 13.5ms
    if (480 <= tick && tick <= 560) {
        index = 0;
        clock.reset();
        return;
    }

    if (index >= 32) {
        index   = 0;
        started = false;
        clock.stop();
        return;
    }

    uint8_t bit;

    if (30 <= tick && tick <= 58) {
        bit = 0;
    } else if (70 <= tick && tick <= 105) {
        bit = 1;
    } else {
        index   = 0;
        started = false;
        clock.stop();
        return;
    }

    uint8_t i = index >> 3;
    buf[i] >>= 1;
    if (bit)
        buf[i] |= 0x80;

    ++index;
    clock.reset();

    if (index == 32) {
        index = 0;
        clock.stop();
        started = false;

        if (static_cast<uint8_t>(buf[0] + buf[1]) == 0xff &&
            static_cast<uint8_t>(buf[2] + buf[3]) == 0xff) {
            uint32_t magic_v =
                (1u << 16) |
                (static_cast<uint32_t>(buf[0]) << 8) |
                static_cast<uint32_t>(buf[2]);

            magic.store(magic_v, std::memory_order_release);
        }
    }
}

#endif // defined(HAL_EXTI_MODULE_ENABLED) && defined(HAL_TIM_MODULE_ENABLED)

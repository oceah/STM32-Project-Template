#ifndef D36A_H
#define D36A_H

#include "pin.h"
#include "pwm.h"

using namespace hal;

// 步进电机驱动
class D36A
{
public:
    struct Part {
        PWM ST;  // 步进信号上升沿有效
        Pin DIR; // 方向控制

        float step_angle = 1.8f; // 步距角度
        float _period;           // 脉冲周期
    };

    enum class Select : uint8_t {
        Part1,
        Part2,
    };

    static const Part nullpart;

    constexpr D36A(const Part &part1, const Part &part2, uint8_t microstep = 1) noexcept;

    void set_rpm(Select sel, float rpm);
    void rotate(Select sel, float v);

private:
    Part part1, part2;
    uint8_t microstep;

    Part *select(Select sel);
};

#pragma region detail

inline const D36A::Part D36A::nullpart{};

constexpr D36A::D36A(const D36A::Part &part1, const Part &part2, uint8_t microstep) noexcept
    : part1(part1), part2(part2), microstep(microstep)
{
}

#pragma endregion

#endif // D36A_H

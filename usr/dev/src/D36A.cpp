#include "D36A.h"

#include <cmath>

void D36A::set_rpm(Select sel, float rpm)
{
    auto part = select(sel);
    if (part == nullptr) {
        return;
    }

    part->_period = (part->step_angle / rpm) / 6.0f;
    part->_period /= microstep;
    uint32_t freq      = 2.0f / part->_period;
    uint32_t prescaler = HAL_RCC_GetPCLK2Freq() / freq;
    uint32_t period    = 2;
    while (prescaler > 0xffffU) {
        prescaler >>= 1;
        period <<= 1;
    }
    auto htim            = part->ST.htim;
    auto channel         = part->ST.channel;
    htim->Init.Prescaler = prescaler - 1;
    htim->Init.Period    = period - 1;
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        Error_Handler();
    }
    __HAL_TIM_SetCompare(htim, channel, period >> 1);
}

void D36A::rotate(Select sel, float v)
{
    auto part = select(sel);
    if (part == nullptr) {
        return;
    }

    bool neg = v < 0.0f;
    if (neg) {
        v = -v;
    }
    if (v < part->step_angle) {
        return;
    }

    if (neg) {
        part->DIR.toggle();
    }
    float pulse = v / (part->step_angle / microstep);
    part->ST.start();
    delay(pulse * part->_period);
    part->ST.stop();
    if (neg) {
        part->DIR.toggle();
    }
}

D36A::Part *D36A::select(Select sel)
{
    Part *part;
    switch (sel) {
        case Select::Part1:
            part = &part1;
            break;
        case Select::Part2:
            part = &part2;
            break;
        default:
            part = nullptr;
    }
    return part;
}

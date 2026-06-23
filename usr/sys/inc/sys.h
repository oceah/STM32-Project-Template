#ifndef SYS_H
#define SYS_H

#include "main.h"
#if defined(STM32F1)
#include "stm32f1xx_hal_conf.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal_conf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void delay(float s);        // 微秒级阻塞延时
void delay_us(uint32_t us); // 微秒级阻塞延时
void delay_ms(uint32_t ms); // 毫秒级阻塞延时
void delay_s(uint32_t s);   // 秒级阻塞延时

#ifdef __cplusplus
}
#endif

#endif // SYS_H

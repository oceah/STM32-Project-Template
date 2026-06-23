#include "DHT11.h"

HAL_StatusTypeDef DHT11::init()
{
    DIO = 1;
    return reset_and_check();
}

DHT11::Data DHT11::get()
{
    Data data{
        .humidity    = 0xff,
        .temperature = 0xff,
    };
    // 检测响应信号
    if (!reset_and_check())
        return data;
    // 读取 40 位数据
    uint8_t buf[5];
    for (uint8_t i = 0; i < 5; i++)
        buf[i] = read_byte();
    // 校验
    if (buf[0] + buf[1] + buf[2] + buf[3] != buf[4])
        return data;
    // 校验成功
    data.humidity    = buf[0];
    data.temperature = buf[2];
    return data;
}

HAL_StatusTypeDef DHT11::reset_and_check()
{
    uint8_t timer = 0;
    __disable_irq();
    DIO = 0;
    delay_ms(20);
    DIO = 1;
    delay_us(30);
    while (!DIO.read()) {
        timer++;
        delay_us(1);
    }
    if (timer > 100 || timer < 20) {
        __enable_irq();
        return HAL_ERROR;
    }
    timer = 0;
    while (DIO.read()) {
        timer++;
        delay_us(1);
    }
    __enable_irq();
    if (timer > 100 || timer < 20)
        return HAL_ERROR;
    return HAL_OK;
}

uint8_t DHT11::read_byte()
{
    uint8_t byt = 0;
    __disable_irq();
    for (uint8_t i = 0; i < 8; i++) {
        while (DIO.read());
        while (!DIO.read());
        delay_us(40);
        byt <<= 1;
        if (DIO.read())
            byt |= 0x01;
    }
    __enable_irq();
    return byt;
}

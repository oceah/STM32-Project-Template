#ifndef DHT11_H
#define DHT11_H

#include "pin.h"

using namespace hal;

// 数字温湿度传感器
class DHT11
{
public:
    struct Data {
        uint8_t humidity;    // 相对湿度
        uint8_t temperature; // 温度
    };

    constexpr DHT11(const Pin &DIO);

    /// @return HAL_OK if 成功 else HAL_ERROR
    HAL_StatusTypeDef init();

    Data get(); // 读取一次数据

private:
    Pin DIO;

    /// @brief DHT11复位和检测
    /// @return HAL_OK if 检测到响应信号 else HAL_ERROR
    HAL_StatusTypeDef reset_and_check();

    /// @brief 读取一字节数据
    /// @return 读到的数据
    uint8_t read_byte();
};

constexpr DHT11::DHT11(const Pin &DIO)
    : DIO(DIO)
{
}

#endif // #ifndef DHT11_H

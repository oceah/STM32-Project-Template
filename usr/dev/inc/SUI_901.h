#ifndef SUI_901_H
#define SUI_901_H

#include "serial_port.h"

#ifdef HAL_UART_MODULE_ENABLED

using namespace hal;

// 串口通信四量程自动切换 uA/mA 电流检测卡
class SUI_901
{
public:
    constexpr SUI_901(UART_HandleTypeDef *huart);

    enum class Unit : uint8_t {
        mA = 0,
        uA = 1,
    };

    struct Current {
        float val;
        Unit unit;
    };

    void reset();

    Current get();

private:
    SerialPort com;
    char buf[15];

#pragma region 快捷指令
    // 获取当前测量的电流值 -> xxx mA/uA
    Current q_get_val();
    // 查询当前的通信地址 -> Add=xx
    uint8_t get_add();
    // 设置通信地址 01~99 -> Add=xx
    void set_add(uint8_t add);
    // 设置测量值自动输出 -> None
    void set_auto_out(bool auto_out);
    /// @brief 设置波特率
    /// @param br 4800|9600|19200|38400|57600|115200
    void set_br(uint32_t br);
    // 设置电流测量频率 10~40 -> CF=xx
    void set_cf(uint8_t cf);
    /// @brief 设置十六进制模式
    /// @param hex 1 自动输出模式下 测量值会以16进制格式输出
    /// @param hex 0 自动输出模式下 测量值会以明文字符串格式输出
    void set_hex(bool hex);
#pragma endregion
};

constexpr SUI_901::SUI_901(UART_HandleTypeDef *huart)
    : com(huart), buf{}
{
}

#endif // HAL_UART_MODULE_ENABLED

#endif // SUI_901_H

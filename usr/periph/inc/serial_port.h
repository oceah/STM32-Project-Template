#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include "sys.h"

#ifdef HAL_UART_MODULE_ENABLED

#include "usart.h"
#include <string_view>

namespace hal
{

    struct SerialPortConfig {
        bool use_dma_transmit = false;
    };

    // UART serial device wrapper.
    class SerialPort
    {
    public:
        constexpr explicit SerialPort(UART_HandleTypeDef *huart) noexcept;
        constexpr SerialPort(UART_HandleTypeDef *huart, const SerialPortConfig &config) noexcept;

        HAL_StatusTypeDef write(const void *data, uint16_t size);
        HAL_StatusTypeDef write(const char *str);

        uint16_t read(void *buffer, uint16_t size = 0xffff);

        // Keep reading while pred(received) is true. Timeout handling belongs to pred.
        template <class Pred>
        uint16_t read_while(void *buffer, Pred &&pred, uint16_t size = 0xffff);

    private:
        UART_HandleTypeDef *huart;
        SerialPortConfig config;

        uint32_t get_timeout(uint32_t size);
    };

#pragma region detail

    constexpr SerialPort::SerialPort(UART_HandleTypeDef *huart) noexcept
        : huart(huart)
    {
    }

    constexpr SerialPort::SerialPort(UART_HandleTypeDef *huart, const SerialPortConfig &config) noexcept
        : huart(huart), config(config)
    {
    }

    template <class Pred>
    uint16_t SerialPort::read_while(void *buffer, Pred &&pred, uint16_t size)
    {
        uint16_t actual_size = 0;
        auto buf             = static_cast<uint8_t *>(buffer);
        if (size == 0)
            return 0;

        auto timeout = get_timeout(1);
        while (actual_size < size) {
            auto status = HAL_UART_Receive(huart, &buf[actual_size], 1, timeout);
            if (status != HAL_OK && status != HAL_TIMEOUT)
                return actual_size;
            if (status != HAL_TIMEOUT)
                ++actual_size;
            if (!pred(std::string_view(static_cast<const char *>(buffer), actual_size)))
                break;
        }
        return actual_size;
    }

#pragma endregion

} // namespace hal

#endif // HAL_UART_MODULE_ENABLED

#endif // SERIAL_PORT_H

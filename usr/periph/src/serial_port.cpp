#include "serial_port.h"
#include <cstring>

#ifdef HAL_UART_MODULE_ENABLED

namespace hal
{

    HAL_StatusTypeDef SerialPort::write(const void *data, uint16_t size)
    {
        auto bytes = static_cast<uint8_t *>(const_cast<void *>(data));

        if (config.use_dma_transmit)
            return HAL_UART_Transmit_DMA(huart, bytes, size);

        auto timeout = get_timeout(size);
        return HAL_UART_Transmit(huart, bytes, size, timeout);
    }

    HAL_StatusTypeDef SerialPort::write(const char *str)
    {
        return write(str, std::strlen(str));
    }

    uint16_t SerialPort::read(void *buffer, uint16_t size)
    {
        uint16_t actual_size = 0;
        auto buf             = static_cast<uint8_t *>(buffer);
        if (size == 0)
            return 0;

        auto timeout = get_timeout(1);
        while (actual_size < size) {
            auto status = HAL_UART_Receive(huart, &buf[actual_size], 1, timeout);
            if (status != HAL_OK)
                return actual_size;
            ++actual_size;
        }
        return actual_size;
    }

    uint32_t SerialPort::get_timeout(uint32_t size)
    {
        uint32_t frame_bits = 1; // start bit
        {
            // data bits
            frame_bits += huart->Init.WordLength == UART_WORDLENGTH_9B ? 9 : 8;
            // parity bit
            if (huart->Init.Parity != UART_PARITY_NONE)
                frame_bits += 1;
            // stop bits
            switch (huart->Init.StopBits) {
                case UART_STOPBITS_2:
                    frame_bits += 2;
                    break;
#ifdef UART_STOPBITS_0_5
                case UART_STOPBITS_0_5:
                    frame_bits += 1;
                    break;
#endif
#ifdef UART_STOPBITS_1_5
                case UART_STOPBITS_1_5:
                    frame_bits += 2;
                    break;
#endif
                case UART_STOPBITS_1:
                default:
                    frame_bits += 1;
                    break;
            }
        }
        uint32_t br         = huart->Init.BaudRate;
        uint32_t total_bits = frame_bits * size;
        uint32_t timeout    = (total_bits * 1000 + br - 1) / br;
        return timeout;
    }

} // namespace hal

#endif // HAL_UART_MODULE_ENABLED

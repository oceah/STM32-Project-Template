#include "ADS1246.h"

#ifdef HAL_SPI_MODULE_ENABLED

namespace
{
    constexpr static uint8_t REG_BCS   = 0x00;
    constexpr static uint8_t REG_VBIAS = 0x01;
    constexpr static uint8_t REG_MUX1  = 0x02;
    constexpr static uint8_t REG_SYS0  = 0x03;
    constexpr static uint8_t REG_ID    = 0x0a;

    constexpr static uint8_t CMD_WAKEUP   = 0x00; // 唤醒
    constexpr static uint8_t CMD_SLEEP    = 0x02; // 睡眠
    constexpr static uint8_t CMD_SYNC     = 0x04; // 同步
    constexpr static uint8_t CMD_RESET    = 0x06; // 复位
    constexpr static uint8_t CMD_NOP      = 0xFF; // 无操作
    constexpr static uint8_t CMD_RDATA    = 0x12; // 一次性读取数据
    constexpr static uint8_t CMD_RDATAC   = 0x14; // 连续读取数据模式
    constexpr static uint8_t CMD_SDATAC   = 0x16; // 停止连续读取数据模式
    constexpr static uint8_t CMD_RREG     = 0x20; // 读寄存器
    constexpr static uint8_t CMD_WREG     = 0x40; // 写寄存器
    constexpr static uint8_t CMD_SYSOCAL  = 0x60; // 系统偏移校准
    constexpr static uint8_t CMD_SYSGCAL  = 0x61; // 系统增益校准
    constexpr static uint8_t CMD_SELFOCAL = 0x62; // 自偏移校准
} // namespace

void ADS1246::init()
{
    CS = 1;

    write_cmd(CMD_RESET);
    delay_ms(1);
    write_cmd(CMD_SDATAC);
    delay_ms(1);

    write_reg(REG_BCS, 0x01);
    write_reg(REG_VBIAS, 0x00);
    write_reg(REG_MUX1, 0x00);
    write_reg(REG_SYS0, 0x00); // PGA = 1, DR = 5 SPS

    write_cmd(CMD_SELFOCAL);
    delay_ms(3202);
}

float ADS1246::get()
{
    CS        = 0;
    tx_buf[0] = CMD_RDATA;
    HAL_SPI_Transmit(hspi, tx_buf, 1, HAL_MAX_DELAY);
    tx_buf[0] = tx_buf[1] = tx_buf[2] = CMD_NOP;
    HAL_SPI_TransmitReceive(hspi, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    CS = 1;

    uint32_t temp = ((uint32_t)rx_buf[0] << 16) |
                    ((uint32_t)rx_buf[1] << 8) |
                    ((uint32_t)rx_buf[2]);
    if (temp & 0x00800000)
        temp |= 0xFF000000;
    int32_t raw_data = (int32_t)temp;
    return (float)raw_data / 4194303.5f * vref;
}

uint8_t ADS1246::get_id()
{
    return read_reg(REG_ID) >> 4;
}

void ADS1246::write_cmd(uint8_t cmd)
{
    CS = 0;
    HAL_SPI_Transmit(hspi, &cmd, 1, HAL_MAX_DELAY);
    CS = 1;
}

uint8_t ADS1246::read_reg(uint8_t reg)
{
    tx_buf[0] = CMD_RREG | (reg & 0x0F);
    tx_buf[1] = 0x00;
    tx_buf[2] = CMD_NOP;

    CS = 0;
    HAL_SPI_TransmitReceive(hspi, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    CS = 1;

    return rx_buf[2];
}

void ADS1246::write_reg(uint8_t reg, uint8_t data)
{
    uint8_t tx_data[3];
    tx_data[0] = CMD_WREG | (reg & 0x0F);
    tx_data[1] = 0x00;
    tx_data[2] = data;

    CS = 0;
    HAL_SPI_Transmit(hspi, tx_data, 3, HAL_MAX_DELAY);
    CS = 1;
}

#endif // HAL_SPI_MODULE_ENABLED

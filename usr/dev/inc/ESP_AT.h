#ifndef ESP_AT_H
#define ESP_AT_H

#include "serial_port.h"

#ifdef HAL_UART_MODULE_ENABLED

using namespace hal;

#include <string_view>

namespace esp
{

    enum class AT_Status : uint8_t {
        OK        = 0,
        ERROR     = 1,
        SEND_FAIL = 4,
    };

    struct AT_Responce {
        AT_Status status;
        std::string_view message;

        operator bool()
        { return status == AT_Status::OK; }

        static AT_Responce ok()
        { return {AT_Status::OK, ""}; }
    };

    enum class WiFi_Mode : uint8_t {
        None           = 0, // 无 Wi-Fi 模式
        Station        = 1, // Station 模式
        SoftAP         = 2, // SoftAP 模式
        SoftAP_Station = 3, // SoftAP+Station 模式
    };

    // 加密方式
    enum class WiFi_Encryption : uint8_t {
        OPEN = 0,
        // WEP = 1, // 不支持 WEP
        WPA_PSK      = 2,
        WPA2_PSK     = 3,
        WPA_WPA2_PSK = 4,
    };

    using IPv4 = uint8_t[4]; // IPv4[0]:IPv4[1]:IPv4[2]:IPv4[3]
    using MAC  = uint8_t[6];

    struct WiFi_AccessPoint {
        const char *ssid; // 目标 AP 的 SSID
        MAC mac;          // 目标 AP 的 MAC 地址
        uint8_t channel;  // 信道号
        int8_t rssi;      // 信号强度
        bool pci_en;      // PCI 认证
    };

    struct WiFi_SoftAccessPoint {
        const char *ssid;    // SoftAP 的 SSID
        const char *pwd;     // SoftAP 的密码
        uint8_t channel;     // 信道号
        WiFi_Encryption ecn; // 加密方式
        uint8_t max_conn;    // 允许连入 SoftAP 的最多 station 数目
        bool hidden;         // 是否隐藏 SSID
    };

    struct IP_Config {
        IPv4 ipv4; // ESP SoftAP 的 IPv4 地址
        MAC mac;   // ESP SoftAP 的 MAC 地址
    };

    class AT_Terminal
    {
    public:
        constexpr AT_Terminal(UART_HandleTypeDef *huart);

        AT_Responce run(std::string_view request);

        /// @brief while (pred(received) && actual_size < size)) read();
        /// @param pred bool pred(std::string_view);
        template <class Pred>
        AT_Responce write_and_read_while(std::string_view request, Pred &&pred);

#pragma region 基础 AT 命令
        AT_Status at();             // 测试 AT 启动
        AT_Status reset();          // 重启模块
        AT_Status set_echo(bool v); // 开启或关闭 AT 回显功能
#pragma endregion

        AT_Responce connect_wifi(std::string_view ssid, std::string_view pwd = "");
        AT_Responce create_wifi_ap(std::string_view ssid, std::string_view pwd = "");
#pragma region Wi-Fi AT 命令集
        WiFi_Mode get_wifi_mode();                 // 查询 Wi-Fi 模式
        AT_Responce set_wifi_mode(WiFi_Mode mode); // 设置 Wi-Fi 模式
        WiFi_AccessPoint get_wifi_ap();            // 查询与 Station 连接的 AP 信息
        WiFi_SoftAccessPoint get_wifi_sap();       // 查询 SoftAP 的配置信息
        // 设置 Station 需连接的 AP
        AT_Responce set_wifi_ap(std::string_view ssid, std::string_view pwd = "");
        // 设置 SoftAP 的 ssid 和 password
        AT_Responce set_wifi_sap(std::string_view ssid, std::string_view pwd = "",
                                 uint8_t channel = 11, WiFi_Encryption ecn = WiFi_Encryption::WPA2_PSK);
#pragma endregion

        AT_Responce connect_tcp(std::string_view host, uint32_t port);
        AT_Responce create_tcp_host(uint16_t port = 8080);
#pragma region TCP/IP AT 命令
        IP_Config ipconfig();                    // 查询本地 IP 地址和 MAC 地址
        AT_Responce ping(std::string_view host); // ping 对端主机

        // 在普通传输模式或 Wi-Fi 透传模式下发送数据
        AT_Responce cip_send(const void *p, uint32_t size, int link_id = -1);
        uint32_t cip_recv(void *p, uint32_t len, int link_id = -1);

        /// @brief 查询 socket 接收模式
        /// @retval -1 查询失败
        /// @retval 0 主动模式
        /// @retval 1 被动模式
        int get_cip_recv_mode();
        AT_Responce set_cip_recv_mode(int mode); // 设置 socket 接收模式

        /// @brief 查询 socket 多连接模式
        /// @retval -1 查询失败
        /// @retval 0 单连接模式
        /// @retval 1 多连接模式
        int get_cip_mux_mode();
        AT_Responce set_cip_mux_mode(int mode); // 设置 socket 多连接模式

        /// @brief 建立/关闭 TCP 或 SSL 服务器
        /// @param enable 0(关闭服务器) 1(建立服务器)
        /// @param param2 端口号(mode=1) 是否关闭所有客户端(mode=0)
        AT_Responce set_tcp_host_status(bool enable, uint16_t param2);

        // 查询 TCP 或 SSL 服务器允许建立的最大连接数
        int get_cip_server_max_conn();
        // 设置 TCP 或 SSL 服务器允许建立的最大连接数
        AT_Responce set_cip_server_max_conn(int num);
#pragma endregion

    private:
        SerialPort com;
        char tx_buf[116];
        char rx_buf[136];

        char host[16];
        uint32_t port;

        AT_Status write(std::string_view &request);
        AT_Responce responce(std::string_view request, uint16_t size);
    };

#pragma region detail

    constexpr AT_Terminal::AT_Terminal(UART_HandleTypeDef *huart)
        : com(huart,
              SerialPortConfig{
                  .use_dma_transmit = false,
              }),
          tx_buf{}, rx_buf{}, host{}, port(0)
    {
    }

    template <class Pred>
    AT_Responce AT_Terminal::write_and_read_while(std::string_view request, Pred &&pred)
    {
        AT_Responce at_responce;
        at_responce.status  = write(request);
        at_responce.message = "";
        if (at_responce.status != AT_Status::OK)
            return at_responce;

        auto size = com.read_while(rx_buf, std::forward<Pred>(pred), sizeof(rx_buf) - 1);
        return responce(request, size);
    }

#pragma endregion

} // namespace ESP

#endif // HAL_UART_MODULE_ENABLED

#endif // ESP_AT_H

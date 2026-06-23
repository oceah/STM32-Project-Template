#include "ESP_AT.h"

#ifdef HAL_UART_MODULE_ENABLED

#include <cctype>
#include <cinttypes>
#include <cstdio>
#include <cstring>

namespace esp
{

    namespace
    {

        bool contains_word(const std::string_view &sv, const std::string_view &word)
        {
            std::size_t pos = 0;
            while ((pos = sv.find(word, pos)) != std::string_view::npos) {
                bool left_ok  = (pos == 0) || std::isspace(sv[pos - 1]);
                bool right_ok = (pos + word.size() >= sv.size()) || std::isspace(sv[pos + word.size()]);
                if (left_ok && right_ok)
                    return true;
                ++pos;
            }
            return false;
        }

        uint8_t hex2dec(char c)
        {
            if (std::isdigit(c))
                return c - '0';
            c = std::tolower(c);
            return c - 'a' + 10;
        }

        std::string_view strip(std::string_view sv)
        {
            while (!sv.empty() && std::isspace(sv.front()))
                sv.remove_prefix(1);
            while (!sv.empty() && std::isspace(sv.back()))
                sv.remove_suffix(1);
            return sv;
        }

    } // namespace

    AT_Responce AT_Terminal::run(std::string_view request)
    {
        auto status = write(request);
        if (status != AT_Status::OK)
            return {status, "write request fail"};

        auto size = com.read(rx_buf, sizeof(rx_buf) - 1);
        return responce(request, size);
    }

    AT_Status AT_Terminal::at()
    {
        return run("AT").status;
    }

    AT_Status AT_Terminal::reset()
    {
        return run("AT+RST").status;
    }

    AT_Status AT_Terminal::set_echo(bool v)
    {
        std::sprintf(tx_buf, "ATE%d", v);
        return run(tx_buf).status;
    }

    AT_Responce AT_Terminal::connect_wifi(std::string_view ssid, std::string_view pwd)
    {
        if (get_wifi_ap().ssid == ssid)
            return AT_Responce::ok();
        if (set_echo(false) != AT_Status::OK)
            return {AT_Status::ERROR, "ATE FAIL"};
        if (!set_wifi_mode(WiFi_Mode::Station))
            return {AT_Status::ERROR, "CWMODE FAIL"};
        if (!set_wifi_ap(ssid, pwd))
            return {AT_Status::ERROR, "CWJAP FAIL"};
        return AT_Responce::ok();
    }

    AT_Responce AT_Terminal::create_wifi_ap(std::string_view ssid, std::string_view pwd)
    {
        if (!set_wifi_mode(WiFi_Mode::SoftAP))
            return {AT_Status::ERROR, "CWMODE FAIL"};
        if (!set_wifi_sap(ssid, pwd))
            return {AT_Status::ERROR, "CWSAP FAIL"};
        return AT_Responce::ok();
    }

    WiFi_Mode AT_Terminal::get_wifi_mode()
    {
        auto responce = run("AT+CWMODE?");
        if (!responce)
            return WiFi_Mode::None;

        auto &msg     = responce.message;
        auto p        = msg.data();
        uint16_t size = msg.size();
        while (size && !std::isdigit(*p)) {
            ++p;
            --size;
        }
        if (!size)
            return WiFi_Mode::None;
        return static_cast<WiFi_Mode>(*p - '0');
    }

    AT_Responce AT_Terminal::set_wifi_mode(WiFi_Mode mode)
    {
        std::sprintf(tx_buf, "AT+CWMODE=%d", static_cast<uint8_t>(mode));
        return run(tx_buf);
    }

    WiFi_AccessPoint AT_Terminal::get_wifi_ap()
    {
        WiFi_AccessPoint cwjap = {0};
        auto responce          = run("AT+CWJAP?");
        if (responce.status != AT_Status::OK)
            return cwjap;
        auto &msg = responce.message;

        auto p                        = const_cast<char *>(msg.data());
        auto size                     = msg.size();
        auto goto_next_quotation_mark = [&p, &size]() {
            while (size && *p != '\"') {
                ++p;
                --size;
            }
        };
        auto get_next_int = [&p, &size](int dft) {
            while (size && !std::isdigit(*p) && *p != '-') {
                ++p;
                --size;
            }
            if (!size)
                return dft;
            bool s = false;
            if (*p == '-') {
                s = true;
                ++p;
                --size;
                if (!size)
                    return dft;
            }
            int v = 0;
            while (size && std::isdigit(*p)) {
                v *= 10;
                v += *p - '0';
                ++p;
                --size;
            }
            return s ? -v : v;
        };

        // ssid
        goto_next_quotation_mark();
        ++p;
        --size;
        if (!size)
            return cwjap;
        cwjap.ssid = p;
        goto_next_quotation_mark();
        *p = '\0';

        // mac
        goto_next_quotation_mark();
        ++p;
        --size;
        if (!size)
            return cwjap;
        for (uint8_t i = 0; i < 6 && size >= 2; ++i) {
            p[0]         = hex2dec(p[0]);
            p[1]         = hex2dec(p[1]);
            cwjap.mac[i] = (p[0] << 4) | p[1];
            p += 3;
            size -= 3;
        }
        if (!size)
            return cwjap;

        // channel
        cwjap.channel = get_next_int(0);
        // rssi
        cwjap.rssi = get_next_int(-128);
        // pci_en
        cwjap.pci_en = get_next_int(0) != 0;

        return cwjap;
    }

    WiFi_SoftAccessPoint AT_Terminal::get_wifi_sap()
    {
        // +CWSAP:"ssid","pwd",channel,ecn,max_conn,hidden
        WiFi_SoftAccessPoint cwsap = {0};
        auto responce              = run("AT+CWSAP?");
        if (responce.status != AT_Status::OK)
            return cwsap;
        auto &msg = responce.message;

        auto p                        = const_cast<char *>(msg.data());
        auto size                     = msg.size();
        auto goto_next_quotation_mark = [&p, &size]() {
            while (size && *p != '\"') {
                ++p;
                --size;
            }
        };
        auto get_next_int = [&p, &size](int dft) {
            while (size && !std::isdigit(*p) && *p != '-') {
                ++p;
                --size;
            }
            if (!size)
                return dft;
            bool s = false;
            if (*p == '-') {
                s = true;
                ++p;
                --size;
                if (!size)
                    return dft;
            }
            int v = 0;
            while (size && std::isdigit(*p)) {
                v *= 10;
                v += *p - '0';
                ++p;
                --size;
            }
            return s ? -v : v;
        };

        // ssid
        goto_next_quotation_mark();
        ++p;
        --size;
        if (!size)
            return cwsap;
        cwsap.ssid = p;
        goto_next_quotation_mark();
        *p = '\0';

        // pwd
        goto_next_quotation_mark();
        ++p;
        --size;
        if (!size)
            return cwsap;
        cwsap.pwd = p;
        goto_next_quotation_mark();
        *p = '\0';

        // channel
        cwsap.channel = get_next_int(0);
        // ecn
        cwsap.ecn = static_cast<WiFi_Encryption>(get_next_int(0));
        // max_conn
        cwsap.max_conn = get_next_int(0);
        // hidden
        cwsap.hidden = get_next_int(0) != 0;

        return cwsap;
    }

    AT_Responce AT_Terminal::set_wifi_ap(std::string_view ssid, std::string_view pwd)
    {
        // 14(AT+CWJAP="","") + 32(ssid) + 64(pwd) + 1('\0') = 111
        int n;
        if (pwd.empty())
            n = std::snprintf(tx_buf, sizeof(tx_buf), "AT+CWJAP=\"%.*s\"", ssid.size(), ssid.data());
        else
            n = std::snprintf(tx_buf, sizeof(tx_buf), "AT+CWJAP=\"%.*s\",\"%.*s\"", ssid.size(), ssid.data(), pwd.size(), pwd.data());
        if (n < 0 || (size_t)n >= sizeof(tx_buf)) {
            AT_Responce at_responce;
            at_responce.status = AT_Status::SEND_FAIL;
            return at_responce;
        }

        AT_Status status = AT_Status::ERROR;
        auto pred        = [this, &status](std::string_view str) {
            if (contains_word(str, "OK")) {
                status = AT_Status::OK;
                return false;
            }
            if (contains_word(str, "ERROR") || contains_word(str, "FAIL")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        auto responce   = AT_Terminal::write_and_read_while(tx_buf, pred);
        responce.status = status;
        return responce;
    }

    AT_Responce AT_Terminal::set_wifi_sap(std::string_view ssid, std::string_view pwd, uint8_t channel, WiFi_Encryption ecn)
    {
        // 19(AT+CWSAP="","",11,0) + 32(ssid) + 64(pwd) + 1('\0') = 116
        int n = std::snprintf(
            tx_buf, sizeof(tx_buf),
            "AT+CWSAP=\"%.*s\",\"%.*s\",%d,%d",
            ssid.size(), ssid.data(),
            pwd.size(), pwd.data(),
            channel, static_cast<int>(ecn));
        if (n < 0 || (size_t)n >= sizeof(tx_buf)) {
            AT_Responce at_responce;
            at_responce.status = AT_Status::SEND_FAIL;
            return at_responce;
        }

        AT_Status status = AT_Status::ERROR;
        auto pred        = [this, &status](std::string_view str) {
            if (contains_word(str, "OK")) {
                status = AT_Status::OK;
                return false;
            }
            if (contains_word(str, "ERROR") || contains_word(str, "FAIL")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        auto responce   = AT_Terminal::write_and_read_while(tx_buf, pred);
        responce.status = status;
        return responce;
    }

    AT_Responce AT_Terminal::connect_tcp(std::string_view host, uint32_t port)
    {
        std::sprintf(tx_buf, "AT+CIPSTART=\"TCP\",\"%s\",%" PRIu32, host.data(), port);
        AT_Status status = AT_Status::ERROR;
        auto pred        = [&status](std::string_view str) {
            if (contains_word(str, "OK") || contains_word(str, "ALREADY CONNECTED")) {
                status = AT_Status::OK;
                return false;
            }
            if (contains_word(str, "ERROR")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        write_and_read_while(tx_buf, pred);
        if (status != AT_Status::OK)
            return {status, "CIPSTART FAIL"};

        if (get_cip_recv_mode() != 1) {
            set_cip_recv_mode(1);
            if (get_cip_recv_mode() != 1)
                return {AT_Status::ERROR, "CIPRECVMODE FAIL"};
        }

        std::memcpy(this->host, host.data(), host.size());
        this->port = port;
        return AT_Responce::ok();
    }

    AT_Responce AT_Terminal::create_tcp_host(uint16_t port)
    {
        // AT+CIPMUX=1
        if (get_cip_mux_mode() != 1) {
            if (!set_cip_mux_mode(1))
                return {AT_Status::ERROR, "CIPMUX FAIL"};
            if (get_cip_mux_mode() != 1)
                return {AT_Status::ERROR, "CIPMUX FAIL"};
        }
        // AT+CIPSERVER=1,port
        if (!set_tcp_host_status(true, port))
            return {AT_Status::ERROR, "CIPSERVER FAIL"};
        // AT+CIPRECVMODE=1
        if (!set_cip_recv_mode(1))
            return {AT_Status::ERROR, "CIPRECVMODE FAIL"};
        return AT_Responce::ok();
    }

    IP_Config AT_Terminal::ipconfig()
    {
        IP_Config ipconfig = {0};

        auto responce = run("AT+CIFSR");
        if (!responce)
            return ipconfig;
        auto &msg = responce.message;

        auto pos = msg.find("+CIFSR:STAIP");
        if (pos == std::string_view::npos) {
            pos = msg.find("+CIFSR:APIP");
            if (pos == std::string_view::npos)
                return ipconfig;
        }
        while (pos != msg.size() && !std::isdigit(msg[pos]))
            ++pos;
        if (pos == msg.size())
            return ipconfig;
        for (uint8_t i = 0; i < 4; ++i) {
            uint8_t v = 0;
            while (pos != msg.size() && std::isdigit(msg[pos])) {
                v *= 10;
                v += msg[pos] - '0';
                ++pos;
            }
            ipconfig.ipv4[i] = v;
            if (pos == msg.size())
                return ipconfig;
            ++pos;
        }

        auto next_pos = msg.find("+CIFSR:STAMAC", pos);
        if (next_pos == std::string_view::npos) {
            next_pos = msg.find("+CIFSR:APMAC", pos);
            if (next_pos == std::string_view::npos)
                return ipconfig;
        }
        pos = next_pos;
        while (pos != msg.size() && msg[pos] != '"')
            ++pos;
        ++pos;
        if (pos >= msg.size())
            return ipconfig;
        for (uint8_t i = 0; i < 6; ++i) {
            if (pos + 1 >= msg.size())
                return ipconfig;
            uint8_t v       = (hex2dec(msg[pos]) << 4) | hex2dec(msg[pos + 1]);
            ipconfig.mac[i] = v;
            pos += 3;
        }

        return ipconfig;
    }

    AT_Responce AT_Terminal::ping(std::string_view host)
    {
        std::sprintf(tx_buf, "AT+PING=\"%s\"", host.data());
        auto pred = [](std::string_view str) {
            if (contains_word(str, "OK") || contains_word(str, "ERROR"))
                return false;
            return true;
        };
        return write_and_read_while(tx_buf, pred);
    }

    AT_Responce AT_Terminal::cip_send(const void *p, uint32_t size, int link_id)
    {
        if (link_id < 0)
            std::snprintf(tx_buf, sizeof(tx_buf), "AT+CIPSEND=%" PRIu32, size);
        else {
            std::snprintf(tx_buf, sizeof(tx_buf), "AT+CIPSEND=%d,%" PRIu32, link_id, size);
        }
        AT_Status status = AT_Status::ERROR;
        auto pred1       = [&status](std::string_view str) {
            if (contains_word(str, ">")) {
                status = AT_Status::OK;
                return false;
            }
            if (contains_word(str, "ERROR") || contains_word(str, "CLOSED")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        auto responce   = write_and_read_while(tx_buf, pred1);
        responce.status = status;
        if (status != AT_Status::OK)
            return responce;

        responce.status = com.write(p, size) == HAL_OK ? AT_Status::OK : AT_Status::SEND_FAIL;
        if (!responce)
            return responce;

        auto pred2 = [&responce](std::string_view str) {
            if (contains_word(str, "SEND OK")) {
                responce.status = AT_Status::OK;
                return false;
            }
            if (
                contains_word(str, "SEND FAIL")) {
                responce.status = AT_Status::SEND_FAIL;
                return false;
            }
            return true;
        };
        com.read_while(rx_buf, pred2, sizeof(rx_buf) - 1);
        return responce;
    }

    uint32_t AT_Terminal::cip_recv(void *p_, uint32_t len, int link_id)
    {
        auto p = static_cast<uint8_t *>(p_);
        if (link_id < 0)
            std::sprintf(tx_buf, "AT+CIPRECVDATA=%" PRIu32, len);
        else {
            std::sprintf(tx_buf, "AT+CIPRECVDATA=%d,%" PRIu32, link_id, len);
        }
        AT_Status status = AT_Status::ERROR;
        auto pred        = [&status](std::string_view str) {
            if (str.ends_with("CIPRECVDATA,")) {
                status = AT_Status::OK;
                return false;
            }
            if (str.ends_with("OK") || str.ends_with("ERROR") || str.ends_with("CLOSED")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        write_and_read_while(tx_buf, pred);
        if (status != AT_Status::OK)
            return 0;

        uint32_t actual_len;
        while (true) {
            if (com.read(rx_buf, 1) != 1)
                return 0;
            if (std::isdigit(rx_buf[0])) {
                actual_len = rx_buf[0] - '0';
                break;
            }
        }

        while (true) {
            if (com.read(rx_buf, 1) != 1)
                return 0;
            if (!std::isdigit(rx_buf[0]))
                break;
            actual_len *= 10;
            actual_len += rx_buf[0] - '0';
        }

        uint32_t received = 0;
        while (received < actual_len) {
            auto r = com.read(p + received, actual_len - received);
            if (r == 0)
                return received;
            received += r;
        }
        return received;
    }

    int AT_Terminal::get_cip_recv_mode()
    {
        auto responce = run("AT+CIPRECVMODE?");
        if (!responce)
            return -1;
        auto &msg = responce.message;
        auto pos  = msg.find("+CIPRECVMODE:");
        if (pos == std::string_view::npos)
            return -1;
        while (pos != msg.size() && !std::isdigit(msg[pos]))
            ++pos;
        if (pos == msg.size())
            return -1;
        return msg[pos] - '0';
    }

    AT_Responce AT_Terminal::set_cip_recv_mode(int mode)
    {
        std::sprintf(tx_buf, "AT+CIPRECVMODE=%d", mode);
        return run(tx_buf);
    }

    int AT_Terminal::get_cip_mux_mode()
    {
        auto responce = run("AT+CIPMUX?");
        if (!responce)
            return -1;
        auto &msg = responce.message;
        auto pos  = msg.find("+CIPMUX:");
        if (pos == std::string_view::npos)
            return -1;
        while (pos != msg.size() && !std::isdigit(msg[pos]))
            ++pos;
        if (pos == msg.size())
            return -1;
        return msg[pos] - '0';
    }

    AT_Responce AT_Terminal::set_cip_mux_mode(int mode)
    {
        std::sprintf(tx_buf, "AT+CIPMUX=%d", mode);
        return run(tx_buf);
    }

    AT_Responce AT_Terminal::set_tcp_host_status(bool enable, uint16_t param2)
    {
        // 20(AT+CIPSERVER=1,65535)
        std::sprintf(tx_buf, "AT+CIPSERVER=%d,%d", enable, param2);
        AT_Status status = AT_Status::ERROR;
        auto pred        = [&status](std::string_view str) {
            if (contains_word(str, "OK")) {
                status = AT_Status::OK;
                return false;
            }
            if (contains_word(str, "ERROR")) {
                status = AT_Status::ERROR;
                return false;
            }
            return true;
        };
        return write_and_read_while(tx_buf, pred);
    }

    int AT_Terminal::get_cip_server_max_conn()
    {
        auto responce = run("AT+CIPSERVERMAXCONN?");
        if (!responce)
            return -1;
        auto &msg = responce.message;
        auto pos  = msg.find("+CIPSERVERMAXCONN:");
        if (pos == std::string_view::npos)
            return -1;
        while (pos != msg.size() && !std::isdigit(msg[pos]))
            ++pos;
        int num = 0;
        while (pos != msg.size() && std::isdigit(msg[pos]))
            num = num * 10 + (msg[pos++] - '0');
        return num;
    }

    AT_Responce AT_Terminal::set_cip_server_max_conn(int num)
    {
        std::sprintf(tx_buf, "AT+CIPSERVERMAXCONN=%d", num);
        return run(tx_buf);
    }

    AT_Status AT_Terminal::write(std::string_view &request)
    {
        request = strip(request);
        if (com.write(request.data(), request.size()) != HAL_OK)
            return AT_Status::SEND_FAIL;
        if (com.write("\r\n", 2) != HAL_OK)
            return AT_Status::SEND_FAIL;

        return AT_Status::OK;
    }

    AT_Responce AT_Terminal::responce(std::string_view request, uint16_t size)
    {
        std::string_view msg(rx_buf, size);
        msg      = strip(msg);
        auto pos = msg.find(request);
        if (pos != std::string_view::npos) {
            msg.remove_prefix(pos + request.size());
            msg = strip(msg);
        }
        if (msg.empty())
            return {AT_Status::ERROR, ""};

        if (msg.ends_with("ERROR")) {
            msg.remove_suffix(5);
            msg = strip(msg);

            const_cast<char *>(msg.data())[msg.size()] = '\0';
            return {AT_Status::ERROR, msg};
        }

        if (msg.ends_with("OK")) {
            msg.remove_suffix(2);
            msg = strip(msg);
        }
        const_cast<char *>(msg.data())[msg.size()] = '\0';
        return {AT_Status::OK, msg};
    }

} // namespace ESP

#endif // HAL_UART_MODULE_ENABLED

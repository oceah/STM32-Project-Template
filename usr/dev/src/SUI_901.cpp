#include "SUI_901.h"

#ifdef HAL_UART_MODULE_ENABLED

#include <cctype>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <cstring>

#include <charconv>
#include <limits>

namespace
{
    std::string_view strip(std::string_view sv)
    {
        while (!sv.empty() && std::isspace(sv.front()))
            sv.remove_prefix(1);
        while (!sv.empty() && std::isspace(sv.back()))
            sv.remove_suffix(1);
        return sv;
    }
}

void SUI_901::reset()
{
    if (get_add() != 1)
        set_add(1);
    set_auto_out(false);
    delay_ms(3);
    set_br(9600);
    delay_ms(937);
    set_cf(10);
    set_hex(false);
}

SUI_901::Current SUI_901::get()
{
    return q_get_val();
}

SUI_901::Current SUI_901::q_get_val()
{
    com.write(">>GetVal");
    auto size = com.read(buf, sizeof(buf));
    std::string_view str(buf, size);
    str = strip(str);

    char *end;
    if (str.ends_with("mA")) {
        str.remove_suffix(2);
        str     = strip(str);
        float v = std::strtof(str.data(), &end);
        if (end != str.data() && end == str.data() + str.size())
            return {v, Unit::mA};
    } else if (str.ends_with("uA")) {
        str.remove_suffix(2);
        str     = strip(str);
        float v = std::strtof(str.data(), &end);
        if (end != str.data() && end == str.data() + str.size())
            return {v, Unit::uA};
    }

    return {std::numeric_limits<float>::quiet_NaN(), Unit::mA};
}

uint8_t SUI_901::get_add()
{
    com.write(">>Add=?");
    auto size = com.read(buf, sizeof(buf));
    std::string_view str(buf, size);
    str      = strip(str);
    auto pos = str.find("Add=");
    if (pos == std::string_view::npos)
        return 0;
    str.remove_prefix(pos + 4);
    uint8_t add;
    auto result = std::from_chars(str.data(), str.data() + str.size(), add);
    return result.ec == std::errc() ? add : 0;
}

void SUI_901::set_add(uint8_t add)
{
    std::sprintf(buf, ">>Add=%02d", add);
    com.write(buf, 8);
}

void SUI_901::set_auto_out(bool auto_out)
{
    std::sprintf(buf, ">>AutoOut=%d", auto_out);
    com.write(buf, 11);
}

void SUI_901::set_br(uint32_t br)
{
    std::strcpy(buf, ">>BR=");
    if (br == 4800)
        buf[5] = '1';
    else if (br == 9600)
        buf[5] = '2';
    else if (br == 19200)
        buf[5] = '3';
    else if (br == 38400)
        buf[5] = '4';
    else if (br == 57600)
        buf[5] = '5';
    else if (br == 115200)
        buf[5] = '6';
    else
        return;
    com.write(buf, 6);
}

void SUI_901::set_cf(uint8_t cf)
{
    std::sprintf(buf, ">>CF=%d", cf);
    com.write(buf, 7);
}

void SUI_901::set_hex(bool hex)
{
    std::sprintf(buf, ">>Hex=%d", hex);
    com.write(buf, 7);
}

#endif // HAL_UART_MODULE_ENABLED

#ifndef OCEAH_HACHIMI
#define OCEAH_HACHIMI

#include <cctype>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace oceah
{

    struct HaChiMi_Command {
        int argc;
        char **argv;
    };

    // Service nodes are owned by the caller.
    struct HaChiMi_Service_Base {
        virtual void handle(const HaChiMi_Command &) = 0;

        virtual ~HaChiMi_Service_Base() = default;

        std::string_view key;

    private:
        template <uint32_t BufferSize, int MaxArgCount>
        friend class HaChiMi;

        HaChiMi_Service_Base *_prev = nullptr;
        HaChiMi_Service_Base *_next = nullptr;
    };

    // Command service wrapper.
    template <class F>
    struct HaChiMi_Service : public HaChiMi_Service_Base {
        explicit HaChiMi_Service(std::string_view key, const F &handler) //
            noexcept(std::is_nothrow_copy_constructible_v<F>)
            : handler(handler)
        {
            HaChiMi_Service_Base::key = key;
        }

        explicit HaChiMi_Service(std::string_view key, F &&handler) //
            noexcept(std::is_nothrow_move_constructible_v<F>)
            : handler(std::move(handler))
        {
            HaChiMi_Service_Base::key = key;
        }

        HaChiMi_Service(const HaChiMi_Service &)            = delete;
        HaChiMi_Service &operator=(const HaChiMi_Service &) = delete;

        HaChiMi_Service(HaChiMi_Service &&)            = delete;
        HaChiMi_Service &operator=(HaChiMi_Service &&) = delete;

        F handler;

        void handle(const HaChiMi_Command &cmd) //
            noexcept(noexcept(std::declval<F &>()(std::declval<const HaChiMi_Command &>()))) override
        {
            handler(cmd);
        }
    };

    // Lightweight streaming command dispatcher.
    template <uint32_t BufferSize = 1024, int MaxArgCount = 8>
    class HaChiMi
    {
        static_assert(BufferSize > 0);
        static_assert(MaxArgCount > 0);

    public:
        using size_type = uint32_t;

        HaChiMi() = default;

        void register_service(HaChiMi_Service_Base *service);
        void unregister_service(HaChiMi_Service_Base *service);

        void push(char c);
        void push(const char *str);
        void push(const void *p, size_type size);

    private:
        char _buf[BufferSize];
        size_type _size = 0;
        char *_argv[MaxArgCount];
        bool _overflow                  = false;
        HaChiMi_Service_Base *_services = nullptr;

        int parse();
        void reset_line();
        void dispatch(HaChiMi_Command cmd) const;
    };

#pragma region detail

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::register_service(HaChiMi_Service_Base *service)
    {
        if (!service)
            return;

        service->_prev = nullptr;
        service->_next = _services;
        if (_services)
            _services->_prev = service;
        _services = service;
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::unregister_service(HaChiMi_Service_Base *service)
    {
        if (!service)
            return;

        auto prev = service->_prev;
        auto next = service->_next;
        if (prev)
            prev->_next = next;
        else if (_services == service) {
            _services = next;
        }
        if (next)
            next->_prev = prev;

        service->_prev = nullptr;
        service->_next = nullptr;
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::push(char c)
    {
        if (_size >= BufferSize) {
            _overflow = true;
        }

        if (c == '\0' || c == '\r' || c == '\n') {
            if (_overflow) {
                reset_line();
                return;
            }

            _buf[_size] = '\0';
            auto argc   = parse();
            if (argc > 0) {
                HaChiMi_Command cmd{argc, _argv};
                dispatch(cmd);
            }
            reset_line();
            return;
        }

        if (_overflow)
            return;

        _buf[_size++] = c;
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::push(const char *str)
    {
        if (!str)
            return;

        while (*str)
            push(*str++);
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::push(const void *p, size_type size)
    {
        if (!p)
            return;

        auto str = static_cast<const char *>(p);
        for (size_type i = 0; i < size; ++i)
            push(str[i]);
    }

    template <uint32_t BufferSize, int MaxArgCount>
    int HaChiMi<BufferSize, MaxArgCount>::parse()
    {
        int argc    = 0;
        size_type i = 0;
        while (true) {
            while (i < _size && std::isspace(static_cast<unsigned char>(_buf[i])))
                ++i;
            if (i >= _size)
                return argc;
            if (argc >= MaxArgCount) {
                return 0;
            }
            if (_buf[i] == '"') {
                ++i;
                _argv[argc++] = &_buf[i];
                while (i < _size && _buf[i] != '"')
                    ++i;
                if (i >= _size) {
                    return 0;
                }
                _buf[i++] = '\0';
            } else {
                _argv[argc++] = &_buf[i];
                while (i < _size && !std::isspace(static_cast<unsigned char>(_buf[i])))
                    ++i;
                if (i < _size)
                    _buf[i++] = '\0';
            }
        }
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::reset_line()
    {
        _size     = 0;
        _overflow = false;
    }

    template <uint32_t BufferSize, int MaxArgCount>
    void HaChiMi<BufferSize, MaxArgCount>::dispatch(HaChiMi_Command cmd) const
    {
        if (cmd.argc <= 0)
            return;

        auto key = std::string_view{cmd.argv[0]};
        for (auto p = _services; p; p = p->_next)
            if (p->key == key) {
                --cmd.argc;
                ++cmd.argv;
                p->handle(cmd);
                return;
            }
    }

#pragma endregion

} // namespace oceah

#endif // OCEAH_HACHIMI

#ifndef EDGE_TRIGGER
#define EDGE_TRIGGER

#include "sys.h"

#ifdef HAL_EXTI_MODULE_ENABLED

#include "pin.h"
#include "isr.h"

#include <type_traits>
#include <utility>

namespace hal
{

    template <class F1, class F2 = void (*)()>
    class EdgeTrigger : public InterruptService
    {
    public:
        EdgeTrigger(const Pin &pin, F1 &&on_falling_edge) //
            noexcept(std::is_nothrow_move_constructible_v<F1>)
            : pin(pin),
              on_falling_edge(std::forward<F1>(on_falling_edge)),
              on_rising_edge(nullptr)
        {
        }

        EdgeTrigger(const Pin &pin, F1 &&on_falling_edge, F2 &&on_rising_edge) //
            noexcept(std::is_nothrow_move_constructible_v<F1> && std::is_nothrow_move_constructible_v<F2>)
            : pin(pin),
              on_falling_edge(std::forward<F1>(on_falling_edge)),
              on_rising_edge(std::forward<F2>(on_rising_edge))
        {
        }

        ~EdgeTrigger();

        void start(); // thread-safe
        void stop();  // thread-safe

        EdgeTrigger(const EdgeTrigger &)            = delete;
        EdgeTrigger &operator=(const EdgeTrigger &) = delete;

        EdgeTrigger(EdgeTrigger &&)            = delete;
        EdgeTrigger &operator=(EdgeTrigger &&) = delete;

    protected:
        void on_interrupt() //
            noexcept(noexcept(std::declval<F1 &>()()) && noexcept(std::declval<F2 &>()())) override
        {
            if (pin.read()) {
                call_rising_edge();
            } else {
                on_falling_edge();
            }
        }

    private:
        Pin pin;
        F1 on_falling_edge;
        F2 on_rising_edge;

        bool active = false;

        void call_rising_edge() //
            noexcept(noexcept(std::declval<F2 &>()()))
        {
            if constexpr (std::is_pointer_v<F2>) {
                if (on_rising_edge != nullptr)
                    on_rising_edge();
            } else {
                on_rising_edge();
            }
        }
    };

#pragma region detail

    ISR *edge_trigger_get_isr(uint16_t GPIO_Pin);

    template <class F1, class F2>
    EdgeTrigger<F1, F2>::~EdgeTrigger()
    {
        stop();
    }

    template <class F1, class F2>
    void EdgeTrigger<F1, F2>::start()
    {
        CriticalSectionGuard _;
        if (!active) {
            auto p = edge_trigger_get_isr(pin.pin);
            if (p) {
                active = true;
                p->register_task(this);
            }
        }
    }

    template <class F1, class F2>
    void EdgeTrigger<F1, F2>::stop()
    {
        CriticalSectionGuard _;
        if (active) {
            auto p = edge_trigger_get_isr(pin.pin);
            if (p) {
                p->unregister_task(this);
            }
            active = false;
        }
    }

#pragma endregion

} // namespace hal

#endif // HAL_EXTI_MODULE_ENABLED

#endif // EDGE_TRIGGER

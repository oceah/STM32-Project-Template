#ifndef SCHEDULED_TASK
#define SCHEDULED_TASK

#include "sys.h"

#ifdef HAL_TIM_MODULE_ENABLED

#include "isr.h"

#include "tim.h"

#include <atomic>
#include <type_traits>
#include <utility>

#define SCHEDULED_TASK_USE_TIM2
#define SCHEDULED_TASK_USE_TIM3

namespace hal
{

    // 时钟源
    struct ClockSource {
        TIM_HandleTypeDef *htim;
        uint32_t period;

        constexpr ClockSource();
        constexpr ClockSource(TIM_HandleTypeDef *htim, uint32_t period = 1);

        constexpr bool valid() const;
        constexpr ClockSource scale(uint32_t scaler) const;
    };

    // 定时任务
    template <class F>
    class ScheduledTask : public InterruptService
    {
    public:
        explicit ScheduledTask(const ClockSource &clk, const F &handler) //
            noexcept(std::is_nothrow_copy_constructible_v<F>)
            : clk(clk), handler(handler)
        {
        }

        explicit ScheduledTask(const ClockSource &clk, F &&handler) //
            noexcept(std::is_nothrow_move_constructible_v<F>)
            : clk(clk), handler(std::move(handler))
        {
        }

        ~ScheduledTask();

        void start(); // thread-safe
        void stop();  // thread-safe

        ScheduledTask(const ScheduledTask &)            = delete;
        ScheduledTask &operator=(const ScheduledTask &) = delete;

        ScheduledTask(ScheduledTask &&)            = delete;
        ScheduledTask &operator=(ScheduledTask &&) = delete;

    protected:
        void on_interrupt() //
            noexcept(noexcept(std::declval<F &>()())) override
        {
            ++val;
            if (val >= clk.period) {
                val = 0;
                handler();
            }
        }

    private:
        ClockSource clk;
        F handler;

        bool enabled = false;
        uint32_t val = 0;
    };

    class Clock
    {
    public:
        constexpr Clock(const ClockSource &clk);

        uint32_t now() const;

        void reset(); // thread-safe
        void start(); // thread-safe
        void stop();  // thread-safe

    private:
        std::atomic<uint32_t> tick;

        struct Handler {
            Clock *self;
            void operator()();
        };

        ScheduledTask<Handler> scheduled_task;

        void isr();
    };

#pragma region detail

    constexpr ClockSource::ClockSource()
        : htim(nullptr), period(0)
    {
    }

    constexpr ClockSource::ClockSource(TIM_HandleTypeDef *htim, uint32_t period)
        : htim(htim), period(period)
    {
    }

    constexpr bool ClockSource::valid() const
    {
        return htim != nullptr && period != 0;
    }

    constexpr ClockSource ClockSource::scale(uint32_t scaler) const
    {
        if (scaler == 0)
            return ClockSource();

        uint32_t v = period * scaler;
        if (v / scaler != period)
            return ClockSource();
        return ClockSource(htim, v);
    }

    ISR *ScheduledTask_GetISR(TIM_HandleTypeDef *htim);

    template <class F>
    ScheduledTask<F>::~ScheduledTask()
    {
        stop();
    }

    template <class F>
    void ScheduledTask<F>::start()
    {
        CriticalSectionGuard _;
        if (!enabled && clk.valid()) {
            auto p = ScheduledTask_GetISR(clk.htim);
            if (p) {
                val     = 0;
                enabled = true;
                p->register_task(this);
            }
        }
    }

    template <class F>
    void ScheduledTask<F>::stop()
    {
        CriticalSectionGuard _;
        if (enabled) {
            auto p = ScheduledTask_GetISR(clk.htim);
            if (p) {
                p->unregister_task(this);
                val = 0;
            }
            enabled = false;
        }
    }

    constexpr Clock::Clock(const ClockSource &clk)
        : tick(0), scheduled_task(clk, Handler(this))
    {
    }

#pragma endregion

} // namespace hal

#endif // HAL_TIM_MODULE_ENABLED

#endif // SCHEDULED_TASK

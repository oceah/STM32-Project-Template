#include "scheduled_task.h"

#ifdef HAL_TIM_MODULE_ENABLED

namespace hal
{

    namespace
    {

        class TimerISR : public ISR
        {
        public:
            void bind(TIM_HandleTypeDef *timer)
            {
                htim = timer;
            }

        protected:
            void start_interrupt() override
            {
                if (htim != nullptr)
                    HAL_TIM_Base_Start_IT(htim);
            }

            void stop_interrupt() override
            {
                if (htim != nullptr)
                    HAL_TIM_Base_Stop_IT(htim);
            }

        private:
            TIM_HandleTypeDef *htim = nullptr;
        };

#ifdef SCHEDULED_TASK_USE_TIM2
        TimerISR tim2_isr;
#endif
#ifdef SCHEDULED_TASK_USE_TIM3
        TimerISR tim3_isr;
#endif

    } // namespace

    extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
    {
        auto p = ScheduledTask_GetISR(htim);
        if (p != nullptr) {
            p->on_interrupt();
        }
    }

    ISR *ScheduledTask_GetISR(TIM_HandleTypeDef *htim)
    {
        if (htim == nullptr)
            return nullptr;
#ifdef SCHEDULED_TASK_USE_TIM2
        if (htim->Instance == TIM2) {
            tim2_isr.bind(htim);
            return &tim2_isr;
        }
#endif
#ifdef SCHEDULED_TASK_USE_TIM3
        if (htim->Instance == TIM3) {
            tim3_isr.bind(htim);
            return &tim3_isr;
        }
#endif
        return nullptr;
    }

#pragma region Clock

    uint32_t Clock::now() const
    {
        return tick.load(std::memory_order_relaxed);
    }

    void Clock::reset()
    {
        tick.store(0, std::memory_order_relaxed);
    }
    void Clock::start()
    {
        scheduled_task.start();
    }

    void Clock::stop()
    {
        scheduled_task.stop();
    }

    void Clock::Handler::operator()()
    {
        self->isr();
    }

    void Clock::isr()
    {
        tick.fetch_add(1, std::memory_order_relaxed);
    }

#pragma endregion

} // namespace hal

#endif // HAL_TIM_MODULE_ENABLED

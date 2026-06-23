#include "isr.h"

namespace hal
{

    CriticalSectionGuard::CriticalSectionGuard() noexcept
    {
        primask = __get_PRIMASK();
        __disable_irq();
    }

    CriticalSectionGuard::~CriticalSectionGuard() noexcept
    {
        __set_PRIMASK(primask);
    }

    void ISR::register_task(InterruptService *task)
    {
        if (task == nullptr)
            return;

        CriticalSectionGuard _;
        if (task->status != InterruptService::Status::Free)
            return;

        if (dispatch_lock_depth != 0) {
            defer_task_update(task, InterruptService::Status::PendingAdd);
            return;
        }

        const bool empty = task_head == nullptr;
        link_task(task);
        task->status = InterruptService::Status::Linked;

        if (empty)
            start_interrupt();
    }

    void ISR::unregister_task(InterruptService *task)
    {
        if (task == nullptr)
            return;

        CriticalSectionGuard _;
        if (task->status != InterruptService::Status::Linked)
            return;

        if (dispatch_lock_depth != 0) {
            defer_task_update(task, InterruptService::Status::PendingDel);
            return;
        }

        unlink_task(task);
        task->status = InterruptService::Status::Free;

        if (task_head == nullptr)
            stop_interrupt();
    }

    void ISR::on_interrupt()
    {
        {
            CriticalSectionGuard _;
            ++dispatch_lock_depth;
        }

        if (task_head != nullptr) {
            auto *task = task_head;
            do {
                task->on_interrupt();
                task = task->next;
            } while (task != task_head);
        }

        bool should_process_deferred = false;
        {
            CriticalSectionGuard _;
            if (dispatch_lock_depth != 0)
                --dispatch_lock_depth;
            if (dispatch_lock_depth == 0 && deferred_head != nullptr) {
                ++dispatch_lock_depth;
                should_process_deferred = true;
            }
        }

        if (should_process_deferred)
            process_deferred_tasks();
    }

    void ISR::start_interrupt()
    {
    }

    void ISR::stop_interrupt()
    {
    }

    void ISR::link_task(InterruptService *task)
    {
        if (task_head == nullptr) {
            task->prev = task;
            task->next = task;
            task_head  = task;
            return;
        }

        auto *tail      = task_head->prev;
        task->prev      = tail;
        task->next      = task_head;
        tail->next      = task;
        task_head->prev = task;
    }

    void ISR::unlink_task(InterruptService *task)
    {
        if (task->next == task) {
            task_head = nullptr;
        } else {
            auto *prev = task->prev;
            auto *next = task->next;
            prev->next = next;
            next->prev = prev;

            if (task_head == task)
                task_head = next;
        }

        task->prev = nullptr;
        task->next = nullptr;
    }

    void ISR::defer_task_update(InterruptService *task, InterruptService::Status status)
    {
        task->deferred_next = deferred_head;
        deferred_head       = task;
        task->status        = status;
    }

    void ISR::process_deferred_tasks()
    {
        while (true) {
            ControlAction action = ControlAction::None;

            {
                CriticalSectionGuard _;

                if (deferred_head == nullptr) {
                    if (dispatch_lock_depth != 0)
                        --dispatch_lock_depth;
                    break;
                }

                action = process_one_deferred_task();
            }

            switch (action) {
                case ControlAction::Start:
                    start_interrupt();
                    break;
                case ControlAction::Stop:
                    stop_interrupt();
                    break;
                case ControlAction::None:
                    break;
            }
        }
    }

    ISR::ControlAction ISR::process_one_deferred_task()
    {
        auto *task = deferred_head;
        if (task == nullptr)
            return ControlAction::None;

        deferred_head       = task->deferred_next;
        task->deferred_next = nullptr;

        const auto status    = task->status;
        const bool was_empty = task_head == nullptr;

        switch (status) {
            case InterruptService::Status::PendingAdd:
                link_task(task);
                task->status = InterruptService::Status::Linked;
                return was_empty ? ControlAction::Start : ControlAction::None;

            case InterruptService::Status::PendingDel:
                unlink_task(task);
                task->status = InterruptService::Status::Free;
                return task_head == nullptr ? ControlAction::Stop : ControlAction::None;

            default:
                return ControlAction::None;
        }
    }

}

#ifndef ISR_H
#define ISR_H

#include "sys.h"

namespace hal
{

    // 临界区保护
    class CriticalSectionGuard
    {
    public:
        CriticalSectionGuard() noexcept;
        ~CriticalSectionGuard() noexcept;

        CriticalSectionGuard(const CriticalSectionGuard &)            = delete;
        CriticalSectionGuard &operator=(const CriticalSectionGuard &) = delete;

        CriticalSectionGuard(CriticalSectionGuard &&)            = delete;
        CriticalSectionGuard &operator=(CriticalSectionGuard &&) = delete;

    private:
        uint32_t primask;
    };

    // 侵入式中断服务节点
    class InterruptService
    {
    public:
        virtual ~InterruptService() = default;

    protected:
        virtual void on_interrupt() = 0;

    private:
        friend class ISR;

        enum class Status : uint8_t {
            Free,
            Linked,
            PendingAdd,
            PendingDel,
        };

        Status status = Status::Free;

        InterruptService *prev = nullptr;
        InterruptService *next = nullptr;

        InterruptService *deferred_next = nullptr;
    };

    // 中断服务派发器
    class ISR
    {
    public:
        void register_task(InterruptService *task);   // 注册中断服务
        void unregister_task(InterruptService *task); // 注销中断服务

        void on_interrupt(); // 分发中断事件

    protected:
        virtual void start_interrupt(); // 打开外设中断
        virtual void stop_interrupt();  // 关闭外设中断

    private:
        enum class ControlAction : uint8_t {
            None,
            Start,
            Stop,
        };

        InterruptService *task_head     = nullptr;
        InterruptService *deferred_head = nullptr;

        uint32_t dispatch_lock_depth = 0;

        void link_task(InterruptService *task);
        void unlink_task(InterruptService *task);
        void defer_task_update(InterruptService *task, InterruptService::Status status);
        void process_deferred_tasks();
        ControlAction process_one_deferred_task();
    };

} // namespace hal

#endif // ISR_H

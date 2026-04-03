#ifndef TASK_HANDLER_H_
#define TASK_HANDLER_H_

#include <Arduino.h>

#include <stdint.h>
#include <functional>
#include <memory>

#include <logging/LogHandler.h>
#include <logging/TagHandler.h>

namespace TaskHandler
{

    class Task;

    // Context is encoded in the task class, can be cast to appropriate structure
    using Task_t = std::function<int32_t(Task *)>;

    // Rates are in microseconds
    enum Rates : int32_t
    {
        ONDEMAND = 0,
        MAX = 100,
        FAST = 1000,
        SLOW = 25000,
    };

    // A task wraps a periodic task and assigns it a tick rate (software timer)
    class Task
    {
    private:
        Rates _tick_rate;
        long long _last_tick;
        long long _sleep_target;

    public:
        Task(Rates rate) : _tick_rate(rate)
        {
            _last_tick = 0;
        }

        Task(const Task &rhs)
        {
            _tick_rate = rhs._tick_rate;
            _last_tick = rhs._last_tick;
        }

        // Perform setup, grab peripherals, etc
        virtual void initialize()
        {
            _last_tick = micros();
            this->setup();
        }
        // Invoke update routine (handle_tick)
        void handle_tick()
        {
            long long us = micros();
            long long delta = abs(us - _last_tick);
            if (_sleep_target > 0)
            {
                _sleep_target -= delta;
                return;
            }
            if (delta >= _tick_rate)
            {
                _last_tick = micros();
                loop();
            }
        }

        void sleep(long long ms)
        {
            _sleep_target = ms * 1000;
        }

        void wait(long long ms)
        {
            vTaskDelay(ms / portTICK_PERIOD_MS);
        }

        virtual void setup() = 0;
        virtual void loop() = 0;
    };

    // Task wraps a simple std::function
    class FunctionalTask : public Task
    {
    private:
        Task_t _handler;

    public:
        FunctionalTask(Task_t handler) : Task(Rates::SLOW), _handler(handler) {}

        void setup()
        {
            // No setup needed
        }
        void loop() override
        {
            if (_handler)
            {
                _handler(this);
            }
        }
    };

    enum Core
    {
        PRO_CPU = PRO_CPU_NUM,
        APP_CPU = APP_CPU_NUM,
    };

    enum Priority
    {
        IDLE = 0,
        REALTIME = 0,
        PRIORITY = 1,
        LAZY = 2,
    };

    // Per-core (per-thread) executor/task queue
    class TaskExecutor
    {
    private:
        static constexpr uint32_t STACK_SIZE = configMINIMAL_STACK_SIZE * 8;
        static constexpr uint32_t INITIAL_CAPACITY = 8;
        std::vector<std::unique_ptr<Task>> _tasks;
        TaskHandle_t _thread_handle;
        const std::string _name;
        const uint32_t _priority;
        const uint32_t _core;

    public:
        TaskExecutor(std::string name, uint32_t priority = IDLE, uint32_t core = APP_CPU) : _name(name), _priority(priority), _core(core)
        {
            _tasks.reserve(INITIAL_CAPACITY);
        }

        TaskExecutor(const char *name, uint32_t priority = IDLE, uint32_t core = APP_CPU) : _name(name), _priority(priority), _core(core)
        {
            _tasks.reserve(INITIAL_CAPACITY);
        }

        // Add a task to the list
        void registerTask(std::unique_ptr<Task> task)
        {
            if (task)
            {
                _tasks.push_back(std::move(task));
            }
        }

        // Run setup code, acquire hardware locks, etc
        void initialize()
        {
            for (auto &&task : _tasks)
            {
                task->initialize();
            }
        }

        // Start the execution thread (TODO: Do we need this? Can we use a lambda?)
        void start()
        {
            auto status = xTaskCreatePinnedToCore(
                [](void *context)
                {
                    TaskHandler::TaskExecutor *manager = static_cast<TaskHandler::TaskExecutor *>(context);
                    if (manager)
                    {
                        manager->run();
                    }
                },
                _name.c_str(),
                STACK_SIZE,
                reinterpret_cast<void *>(this),
                tskIDLE_PRIORITY + _priority,
                &_thread_handle,
                _core);

            if (status != pdPASS)
            {
                LogHandler::error(_name.c_str(), "Could not start task.");
            }
        }

        void run()
        {
            for (;;)
            {
                for (auto &&task : _tasks)
                {
                    task->handle_tick();
                }
                taskYIELD();
            }
        }
    };

    class TaskManager
    {
    private:
        TaskExecutor realtimeThread;
        TaskExecutor priorityThread;
        TaskExecutor lazyThread;

    public:
        TaskManager() : realtimeThread("realtime", Priority::REALTIME, Core::PRO_CPU),
                        priorityThread("priority", Priority::PRIORITY, Core::APP_CPU),
                        lazyThread("lazy", Priority::LAZY, Core::APP_CPU) {}

        TaskExecutor *realtimeTasks() { return &realtimeThread; }
        TaskExecutor *priorityTasks() { return &priorityThread; }
        TaskExecutor *lazyTasks() { return &lazyThread; }

        void start()
        {
            realtimeThread.start();
            priorityThread.start();
            lazyThread.start();
        }

        void realtime(Task *t, Rates rate = Rates::FAST)
        {
            realtimeThread.registerTask(std::make_unique<Task>(t));
        }

        void priority(Task *t, Rates rate = Rates::FAST)
        {
            priorityThread.registerTask(std::make_unique<Task>(t));
        }

        void lazy(Task *t, Rates rate = Rates::FAST)
        {
            lazyThread.registerTask(std::make_unique<Task>(t));
        }
    };

    // Global instance
    static TaskManager taskManager;
};

#endif // TASK_HANDLER_H_
#ifndef TASK_HANDLER_H_
#define TASK_HANDLER_H_

#include <Arduino.h>

#include <stdint.h>
#include <vector>

#include "logging/LogHandler.h"
#include "logging/TagHandler.h"

namespace TaskHandler
{

    // Rates are in microseconds
    enum class Rates : int32_t
    {
        ONDEMAND = 0,
        MAX = 10,
        FAST = 1000,
        SLOW = 10000,
    };

    // Lightweight cooperative task base class.
    // All tasks are polled from a single loop — no FreeRTOS tasks are created here.
    class Task
    {
    private:
        Rates _tickRate;
        uint64_t _lastTickUs = 0;
        int64_t _sleepRemainingUs = 0;
        bool _initialized = false;

    public:
        explicit Task(Rates rate) : _tickRate(rate) {}
        virtual ~Task() = default;

        void initialize()
        {
            if (_initialized)
                return;
            _initialized = true;
            _lastTickUs = micros();
            setup();
        }

        void handle_tick()
        {
            if (!_initialized)
                return;
            if (_tickRate == Rates::ONDEMAND)
                return;

            const uint64_t nowUs = micros();
            const uint64_t deltaUs = nowUs - _lastTickUs;

            if (_sleepRemainingUs > 0)
            {
                _sleepRemainingUs -= static_cast<int64_t>(deltaUs);
                _lastTickUs = nowUs;
                return;
            }

            if (deltaUs >= static_cast<uint64_t>(_tickRate))
            {
                _lastTickUs = nowUs;
                loop();
            }
        }

        void sleep(long long ms)
        {
            _sleepRemainingUs = ms * 1000;
        }

        void wait(long long ms)
        {
            vTaskDelay(ms / portTICK_PERIOD_MS);
        }

        virtual void setup() = 0;
        virtual void loop() = 0;
    };

    // Cooperative task list — call poll() from a single FreeRTOS task or loop().
    class TaskList
    {
    private:
        std::vector<Task *> _tasks;

    public:
        void add(Task *task)
        {
            if (task)
                _tasks.push_back(task);
        }

        // Initialize any tasks that haven't been setup yet, then tick all.
        void poll()
        {
            for (Task *t : _tasks)
            {
                t->initialize();
                t->handle_tick();
            }
        }
    };

    // Backward-compatible Manager API — now just holds a single TaskList that
    // is polled cooperatively from Arduino loop() (Core 1 / APP_CPU). The
    // motor-control task lives in its own pinned FreeRTOS task on Core 0
    // (PRO_CPU); see motorTaskFunc in main.cpp. There is no separate Core 0
    // cooperative list — anything time-critical should be inside the motor
    // task, not the cooperative poll loop.
    class TaskManager
    {
    private:
        inline static TaskManager *singleton = nullptr;

        TaskList _tasks; // All cooperatively-polled tasks (Core 1 / loop()).

        TaskManager() = default;

    public:
        static TaskManager &global()
        {
            if (!singleton)
                singleton = new TaskManager();
            return *singleton;
        }

        // Register a cooperatively-polled task. All registration aliases route
        // here — the legacy critical/priority/auxiliary distinction is gone
        // because every cooperative task runs on the same core / poll loop.
        void add(Task* task) { _tasks.add(task); }
        void priority(Task* task) { _tasks.add(task); }
        void auxiliary(Task* task) { _tasks.add(task); }
        void critical(Task* task) { _tasks.add(task); }
        void realtime(Task* task) { _tasks.add(task); }
        void lazy(Task* task) { _tasks.add(task); }

        TaskList& tasks() { return _tasks; }

        void start() {}  // No-op: caller polls directly
        void update() {} // No-op
    };

    using Manager = TaskManager;

    inline TaskManager &global()
    {
        return TaskManager::global();
    }

} // namespace TaskHandler

namespace Tasks
{
    using Rates = TaskHandler::Rates;
}

#endif // TASK_HANDLER_H_
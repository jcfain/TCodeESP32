#ifndef TASK_HANDLER_H_
#define TASK_HANDLER_H_

#include <Arduino.h>

#include <stdint.h>
#include <functional>
#include <vector>
#include <string>

#include "logging/LogHandler.h"
#include "logging/TagHandler.h"

namespace TaskHandler
{
    class Task;
    using Task_t = std::function<int32_t(Task*)>;

    // Rates are in microseconds
    enum class Rates : int32_t
    {
        ONDEMAND = 0,
        MAX = 10,
        FAST = 1000,
        SLOW = 10000,
    };

    enum class Core : uint8_t
    {
    PRO_CPU = PRO_CPU_NUM,
    APP_CPU = APP_CPU_NUM,
};

enum class Priority : uint8_t
{
    CRITICAL = 4,
    PRIORITY = 3,
    AUXILIARY = 1,
};

// A task wraps a periodic routine and assigns it a tick rate.
class Task
{
private:
    Rates _tickRate;
    uint64_t _lastTickUs = 0;
    int64_t _sleepRemainingUs = 0;

public:
    explicit Task(Rates rate) : _tickRate(rate) {}
    virtual ~Task() = default;

    virtual void initialize()
    {
        _lastTickUs = micros();
        setup();
    }

    void handle_tick()
    {
        if (_tickRate == Rates::ONDEMAND)
        {
            return;
        }

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

class FunctionalTask : public Task
{
private:
    Task_t _handler;

public:
    explicit FunctionalTask(Task_t handler) : Task(Rates::SLOW), _handler(std::move(handler)) {}

    void setup() override {}

    void loop() override
    {
        if (_handler)
        {
            _handler(this);
        }
    }
};

class TaskExecutor
{
private:
    static constexpr uint32_t STACK_SIZE = configMINIMAL_STACK_SIZE * 8;
    static constexpr uint32_t INITIAL_CAPACITY = 8;

    std::vector<Task*> _tasks;
    size_t _initializedTaskCount = 0;
    TaskHandle_t _threadHandle = nullptr;
    const std::string _name;
    const uint32_t _priority;
    const BaseType_t _core;
    SemaphoreHandle_t _tasksMutex;
    bool _started = false;

    static void threadMain(void* context)
    {
        TaskExecutor* executor = static_cast<TaskExecutor*>(context);
        if (executor)
        {
            executor->run();
        }
        vTaskDelete(nullptr);
    }

public:
    TaskExecutor(const char* name, Priority priority, Core core)
        : _name(name), _priority(static_cast<uint32_t>(priority)), _core(static_cast<BaseType_t>(core))
    {
        _tasks.reserve(INITIAL_CAPACITY);
        _tasksMutex = xSemaphoreCreateMutex();
    }

    void registerTask(Task* task)
    {
        if (task)
        {
            xSemaphoreTake(_tasksMutex, portMAX_DELAY);
            _tasks.push_back(task);
            xSemaphoreGive(_tasksMutex);
        }
    }

    void start()
    {
        if (_started)
        {
            return;
        }

        _started = true;

        const BaseType_t status = xTaskCreatePinnedToCore(
            threadMain,
            _name.c_str(),
            STACK_SIZE,
            this,
            tskIDLE_PRIORITY + _priority,
            &_threadHandle,
            _core);

        if (status != pdPASS)
        {
            LogHandler::error(Tags::Tasks, "Could not start task executor: %s", _name.c_str());
            _started = false;
            return;
        }
    }

    void run()
    {
        // Persistent list of tasks that have completed setup() and are safe to tick.
        // Avoids per-iteration heap allocation from rebuild and guarantees
        // that handle_tick() / loop() is never called before setup() has returned.
        std::vector<Task*> readyTasks;
        readyTasks.reserve(INITIAL_CAPACITY);

        for (;;)
        {
            Task* taskToInitialize = nullptr;

            xSemaphoreTake(_tasksMutex, portMAX_DELAY);
            if (_initializedTaskCount < _tasks.size())
            {
                taskToInitialize = _tasks[_initializedTaskCount++];
            }
            xSemaphoreGive(_tasksMutex);

            // Run setup() on the executor thread; only add to readyTasks once done.
            if (taskToInitialize)
            {
                taskToInitialize->initialize();
                readyTasks.push_back(taskToInitialize);
            }

            for (Task* task : readyTasks)
            {
                task->handle_tick();
            }

            // Yield for one tick so the executor does not spin at 100 % CPU,
            // which caused heap contention and potential corruption.
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
};

class TaskManager
{
private:
    inline static TaskManager* singleton = nullptr;

    TaskExecutor _criticalThread;
    TaskExecutor _priorityThread;
    TaskExecutor _auxiliaryThread;

    TaskManager()
        : _criticalThread("critical", Priority::CRITICAL, Core::PRO_CPU),
        _priorityThread("priority", Priority::PRIORITY, Core::APP_CPU),
        _auxiliaryThread("aux", Priority::AUXILIARY, Core::APP_CPU)
    {
    }

public:
    static TaskManager& global()
    {
        if (!singleton)
        {
            singleton = new TaskManager();
        }
        return *singleton;
    }

    TaskExecutor* criticalTasks() { return &_criticalThread; }
    TaskExecutor* priorityTasks() { return &_priorityThread; }
    TaskExecutor* auxiliaryTasks() { return &_auxiliaryThread; }

    void start()
    {
        _criticalThread.start();
        _priorityThread.start();
        _auxiliaryThread.start();
    }

    void critical(Task* task)
    {
        _criticalThread.registerTask(task);
    }

    void priority(Task* task)
    {
        _priorityThread.registerTask(task);
    }

    void auxiliary(Task* task)
    {
        _auxiliaryThread.registerTask(task);
    }

    // Backward-compatible aliases
    void realtime(Task* task) { critical(task); }
    void lazy(Task* task) { auxiliary(task); }

    void update()
    {
        // No-op: executors run on dedicated FreeRTOS threads.
    }
};

using Manager = TaskManager;

inline TaskManager& global()
{
    return TaskManager::global();
}
}

namespace Tasks
{
    using Rates = TaskHandler::Rates;
}

#endif // TASK_HANDLER_H_
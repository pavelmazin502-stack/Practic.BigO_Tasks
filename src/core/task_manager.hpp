#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <string>
#include <cstddef>

#include "core/task.hpp"
#include "ds/binary_heap.hpp"
#include "ds/dynamic_array.hpp"
#include "ds/hash_table.hpp"

// Главная программа, которая связывает все структуры в единую систему

struct TaskStats { // Простая структура для метода get_stats()
    size_t total = 0; // Общее количество задач

    // Количество задач в каждом состоянии
    size_t queued = 0;
    size_t running = 0;
    size_t completed = 0;
    size_t failed = 0;
    size_t cancelled = 0;
    size_t queue_size = 0;
    bool has_max_queued_priority = false;
    int max_queued_priority = 0;
};

class TaskManager {
private:
    uint64_t next_id_num = 1;
    // Структура, которая хранит приоритет и индекс задачи в массиве таскс (чтобы не копировать в очередь объект Task)
    struct QueueItem {
        int priority = 0; 
        size_t index = 0;

        bool operator<(const QueueItem& other) const { // Оператор сравнения (нужен для того, чтобы компьютер понимал как сравнивать наши структуры)
            if (priority != other.priority) {
                return priority < other.priority;
            }
            return index > other.index; // При равном приоритете — кто раньше пришёл
        }
    };

    // Структуры данных
    DynamicArray<Task> tasks; // В Динамическом массиве лежат все задачи
    HashTable<string, size_t> id_to_index; // Хранит пары ID задачи - ее индекс в массиве tasks
    BinaryHeap<QueueItem> ready_queue; // Очередь приоритетов

    static uint64_t current_time(){ // Функция получения текущего времени (не использует поля класса)
        return static_cast<uint64_t>(time(nullptr)); // Явное приведение типа к uint64_t
    }

    // Функция поиска задачи по id и получение доступа
    Task& find_task_mut(const string& id) {
        if (id_to_index.contains(id) == false) { // Если такого ключа нет в таблице
            throw runtime_error("Task not found: " + id);
        }
        size_t idx = id_to_index.get(id);
        return tasks[idx]; // Возвращаем ссылку на реальный объект в дин массиве
    }

public:
    // Функция генерации айди
    string generate_id() {
        while (true) {
            string candidate = "task-" + to_string(next_id_num++);
            if (!id_to_index.contains(candidate)) {
                return candidate;
            }
        }
    }
    // Создание новой задачи: кладём сразу во все 3 структуры
    const Task& add_task(const string& id, const string& type,int priority,const string& payload) {

        if (id.empty()) {
            throw invalid_argument("Task ID cannot be empty");
        }

        if (priority < 0 || priority > 100) { 
            throw invalid_argument("Priority must be between 0 and 100");
        }

        if (id_to_index.contains(id)) { // Пустой id запрещен
            throw runtime_error("Task already exists: " + id);
        }

        uint64_t now = current_time();
        Task task; // Временная структура далее заполняем
        task.id = id;
        task.type = type;
        task.priority = priority;
        task.payload = payload;
        task.status = TaskStatus::Queued;
        task.created_at = now;
        task.updated_at = now;

        size_t new_index = tasks.get_size(); // Индекс, под которым задача ляжет в конец
        tasks.push_back(task); // Кладем задачу в конец массива
        id_to_index.put(id, new_index); // Региструруем в хеш-таблице где она лежит
        ready_queue.push(QueueItem{priority, new_index}); // Создаем структуру с нужным приоритетом и индексом и кидаем в кучу 

        return tasks[new_index];
    }

    // Загрузка уже существующей задачи из файла (сохраняет старые статус и время)
    void load_task(const Task& task) {
        if (task.id.empty()) {
            throw invalid_argument("Task ID in storage cannot be empty");
        }

        if (task.priority < 0 || task.priority > 100) { // в load_task: task.priority
            throw invalid_argument("Priority must be between 0 and 100");
        }

        if (id_to_index.contains(task.id)) {
            throw runtime_error("Duplicate task ID in storage: " + task.id);
        }

        size_t new_index = tasks.get_size();// Узнаем текущий размер массива
        tasks.push_back(task); // Добавляем задачу в конец массива
        id_to_index.put(task.id, new_index);

        // В очередь приоритетов кладём только те, что ещё ждут запуска
        if (task.status == TaskStatus::Queued) {
            ready_queue.push(QueueItem{task.priority, new_index});
        }
    }

    // Получить задачу только для чтения
    const Task& get_task(const string& id) const {
        if (!id_to_index.contains(id)) {
            throw runtime_error("Task not found: " + id);
        }
        size_t idx = id_to_index.get(id);
        return tasks[idx]; // отдаем ссылку на задачцу из массива
    }

    // Запустить самую приоритетную задачу (queued -> running)
    const Task& start_next() {
        while (ready_queue.is_empty() == false) {
            QueueItem item = ready_queue.top();
            ready_queue.pop();

            Task& task = tasks[item.index];

            // Если задача запущена - пропускаем
            if (task.status != TaskStatus::Queued) {
                continue;
            }

            task.status = TaskStatus::Running;
            task.updated_at = current_time();
            return task;
        }
        throw runtime_error("No queued tasks available");
    }

    // Завершить успешно (running -> completed)
    const Task& complete_task(const string& id) {
        Task& task = find_task_mut(id);
        if (transition(task.status, TaskStatus::Completed) == false) {
            throw runtime_error("Cannot complete task in status " + status_terminal(task.status) + ": " + id);
        }

        task.status = TaskStatus::Completed; // Меняем статус на финальный
        task.updated_at = current_time(); // Обновляем временную метку последнего изменения
        return task;
    }

    // Завершить с ошибкой
    const Task& fail_task(const string& id, const string& error_message) {
        Task& task = find_task_mut(id);
        if (transition(task.status, TaskStatus::Failed) == false) {
            throw runtime_error("Cannot fail task in status " + status_terminal(task.status) + ": " + id);
        }

        task.status = TaskStatus::Failed;
        task.error = error_message;
        task.updated_at = current_time();
        return task;
    }

    // Отменить задачу
    const Task& cancel_task(const string& id) {
        Task& task = find_task_mut(id);
        if (transition(task.status, TaskStatus::Cancelled) == false) {
            throw runtime_error("Cannot cancel task in status " +
                                     status_terminal(task.status) + ": " + id);
        }

        // Из кучи не удаляем — start_next() сам пропустит её позже
        task.status = TaskStatus::Cancelled;
        task.updated_at = current_time();
        return task;
    }

    // Все задачи (для команды list и записи в файл)
    const DynamicArray<Task>& get_all_tasks() const {
        return tasks;
    }

    // Подсчёт статистики по всем статусам
    TaskStats get_stats() const {
        TaskStats stats;
        stats.total = tasks.get_size();

        for (size_t i = 0; i < tasks.get_size(); ++i) {
            switch (tasks[i].status) {
                case TaskStatus::Queued:    
                    ++stats.queued; 
                    ++stats.queue_size;
                    if (!stats.has_max_queued_priority || tasks[i].priority > stats.max_queued_priority) {
                        stats.max_queued_priority = tasks[i].priority;
                        stats.has_max_queued_priority = true;
                    }
                    break;
                case TaskStatus::Running:   
                    ++stats.running; 
                    break;
                case TaskStatus::Completed:
                    ++stats.completed; 
                    break;
                case TaskStatus::Failed:
                    ++stats.failed; 
                    break;
                case TaskStatus::Cancelled: 
                    ++stats.cancelled; 
                    break;
            }
        }
        return stats;
    }
    size_t get_index_resize_count() const { 
        return id_to_index.get_resize_count(); 
    }
    double get_index_max_load_factor() const { 
        return id_to_index.get_max_load_factor();
    }
};

#endif
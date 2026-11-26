#ifndef TASK_H
#define TASK_H

#include <cstdint>
#include <string>
#include <stdexcept>

using namespace std;

// Модель данных

enum class TaskStatus{
    Queued,
    Running,
    Completed,
    Failed,
    Cancelled
};

struct Task{
    string id;
    string type;
    int priority = 0;
    string payload;
    TaskStatus status = TaskStatus::Queued;
    string error; // Текст ошибки сущ. если статус Failed
    uint64_t created_at = 0;
    uint64_t updated_at = 0;
};

// Разрешение для программы держать одну и ту же функцию/переменную сразу в нескольких файлах
inline string status_terminal(TaskStatus status){
    switch (status){
        case TaskStatus::Queued:
            return "queued";
        case TaskStatus::Running:
            return "running";
        case TaskStatus::Completed:
            return "completed";
        case TaskStatus::Failed:
            return "failed";
        case TaskStatus::Cancelled:
            return "cancelled";
    }
    return "unknown";
}
// Переходы
inline bool transition(TaskStatus from, TaskStatus to) {
    if (from == TaskStatus::Queued) {
        return to == TaskStatus::Running || to == TaskStatus::Cancelled;
    }
    if (from == TaskStatus::Running) {
        return to == TaskStatus::Completed || to == TaskStatus::Failed;
    }

    return false;
}

inline TaskStatus terminalinstatus(const std::string& str) {
    if (str == "queued"){
        return TaskStatus::Queued;
    }   
    if (str == "running"){
        return TaskStatus::Running;
    }   
    if (str == "completed"){
        return TaskStatus::Completed;
    } 
    if (str == "failed"){
        return TaskStatus::Failed;
    }    
    if (str == "cancelled"){
        return TaskStatus::Cancelled;
    } 
    throw std::invalid_argument("Unknown task status: " + str);
}

#endif
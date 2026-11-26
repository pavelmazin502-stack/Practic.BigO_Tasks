// Файл для реализации CLI интерфейса Память - Терминал
// Строки в Json формате оборачиваются в ""
#ifndef CLI_H
#define CLI_H

#include <iostream>
#include <stdexcept>
#include <string>

#include "../core/task_manager.hpp"
#include "../storage/file_storage.hpp"

// Функия, которая проверяет наличие "" при выводе текста в консоль
inline string json(const string& text) {
    string result;
    for (char c : text) {
        if (c == '\\'){
            result += "\\\\";
        } else if (c == '"'){
            result += "\\\"";
        } else if (c == '\t'){
            result += "\\t";
        } else if (c == '\n'){
            result += "\\n";
        } else if (c == '\r'){
            result += "\\r";
        } else { 
            result.push_back(c);
        }
    }
    return result;
}

// Сборка задачи в формат
inline string task_json(const Task& t) {
    return "{\"id\":\"" + json(t.id) + "\",\"type\":\"" + json(t.type) +
           "\",\"priority\":" + to_string(t.priority) + ",\"payload\":\"" + json(t.payload) +
           "\",\"status\":\"" + status_terminal(t.status) + "\",\"error\":\"" + json(t.error) +
           "\",\"created_at\":" + to_string(t.created_at) + ",\"updated_at\":" + to_string(t.updated_at) + "}";
}

// Если пользователь передал формат json - true 
inline void print_task(const Task& t, bool as_json) {
    if (as_json == false) {
        std::cout << "ID: " << t.id << " | type: " << t.type << " | priority: " << t.priority
                  << " | status: " << status_terminal(t.status) << " | payload: " << t.payload;
        if (!t.error.empty()) { // Поле показываем только тогда когда задача упала
            std::cout << " | error: " << t.error;
        }
        std::cout << " | created_at: " << t.created_at << " | updated_at: " << t.updated_at << '\n';
    } else {
        cout << task_json(t) << '\n';
    }
}

inline void print_tasks(const DynamicArray<Task>& tasks, bool as_json) {
    if (as_json == false) { // Вывод обычным текстом
        if (tasks.get_size() == 0) {
            cout << "No tasks\n";
            return;
        }
        for (size_t i = 0; i < tasks.get_size(); ++i) {
            print_task(tasks[i], false);
        }
    } else { 
        cout << "[";
        for (size_t i = 0; i < tasks.get_size(); ++i) {
            if (i > 0){
                cout << ","; // Запятая ставится перед каждым элементом кроме первого 
            }
            cout << task_json(tasks[i]);
        }
        cout << "]\n";
    }
}

// Печать статистики

inline void print_stats(const TaskStats& s, bool as_json) {
    if (as_json == false) {
        cout << "total: " << s.total << '\n'
             << "queued: " << s.queued << '\n'
             << "running: " << s.running << '\n'
             << "completed: " << s.completed << '\n'
             << "failed: " << s.failed << '\n'
             << "cancelled: " << s.cancelled << '\n'
             << "queue_size: " << s.queue_size << '\n'
             << "max_queued_priority: "
             << (s.has_max_queued_priority ? to_string(s.max_queued_priority) : "none") << '\n';
    } else {
        cout << "{\"total\":" << s.total
             << ",\"queued\":" << s.queued
             << ",\"running\":" << s.running
             << ",\"completed\":" << s.completed
             << ",\"failed\":" << s.failed
             << ",\"cancelled\":" << s.cancelled
             << ",\"queue_size\":" << s.queue_size
             << ",\"max_queued_priority\":"
             << (s.has_max_queued_priority ? to_string(s.max_queued_priority) : "null") << "}\n";
    }
}

// Защита от некорректного ввода (например не введен путь до файла и тп)
inline string next_arg(int argc, char* argv[], int& i, const string& flag) {
    if (i + 1 >= argc) {
        throw runtime_error("Missing value for " + flag);
    }
    return argv[++i];
}

inline void run_cli(int argc, char* argv[]) {
    if (argc < 2) {
        throw runtime_error("Usage: taskctl --data <path> [--format text|json] <command> [args]");
    }

    string data_path;
    bool as_json = false;
    int i = 1;

    while (i < argc) {
        string arg = argv[i];
        if (arg == "--data") {
            data_path = next_arg(argc, argv, i, "--data");
            ++i;
        } else if (arg == "--format") {
            string fmt = next_arg(argc, argv, i, "--format");
            if (fmt == "json") {
                as_json = true;
            } else if (fmt == "text") {
                as_json = false;
            } else {
                throw runtime_error("Unknown format: " + fmt);
            }
            ++i;
        } else {
            break;
        }
    }

    if (data_path.empty()) {
        throw runtime_error("Missing required flag: --data <path>");
    }
    if (i >= argc) {
        throw runtime_error("Missing command");
    }

    string cmd = argv[i++];

    TaskManager manager;
    load_file(data_path, manager);

    if (cmd == "create") {
        string id;
        string type;
        string payload;
        int priority = 0;
        bool has_id = false;
        bool has_type = false;
        bool has_priority = false;
        bool has_payload = false;

        while (i < argc) {
            string opt = argv[i];
            if (opt == "--id") {
                id = next_arg(argc, argv, i, "--id");
                has_id = true;
            } else if (opt == "--type") {
                type = next_arg(argc, argv, i, "--type");
                has_type = true;
            } else if (opt == "--priority") {
                priority = parse_int(next_arg(argc, argv, i, "--priority"));
                has_priority = true;
            } else if (opt == "--payload") {
                payload = next_arg(argc, argv, i, "--payload");
                has_payload = true;
            } else {
                throw runtime_error("Unknown option for create: " + opt);
            }
            ++i;
        }

        if (!has_id || !has_type || !has_priority || !has_payload) {
            throw runtime_error("Usage: create --id <id> --type <type> --priority <number> --payload <text>");
        }

        if (!has_id) {
            id = manager.generate_id();
        }

        Task t = manager.add_task(id, type, priority, payload);
        save_file(data_path, manager);
        print_task(t, as_json);

    } else if (cmd == "get") {
        if (i + 1 != argc) {
            throw runtime_error("Usage: get <id>");
        }
        print_task(manager.get_task(argv[i]), as_json);

    } else if (cmd == "list") {
        if (i == argc) {
            print_tasks(manager.get_all_tasks(), as_json);
        } else if (string(argv[i]) == "--status" && i + 2 == argc) {
            TaskStatus need = terminalinstatus(argv[i + 1]);
            const DynamicArray<Task>& all = manager.get_all_tasks();
            DynamicArray<Task> filtered;
            for (size_t k = 0; k < all.get_size(); ++k) {
                if (all[k].status == need) {
                    filtered.push_back(all[k]);
                }
            }
            print_tasks(filtered, as_json);
        } else {
            throw runtime_error("Usage: list [--status <status>]");
        }

    } else if (cmd == "start-next") {
        if (i != argc) {
            throw runtime_error("Usage: start-next");
        }
        Task t = manager.start_next();
        save_file(data_path, manager);
        print_task(t, as_json);

    } else if (cmd == "complete") {
        if (i + 1 != argc) {
            throw runtime_error("Usage: complete <id>");
        }
        string id = argv[i];
        manager.complete_task(id);
        save_file(data_path, manager);
        print_task(manager.get_task(id), as_json);

    } else if (cmd == "fail") {
        if (i >= argc) {
            throw runtime_error("Usage: fail <id> --error <text>");
        }
        string id = argv[i++];
        if (i + 2 != argc || string(argv[i]) != "--error") {
            throw runtime_error("Usage: fail <id> --error <text>");
        }
        manager.fail_task(id, argv[i + 1]);
        save_file(data_path, manager);
        print_task(manager.get_task(id), as_json);

    } else if (cmd == "cancel") {
        if (i + 1 != argc) {
            throw runtime_error("Usage: cancel <id>");
        }
        string id = argv[i];
        manager.cancel_task(id);
        save_file(data_path, manager);
        print_task(manager.get_task(id), as_json);

    } else if (cmd == "stats") {
        if (i != argc) {
            throw runtime_error("Usage: stats");
        }
        print_stats(manager.get_stats(), as_json);

    } else {
        throw runtime_error("Unknown command: " + cmd);
    }
}

#endif
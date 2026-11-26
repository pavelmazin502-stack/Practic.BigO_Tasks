#include <chrono>
#include <iostream>
#include <string>

#include "core/task_manager.hpp"
#include "ds/dynamic_array.hpp"

using namespace std;
using namespace std::chrono;

int main() {
    size_t N = 100000;
    TaskManager manager;

    // Заранее генерируем 100 000 айди
    DynamicArray<string> ids;
    for (size_t i = 0; i < N; ++i) {
        ids.push_back("task-" + to_string(i));
    }

    // Замер добавления
    auto start_add = steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        int prio = static_cast<int>((i * 37) % 101); // приоритеты от 0 до 100
        manager.add_task(ids[i], "job", prio, "payload");
    }
    auto end_add = steady_clock::now();

    // 2. Замер поиска по ID
    size_t sum = 0;
    auto start_find = steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        sum += static_cast<size_t>(manager.get_task(ids[i]).priority);
    }
    auto end_find = steady_clock::now();

    // 3. Замер последовательного извлечения
    auto start_pop = steady_clock::now();
    for (size_t i = 0; i < N; ++i) {
        const Task& t = manager.start_next();
        sum += static_cast<size_t>(t.priority);
    }
    auto end_pop = steady_clock::now();

    double add_ms = duration<double, milli>(end_add - start_add).count();
    double find_ms = duration<double, milli>(end_find - start_find).count();
    double pop_ms = duration<double, milli>(end_pop - start_pop).count();

    cout << "Tasks: " << N << "\n";
    cout << "Add time:                  " << add_ms << " ms\n";
    cout << "Find time:                 " << find_ms << " ms\n";
    cout << "Extract time:              " << pop_ms << " ms\n";
    cout << "HashTable resizes:         " << manager.get_index_resize_count() << "\n";
    cout << "HashTable max load factor: " << manager.get_index_max_load_factor() << "\n";
    cout << "Check sum:                 " << sum << "\n";

    return 0;
}
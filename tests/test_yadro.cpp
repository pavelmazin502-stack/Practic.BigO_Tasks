#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "core/task_manager.hpp"
#include "storage/file_storage.hpp"

using namespace std;

// Проверка создания задач, границ приоритета и порядка FIFO
void test_create_and_priority() {
    TaskManager manager;

    // Приоритет меньше 0 должен кидать ошибку
    bool has_error = false;
    try {
        manager.add_task("bad1", "email", -5, "text");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Приоритет больше 100 тоже запрещён
    has_error = false;
    try {
        manager.add_task("bad2", "email", 101, "text");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Пустой ID запрещён
    has_error = false;
    try {
        manager.add_task("", "email", 10, "text");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Проверяем автогенерацию ID
    string gen_id = manager.generate_id();
    assert(gen_id.empty() == false);

    // Добавляем задачи с разными и одинаковыми приоритетами
    manager.add_task("t_low", "email", 10, "p1");
    manager.add_task("t_high1", "email", 100, "p2");
    manager.add_task("t_high2", "email", 100, "p3");
    manager.add_task("t_zero", "email", 0, "p4");

    // Дубликат ID должен вызывать ошибку
    has_error = false;
    try {
        manager.add_task("t_low", "email", 20, "dup");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // При равном приоритете сначала выходит та, что добавлена раньше
    assert(manager.start_next().id == "t_high1");
    assert(manager.start_next().id == "t_high2");
    assert(manager.start_next().id == "t_low");
    assert(manager.start_next().id == "t_zero");

    cout << "test_create_and_priority OK\n";
}

// Проверка разрешённых и запрещённых переходов статусов
void test_statuses() {
    TaskManager manager;
    manager.add_task("t1", "backup", 50, "db");
    manager.add_task("t2", "backup", 40, "files");

    // Нельзя завершить задачу, пока она в queued
    bool has_error = false;
    try {
        manager.complete_task("t1");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Нельзя зафейлить задачу из queued
    has_error = false;
    try {
        manager.fail_task("t1", "boom");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Запускаем t1 (queued -> running)
    manager.start_next();
    assert(manager.get_task("t1").status == TaskStatus::Running);

    // Из running отменять нельзя
    has_error = false;
    try {
        manager.cancel_task("t1");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Завершаем успешно (running -> completed)
    manager.complete_task("t1");
    assert(manager.get_task("t1").status == TaskStatus::Completed);

    // Из финального состояния completed больше никуда нельзя перейти
    has_error = false;
    try {
        manager.fail_task("t1", "late error");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Запускаем вторую задачу и роняем её с ошибкой (running -> failed)
    manager.start_next();
    manager.fail_task("t2", "disk full");
    assert(manager.get_task("t2").status == TaskStatus::Failed);
    assert(manager.get_task("t2").error == "disk full");

    // Из failed переходы тоже запрещены
    has_error = false;
    try {
        manager.complete_task("t2");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    cout << "test_statuses OK\n";
}

// Проверка отмены, согласованности кучи и индекса, статистики и пустой очереди
void test_cancel_and_queue() {
    TaskManager manager;
    manager.add_task("low", "mail", 15, "a");
    manager.add_task("top", "mail", 95, "b");

    TaskStats st = manager.get_stats();
    assert(st.total == 2);
    assert(st.queued == 2);
    assert(st.queue_size == 2);
    assert(st.has_max_queued_priority == true);
    assert(st.max_queued_priority == 95);

    // Отменяем самую приоритетную задачу
    manager.cancel_task("top");
    assert(manager.get_task("top").status == TaskStatus::Cancelled);

    // Из cancelled завершать нельзя
    bool has_error = false;
    try {
        manager.complete_task("top");
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // start_next должен пропустить отменённую top и взять low
    assert(manager.start_next().id == "low");
    manager.complete_task("low");

    // Очередь теперь пустая -> start_next должен бросить ошибку
    has_error = false;
    try {
        manager.start_next();
    } catch (const exception&) {
        has_error = true;
    }
    assert(has_error == true);

    // Проверяем итоговую статистику
    TaskStats end_st = manager.get_stats();
    assert(end_st.queue_size == 0);
    assert(end_st.has_max_queued_priority == false);
    assert(end_st.completed == 1);
    assert(end_st.cancelled == 1);

    cout << "test_cancel_and_queue OK\n";
}

// Проверка сохранения в файл и восстановления после перезапуска
void test_save_and_load() {
    string path = "test_tasks_tmp.tsv";
    remove(path.c_str());

    // Если файла нет, должно создаться пустое состояние без ошибки
    TaskManager empty_manager;
    load_file(path, empty_manager);
    assert(empty_manager.get_all_tasks().get_size() == 0);

    // Создаём задачи со спецсимволами и сохраняем на диск
    TaskManager writer;
    writer.add_task("t1", "shell", 50, "a\tb\\c\"d\ne");
    writer.add_task("t2", "net", 80, "ok");
    writer.start_next();
    writer.fail_task("t2", "line1\nline2\t\\err");
    save_file(path, writer);

    // Читаем заново в новый объект и сверяем все поля
    TaskManager reader;
    load_file(path, reader);
    assert(reader.get_all_tasks().get_size() == 2);
    assert(reader.get_task("t1").payload == "a\tb\\c\"d\ne");
    assert(reader.get_task("t1").status == TaskStatus::Queued);
    assert(reader.get_task("t2").status == TaskStatus::Failed);
    assert(reader.get_task("t2").error == "line1\nline2\t\\err");

    remove(path.c_str());
    cout << "test_save_and_load OK\n";
}

// Проверка реакции на повреждённые файлы
void test_broken_files() {
    string path = "test_broken_tmp.tsv";

    string bad_files[7] = {
        "",                                                      // пустой файл
        "WRONG_VERSION\n",                                       // неверная шапка
        "TASKCTL_VERSION=1\nonly\t3\tcols\n",                    // мало колонок
        "TASKCTL_VERSION=1\nt1\ta\t10abc\tp\tqueued\t\t1\t1\n",  // мусор в приоритете
        "TASKCTL_VERSION=1\nt1\ta\t50\tp\tunknown\t\t1\t1\n",    // неизвестный статус
        "TASKCTL_VERSION=1\nt1\ta\t150\tp\tqueued\t\t1\t1\n",    // приоритет больше 100
        "TASKCTL_VERSION=1\nt1\ta\t50\tp\tqueued\t\t1\t1\n"
        "t1\ta\t50\tp\tqueued\t\t1\t1\n"                         // дубликат ID
    };

    for (int i = 0; i < 7; ++i) {
        ofstream out(path);
        out << bad_files[i];
        out.close();

        TaskManager manager;
        bool has_error = false;
        try {
            load_file(path, manager);
        } catch (const exception&) {
            has_error = true;
        }
        assert(has_error == true);
        assert(manager.get_all_tasks().get_size() == 0); // частично загруженные данные не должны остаться
    }

    remove(path.c_str());
    cout << "test_broken_files OK\n";
}

int main() {
    test_create_and_priority();
    test_statuses();
    test_cancel_and_queue();
    test_save_and_load();
    test_broken_files();

    cout << "All core tests passed!\n";
    return 0;
}
// Файл для реализации работы с диском
// Фалй включает в себя сериализацию - превращение объектов в текст, а также десериализация - восстановление структур данных в новом запуске
#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

#include <cstddef>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdint>
#include <stdexcept>

#include "core/task_manager.hpp"

inline const string STORAGE_HEADER = "TASKCTL_VERSION=1"; // Номер версии формата по заданию

inline string escape_field(const string& text){
    string res;

    // Проверка на ненужные символы при записи в файл (чтобы текст не сломал структуру нашего файла)
    // Цикл проверяет наличие невидимых спецсимволов и при наличии заменяет их на текст

    for (char c : text) {
        if (c == '\\') { // Замена реального слеша на 3 слеша
            res += "\\\\";
        } else if (c == '\t') { // Замена tab - приклеиваем \t 
            res += "\\t";
        } else if (c == '\n') { // Приклеиваем \t
            res += "\\n";
        } else {
            res.push_back(c);
        }
    }
    return res;
}

inline string unescape_field(const string& text) {
    string res;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size()) { // Если текущий символ это \ и справа есть какой-то символ 
            ++i;
            char next = text[i];
            if (next == 't') {
                res.push_back('\t');
            } else if (next == 'n') {
                res.push_back('\n');
            } else {
                res.push_back(next);
            }
        } else {
            res.push_back(text[i]);
        }
    }
    return res;
}

// Далее нам нужна функция для нарезки строки по табуляции

inline DynamicArray<string> split(const string& line) {
    DynamicArray<string> parts;
    string cur;

    for (char c : line) { // Проходимся по строке
        if (c == '\t') {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

inline int parse_int(const string& s) { // На вход подается исходная строка
    if (s.empty()) {
        throw std::runtime_error("Empty integer value");
    }
    size_t pos = 0;
    int val = std::stoi(s, &pos);
    if (pos != s.size()) {
        throw std::runtime_error("Invalid integer: " + s);
    }
    return val;
}

inline uint64_t parse_uint64(const string& s) {
    if (s.empty() || s[0] == '-') { // Проверка что если число отрицательное
        throw std::runtime_error("Invalid timestamp: " + s);
    }
    size_t pos = 0;
    uint64_t val = std::stoull(s, &pos);
    if (pos != s.size()) {
        throw std::runtime_error("Invalid timestamp: " + s);
    }
    return val;
}

// Далее напишем функцию, которая воспроизводит чтение с диска в TaskManager

inline void load_file(const string& path, TaskManager& manager){ // Принимаем путь и классь менеджер в дальнейшем заполним полями
    
    if (filesystem::exists(path) == false){
        return; // Если файл нет, оставляем пустое состояние
    }

    ifstream in(path); // Открываем поток данных из файла
    if (in.is_open() == false){
        throw runtime_error("Cannot open file " + path);
    }

    // Проверка первой строки - маркер версии формата
    string line;
    if (!getline(in, line)){
        throw runtime_error("File is empty " + path); 
    }

    if (line != STORAGE_HEADER){
        throw runtime_error("Invalid storage version " + line);
    }

    // Грузим задачи во временный менеджер. Если случится ошибка временный файл уничтожится, а основной файл останется

    TaskManager PROGA;

    size_t line_size = 1; 

    // Читаем построчно все строки и собираем задачи
    while (getline(in, line)){
        ++line_size;

        if (line.empty() == false){
            line.pop_back();
        }
        if (line.empty() == true){
            continue; // Пропускаем пустые строчки
        }
        // У каждой задачи ровно 8 полей (id, type ..)
        DynamicArray<string> p = split(line); // Режем строки
        
        if (p.get_size() != 8){
            throw runtime_error("Error: wrong number of fields");
        }

        // Далее раскладываем 8 колонок в структуру Task

        Task n;
        n.id = unescape_field(p[0]);
        n.type = unescape_field(p[1]);
        n.priority = parse_int(p[2]);
        n.payload = unescape_field(p[3]);
        n.status = terminalinstatus(p[4]);
        n.error = unescape_field(p[5]);
        n.created_at = parse_uint64(p[6]);
        n.updated_at = parse_uint64(p[7]);

        PROGA.load_task(n);
    }

    manager = PROGA; // Фиксируем состояние
}
// Атомарное сохранение состояния на диск через PROGA
inline void save_file(const string& path, const TaskManager& manager){

    // Для начала создаем временный файл рядом с основным
    string copy = path + ".tmp";
    ofstream out(copy); // Поток вывода, текст отправляется в файл создается новый файл

    if (!out.is_open()) {
        throw std::runtime_error("Cannot open temp file: " + copy);
    }

    out << STORAGE_HEADER << '\n';


    // Запись одной задачи в одну строку файла
    const DynamicArray<Task>& tasks = manager.get_all_tasks();
    for (size_t i = 0; i < tasks.get_size(); ++i) {
        const Task& t = tasks[i];
        out << escape_field(t.id) << '\t' << escape_field(t.type) << '\t'
            << t.priority << '\t'
            << escape_field(t.payload) << '\t'
            << status_terminal(t.status) << '\t'
            << escape_field(t.error) << '\t'
            << t.created_at << '\t'
            << t.updated_at << '\n';
    }

    if (!out.good()) { // Проверка сбоя при записи на диск
        out.close();
        error_code rm_ec; // Контейнер куда будем ложит ькод ошибки
        filesystem::remove(copy, rm_ec); // Удаление файла
        throw std::runtime_error("Failed writing temp file: " + copy);
    }
    
    out.close();


    error_code rename_ec; // Переменная типа, принимает код результата операции 
    filesystem::rename(copy, path, rename_ec); // Переименование файла copy - path
    if (rename_ec) {
        error_code rm_ec;
        filesystem::remove(copy, rm_ec);
        throw runtime_error("Failed replacing storage file: " + path);
    }

}
#endif
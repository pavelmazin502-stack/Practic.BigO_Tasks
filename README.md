# taskctl

Локальное ядро системы управления заданиями на C++17 с собственными структурами данных (`DynamicArray`, `HashTable`, `BinaryHeap`), CLI-интерфейсом и атомарным сохранением состояния в файл.

## Структура проекта

```text
.
├── CMakeLists.txt
├── README.md
├── ARCHITECTURE.md
├── COMPLEXITY.md
├── src/
│   ├── main.cpp
│   ├── ds/
│   │   ├── dynamic_array.hpp
│   │   ├── hash_table.hpp
│   │   └── binary_heap.hpp
│   ├── core/
│   │   ├── task.hpp
│   │   └── task_manager.hpp
│   ├── storage/
│   │   └── file_storage.hpp
│   └── cli/
│       └── cli.hpp
└── tests/
    ├── test_da.cpp
    ├── test_ht.cpp
    ├── test_bh.cpp
    ├── test_yadro.cpp
    └── bench.cpp
```

## Сборка

Нужен компилятор с поддержкой C++17 и CMake (от 3.20):

```bash
cmake -S . -B build
cmake --build build
```

## Тесты и бенчмарк

Запуск всех тестов:
```bash
ctest --test-dir build --output-on-failure
```

Запуск бенчмарка на 100 000 задач (с `-O2`, без санитайзеров):
```bash
g++ -std=c++17 -O2 -Isrc tests/bench.cpp -o build/bench_opt
./build/bench_opt
```

## Как запускать

```bash
./build/taskctl --data <путь_к_файлу> [--format text|json] <команда> [аргументы]
```

* `--data <путь>` — файл базы данных. Если файла нет, программа начнёт с пустого списка.
* `--format text|json` — формат вывода (по умолчанию `text`, для JSON передать `--format json`). Оба флага пишутся до названия команды.

## Примеры команд

```bash
# Создать задачу (приоритет от 0 до 100, --id можно не указывать — сгенерируется сам)
./build/taskctl --data tasks.tsv create --type email --priority 50 --payload "hello"
./build/taskctl --data tasks.tsv create --id t1 --type backup --priority 90 --payload "db"

# Найти задачу по ID
./build/taskctl --data tasks.tsv get t1

# Показать все задачи или отфильтровать по статусу
./build/taskctl --data tasks.tsv list
./build/taskctl --data tasks.tsv list --status queued

# Взять в работу задачу с самым высоким приоритетом
./build/taskctl --data tasks.tsv start-next

# Завершить, пометить с ошибкой или отменить
./build/taskctl --data tasks.tsv complete t1
./build/taskctl --data tasks.tsv fail t1 --error "disk full"
./build/taskctl --data tasks.tsv cancel t1

# Статистика (в тексте или в JSON)
./build/taskctl --data tasks.tsv stats
./build/taskctl --data tasks.tsv --format json stats
```

## Особенности работы

* `list` всегда выводит задачи в порядке их создания.
* Если очередь пуста, `start-next` выводит сообщение об ошибке и завершается с кодом `1`.
* При любой ошибке (неверный флаг, приоритет не от 0 до 100, запрещённый переход статуса, битый файл) программа печатает текст ошибки в `stderr`, возвращает код `1` и не меняет файл с данными.
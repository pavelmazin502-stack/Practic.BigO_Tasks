cmake_minimum_required(VERSION 3.20)
project(taskctl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -fsanitize=address,undefined -g)
add_link_options(-fsanitize=address,undefined) # Настройка на этапе линковки

include_directories(src) # Путь к нашим файлам

add_executable(test_dynamic_array tests/test_da.cpp)
add_executable(test_hash_table tests/test_ht.cpp)
add_executable(test_binary_heap tests/test_bh.cpp)
add_executable(taskctl src/main.cpp)
add_executable(test_core tests/test_yadro.cpp)
add_executable(bench_100k tests/bench.cpp)


enable_testing() #Рубильник для тестирования (создание внутри папки build списка тестов)
add_test(NAME DynamicArrayTest COMMAND test_dynamic_array)
add_test(NAME HashTableTest COMMAND test_hash_table)
add_test(NAME BinaryHeapTest COMMAND test_binary_heap)
add_test(NAME CoreTest COMMAND test_core)

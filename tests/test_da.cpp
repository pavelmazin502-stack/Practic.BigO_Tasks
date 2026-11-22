#include <cassert>  
#include <iostream> 
#include <stdexcept> 
#include "ds/dynamic_array.hpp"

void test_dynamic_array() {
    DynamicArray<int> arr;

   
    assert(arr.is_empty());
    assert(arr.get_size() == 0);

    for (int i = 0; i < 10; ++i) {
        arr.push_back(i * 10);
    }
    assert(arr.get_size() == 10);
    assert(arr.get_capacity() >= 10);
    assert(arr.get(0) == 0);
    assert(arr.get(9) == 90);

    // Изменение элемента
    arr.set(5, 999);
    assert(arr.get(5) == 999);

    // Проверяем удаление из середины (удаляем элемент с индексом 5)
    arr.remove_at(5);
    assert(arr.get_size() == 9);
    assert(arr.get(5) == 60); // На место 5-го должен сдвинуться 6-й (60)

    // Проверяем удаление с конца
    arr.pop_back();
    assert(arr.get_size() == 8);

    // Проверяем копирование
    DynamicArray<int> copy_arr = arr;
    copy_arr.set(0, 777);
    assert(arr.get(0) == 0);     
    assert(copy_arr.get(0) == 777);

   
    bool caught = false; // Ловим исключение на index out
    try {
        arr.get(100); // Пытаемся залезть в 100-ю ячейку
    } catch (const std::out_of_range&) {
        caught = true; // Поймали ошибку — всё сработало правильно
    }
    assert(caught);

    std::cout << "All DynamicArray tests passed!\n";
}

int main() {
    test_dynamic_array();
    return 0;
}
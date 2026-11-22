// Бинарная куча - структура данных (за O(1) просмотр самого приоритетного элемента)
// Бинарная куча - полное бинарное дерево (3 условия), любой родитель больше или равен своим детям
#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <cstddef>
#include <stdexcept>
#include "ds/dynamic_array.hpp"

template <typename T>
class BinaryHeap {
private:
    DynamicArray<T> data; // Вся куча лежит в массиве

    void swap_elements(T& a, T& b) {
        T temp = a;
        a = b;
        b = temp;
    }

    void sift_up(size_t index) { // Подъем вверх (если закинули элемент в конец массива)
        while (index > 0) {
            size_t parent = (index - 1) / 2; // Формула родителя в массиве
            if (data[parent] < data[index]) {
                swap_elements(data[parent], data[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(size_t index) { // Функция вызывается при удалении корня
        size_t n = data.get_size();
        while (true) {
            size_t left = 2 * index + 1; // Индексы левого и правиого ребенка
            size_t right = 2 * index + 2;
            size_t largest = index;

            if (left < n && data[largest] < data[left]) {
                largest = left;
            }
            if (right < n && data[largest] < data[right]) {
                largest = right;
            }

            if (largest == index) {
                break;
            }

            swap_elements(data[index], data[largest]);
            index = largest;
        }
    }

public:
    void push(const T& value) { // Добавление в конец массива и поднимание через sift_up
        data.push_back(value);
        sift_up(data.get_size() - 1);
    }

    const T& top() const {
        if (is_empty()) {
            throw std::out_of_range("Heap is empty");
        }
        return data[0]; // Возвращение макс элемента
    }

    void pop() { // Функция удаления максимального элемента
        if (is_empty()) {
            throw std::out_of_range("Heap is empty");
        }

        size_t last_index = data.get_size() - 1; // Находим индекс самого последнего элемента в массиве
        if (last_index > 0) {
            data[0] = data[last_index];
        }

        data.pop_back();

        if (!is_empty()) {
            sift_down(0);
        }
    }

    size_t get_size() const {
        return data.get_size();
    }

    bool is_empty() const {
        return data.get_size() == 0;
    }

    void clear() {
        data.clear();
    }
};

#endif
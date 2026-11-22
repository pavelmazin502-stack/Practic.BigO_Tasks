// Бинарная куча - структура данных (за O(1) просмотр самого приоритетного элемента)
#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <cstddef>
#include <stdexcept>
#include "ds/dynamic_array.hpp"

template <typename T>
class BinaryHeap {
private:
    DynamicArray<T> data;

    void swap_elements(T& a, T& b) {
        T temp = a;
        a = b;
        b = temp;
    }

    void sift_up(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (data[parent] < data[index]) {
                swap_elements(data[parent], data[index]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void sift_down(size_t index) {
        size_t n = data.get_size();
        while (true) {
            size_t left = 2 * index + 1;
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
    void push(const T& value) {
        data.push_back(value);
        sift_up(data.get_size() - 1);
    }

    const T& top() const {
        if (is_empty()) {
            throw std::out_of_range("Heap is empty");
        }
        return data[0];
    }

    void pop() {
        if (is_empty()) {
            throw std::out_of_range("Heap is empty");
        }

        size_t last_index = data.get_size() - 1;
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
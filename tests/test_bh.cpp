#include <cassert>
#include <iostream>
#include <stdexcept>
#include "ds/binary_heap.hpp"

void test_binary_heap() {
    BinaryHeap<int> heap;

    assert(heap.is_empty());
    assert(heap.get_size() == 0);

    bool caught_top = false;
    try {
        heap.top();
    } catch (const std::out_of_range&) {
        caught_top = true;
    }
    assert(caught_top);

    bool caught_pop = false;
    try {
        heap.pop();
    } catch (const std::out_of_range&) {
        caught_pop = true;
    }
    assert(caught_pop);

    // Проверка вставки и всплытия максимума
    heap.push(10);
    assert(heap.top() == 10);
    heap.push(30);
    assert(heap.top() == 30);
    heap.push(20);
    assert(heap.top() == 30);
    heap.push(50);
    assert(heap.top() == 50);
    heap.push(30); // Дубликат
    assert(heap.get_size() == 5);

    // Проверка извлечения
    int expected[] = {50, 30, 30, 20, 10};
    for (int i = 0; i < 5; ++i) {
        assert(heap.top() == expected[i]);
        heap.pop();
    }
    assert(heap.is_empty());
    assert(heap.get_size() == 0);

    for (int i = 1; i <= 100; ++i) { // Проверка на большом количествеп элементов
        heap.push(i);
    }
    assert(heap.get_size() == 100);
    for (int i = 100; i >= 1; --i) {
        assert(heap.top() == i);
        heap.pop();
    }
    assert(heap.is_empty());

    heap.push(42);
    heap.push(99);
    heap.clear();
    assert(heap.is_empty());
    assert(heap.get_size() == 0);

    std::cout << "All BinaryHeap tests passed!\n";
}

int main() {
    test_binary_heap();
    return 0;
}
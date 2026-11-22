#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <cstddef>
#include <stdexcept>

template <typename T> // Шаблон класса

class DynamicArray{
private:
    T* data = nullptr;
    size_t size = 0;
    size_t capacity = 0;

    void grow(){ // Автоматическое выделение памяти
        size_t new_capacity;
        if (capacity == 0){
            new_capacity = 4; // Сначала берем 4 ячейки
        } else {
            new_capacity = capacity * 2;
        }
        T* new_data = new T[new_capacity];
        // Увеличиваем размер массива и перекладываем туда элементы из прошлого массива
        for (size_t i = 0; i < size; ++i){
            new_data[i] = data[i];
        }
        delete[] data;
        data = new_data;
        capacity = new_capacity;

    }
    
public:
    DynamicArray(){}

    explicit DynamicArray(size_t init_capacity){ // Защита от неявного преобразования
        if (init_capacity > 0){
            data = new T[init_capacity];
            capacity = init_capacity;
        }
    }
    ~DynamicArray(){
        delete [] data;
    }

    void push_back(const T& value){ // Добавление в конец
        if (size == capacity){
            grow();
        }
        data[size] = value;
        size++;
    }

    size_t get_size() const {
        return size;
    }

    size_t get_capacity() const {
        return capacity;
    }

    bool is_empty() const { // Проверка на пустоту массива
        return size == 0;
    }

    const T& get(size_t index) const{ // Получение элемента
        if (index >= size){
            throw std::out_of_range("Index out of the rangfe");
        }
        return data[index];
    }

    void set(size_t index, const T& value){ // Изменение элемента по индексу
        if (index >= size){
            throw std::out_of_range("Index out of the range");
        }
        data[index] = value;
    }

    void remove_at(size_t index){ // Удаление элемента
        if (index >= size){
            throw std::out_of_range("Index out of the rangfe");
        }
        for (size_t i = index; i + 1 < size; i++){
            data[i] = data[i+1];
        }
        size--;
    }

    void pop_back() { // Удаление последнего элемента
        if (size == 0) {
            throw std::out_of_range("Array is empty");
        }
        --size;
    }

    DynamicArray(const DynamicArray& other) {
        size = other.size;
        capacity = other.capacity;
        if (capacity > 0) {
            data = new T[capacity]; // Создание нового класса
            for (size_t i = 0; i < size; ++i) {
                data[i] = other.data[i]; // Само копирование
            }
        }
    }

    // Оператор присваивания (заменяем свою старую коробку на новую)
    DynamicArray& operator=(const DynamicArray& other) {

        if (this == &other) { // Защита от самоприсваивания
            return *this;
        }

        // Очистка памяти (старой)
        delete[] data;

        // Копируем всё из other
        size = other.size;
        capacity = other.capacity;
        if (capacity > 0) {
            data = new T[capacity];
            for (size_t i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        } else {
            data = nullptr; 
        }

        return *this; // Возвращаем сам объект 
    }

    T& operator[](size_t index) {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    const T& operator[](size_t index) const {
        if (index >= size) {
            throw std::out_of_range("Index out of range");
        }
        return data[index];
    }

    void clear() {
        size = 0;
    }
};


#endif 
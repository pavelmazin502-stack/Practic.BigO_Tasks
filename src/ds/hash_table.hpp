#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <stdexcept>

template <typename K,typename V> // шаблон с двумя параметрами - ключ и значение

// Хеш таблица, разрешение коллизий методом цепочек через односвязный список

class HashTable{
private:
    struct Node{// Сама реализация односвязного списка
        K key;
        V value;
        Node* next = nullptr;

        Node(const K& k, const V& v){ // Конструктор для узла
            key = k;
            value = v;
            next = nullptr;
        }
    };

    Node** buckets = nullptr; // Массив указателей для решения коллизий
    size_t capacity = 0;
    size_t size = 0;

    void free_nodes() { // Удаление всех цепочек (чтобы не допустить утечку памяти)
        for (size_t i = 0; i < capacity; ++i) {
            Node* current = buckets[i];
            while (current != nullptr) {
                Node* next_node = current->next;
                delete current;                  
                current = next_node;          
            }
            buckets[i] = nullptr; // Шкафчик теперь пуст
        }
        size = 0;
    }

    // реализация самой хеш-функции
    size_t get_bucket_index(const std::string& key) const {
        uint64_t hash = 0;
        for (size_t i = 0; i < key.size(); ++i){
            hash = hash * 31 + static_cast<unsigned char> (key[i]);
        }
        return static_cast<size_t>(hash%capacity);
    }

    size_t get_bucket_index(size_t key) const {
        return key % capacity;
    }

    // Функция увеличения шкафичокв
    void rehash() {
        size_t old_capacity = capacity;
        Node** old_buckets = buckets;

        capacity = old_capacity * 2; // Создаем новый массив указатей - шкафчиков
        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr; // заполняем 0
        }

        for (size_t i = 0; i < old_capacity; ++i) {
            Node* current = old_buckets[i];
            while (current != nullptr) {
                Node* next_node = current->next; 
                
                size_t new_index = get_bucket_index(current->key);

                // Перевешиваем готовый узел в начало новой цепочки
                current->next = buckets[new_index];
                buckets[new_index] = current;

                current = next_node; // Шагаем к следующему узлу старой цепочки
            }
        }

        // Выкидываем старую пустую полку со шкафчиками
        delete[] old_buckets;
        ++resize_count;
    }

    size_t resize_count = 0;
    double max_load_factor = 0.0;

// Запрет на неявное приведения типов
public:
    explicit HashTable(size_t init_capacity = 16){ // Для начала выделеям 16 ячеек памяти
        if (init_capacity == 0){
            init_capacity = 16; // Защита от деления на 0 (вычисление хеша)
        }
        capacity = init_capacity;
        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr; 
        }
    }

    // Деструктор
    ~HashTable() {
        free_nodes();
        delete[] buckets;
    }


    void put(const K& key, const V& value) { // Положить пару ключ-значение

        if ((size+1) * 4 >= capacity * 3){ // Когда таблица заполняется на 75% мы увеличиваем размер
            rehash();
        }

        size_t index = get_bucket_index(key); // Номер шкафчика

        Node* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;// Ключ нашли — просто обновляем значение
                return;                
            }
            current = current->next;
        }

        // Если ключа нет — создаём новый узел и вешаем его в начало
        Node* new_node = new Node(key, value);
        new_node->next = buckets[index]; 
        buckets[index] = new_node;
        ++size;

        double lf = static_cast<double>(size) / static_cast<double>(capacity); // Явное приведение типов
        if (lf > max_load_factor) {
            max_load_factor = lf;
        }
    }

    // Проверяем, есть ли такой ключ в таблице
    bool contains(const K& key) const {
        size_t index = get_bucket_index(key);
        Node* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                return true; 
            }
            current = current->next;
        }
        return false; 
    }

    // Достаём значение по ключу
    V& get(const K& key) {
        size_t index = get_bucket_index(key);
        Node* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                return current->value; // Отдаём само значение по ссылке
            }
            current = current->next;
        }
        throw std::out_of_range("Key not found");
    }

    const V& get(const K& key) const {
        size_t index = get_bucket_index(key);
        Node* current = buckets[index];
        while (current != nullptr) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        throw std::out_of_range("Key not found");
    }

    // Удаление ключа по значению
    bool remove(const K& key) {
        size_t index = get_bucket_index(key);
        Node* current = buckets[index];
        Node* prev = nullptr; // Предыдущий узел

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    buckets[index] = current->next; // Удаляем первый узел в шкафе
                } else {
                    prev->next = current->next;
                }

                delete current;
                --size;         
                return true;    
            }
            prev = current; // Сдвигаем указатели
            current = current->next;
        }
        return false; 
    }

        // Чтобы при выполнении 1000000 задач наша таблица не сломалась нужно написать функцию
        // расширения шкафчиков, напишем в private
    
    // Копирующий конструктор
    HashTable(const HashTable& other) {
        capacity = other.capacity;
        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }

        // Проходим по всем шкафчикам чужой таблицы и кладём копии элементов к себе
        for (size_t i = 0; i < other.capacity; ++i) {
            Node* current = other.buckets[i];
            while (current != nullptr) {
                put(current->key, current->value);
                current = current->next;
            }
        }
    }

    // Оператор присваивания
    HashTable& operator=(const HashTable& other) {
        if (this == &other) {
            return *this;
        }

        // Очищаем и удаляем свой старый гардероб
        free_nodes();
        delete[] buckets;

        //Создаём новый пустой гардероб такого же размера
        capacity = other.capacity;
        buckets = new Node*[capacity];
        for (size_t i = 0; i < capacity; ++i) {
            buckets[i] = nullptr;
        }

        // Копируем все пары (ключ, значение) из чужой таблицы
        for (size_t i = 0; i < other.capacity; ++i) {
            Node* current = other.buckets[i];
            while (current != nullptr) {
                put(current->key, current->value);
                current = current->next;
            }
        }

        return *this;
    }

    size_t get_resize_count() const {
        return resize_count;
    }

    double get_max_load_factor() const {
        return max_load_factor;
    }

    size_t get_size() const {
        return size;
    }

    bool is_empty() const {
        return size == 0;
    }

    void clear() {
        free_nodes();
    }
};



#endif
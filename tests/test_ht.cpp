#include <cassert>  
#include <iostream> 
#include <stdexcept> 
#include <string>
#include "ds/hash_table.hpp"

void test_hash_table() {
    HashTable<std::string, int> ht;

    assert(ht.is_empty());
    assert(ht.get_size() == 0);
    assert(!ht.contains("Pavel-1"));

    
    ht.put("Pavel-1", 10);
    ht.put("Victor-2", 20);
    ht.put("Ivan-3", 30);
    assert(ht.get_size() == 3);
    assert(ht.contains("Victor-2")); // Проверка ключа
    assert(ht.get("Pavel-1") == 10);
    assert(ht.get("Victor-2") == 20);

  
    ht.put("Victor-2", 999);
    assert(ht.get_size() == 3); 
    assert(ht.get("Victor-2") == 999);

    assert(ht.remove("Victor-2") == true);   // Удалили существующий
    assert(ht.remove("Victor-2") == false); 
    assert(ht.get_size() == 2);
    assert(!ht.contains("Victor-2"));

    //Проверяем ошибку при поиске несуществующего ключа
    bool caught = false;
    try {
        ht.get("no-such-key");
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    // Проверяем rehash() — кладём 50 элементов (изначально шкафчиков всего 16)
    for (int i = 0; i < 50; ++i) {
        ht.put("key-" + std::to_string(i), i * 100);
    }
    assert(ht.get_size() == 52); 
    for (int i = 0; i < 50; ++i) {
        assert(ht.get("key-" + std::to_string(i)) == i * 100);
    }

    // Проверяем копирование (Правило трёх)
    HashTable<std::string, int> copy_ht = ht;
    copy_ht.put("Pavel-1", 777);
    assert(ht.get("Pavel-1") == 10);       // Оригинал не изменился
    assert(copy_ht.get("Pavel-1") == 777); // Копия изменилась

    // Проверяем очистку
    ht.clear();
    assert(ht.is_empty());
    assert(ht.get_size() == 0);

    std::cout << "All HashTable tests passed!\n";
}

int main(){
    test_hash_table(); 
    return 0;
}
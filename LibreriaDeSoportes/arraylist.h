#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <cstddef>
#include <stdexcept>

using namespace std;

template<typename T>
class ArrayList {
private:
    T* data;
    size_t count;
    size_t capacity;
    size_t iterator;
    
    void resize();
    
public:
    ArrayList();
    ~ArrayList();
    

    void add(const T& element);
    T& get(size_t index);
    const T& get(size_t index) const;
    void remove(size_t index);
    T* next();
    T* prev();
    T& last();
    T& first();
    size_t size() const;
    void reset_iterator();
};

template<typename T>
ArrayList<T>::ArrayList() : count(0), capacity(10), iterator(0) {
    data = new T[capacity];
}

template<typename T>
ArrayList<T>::~ArrayList() {
    delete[] data;
}
 

template<typename T>
void ArrayList<T>::resize() {
    size_t newCapacity = capacity * 2;
    T* newData = new T[newCapacity];
    
    for (size_t i = 0; i < count; i++) {
        newData[i] = data[i];
    }
    
    delete[] data;
    data = newData;
    capacity = newCapacity;
}

template<typename T>
void ArrayList<T>::add(const T& element) {
    if (count >= capacity) {
        resize();
    }
    data[count] = element;
    count++;
}


template<typename T>
T& ArrayList<T>::get(size_t index) {
    if (index >= count) {
        throw std::out_of_range("Indice fuera de rango");
    }
    return data[index];
}

template<typename T>
const T& ArrayList<T>::get(size_t index) const {
    if (index >= count) {
        throw std::out_of_range("Indice fuera de rango");
    }
    return data[index];
}

template<typename T>
void ArrayList<T>::remove(size_t index) {
    if (index >= count) {
        throw out_of_range("Indice fuera de rango");
    }
    
    for (size_t i = index; i < count - 1; i++) {
        data[i] = data[i + 1];
    }
    
    count--;
    if (iterator > index) {
        iterator--;
    }
}

template<typename T>
T* ArrayList<T>::next() {
    if (iterator >= count) {
        return nullptr;
    }
    return &data[iterator++];
}

template<typename T>
T* ArrayList<T>::prev() {
    if (iterator == 0) {
        return nullptr;
    }
    iterator--;
    return &data[iterator];
}

template<typename T>
T& ArrayList<T>::last() {
    if (count == 0) {
        throw out_of_range("Lista vacia");
    }
    return data[count - 1];
}

template<typename T>
T& ArrayList<T>::first() {
    if (count == 0) {
        throw out_of_range("Lista vacia");
    }
    return data[0];
}

template<typename T>
size_t ArrayList<T>::size() const {
    return count;
}

template<typename T>
void ArrayList<T>::reset_iterator() {
    iterator = 0;
}

#endif
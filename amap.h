#pragma once

#include <string.h>

template <class T>
class AList {
public:
    int _size = 10;
    int count = 0;
    T *elements;
    AList(AList<T> &source) { 
        elements = (T*)malloc(sizeof(T) * source._size);
#ifdef MALLOC_DEBUG
        Serial.print("Cloned malloc: ");
        Serial.println(int(elements));
#endif
        memcpy(elements, source.elements, sizeof(T) * source._size);
        _size = source._size;
        count = source.count;
    }
    AList() {
        _size = 10;
        count = 0;
        elements = (T*)malloc(sizeof(T) * 10);
#ifdef MALLOC_DEBUG
        Serial.print("Malloc: ");
        Serial.println(int(elements));
#endif
    }
    ~AList() {
#ifdef MALLOC_DEBUG
        Serial.print("Freeing: ");
        Serial.println(int(elements));
#endif
        free(elements);
    }
    void append(T value) {
        if(count == _size) {
            _size *= 2;
            elements = (T*)realloc(elements, sizeof(T) * _size);
        }
        //For some reason assigning to the location causes a hang with T = Result
        //Instead I'll just memcpy I guess
        memcpy(&elements[count], &value, sizeof(T));
        count++;
    }
    T &get(int i) {
        return elements[i];
    }
    T &operator [](int i) {
        return this->get(i);
    }
    int id() { return int(this); }
    typedef T * iterator;
    typedef const T * const_iterator;
    int size() { return count; }
    iterator begin() { return &elements[0]; }
    iterator end() { return &elements[size()]; }
private:
    AList(const AList&);
};


template <class T>
class KeyValuePair {
public:
    bool valid = true;
    char key[64];
    T value;
};

template <class T>
class AMap : public AList<KeyValuePair<T>> {
public:
    AMap(AMap<T> &source) : AList<KeyValuePair<T>>(source) { };
    AMap() { };
    using AList<KeyValuePair<T>>::get;
    void set(const char* key, T value) {
        T* current = get_create(key);
        *current = value;
    }
    T* get(const char* key) {
        for(int i = 0; i < this->size(); i++) {
            KeyValuePair<T> &kvp = this->get(i);
            if(kvp.valid && strcmp(key, kvp.key) == 0)
                return &kvp.value;
        }
        return NULL;
    };
    T *remove(const char* key) {
        for(auto &kvp : *this) {
            if(strcmp(key, kvp.key) == 0) {
                kvp.valid = false;
                return &kvp.value;
            }
        }
        return NULL;
    }
    bool has(const char*key) {
        return bool(get(key));
    };
    bool has(int key) {
        char key_buf[64];
        itoa(key, key_buf, 10);
        return has(key_buf);
    };
    T &operator [](const char* key) {
        return *get_create(key);
    }
    T &operator [](int key) {
        return *get_create(key);
    }
    T* get_create(int key) {
        char key_buf[64];
        itoa(key, key_buf, 10);
        return get_create(key_buf);
    }
    T* get_create(const char* key) {
        T* current = get(key);
        if(current == NULL) {
            struct KeyValuePair<T> to_add;
            strncpy(to_add.key, key, 64);
            //Try to find an empty item
            for(auto &kvp : *this) {
                if(!kvp.valid) {
                    kvp = to_add;
                    return &kvp.value;
                }
            }
            //Otherwise add a new one
            this->append(to_add);
            current = &this->get(this->size() - 1).value;
        }
        return current;
    }
private:
    AMap(const AMap&);
};
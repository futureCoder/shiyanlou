/********************
 *  LeakDetector.cpp*
 ********************/

#include <iostream>
#include <cstring>

#define __NEW_OVERLOAD_IMPLEMENTATION__
#include "LeakDetector.hpp"

typedef struct SMemoryList {
    struct SMemoryList *next, *prev;
    size_t size;
    bool isArray;
    char *file;
    unsigned int line;
}SMemoryList;

static unsigned long s_ulMemoryAllocated = 0;

static SMemoryList root = {
    &root, &root,
    0, false,
    NULL, 0
};

unsigned int CLeakDetector::m_uCallCount = 0;

void* AllocateMemory(size_t _size, bool _array, char *_file, unsigned _line) {
    size_t newSize = sizeof(SMemoryList) + _size;
    SMemoryList *newElem = (SMemoryList*)malloc(newSize);
    newElem->next = root.next;
    newElem->prev = &root;
    newElem->size = _size;
    newElem->isArray = _array;
    newElem->file = NULL;

    if(_file) {
        newElem->file = (char *)malloc(strlen(_file) + 1);
        strcpy(newElem->file, _file);
    }
    newElem->line = _line;

    root.next->prev = newElem;
    root.next = newElem;
    s_ulMemoryAllocated += _size;

    return (char *)newElem + sizeof(SMemoryList);
}

void DeleteMemory(void* _ptr, bool _array) {
    SMemoryList *currentElem = (SMemoryList*)((char*)_ptr - sizeof(SMemoryList));
    if(currentElem->isArray != _array) 
        return ;
    currentElem->prev->next = currentElem->next;
    currentElem->next->prev = currentElem->prev;
    s_ulMemoryAllocated -= currentElem->size;
    if(currentElem->file)
        free(currentElem->file);
    free(currentElem);
}

void* operator new(size_t _size) {
    return AllocateMemory(_size, false, NULL, 0);
}

void* operator new[](size_t _size) {
    return AllocateMemory(_size, true, NULL, 0);
}

void* operator new(size_t _size, char *_file, unsigned int _line) {
    return AllocateMemory(_size, false, _file, _line);
}

void* operator new[](size_t _size, char *_file, unsigned int _line) {
    return AllocateMemory(_size, true, _file, _line);
}

void operator delete(void *_ptr) noexcept {
    DeleteMemory(_ptr, false);
}

void operator delete[](void *_ptr) noexcept {
    DeleteMemory(_ptr, true);
}

unsigned int CLeakDetector::LeakDetector(void) noexcept {
   unsigned int count = 0;
   SMemoryList *ptr = root.next;
   while (ptr && ptr != &root) {
       if(ptr->isArray)
           std::cout << "leak[] ";
       else
           std::cout << "leak   ";
       std::cout << ptr << " size " << ptr->size;
       if(ptr->file)
           std::cout << " (locate in " << ptr->file << " line " << ptr->line << ")";
       else
           std::cout << " (Cannot find position)";
       std::cout << std::endl;
       ++count;
       ptr = ptr->next;
   }
   if(count)
       std::cout << "Total " << count << " leaks, size " << s_ulMemoryAllocated << " byte. " << std::endl;
   return count;
}

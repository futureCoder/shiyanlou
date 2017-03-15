#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <iostream>
#include <cstdlib>

template <typename T>
class Queue {
  Queue();
  ~Queue();
  void clear();
  T* enQueue(T &node);
  T* deQueue();
  T* orderEnqueue(Event &event);
  int length();
private:
  T *front;
  T *rear;
};

#endif

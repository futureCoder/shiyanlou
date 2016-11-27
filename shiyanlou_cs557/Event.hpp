#ifndef EVENT_HPP
#define EVENT_HPP

#include "Random.hpp"

const int Random_Parameter = 100;

struct Event {
  int occur_time;

  // 使用 -1 表示到达事件, >=0 表示离开事件, 同时数值表示所离开的服务窗口
  int event_type;

  Event *next;

  // 默认为到达事件，发生事件随机
  Event(int occur_time = Random::uniform(Random_Parameter), int event_type = -1) :
      occur_time(occur_time), event_type(event_type), next(NULL) {}
};

#endif

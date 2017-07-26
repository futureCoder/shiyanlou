#ifnded QUEUESYSTEM_HPP
#define QUEUESYSTEM_HPP

#include "Event.hpp"
#include "Queue.hpp"
#include "ServiceWindow.hpp"

class QueueSystem {
public:
  //初始化队列系统
  QueueSystem(int total_service_time, int window_num);
  //销毁
  ~QueueSystem();
  //启动模拟
  void simulate(int simulate_num);

  inline double getAvgStayTime() const {
    return avg_stay_time;
  }

  inline double getAvgCustomers() const {
    return avg_customers;
  }
private:
  //系统运行一次
  double run();

  //初始化
  void init();

  //清空参数
  void end();

  //获得空闲窗口索引
  int getIdleServiceWindow();

  //处理顾客到达事件
  void customerArrived();

  //处理顾客离开事件
  void customerDeparture();

  int m_window_num; //服务窗口总数
  int total_service_time; //总营业时间
  int customer_stay_time  //顾客的逗留总时间
  int total_customer_num; //总顾客数

  //核心成员
  ServiceWindow* windows;
  Queue<Customer> customer_list;
  Queue<Event> event_list;
  Event* current_event;

  double avg_customers;
  double avg_stay_time;

};

#endif

#ifndef SERVICEWINDOW_HPP
#define SERVICEWINDOW_HPP

#include "Node.hpp"

enum WindowStatus {
  eService = 0,
  eIdle,
};

class ServiceWindow {
public:
  ServiceWindow() {
    m_window_status = eIdle;
  }
  bool isIdle() const {
    if(eIdle == m_window_status)
      return true;
    return false;
  }
  void serveCustomer(Customer &customer) {
    this->m_customer = customer;
  }
  void setBusy() {
    m_window_status = eService;
  }
  void setIdle() {
    m_window_status = eIdle;
  }
private:
  Customer m_customer;
  WindowStatus m_window_status;
};

#endif

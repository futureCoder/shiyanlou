#include "QueueSystem.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    std::srand(static_cast<unsigned>(std::time(0)));    //使用当前时间作为随机数种子
    const int total_service_time = 240;    //按分钟计算
    const int window_num = 4;
    const int simulate_num = 100000;       //模拟次数

    QueueSystem system(total_service_time, window_num);
    system.simulate(simulate_num);

    std::cout << "The average time of customer stay in bank: "
              << system.getAvgStayTime() << std::endl;
    std::cout << "The number of customer arrive bank per minute: "
              << system.getAvgCustomers() << std::endl;

    return 0;
}

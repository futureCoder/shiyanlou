#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstdlib>
#include <cmath>

class Random {
public:
  //[0, 1) 之间的服从均匀分布的随机值
  static double uniform(double max = 1) {
    return ((double)std::rand() / (RAND_MAX)) * max;
  }
};

#endif /* RANDOM_HPP */

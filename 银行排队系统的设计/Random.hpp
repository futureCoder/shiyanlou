#ifndef _RANDOM_HPP_
#define _RANDOM_HPP_
#include <cstdlib>
#include <random>
class Random {
    /*[0, 1) 之间的服从均匀分布的随机值*/
    /*std::rand() % n这个运算,若n - 1 的二进制表示不全都是由1组成,其生成的随机数
不会服从均匀分布*/
    static double uniform(double max = 1) {
        return (static_cast<double>(std::rand()) / (RAND_MAX) * max);
    }
};
#endif

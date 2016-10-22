#include <iostream>

#include "LeakDetector.hpp"


class Err {
public:
    Err(int n) {
        if(n == 0)
            throw 1000;
        data = new int[n];
    }
    ~Err() {
        delete[] data;
    }
private:
    int *data;
};
class A {
public:
    int a;
    A() {
        a = 10;
    }
};
int main() {
    int *a = new int;
    int *b = new int;
    A *pa = new A;
    delete a;
    std::cout << pa->a << std::endl;
    try {
        Err* e = new Err(0);
        delete e;
    } catch (int &ex) {
        std::cout << "Exception catch: " << ex << std::endl;
    };
    return 0;
}

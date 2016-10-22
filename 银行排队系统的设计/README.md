# C++ 从实现银行排队服务到 CPU 资源争夺模拟 - 银行排队系统的设计与建模
-----


## 一、概述

### 实验所需的前置知识

- C++ 基本语法知识

### 实验所巩固并运用的知识

- OOP 编程思想
- `std::rand()` 函数原理
- 概率编程
- 排队理论
- 链式队列数据结构及其模板实现
- 事件驱动的设计
- 蒙特卡洛方法
- CPU 资源争夺模型
- 时间片轮转调度

### 要解决的问题

蒙特卡洛方法这个名字听起来很高大上，但它的本质其实是使用计算机的方法对问题进行模拟和复现。本次实验将使用蒙特卡洛方法来模拟银行排队这个问题：

&gt; 端午节当天，某个银行从早上八点开始服务并只服务到中午十二点就停止营业。假设当天银行只提供了 w 个服务窗口进行服务，问：

&gt; 1. 平均每分钟有多少个顾客抵达银行?
&gt; 2. 平均每个顾客占用服务窗口的时间是多少？

我们先来分析一下这个业务的逻辑：

首先我们要分析银行提供服务的逻辑。在银行服务中，所有顾客都是通过取号排队的方式等待服务的，这和火车站买票有所不同，在火车站买票时，顾客必须在某一个窗口所排的队列下进行排队，且无法变更自己所属的窗口，否则只能从队尾重新排队。换句话说，对于银行提供的服务来说，所有用户都是位于同一个队列上的，当某个服务窗口可用时，才会从排队队列的队首取出一个新的用户来办理银行业务。即代码实现过程中，服务窗口可以创建 w 个，但只需要实现一个顾客队列即可。

其次，对于顾客而言，有两个属性是能够被抽象出来的：

1. 到达银行的时间；
2. 需要服务的时间。

并且，这两个属性是随机的。到此，我们整个的排队模型就变成了：

![此处输入图片的描述](https://dn-anything-about-doc.qbox.me/document-uid29879labid1881timestamp1467948138643.png/wm)

下面我们来详细对这个问题的实现逻辑进行分析，让我们的程序能够给出类似下面的结果：

![此处输入图片的描述](https://dn-anything-about-doc.qbox.me/document-uid29879labid1881timestamp1467948680863.png/wm)

## 一、计算机中的随机

### std::rand() 函数的原理

C++ 中的 `std::rand()` 函数产生的随机数并不是真正意义上的随机数，它并不服从数学上的均匀分布。为了使我们的模拟系统变得更加真实，我们需要知道 `std::rand()` 函数的原理。

`std::rand()` 生成的是一个随机的二进制序列（在硬件底层更好实现），这个序列的每一位0或者1的概率都是相等的。而对于 `std::rand()%n` 这个运算，会在 [0, n-1] 之间生成随机数，所以，如果 n-1 的二进制表示的值不都是由 1 组成，那么这里面的数是不会从均匀分布了（因为某些位可能不能为 1）。

所以，当且仅当 [0, n-1] 中的随机数可以用这个序列的子序列表示时，才能满足均匀分布。换句话说，仅当 n-1 的二进制数全为1 时，0，1出现的概率才是均等的。

我们先来实现随机这个类：

```c++
//
//  Random.hpp
//  QueueSystem
//

#ifndef Random_hpp
#define Random_hpp

#include &lt;cstdlib&gt;
#include &lt;cmath&gt;

class Random {
public:
    // [0, 1) 之间的服从均匀分布的随机值
    static double uniform(double max = 1) {
        return ((double)std::rand() / (RAND_MAX))*max;
    }
};
#endif /* Random_hpp */
```

这样的话，当我们调用 `Random::uniform()` 时，便能获得真正的服从均匀分布的随机数了。当指定参数后，便能够生成 [0, max) 之间的随机值了。


## 二、主函数逻辑设计

对于一个银行而言，对外界来说只需要提供两个参数：

1. 总共的服务时间
2. 服务窗口的数量

所以我们希望实现这样的代码：

```c++
//
// main.cpp
// QueueSystem
//

#include &#34;QueueSystem.hpp&#34;

#include &lt;iostream&gt;
#include &lt;cstdlib&gt;

int main() {
    
    std::srand((unsigned)std::time(0)); // 使用当前时间作为随机数种子

    int total_service_time = 240;       // 按分钟计算
    int window_num         = 4;
    int simulate_num       = 100000;    // 模拟次数
    
    QueueSystem system(total_service_time, window_num);
    system.simulate(simulate_num);
    
    std::cout &lt;&lt; &#34;The average time of customer stay in bank: &#34;
              &lt;&lt; system.getAvgStayTime() &lt;&lt; std::endl;
    std::cout &lt;&lt; &#34;The number of customer arrive bank per minute: &#34;
              &lt;&lt; system.getAvgCustomers() &lt;&lt; std::endl;
    
    return 0;
}
```

## 三、对象及逻辑设计

&gt; 总结一下，现在我们需要实现的东西有：

1. 服务窗口类(会被创建 w 个)
2. 顾客队列类(只会被创建一个)
3. 顾客结构(包含两个随机属性: 到达时间, 服务时间)

&gt; 为了更好练习 C++，我们会弃用诸如 vector 这些快捷编码的标准库来进行『过度编码』，自行编写模板类。


根据前面的问题描述，我们可以初步确定这样一些类的设计需求：

1. QueueSystem 类: 负责整个队列系统的模拟；
2. ServiceWindow 类: 队列系统的服务窗口对象，每当一个银行创建时，服务窗口会被创建，为了让整个问题更加灵活，我们假设需要创建 window_num 个窗口；
3. Queue 类: 银行队列系统的顾客排队的队列；
4. Random 类: 在第二节中已经讨论过。

然而，在设计 ServiceWindow 之前，我们要考虑 ServiceWindow 类到底要放置什么成员，首先，对于一个服务窗口，会有一个顾客属性，用于存放顾客。另一方面，一个窗口只会有两种状态：要么正在服务（被占用），要么空闲。因此 ServiceWindow 中首先会有下面的枚举：

```c++
//
// ServiceWindow.hpp
// QueueSystem
//

enum WindowStatus {
    SERVICE,
    IDLE,
};
```

既然我们要在 ServiceWindow 中存放顾客，由于顾客本身并不需要提供什么方法，因此可以直接将顾客设计为一个结构体 `Customer`，同时，顾客也会成为等待队列中的一员。所以，Customer 也可以被称之为队列的一个 `Node`，此外，每个顾客说需要的服务时间是随机的，但是到达时间并不应该由顾客自身确定（我们在下一节再讨论为什么），所以`Customer`结构的默认构造应该被设计出来：

```c++
//
//  Node.hpp
//  QueueSystem
//

#ifndef Node_hpp
#define Node_hpp

#include &#34;Random.hpp&#34;

#define RANDOM_PARAMETER 100

struct Node {
    int arrive_time;
    int duration;
    struct Node *next;
    
    // 默认到达事件为0，需要服务的事件是随机的
    Node(int arrive_time = 0,
         int duration = Random::uniform(RANDOM_PARAMETER)):
        arrive_time(arrive_time),
        duration(duration),
        next(NULL) {}
};

typedef struct Node Node;
typedef struct Node Customer;

#endif /* Node_h */

```

那么，结合前面的 `WindowStatus`枚举和 `Customer`结构，我们的 ServiceWindow 类可以这样设计，因为窗口本身涉及的操作还算是比较简单，比如设置窗口状态是否繁忙，获取当前服务顾客的到达时间来方便后续计算等等，因此我们直接将其设计成类内的 `inline` 函数：

```c++
//
//  ServiceWindow.hpp
//  QueueSystem
//

#ifndef ServiceWindow_hpp
#define ServiceWindow_hpp

#include &#34;Node.hpp&#34;

enum WindowStatus {
    SERVICE,
    IDLE,
};

class ServiceWindow {
public:
    inline ServiceWindow() {
        window_status = IDLE;
    };
    inline bool isIdle() const {
        if (window_status == IDLE) {
            return true;
        } else {
            return false;
        }
    }
    inline void serveCustomer(Customer &amp;customer) {
        this-&gt;customer = customer;
    }
    inline void setBusy() {
        window_status = SERVICE;
    }
    inline void setIdle() {
        window_status = IDLE;
    }
    inline int getCustomerArriveTime() const {
        return customer.arrive_time;
    }
    inline int getCustomerDuration() const {
        return customer.duration;
    }
private:
    Customer customer;
    WindowStatus window_status;
};

#endif /* ServiceWindow_hpp */
```

## 三、事件驱动的设计

有了上面的这些设计，似乎我们只要编写好用户排队队列，就已经足够描述整个排队的系统了，然而，在上面的设计中，还有一个很大的问题，那就是：整个系统还处于静止状态。当顾客位于等待队列时，窗口什么时候服务下一个顾客，如何处理这里面的逻辑，到目前为止，我们都没有思考过。

为了让整个系统『运行』起来，我们还要考虑整个系统的运行时间线。这里我们给出一种**事件驱动**的设计。

在前面的分析中，我们知道整个系统中，无非出现两种事件：

1. 有顾客到达
2. 有顾客离开

其中，第二种顾客离开的事件，同时还包含了窗口服务等待队列中的下一个顾客这个事件。所以，我们如果能够维护一个事件列表，那么就能够驱动整个队列系统的运行了。因为，当事件发生时，我们通知这个队列系统更新他自身的状态即可。

综上所述，我们可以先设计事件表中的事件结构：

```c++
//
//  Event.hpp
//  QueueSystem
//

#ifndef Event_hpp
#define Event_hpp

#include &#34;Random.hpp&#34;
#define RANDOM_PARAMETER 100

struct Event {
    int occur_time;
    
    // 使用 -1 表示到达事件, &gt;=0 表示离开事件, 同时数值表示所离开的服务窗口
    int event_type;
    
    Event* next;
    
    // 默认为到达事件，发生事件随机
    Event(int occur_time = Random::uniform(RANDOM_PARAMETER),
          int event_type = -1):
        occur_time(occur_time),
        event_type(event_type),
        next(NULL) {}
};

#endif /* Event_hpp */
```

这里我们使用了一个小小的 trick，那就是用整数来表示事件的类型，而不是简单的使用枚举。

这是因为，对于 ServiceWindow 来说，我们可以使用数组来管理多个 ServiceWindow，那么对应的事件类型如果涉及为整数，事件类型就可以同时作为 ServiceWindow 的索引下标了，当 `event_type` 大于等于 0 时，数值还表示离开的服务窗口。

又因为事件列表、顾客队列，本质上可以归类为同一个结构，那就是队列：只不过他们的入队方式有所差异，对于事件列表而言，入队方式必须按发生事件的时间顺序入队，而对于顾客，则是直接添加到队尾。考虑到了这一点，我们便能很容易的利用模板来设计队列的基本需求了：

```c++
//
//  Queue.hpp
//  QueueSystem
//

#ifndef Queue_hpp
#define Queue_hpp

#include &lt;iostream&gt;
#include &lt;cstdlib&gt;

#include &#34;Event.hpp&#34;

// 带头结点的队列
template &lt;typename T&gt;
class Queue
{
public:
    Queue();
    ~Queue();
    void clearQueue();             // 清空队列
    T* enqueue(T &amp;node);
    T* dequeue();
    T* orderEnqueue(Event &amp;event); // 只适用于事件入队
    int  length();
private:
    T *front;  // 头结点
    T *rear;   // 队尾
};
#endif /* Queue_hpp */
```

## 四、QueueSystem

经过前面的讨论，我们已经完成了对所有基本结构的设计，根据这些设计，我们能够初步确定我们要实现的队列系统的基本结构。

首先，根据对主函数的设计，初始化整个队列系统我们需要两个参数：

1. 银行的总服务时间(分钟) `int total_service_time`
2. 银行开放的服务窗口数 `int window_num`

其次，我们需要 `QueueSystem` 发开放至少三个接口：

1. 模拟 `simulate()`
2. 获得顾客平均逗留时间 `getAvgStayTime()`
3. 获得平均每分钟顾客数 `getAvgCustomers()`

第三，内部需要实现的内容包括：

1. 系统运行前的初始化 `init()`
2. 让系统运行的 `run()`
3. 系统结束一次运行的清理工作 `end()`

第四，整个系统需要管理的核心成员有：

1. 可供服务的窗口 `ServiceWindow* windows`
2. 顾客等待队列 `Queue&lt;Customer&gt; customer_list`
3. 事件列表 `Queue&lt;Event&gt; event_list`
4. 当前的系统事件 `Event* current_event`

第五，处理事件的方法：

1. 处理顾客到达事件 `void customerArrived()`
2. 处理顾客离开事件 `void customerDeparture()`

最后，我们所希望的平均顾客逗留时间和平均每分钟的顾客数涉及的四个变量：

1. 顾客的总逗留时间 `int total_customer_stay_time`
2. 一次运行中系统服务的中顾客数量 `int total_customer_num`
3. 每分钟平均顾客数 `double avg_customers`
4. 顾客平均逗留时间 `double avg_stay_time`

&gt; 事实上，可以预见的是，在处理顾客服务逻辑的时候，我们还需要一个方法 `getIdleServiceWindow` 来获取当前服务窗口的状态，从而增加代码的复用度。

所以，整个 QueueSystem 类的代码设计为：

```
//
//  QueueSystem.hpp
//  QueueSystem
//

#ifndef QueueSystem_hpp
#define QueueSystem_hpp

#include &#34;Event.hpp&#34;
#include &#34;Queue.hpp&#34;
#include &#34;ServiceWindow.hpp&#34;

class QueueSystem {
    
public:
    // 初始化队列系统
    QueueSystem(int total_service_time, int window_num);
    
    // 销毁
    ~QueueSystem();
    
    // 启动模拟
    void simulate(int simulate_num);
    
    inline double getAvgStayTime() const {
        return avg_stay_time;
    }
    inline double getAvgCustomers() const {
        return avg_customers;
    }
    
private:
    // 让队列系统运行一次
    double run();
    
    // 初始化各种参数
    void init();
    
    // 清空各种参数
    void end();
    
    // 获得空闲窗口索引
    int getIdleServiceWindow();
    
    // 处理顾客到达事件
    void customerArrived();
    
    // 处理顾客离开事件
    void customerDeparture();
    
    // 服务窗口的总数
    int window_num;
    
    // 总的营业时间
    int total_service_time;
    
    // 顾客的逗留总时间
    int customer_stay_time;
    
    // 总顾客数
    int total_customer_num;
    
    // 核心成员
    ServiceWindow*  windows;
    Queue&lt;Customer&gt; customer_list;
    Queue&lt;Event&gt;       event_list;
    Event*          current_event;
    
    // 给外部调用的结果
    double avg_customers;
    double avg_stay_time;

};


#endif /* QueueSystem_hpp */
```

## 本节回顾

在这一节中，我们设计了整个银行排队系统的基本逻辑，并借鉴了事件驱动的思想设计了驱动队列系统的事件类。本节中我们一共创建了：

1. `Event.hpp`
2. `Node.hpp`
3. `Queue.hpp`
4. `Random.hpp`
5. `ServiceWindow.hpp`
6. `QueueSystem.hpp`
7. `main.cpp`

现在我们的代码还不能够直接运行，本节我们先关注理清我们的业务逻辑。在下一节中，我们将实现这些代码的详细逻辑，这包括：

1. `Queue.hpp` 中模板链式队列的具体实现
2. `QueueSystem.cpp` 中的详细服务逻辑
3. `Random.hpp` 中更复杂的随机概率分布

在这些实现中，我们将进一步巩固下面的知识的运用：

1. C++ 类模板
2. 链式队列的数据结构
3. 概率编程

## 进一步阅读的参考

1. [排队理论](https://zh.wikipedia.org/zh/等候理論)
2. [随机过程](https://zh.wikipedia.org/zh/随机过程)
3. [事件驱动程序设计](https://zh.wikipedia.org/wiki/事件驅動程式設計)
#ifndef _THREADPOOL__H_
#define _THREADPOOL__H_
#include <vector>               // std::vector
#include <queue>                // std::queue
#include <memory>               // std::make_shared
#include <stdexcept>            // std::runtime_error
#include <thread>               // std::thread
#include <mutex>                // std::mutex,        std::unique_lock
#include <condition_variable>   // std::condition_variable
#include <future>               // std::future,       std::packaged_task
#include <functional>           // std::function,     std::bind
#include <utility>              // std::move,         std::forward
using namespace std;
class ThreadPool {
public:

	// 在线程池中创建 threads 个工作线程
	ThreadPool(size_t threads);

	// 向线程池中增加线程
	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>;

	~ThreadPool();
private:
	// 需要持续追踪线程来保证可以使用 join
	std::vector< std::thread > workers;
	// 任务队列
	std::queue< std::function<void()> > tasks;

	// 同步相关
	std::mutex queue_mutex;             // 互斥锁
	std::condition_variable condition;  // 互斥条件变量

	// 停止相关
	bool stop;
};

// 构造函数只负责启动一定数量的工作线程(worker)
inline ThreadPool::ThreadPool(size_t threads)
	: stop(false)
{
	// 启动 threads 数量的工作线程(worker)
	for (size_t i = 0; i<threads; ++i)
		workers.emplace_back(
			// 此处的 lambda 表达式捕获 this, 即线程池实例
			[this]
	{
		// 循环避免虚假唤醒
		for (;;)
		{
			// 定义函数对象的容器, 存储任意的返回类型为 void 参数表为空的函数
			std::function<void()> task;

			// 临界区
			{
				// 创建互斥锁
				std::unique_lock<std::mutex> lock(this->queue_mutex);

				// 阻塞当前线程, 直到 condition_variable 被唤醒
				this->condition.wait(lock,
					[this] { return this->stop || !this->tasks.empty(); });

				// 如果当前线程池已经结束且等待任务队列为空, 则应该直接返回
				if (this->stop && this->tasks.empty())
					return;

				// 否则就让任务队列的队首任务作为需要执行的任务出队
				task = std::move(this->tasks.front());
				this->tasks.pop();
			}

			// 执行当前任务
			task();
		}
	}
	);
}

// 销毁所有线程池中创建的线程
inline ThreadPool::~ThreadPool()
{
	// 临界区
	{
		// 创建互斥锁
		std::unique_lock<std::mutex> lock(queue_mutex);

		// 设置线程池状态
		stop = true;
	}

	// 通知所有等待线程
	condition.notify_all();

	// 使所有异步线程转为同步执行, 此处循环为 c++11 新提供的循环语法 for(value:values)
	for (std::thread &worker : workers)
		worker.join();
}

// 添加一个新的线程到线程池中
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
	// 推导任务返回类型
	using return_type = typename std::result_of<F(Args...)>::type;

	// 获得当前任务
	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	// 获得 std::future 对象以供实施线程同步
	std::future<return_type> res = task->get_future();

	// 临界区
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		// 禁止在线程池停止后加入新的线程
		if (stop)
			throw std::runtime_error("enqueue on stopped ThreadPool");

		// 将线程添加到执行任务队列中
		tasks.emplace([task] { (*task)(); });
	}

	// 通知一个正在等待的线程
	condition.notify_one();
	return res;
}

#endif // !_THREADPOOL__H_

#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <source_location>
#include <functional>
#include <queue>
#include <future>
#include <optional>

#define TIME_COUNT(name) time_counter name (std::source_location::current())

class time_counter
{
	std::chrono::steady_clock::time_point start, end;
	std::chrono::duration<float> period;
	std::optional<const std::source_location> location;

public:
	time_counter(const std::source_location& location) :location(location)
	{
		start = std::chrono::high_resolution_clock::now();
		period = std::chrono::duration<float>();
	}

	time_counter() :location()
	{
		start = std::chrono::high_resolution_clock::now();
		period = std::chrono::duration<float>();
	}

	~time_counter()
	{
		end = std::chrono::high_resolution_clock::now();
		period = end - start;
		if (location)
			std::cout << "\nTotal time for function \"" << (*location).function_name() << "\": " << period.count() << "s\n";
		else
			std::cout << "\nTotal time: " << period.count() << "s\n";
	}
};

class thread_pool
{
	#define THREADPOOL_MAX_NUM 50
	typedef std::function<void()> Task;

	std::vector<std::thread> _pool;
	std::atomic<int> _num;
	std::queue<Task> _Task;
	std::mutex _mtx;
	std::condition_variable _task_cv;
	std::atomic<bool> _using;

public:
	thread_pool() :_num(10), _Task(), _using(true), _mtx(), _task_cv() { add_thread(10); };
	~thread_pool()
	{
		_using = false;
		_task_cv.notify_all();
		for (auto& thr : _pool) {
			if (thr.joinable())
				thr.join();
		}
	}
	void add_thread(int num)
	{
		for (int i = 0; i < num; i++) {
			_pool.emplace_back([this]()->void {
				Task task;
				while (_using) {
					{
						std::unique_lock<std::mutex>locker(_mtx);
						_task_cv.wait(locker, [this]()->bool {return !_using || !_Task.empty(); });

						if (!_using && _Task.empty())
							return;

						task = std::move(_Task.front());
						_Task.pop();
					}
					_num--;
					task();
					_num++;
				}
			});
		}
	}
	template<class F, class... Args>
	auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
		if (!_using)
			throw std::runtime_error("This thread pool is not working.");

		auto task = std::make_shared<std::packaged_task<decltype(f(args...))()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<decltype(f(args...))> future = task->get_future();
		{
			std::lock_guard<std::mutex> lock{ _mtx };
			_Task.emplace([task]() {
				(*task)();
				});
		}

		if (_num < 1 && _pool.size() < THREADPOOL_MAX_NUM)
			add_thread(1);

		_task_cv.notify_one();

		return future;
	}
};
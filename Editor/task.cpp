#include "task.h"

#include <cassert>
#include <memory>
#include <thread>

#include <rigtorp/SPSCQueue.h>

namespace
{

constexpr size_t task_queue_size = 10;

class TaskManager final
{
public:
	TaskManager() :
		_output_thread_id(std::this_thread::get_id()),
		_input_queue(task_queue_size),
		_output_queue(task_queue_size)
	{
		_thread = std::thread([this](){
			for (;;)
			{
				while (!_input_queue.front())
				{
					std::this_thread::yield();
				}

				auto task = *_input_queue.front();
				_input_queue.pop();

				if (!task.run)
				{
					break;
				}
				task.result = task.run();
				_output_queue.emplace(task);
			}
		});
	}

	~TaskManager()
	{
	}

	void Stop()
	{
		Run(editor::task::Task());
		if (_thread.joinable())
		{
			_thread.join();
		}
	}

	bool Run(editor::task::Task task)
	{
		assert(std::this_thread::get_id() == _output_thread_id);
		return _input_queue.try_emplace(task);
	}

	void FinishTasks()
	{
		assert(std::this_thread::get_id() == _output_thread_id);
		while (_output_queue.front())
		{
			auto task = *_output_queue.front();
			_output_queue.pop();
			task.finish(task.result);
		}
	}

private:
	const std::thread::id _output_thread_id;
	std::thread _thread;
	rigtorp::SPSCQueue<editor::task::Task> _input_queue;
	rigtorp::SPSCQueue<editor::task::Task> _output_queue;
};

std::unique_ptr<TaskManager> task_manager;

}

void editor::task::StartTaskManager()
{
	assert(task_manager == nullptr);
	task_manager = std::make_unique<TaskManager>();
}

void editor::task::StopTaskManager()
{
	assert(task_manager != nullptr);
	task_manager->Stop();
}

bool editor::task::Run(editor::task::Task task)
{
	assert(task_manager != nullptr);
	return task_manager->Run(std::move(task));
}

void editor::task::FinishTasks()
{
	assert(task_manager != nullptr);
	task_manager->FinishTasks();
}

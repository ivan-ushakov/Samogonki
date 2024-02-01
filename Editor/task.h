#pragma once

#include <functional>

namespace editor::task
{

struct Task
{
	std::function<void *()> run;
	std::function<void (void *)> finish;
	void *result = nullptr;
};

void StartTaskManager();
void StopTaskManager();
bool Run(Task task);
void FinishTasks();

}

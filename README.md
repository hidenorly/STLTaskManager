# STLTaskManager

This is utility class library to use std::thread with C++ classs easier.
It's based on STL11++ (std::thread, mutex).

What you need to do is to delive ```CTask``` class and override onExecute and the instance to ```CTaskManager``` with ```addTask()```.

In general, you need to use global function or static function for ```std::thread``` (not class method).
But this utility class enables you to use class method for threading.

You can easily manage the threading on multi platforms with this utility class.

# How to use

```
#include "CTaskManager.h"

#include <iostream>
#include <thread>
#include <chrono>

class CMyTask : public CTask
{
	virtual void onExecute(void)
	{
		std::cout << "CMyTask is running..." << std::endl;
		for(int i=0; i<1000; i++){
			volatile int j = i * i;
			if ( mStopRunning ) break;
		}
	}
};

int main() {
	CTaskManager taskMan(4); // using 4 threads
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.executeAllTasks();

	std::this_thread::sleep_for(std::chrono::microseconds(1000*1000)); // 1sec
	taskMan.finalize();

	return 0;
}
```

You can use ```cancelTask(CTask*)```, ```stopAllTasks()```.
In your ```onExecute()``` delived from Class, you can refer to ```mStopRunning``` for checking your thread is needed to stop or not.

# Confirmed environment

* CLion 2016.3 EAP, Build #CL-163.7743.15 on MBP with OSX 10.11.6

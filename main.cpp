/*
 Copyright (C) 2016 hidenorly

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include <iostream>
#include "CTaskManager.h"
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
			std::this_thread::sleep_for(std::chrono::microseconds(1000));
		}
	}
};

int main() {
	std::cout << "Hello, World!" << std::endl;

	CTaskManager taskMan;
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.addTask(new CMyTask());
	taskMan.executeAllTasks();

	std::this_thread::sleep_for(std::chrono::microseconds(1000*1000)); // 1sec
	taskMan.finalize();

	return 0;
}
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


#ifndef TASKMANAGER_CTASKMANAGER_H
#define TASKMANAGER_CTASKMANAGER_H

#include <vector>
#include <mutex>
#include <thread>
#include <map>

class CTaskManager;

class CTask
{
public:
	CTask(void):isRunning(false),mStopRunning(false){};
	virtual ~CTask(void){};

	// for override
	virtual void onExecute(void)=0;
	virtual void onComplete(void){};

	// for task manager
public:
	static void execute(CTask *pTask);
	virtual void cancel(void);

public:
	std::atomic<bool> isRunning;
	CTaskManager* pTaskManager = NULL;

protected:
	void _onComplete(void);

protected:
	std::atomic<bool> mStopRunning;
};

class CTaskManager
{
public:
	CTaskManager(int nMaxThread=4);
	virtual ~CTaskManager();

public:
	void addTask(CTask* pTask);
	void cancelTask(CTask* pTask);
	void executeAllTasks(void);
	void stopAllTasks(void);
	bool isRunning(void);
	bool isRemainingTasks(void);
	void finalize(void);

	void _onTaskCompletion(CTask* pTask);

protected:
	int mMaxThread;
	std::atomic<int> mNumOfRunningTasks;
	std::atomic<bool> mStopping;

	std::vector<CTask*> mTasks;
	std::map<CTask*,std::thread> mThreads;
	std::mutex mMutexTasks;
	std::mutex mMutexThreads;
};


#endif //TASKMANAGER_CTASKMANAGER_H

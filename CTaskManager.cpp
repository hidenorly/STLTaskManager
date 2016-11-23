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


#include "CTaskManager.h"
#include <chrono>

#include <iostream>

// -- CTask
void CTask::execute(CTask *pTask)
{
	pTask->mStopRunning = false;
	pTask->isRunning = true;
		pTask->onExecute();
		pTask->onComplete();
		pTask->_onComplete();
	pTask->isRunning = false;
	pTask->mStopRunning = false;
}

void CTask::cancel(void)
{
	if( isRunning ) {
		mStopRunning = true;
	}
}

void CTask::_onComplete(void)
{
	pTaskManager->_onTaskCompletion(this);
}


// --- CTaskManager
CTaskManager::CTaskManager(int nMaxThread):mMaxThread(nMaxThread),mNumOfRunningTasks(0),mStopping(false)
{

}

CTaskManager::~CTaskManager()
{

}

void CTaskManager::addTask(CTask *pTask)
{
	mMutexTasks.lock();
	{
		pTask->pTaskManager=this;
		mTasks.push_back(pTask);
	}
	mMutexTasks.unlock();
}

void CTaskManager::cancelTask(CTask *pTask)
{
	if (pTask) {
		// cancel notify & wait & remove the task from mThreads
		mMutexThreads.lock();
		{
			std::map<CTask *, std::thread>::iterator pItThreadTask = mThreads.find(pTask);
			// if the task is managed by mThreads
			if (pItThreadTask != mThreads.end()) {
				// try to stop task if running
				if (pTask->isRunning) {
					pTask->cancel();
					// try to wait the task thread finished.
					if (mThreads[pTask].joinable()) {
						mThreads[pTask].detach();//join();
					}
					// remove the task from task-thread pool
					mThreads.erase(pItThreadTask);
				}
			}
		}
		mMutexThreads.unlock();

		// remove the task canceled/finished task
		mMutexTasks.lock();
		{
			// remove current task from mTasks
			for (std::vector<CTask *>::iterator it = mTasks.begin(); it != mTasks.end(); it++) {
				CTask *pCurTask = *it;
				if (pCurTask == pTask) {
					it = mTasks.erase(it);
					break;
				}
			}
		}
		mMutexTasks.unlock();

		delete pTask;
		pTask = NULL;
	}
}

void CTaskManager::executeAllTasks(void)
{
	mStopping = false;
	if( mNumOfRunningTasks<mMaxThread ) {
		// enumerate candidate task to run
		std::vector<CTask*> candidateTasks;
		mMutexTasks.lock();
		{
			for (std::vector<CTask *>::iterator it = mTasks.begin();
				 it != mTasks.end() && mNumOfRunningTasks < mMaxThread; it++) {
				CTask *pTask = *it;
				if (!pTask->isRunning) {
					candidateTasks.push_back(pTask);
					mNumOfRunningTasks++;
				}
			}
		}
		mMutexTasks.unlock();

		// execute candidate tasks
		mMutexThreads.lock();
		{
//			for (std::vector<CTask *>::iterator it = candidateTasks.begin(); it != candidateTasks.end(); it++) {
			for( auto& pTask : candidateTasks ) {
				mThreads.insert( std::map<CTask*, std::thread>::value_type( pTask, std::thread(&CTask::execute, pTask)) );
			}
		}
		mMutexThreads.unlock();
	}
}

void CTaskManager::stopAllTasks(void)
{
	mStopping = true;
	while( isRunning() ) {
		mMutexTasks.lock();
		{
			for (auto &aTask : mTasks) {
				if (aTask->isRunning) {
					aTask->cancel();
				}
			}
		}
		mMutexTasks.unlock();
		std::this_thread::sleep_for(std::chrono::microseconds(1000)); // 1 msec
	}
}

void CTaskManager::_onTaskCompletion(CTask *pTask)
{
	if( pTask ) {
		CTaskManager *pTaskManager = (CTaskManager *) pTask->pTaskManager;
		if( pTaskManager ) {
			pTaskManager->cancelTask(pTask);
			if( !pTaskManager->mStopping) {
				pTaskManager->executeAllTasks();
			}
		}
	}
}

bool CTaskManager::isRunning(void)
{
	bool bRunning = false;
	mMutexTasks.lock();
	{
		for ( auto& pTask : mTasks ) {
			bRunning |= pTask->isRunning;
			if (bRunning) break;
		}
	}
	mMutexTasks.unlock();
	return bRunning;
}

bool CTaskManager::isRemainingTasks(void)
{
	return !mTasks.empty();
}

void CTaskManager::finalize(void)
{
	stopAllTasks();

	// remove all remaining tasks
	mMutexTasks.lock();
	{
		for( auto& pTask : mTasks ){
			delete pTask;
		}
		mTasks.clear();
	}
	mMutexTasks.unlock();
}

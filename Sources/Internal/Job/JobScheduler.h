/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_JOB_SCHEDULER_H__
#define __DAVAENGINE_JOB_SCHEDULER_H__

#include "Base/BaseTypes.h"
#include "Platform/Mutex.h"
#include "Base/Singleton.h"
#include "Job/JobManager.h"

namespace DAVA
{

class Job;
class WorkerThread;
class TaggedWorkerJobsWaiter;

class JobScheduler : public Singleton<JobScheduler>
{
public:
    JobScheduler(int32 workerThreadsCount);
    ~JobScheduler();
    
    void PushJob(Job * job); //0...MAX_TAG_VALUE
    void PushIdleThread(WorkerThread * thread);
    void Schedule();
    void OnJobCompleted(Job * job);
    
    JobManager::eWaiterRegistrationResult RegisterWaiterAndWait(TaggedWorkerJobsWaiter * waiter);
    void UnregisterWaiter(TaggedWorkerJobsWaiter * waiter);

    Mutex & GetWaiterMutex();

    int32 GetJobsCountForTag(int32 tag);
    
    int32 GetThreadsCount();
    
private:
    const int32 workerThreadsCount;
    Mutex scheduleMutex;
    
    List<Job*> jobQueue;
    Mutex jobQueueMutex;
    Job * PopJob();
    
    Vector<WorkerThread*> workerThreads;
    
    List<WorkerThread*> idleThreads;
    Mutex idleThreadsMutex;
    WorkerThread * PopIdleThread();

    static const int32 MAX_TAG_VALUE = 999;
    Vector<int32> taggedJobsCount;
    
    Map<int32, TaggedWorkerJobsWaiter*> taggedJobsWaiters;
    Mutex waiterMutex;
};

inline Mutex & JobScheduler::GetWaiterMutex()
{
    return waiterMutex;
}

inline int32 JobScheduler::GetJobsCountForTag(int32 tag)
{
    return taggedJobsCount[tag];
}

inline int32 JobScheduler::GetThreadsCount()
{
    return workerThreadsCount;
}

}

#endif //__DAVAENGINE_JOB_SCHEDULER_H__
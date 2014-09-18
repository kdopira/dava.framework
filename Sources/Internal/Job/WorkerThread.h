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

#ifndef __DAVAENGINE_WORKER_THREAD_H__
#define __DAVAENGINE_WORKER_THREAD_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"

namespace DAVA
{

class Job;
class JobScheduler;

class WorkerThread
{
public:
    WorkerThread(JobScheduler * scheduler);
    ~WorkerThread();
    
    void SetActiveJob(Job * job);
    void Wake();
    void Stop();
    Mutex * GetMutex();
    
private:
    void ThreadFunc(BaseObject * bo, void * userParam, void * callerParam);
    bool needStop;
    
    JobScheduler * scheduler;
    Thread * thread;
    ConditionalVariable cv;
    Mutex mutex;
    
    Job * activeJob;
};

inline void WorkerThread::SetActiveJob(Job * job)
{
    DVASSERT(activeJob == 0);
    activeJob = job;
}

inline void WorkerThread::Wake()
{
    mutex.Lock();
    Thread::Signal(&cv);
    mutex.Unlock();
}

inline Mutex * WorkerThread::GetMutex()
{
    return &mutex;
}
    
}

#endif //__DAVAENGINE_WORKER_THREAD_H__
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

#include "Job/JobWaiter.h"
#include "Job/JobManager.h"

namespace DAVA
{

ThreadIdJobWaiter::ThreadIdJobWaiter(Thread::Id _threadId/* = Thread::GetCurrentId()*/)
:	threadId(_threadId)
{
	

}

ThreadIdJobWaiter::~ThreadIdJobWaiter()
{
	JobManager::Instance()->UnregisterWaiterForCreatorThread(this);
}

void ThreadIdJobWaiter::Wait()
{
	if(JobManager::WAITER_WILL_WAIT == JobManager::Instance()->RegisterWaiterForCreatorThread(this))
	{
        Mutex mutex;
        mutex.Lock();
		Thread::Wait(&cv, &mutex);
	}
}

JobInstanceWaiter::JobInstanceWaiter(Job * _job)
:	job(_job)
{

}

JobInstanceWaiter::~JobInstanceWaiter()
{
	JobManager::Instance()->UnregisterWaiterForJobInstance(this);
}

void JobInstanceWaiter::Wait()
{
	if(JobManager::WAITER_WILL_WAIT == JobManager::Instance()->RegisterWaiterForJobInstance(this))
	{
        Mutex mutex;
        mutex.Lock();
		Thread::Wait(&cv, &mutex);
	}
}

//////
TaggedWorkerJobsWaiter::TaggedWorkerJobsWaiter(int32 _tag)
    :   tag(_tag)
{
}

TaggedWorkerJobsWaiter::~TaggedWorkerJobsWaiter()
{
//    JobScheduler::Instance()->UnregisterWaiter(this);
}

void TaggedWorkerJobsWaiter::Wait()
{
//    if(JobManager::WAITER_WILL_WAIT == JobScheduler::Instance()->RegisterWaiterAndWait(this))
    {
        Thread::Wait(&cv, &mutex);
        mutex.Unlock();
    }
}



}
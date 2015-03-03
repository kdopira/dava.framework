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



#include "ThreadSyncTest.h"

ThreadSyncTest::ThreadSyncTest()
: TestTemplate<ThreadSyncTest>("ThreadSyncTest")
{
    RegisterFunction(this, &ThreadSyncTest::ThreadSyncTestFunction, "ThreadSyncTestFunction", NULL);
    RegisterFunction(this, &ThreadSyncTest::ThreadSleepTestFunction, "ThreadSleepTestFunction", NULL);
    RegisterFunction(this, &ThreadSyncTest::TestThread, "TestThread", NULL);
}

void ThreadSyncTest::LoadResources()
{

}

void ThreadSyncTest::UnloadResources()
{

}

void ThreadSyncTest::SomeThreadFunc(BaseObject * caller, void * callerData, void * userData)
{
    someValue = 0;
    cvMutex.Lock();
    Thread::Signal(&cv);
    cvMutex.Unlock();
}

void ThreadSyncTest::ThreadSleepTestFunction(PerfFuncData * data)
{
    uint64 time = SystemTimer::Instance()->AbsoluteMS();
    Thread::Sleep(300);
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS() - time;
    //elapsed time can be rounded to lowest, so -1 here
    TEST_VERIFY(elapsedTime >= 299);
}

void ThreadSyncTest::ThreadSyncTestFunction(PerfFuncData * data)
{
    cvMutex.Lock();
    someThread = Thread::Create(Message(this, &ThreadSyncTest::SomeThreadFunc));
    someValue = -1;
    someThread->Start();
    Thread::Wait(&cv, &cvMutex);
    cvMutex.Unlock();

    TEST_VERIFY(someValue == 0);
    someThread->Join();
    TEST_VERIFY(0 == someThread->Release());
    someThread = NULL;
}

void ThreadSyncTest::InfiniteThreadFunction(BaseObject * caller, void * callerData, void * userData)
{
    Thread *thread = static_cast<Thread *>(caller);
    while(thread && !thread->IsCancelling())
    {
        Thread::Sleep(200);
    }
    
}

void ThreadSyncTest::ShortThreadFunction(BaseObject * caller, void * callerData, void * userData)
{
    uint32 i = 200;
    Thread *thread = static_cast<Thread *>(caller);
    while(thread && i-- > 0)
    {
        Thread::Sleep(1);
        if (thread->IsCancelling())
            break;
    }
}

void ThreadSyncTest::TestThread(PerfFuncData * data)
{
	Logger::Debug("[ThreadSyncTest] testing threads logic");

    TEST_VERIFY(true == Thread::IsMainThread());

    Thread *infiniteThread = Thread::Create(Message(this, &ThreadSyncTest::InfiniteThreadFunction));
    
    TEST_VERIFY(Thread::STATE_CREATED == infiniteThread->GetState());
    infiniteThread->SetName("Infinite test thread");
    infiniteThread->Start();
    
    uint32 timeout = 3000;
    while (timeout-- > 0 && Thread::STATE_RUNNING != infiniteThread->GetState())
    {
        Thread::Sleep(1);
    }
    TEST_VERIFY(timeout > 0);

    infiniteThread->Cancel();
    infiniteThread->Join();
    TEST_VERIFY(Thread::STATE_ENDED == infiniteThread->GetState());

    Thread *shortThread = Thread::Create(Message(this, &ThreadSyncTest::ShortThreadFunction));
    shortThread->Start();
    shortThread->Join();
    TEST_VERIFY(Thread::STATE_ENDED == shortThread->GetState());

    TEST_VERIFY(0 == shortThread->Release());
    shortThread = NULL;
    TEST_VERIFY(0 == infiniteThread->Release());
    infiniteThread = NULL;
/*
    //  Disable because it affects all threads including JobManager threads and worker threads,
    //  so this test is intrusive
    infiniteThread = Thread::Create(Message(this, &ThreadSyncTest::InfiniteThreadFunction));
    shortThread = Thread::Create(Message(this, &ThreadSyncTest::ShortThreadFunction));

    infiniteThread->Start();
    shortThread->Start();

    Thread::Sleep(50);
    timeout = 3000;
    while (timeout-- > 0
        && Thread::STATE_RUNNING != infiniteThread->GetState()
        && Thread::STATE_RUNNING != shortThread->GetState())
    {
        Thread::Sleep(1);
    }

    Thread::Id itid = infiniteThread->GetId();
    Thread::Id stid = shortThread->GetId();
    TEST_VERIFY(itid != stid);

    infiniteThread->Kill();
    TEST_VERIFY(0 == shortThread->Release());

    shortThread->Kill();
    TEST_VERIFY(0 == shortThread->Release());

    Thread::KillAll();
    
    shortThread->Join();
    TEST_VERIFY(Thread::STATE_KILLED == shortThread->GetState());

    infiniteThread->Join();
    TEST_VERIFY(Thread::STATE_KILLED == infiniteThread->GetState());

    TEST_VERIFY(0 != shortThread);
    TEST_VERIFY(0 == shortThread->Release());
    shortThread = NULL;

    TEST_VERIFY(0 == infiniteThread->Release());
    infiniteThread = NULL;

	Logger::Debug("[ThreadSyncTest] Done.");
*/
}




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

#include "FWSpinlock.h"
#include "Thread.h"

namespace DAVA
{
    
#if defined(__DAVAENGINE_WIN32__)
    Spinlock::Spinlock() : spin(0)
    { }
    
    Spinlock::~Spinlock()
    { }
    
    void Spinlock::Lock()
    {
        while(!AtomicCompareAndSwap(0, 1, spin))
        {
			YieldProcessor();
        }
    }
    
    void Spinlock::Unlock()
    {
        AtomicCompareAndSwap(1, 0, spin);
    }
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    Spinlock::Spinlock() : spin(0)
    { }
    
    Spinlock::~Spinlock()
    { }
    
    void Spinlock::Lock()
    {
        OSSpinLockLock(&spin);
    }
    
    void Spinlock::Unlock()
    {
        OSSpinLockUnlock(&spin);
    }

#elif defined(__DAVAENGINE_ANDROID__)
    Spinlock::Spinlock() : spin(0)
    {
		pthread_mutex_init(&spinMutex, 0);
	}

    Spinlock::~Spinlock()
    { 
		pthread_mutex_destroy(&spinMutex);
	}

    void Spinlock::Lock()
    {
    	//static int volatile a = 0;
        //while(!AtomicCompareAndSwap(0, 1, spin))
        //{
        //	a = a | 1;
        //}
		pthread_mutex_lock(&spinMutex);
    }

    void Spinlock::Unlock()
    {
    	//AtomicCompareAndSwap(1, 0, spin);
		pthread_mutex_unlock(&spinMutex);
	}
#endif //PLATFORMS


};
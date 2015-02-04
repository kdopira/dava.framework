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


#ifndef __RHI_POOL_H__
#define __RHI_POOL_H__

#include "../rhi_Base.h"

namespace rhi
{

enum
{
    HANDLE_INDEX_MASK       = 0x0000FFFFU,
    HANDLE_INDEX_SHIFT      = 0,

    HANDLE_GENERATION_MASK  = 0x00FF0000U,
    HANDLE_GENERATION_SHIFT = 16,
    
    HANDLE_TYPE_MASK        = 0xFF000000U,
    HANDLE_TYPE_SHIFT       = 24,

    HANDLE_FORCEUINT32      = 0xFFFFFFFFU 
};


template <class T, ResourceType RT>
class
Pool
{
public:

    static Handle   Alloc();
    static void     Free( Handle h );

    static T*       Get( Handle h );


private:

    struct
    Entry
    {
        T       object;
        uint32  allocated:1;
        uint32  generation:8;
    };

    static Entry*   Object;
    static unsigned ObjectCount;
};

#define RHI_IMPL_POOL(T,RT) \
template<> rhi::Pool<T,RT>::Entry*  rhi::Pool<T,RT>::Object      = 0; \
template<> unsigned                 rhi::Pool<T,RT>::ObjectCount = 1024; \


//------------------------------------------------------------------------------

template <class T, ResourceType RT>
inline Handle
Pool<T,RT>::Alloc()
{
    uint32  handle = InvalidHandle;

    if( !Object )
    {
        Object = new Entry[ObjectCount];
        
        for( Entry* e=Object,*e_end=Object+ObjectCount; e!=e_end; ++e )
        {
            e->allocated  = false;
            e->generation = 0;
        }
    }

    for( Entry* e=Object,*e_end=Object+ObjectCount; e!=e_end; ++e )
    {
        if( !e->allocated )
        {
            e->allocated = true;
            ++e->generation;
            
            handle = 0;
            handle = ((uint32(e-Object)<<HANDLE_INDEX_SHIFT) & HANDLE_INDEX_MASK) | 
                     (((e->generation)<<HANDLE_GENERATION_SHIFT)&HANDLE_GENERATION_MASK) |
                     ((RT<<HANDLE_TYPE_SHIFT)&HANDLE_TYPE_MASK);
            break;
        }
    }
    
    return handle;
}


//------------------------------------------------------------------------------

template <class T, ResourceType RT>
inline void
Pool<T,RT>::Free( Handle h )
{
    uint32  index = (h&HANDLE_INDEX_MASK) >> HANDLE_INDEX_SHIFT;    
    uint32  type  = (h&HANDLE_TYPE_MASK) >> HANDLE_TYPE_SHIFT;    
    DVASSERT(type == RT);
    DVASSERT(index < ObjectCount);
    Entry*  e     = Object + index;

    DVASSERT(e->allocated);
    e->allocated = false;
}


//------------------------------------------------------------------------------

template <class T, ResourceType RT>
inline T*
Pool<T,RT>::Get( Handle h )
{
    T*  object = 0;

    if( h != InvalidHandle )
    {
        uint32  type   = (h&HANDLE_TYPE_MASK) >> HANDLE_TYPE_SHIFT;
        DVASSERT(type == RT);
        uint32  index  = (h&HANDLE_INDEX_MASK)>>HANDLE_INDEX_SHIFT;
        uint32  gen    = (h&HANDLE_GENERATION_MASK)>>HANDLE_GENERATION_SHIFT;
        DVASSERT(index < ObjectCount);
        Entry*  e     = Object + index;

        DVASSERT(e->allocated);
        DVASSERT(e->generation == gen);

        object = &(e->object);
    }

    return object;
}



} // namespace rhi
#endif // __RHI_POOL_H__
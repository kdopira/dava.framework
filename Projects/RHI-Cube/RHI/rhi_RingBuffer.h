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


#ifndef __RHI_RINGBUFFER_H__
#define __RHI_RINGBUFFER_H__

    #include "../rhi_Type.h"


namespace rhi
{

class
RingBuffer
{
public:
                RingBuffer();

    void        Initialize( unsigned sz );
    void        Uninitialize();

    float*      Alloc( unsigned cnt );
    void        Reset();
    

private:

    unsigned    size;
    uint8*      dataPtr;
    uint8*      cur;

    unsigned    memUsed;
    unsigned    allocCount;
};


//------------------------------------------------------------------------------

inline
RingBuffer::RingBuffer()
  : size(0),
    dataPtr(0),
    cur(0),
    memUsed(0)
{
}


//------------------------------------------------------------------------------

inline void
RingBuffer::Initialize( unsigned sz )
{
    size        = sz;
    dataPtr     = (uint8*)::malloc( sz );    

    cur         = dataPtr;
    memUsed     = 0;
    allocCount  = 0;
}


//------------------------------------------------------------------------------

inline void        
RingBuffer::Uninitialize()
{
    if( dataPtr )
        ::free( dataPtr );

    size    = 0;
    dataPtr = 0;
    cur     = 0;
}


//------------------------------------------------------------------------------

inline float*
RingBuffer::Alloc( unsigned cnt )
{
    DVASSERT(cur);

    unsigned    sz  = L_ALIGNED_SIZE(cnt*sizeof(float),16);
    uint8*      buf = cur + sz;
    uint8*      p   = cur;

    if( buf > dataPtr + size )
    {
        buf = dataPtr + sz;
        p   = dataPtr;
    }

    cur         = buf;
    memUsed    += sz;
    ++allocCount;

    return (float*)p;
}


//------------------------------------------------------------------------------

inline void
RingBuffer::Reset()
{
    memUsed    = 0;
    allocCount = 0;
}


} // namespace rhi
#endif // __RHI_RINGBUFFER_H__
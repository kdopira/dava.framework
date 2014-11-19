/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include <cstdio>

#include <libuv/uv.h>

#include <Debug/DVAssert.h>

#include "Endpoint.h"

namespace DAVA
{

Endpoint::Endpoint(const char8* address, uint16 port) : data()
{
    InitSockaddrIn(IPAddress::FromString(address).ToUInt(), port);
}

Endpoint::Endpoint(const sockaddr* sa)
{
    DVASSERT(sa);
    Memcpy(&data, sa, sizeof(data));
}

Endpoint::Endpoint(const sockaddr_in* sin)
{
    DVASSERT(sin);
    Memcpy(&data, sin, sizeof(data));
}

bool Endpoint::ToString(char8* buffer, std::size_t size) const
{
    DVASSERT(buffer != NULL && size > 0);
    if(Address().ToString(buffer, size))
    {
        char port[20];
#ifdef __DAVAENGINE_WIN32__
        _snprintf_s(port, COUNT_OF(port), _TRUNCATE, ":%hu", Port());
#else   // __DAVAENGINE_WIN32__
        snprintf(port, COUNT_OF(port), ":%hu", Port());
#endif  // __DAVAENGINE_WIN32__

        std::size_t addrLen = strlen(buffer);
        std::size_t portLen = strlen(port);
        if(addrLen + portLen < size)
        {
            strcat(buffer + addrLen, port);
            return true;
        }
    }
    return false;
}

String Endpoint::ToString() const
{
    char8 buf[50];
    if(ToString(buf, COUNT_OF(buf)))
        return String(buf);
    return String();
}

void Endpoint::InitSockaddrIn(uint32 addr, uint16 port)
{
    data.sin_family = AF_INET;
    data.sin_port   = htons(port);
#ifdef __DAVAENGINE_WIN32__
    data.sin_addr.S_un.S_addr = htonl(addr);
#else   // __DAVAENGINE_WIN32__
    data.sin_addr.s_addr      = htonl(addr);
#endif  // __DAVAENGINE_WIN32__
}

uint32 Endpoint::GetSockaddrAddr() const
{
#ifdef __DAVAENGINE_WIN32__
    return ntohl(data.sin_addr.S_un.S_addr);
#else   // __DAVAENGINE_WIN32__
    return ntohl(data.sin_addr.s_addr);
#endif  // __DAVAENGINE_WIN32__
}

}   // namespace DAVA
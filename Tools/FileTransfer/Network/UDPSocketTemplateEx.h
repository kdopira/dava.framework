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

#ifndef __DAVAENGINE_UDPSocketTemplateEx_H__
#define __DAVAENGINE_UDPSocketTemplateEx_H__

#include "Endpoint.h"
#include "Buffer.h"
#include "UDPSocketBase.h"

namespace DAVA
{

class IOLoop;

/*
 Template class UDPSocketTemplateEx provides basic capabilities: reading from and sending to datagram socket
 Template parameter T specifies type that inherits UDPSocketTemplateEx(CRTP idiom)
 Bool template parameter autoRead specifies read behaviour:
    when autoRead is true libuv automatically issues next read operations until StopAsyncReceive is called
    when autoRead is false user should explicitly issue next read operation
 Multiple simultaneous read operations lead to undefined behaviour.

 Type specified by T should implement methods:
    void HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial)
        This method is called after data with length of nread bytes has been arrived
        Parameter error is non zero on error
        Parameter partial is true when read buffer was too small and rest of the message was discarded by OS
    template<typename SendRequestType>
    void HandleSend(SendRequestType* request, int32 error)
        This method is called after data has been sent
    void HandleClose()
        This method is called after underlying socket has been closed by libuv

 Summary of methods that should be implemented by T:
    void HandleReceive(int32 error, std::size_t nread, const uv_buf_t* buffer, const Endpoint& endpoint, bool partial);
    template<typename SendRequestType>
    void HandleSend(SendRequestType* request, int32 error);
    void HandleClose();
*/
template <typename T>
class UDPSocketTemplateEx : public UDPSocketBase
{
public:
    typedef UDPSocketBase BaseClassType;
    typedef T             DerivedClassType;

protected:
    /*struct SendRequestBase
    {
        DerivedClassType* pthis;
        uv_buf_t          buffer;
        uv_udp_send_t     request;
    };*/

    static const std::size_t MAX_WRITE_BUFFERS = 10;

    struct SendRequest
    {
        uv_udp_send_t     request;
        DerivedClassType* pthis;
        Buffer            buffers[MAX_WRITE_BUFFERS];
        std::size_t       bufferCount;
    };

public:
    explicit UDPSocketTemplateEx(IOLoop* ioLoop);
    ~UDPSocketTemplateEx() {}

    void Close();

    int32 StopAsyncReceive();

    int32 LocalEndpoint(Endpoint& endpoint);

protected:
    int32 InternalAsyncReceive(Buffer buffer);

    /*
     SendRequestType should have following public members (if it derived from SendRequestBase then everything is ok):
        DerivedClassType* pthis   - pointer to DerivedClassType instance
        uv_buf_t          buffer  - libuv buffer to write
        uv_udp_send_t     request - libuv UDP send request
    */
    template <typename SendRequestType>
    int32 InternalAsyncSend(SendRequestType* request, const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint);

    int32 InternalAsyncSend(SendRequestType* request, const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint);
    int32 InternalAsyncSendSerialized(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint);

private:
    void HandleClose() {}
    void HandleAlloc(std::size_t /*suggested_size*/, uv_buf_t* buffer);
    void HandleReceive(int32 /*error*/, std::size_t /*nread*/, const uv_buf_t* /*buffer*/, const Endpoint& /*endpoint*/, bool /*partial*/) {}
    template<typename SendRequestType>
    void HandleSend(SendRequestType* /*request*/, int32 /*error*/) {}

    static void HandleCloseThunk(uv_handle_t* handle);
    static void HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer);
    static void HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags);
    template <typename SendRequestType>
    static void HandleSendThunk(uv_udp_send_t* sendRequest, int error);

protected:
    Buffer      externalReadBuffer;
    SendRequest sendRequestSerialized;
};

//////////////////////////////////////////////////////////////////////////
template <typename T>
UDPSocketTemplateEx<T>::UDPSocketTemplateEx(IOLoop* ioLoop) : UDPSocketBase(ioLoop)
                                                            , externalReadBuffer()
                                                            , sendRequestSerialized()
{
    handle.data                          = static_cast<DerivedClassType*>(this);
    sendRequestSerialized.request.handle = static_cast<DerivedClassType*>(this);
}

template <typename T>
void UDPSocketTemplateEx<T>::Close()
{
    BaseClassType::InternalClose(&HandleCloseThunk);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::StopAsyncReceive()
{
    return uv_udp_recv_stop(Handle());
}

template <typename T>
int32 UDPSocketTemplateEx<T>::LocalEndpoint(Endpoint& endpoint)
{
    int size = static_cast<int> (endpoint.Size());
    return uv_udp_getsockname(Handle(), endpoint.CastToSockaddr(), &size);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::InternalAsyncReceive(Buffer buffer)
{
    DVASSERT(buffer.base != NULL && buffer.len > 0);

    externalReadBuffer = buffer;
    return uv_udp_recv_start(Handle(), &HandleAllocThunk, &HandleReceiveThunk);
}

template <typename T>
template <typename SendRequestType>
int32 UDPSocketTemplateEx<T>::InternalAsyncSend(SendRequestType* request, const void* buffer, std::size_t size, const Endpoint& endpoint)
{
    DVASSERT(request != NULL && buffer != NULL && size > 0);

    request->pthis        = static_cast<DerivedClassType*>(this);
    request->buffer       = uv_buf_init(static_cast<char*>(const_cast<void*>(buffer)), size);    // uv_buf_init doesn't modify buffer
    request->request.data = request;

    return uv_udp_send(&request->request, Handle(), &request->buffer, 1, endpoint.CastToSockaddr(), &HandleSendThunk<SendRequestType>);
}

template <typename T>
int32 UDPSocketTemplateEx<T>::InternalAsyncSendSerialized(const Buffer* buffers, std::size_t bufferCount, const Endpoint& endpoint)
{
    DVASSERT(buffers != NULL && 0 < bufferCount && bufferCount <= MAX_WRITE_BUFFERS);

    sendRequestSerialized.pthis       = static_cast<DerivedClassType*>(this);
    sendRequestSerialized.bufferCount = bufferCount;
    for (std::size_t i = 0;i < bufferCount;++i)
        sendRequestSerialized.buffers[i] = buffers[i];

    return uv_udp_send(&sendRequestSerialized.request, Handle(), sendRequestSerialized.buffers
                                                               , sendRequestSerialized.bufferCount
                                                               , endpoint.CastToSockaddr()
                                                               , 0);
}

template <typename T>
void UDPSocketTemplateEx<T, autoRead>::HandleAlloc(std::size_t /*suggested_size*/, uv_buf_t* buffer)
{
    *buffer = uv_buf_init(static_cast<char*>(externalReadBuffer), externalReadBufferSize);
}

template <typename T>
void UDPSocketTemplateEx<T, autoRead>::HandleCloseThunk(uv_handle_t* handle)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->CleanUpBeforeNextUse();
    pthis->HandleClose();
}

template <typename T>
void UDPSocketTemplateEx<T, autoRead>::HandleAllocThunk(uv_handle_t* handle, std::size_t suggested_size, uv_buf_t* buffer)
{
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    pthis->HandleAlloc(suggested_size, buffer);
}

template <typename T>
void UDPSocketTemplateEx<T, autoRead>::HandleReceiveThunk(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buffer, const sockaddr* addr, unsigned int flags)
{
    // According to libuv documentation under such conditions there is nothing to read
    if(0 == nread && NULL == addr)
        return;

    int32 error = 0;
    if(nread < 0)
    {
        error = nread;
        nread = 0;
    }
    DerivedClassType* pthis = static_cast<DerivedClassType*>(handle->data);
    if(!autoReadFlag && 0 == error)
    {
        pthis->StopAsyncReceive();
    }
    pthis->HandleReceive(error, nread, buffer, Endpoint(addr), UV_UDP_PARTIAL == flags);
}

template <typename T>
template <typename SendRequestType>
void UDPSocketTemplateEx<T, autoRead>::HandleSendThunk(uv_udp_send_t* sendRequest, int error)
{
    SendRequestType* request = static_cast<SendRequestType*>(sendRequest->data);
    request->pthis->HandleSend(request, error);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_UDPSocketTemplateEx_H__
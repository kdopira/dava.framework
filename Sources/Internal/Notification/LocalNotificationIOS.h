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

#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_IOS_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_IOS_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "LocalNotificationImpl.h"

#include "Base/Message.h"

namespace DAVA
{
    
struct UILocalNotificationWrapper;

class LocalNotificationIOS : public LocalNotificationImpl
{
public:
	LocalNotificationIOS(const String &_id);
    virtual ~LocalNotificationIOS();

    virtual void SetAction(const WideString &action);
    virtual void Hide();
    virtual void ShowText(const WideString &title, const WideString text);
    virtual void ShowProgress(const WideString &title, const WideString text, const uint32 total, const uint32 progress);
    virtual void PostDelayedNotification(const WideString &title, const WideString &text, int delaySeconds);
    virtual void RemoveAllDelayedNotifications();

public:
    UILocalNotificationWrapper *notification;
};

}

#endif

#endif /* defined __DAVAENGINE_NOTIFICATION_MACOS_H__ */
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

#include "Render/RenderTarget/FramebufferDescriptor.h"

namespace DAVA
{
FramebufferDescriptor::FramebufferDescriptor() :
framebufferType(FramebufferDescriptor::FRAMEBUFFER_INVALID),
framebufferFormat(FramebufferDescriptor::FORMAT_INVALID),
framebufferWidth(0),
framebufferHeight(0),
preRenderAction(FramebufferDescriptor::PRE_ACTION_DONTCARE),
postRenderAction(FramebufferDescriptor::POST_ACTION_DONTCARE)
{

}

FramebufferDescriptor::FramebufferDescriptor(FramebufferDescriptor::FramebufferType type,
                                             FramebufferDescriptor::FramebufferFormat format,
                                             FramebufferDescriptor::PreRenderAction beforeRender,
                                             FramebufferDescriptor::PostRenderAction afterRender,
                                             uint32 width,
                                             uint32 height) :
framebufferType(type),
framebufferFormat(format),
framebufferWidth(width),
framebufferHeight(height),
preRenderAction(beforeRender),
postRenderAction(afterRender)
{

}

void FramebufferDescriptor::SetFramebufferType(FramebufferDescriptor::FramebufferType type)
{
    framebufferType = type;
}

FramebufferDescriptor::FramebufferType FramebufferDescriptor::GetFramebufferType() const
{
    return framebufferType;
}

void FramebufferDescriptor::SetFramebufferFormat(FramebufferDescriptor::FramebufferFormat format)
{
    framebufferFormat = format;
}

FramebufferDescriptor::FramebufferFormat FramebufferDescriptor::GetFramebufferFormat() const
{
    return framebufferFormat;
}

void FramebufferDescriptor::SetFramebufferWidth(uint32 width)
{
    framebufferWidth = width;
}

uint32 FramebufferDescriptor::GetFramebufferWidth() const
{
    return framebufferWidth;
}

void FramebufferDescriptor::SetFramebufferHeight(uint32 height)
{
    framebufferHeight = height;
}

uint32 FramebufferDescriptor::GetFramebufferHeight() const
{
    return framebufferHeight;
}

void FramebufferDescriptor::SetPreRenderAction(FramebufferDescriptor::PreRenderAction action)
{
    preRenderAction = action;
}

FramebufferDescriptor::PreRenderAction FramebufferDescriptor::GetPreRenderAction() const
{
    return preRenderAction;
}

void FramebufferDescriptor::SetPostRenderAction(FramebufferDescriptor::PostRenderAction action)
{
    postRenderAction = action;
}

FramebufferDescriptor::PostRenderAction FramebufferDescriptor::GetPostRenderAction() const
{
    return postRenderAction;
}

bool FramebufferDescriptor::IsValid() const
{
    return (framebufferType != FramebufferDescriptor::FRAMEBUFFER_INVALID) &&
    (framebufferFormat != FramebufferDescriptor::FORMAT_INVALID) &&
    (framebufferWidth > 0) &&
    (framebufferHeight > 0);
}

};
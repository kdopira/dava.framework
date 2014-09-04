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


#ifndef __DAVAENGINE_RENDERTARGETOGL_H__
#define __DAVAENGINE_RENDERTARGETOGL_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderResource.h"

#include "Render/RenderTarget/RenderTarget.h"

#include "Render/RenderTarget/OpenGL/ColorFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/DepthFramebufferAttachmentOGL.h"
#include "Render/RenderTarget/OpenGL/StencilFramebufferAttachmentOGL.h"

namespace DAVA
{

class RenderTargetOGL : public RenderTarget
{
protected:

    virtual ~RenderTargetOGL();

public:

    RenderTargetOGL();

    virtual void BeginRender();
    virtual void EndRender();

    void Initialize();
    void AttachRenderBuffers();
    void BindRenderTarget();
    void UnbindRenderTarget();

    void AddColorAttachment(ColorFramebufferAttachmentOGL* attachment);
    void SetDepthAttachment(DepthFramebufferAttachmentOGL* attachment);
    void SetStencilAttachment(StencilFramebufferAttachmentOGL* attachment);

protected:

    bool CheckRenderTargetCompleteness();
    void ProcessPreRenderActions(const Rect& viewport);
    void ProcessPostRenderActions();

    void CalculateViewport(Rect& viewport);

protected:

    GLuint framebufferId;

    GLuint prevFramebufferId;
    Rect prevViewport;
    int32 prevRenderOrientation;

    bool renderBuffersAttached;
};

};

#endif /* defined(__DAVAENGINE_RENDERTARGETOGL_H__) */
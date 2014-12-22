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


#include "RenderSystem2D.h"
#include "VirtualCoordinatesSystem.h"
#include "Render/RenderManager.h"
#include "Render/ShaderCache.h"
#include <Render/RenderHelper.h>

#include "UI/UIControl.h"
#include "UI/UIControlBackground.h"

namespace DAVA
{

#define USE_MAPPING 0
#define USE_BATCHING true

FastName RenderSystem2D::FLAT_COLOR_SHADER("~res:/Shaders/renderer2dColor");
FastName RenderSystem2D::TEXTURE_FLAT_COLOR_SHADER("~res:/Shaders/renderer2dTexture");

Shader * RenderSystem2D::FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_ADD_FLAT_COLOR_IMAGE_A8 = 0;
Shader * RenderSystem2D::TEXTURE_MUL_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_MUL_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_MUL_COLOR_IMAGE_A8 = 0;
Shader * RenderSystem2D::TEXTURE_ADD_COLOR = 0;
Shader * RenderSystem2D::TEXTURE_ADD_COLOR_ALPHA_TEST = 0;
Shader * RenderSystem2D::TEXTURE_ADD_COLOR_IMAGE_A8 = 0;


VboPool::VboPool(uint32 size, uint8 count)
{
    vertexStride = sizeof(float32) * 8; //XYUVRGBA
    currentVertexBufferSize = size * vertexStride;
    currentIndexBufferSize = size * 2 * sizeof(uint16);
    for (uint8 i = 0; i < count; ++i)
    {
        RenderDataObject* obj = new RenderDataObject();
#if USE_MAPPING
        obj->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, 0);
        obj->BuildVertexBuffer(size, RenderDataObject::DYNAMIC_DRAW, false);
        obj->SetIndices(EIF_16, 0, size * 2);
        obj->BuildIndexBuffer(false);
#endif
        dataObjects.push_back(obj);
    }
    currentDataObjectIndex = 0;
    currentDataObject = dataObjects[currentDataObjectIndex];
}

void VboPool::Next()
{
    currentDataObjectIndex = (currentDataObjectIndex + 1) % dataObjects.size();
    currentDataObject = dataObjects[currentDataObjectIndex];
}

void VboPool::SetVertexData(uint32 count, float32* data)
{
    currentDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, data);
    currentDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, data + 2);
    currentDataObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, data + 4);
    currentDataObject->UpdateVertexBuffer(count, RenderDataObject::DYNAMIC_DRAW);
    currentDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, 0);
    currentDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, (void*)8);
    currentDataObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, (void*)16);
}

void VboPool::SetIndexData(uint32 count, uint8* data)
{
    currentDataObject->SetIndices(EIF_16, data, count);
    currentDataObject->UpdateIndexBuffer();
}

void VboPool::MapBuffers()
{
#if USE_MAPPING
    uint32 vbid = currentDataObject->GetVertexBufferID();
    uint32 ibid = currentDataObject->GetIndexBufferID();

    RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vbid);
    RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibid);
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(currentVertexBufferPointer = (float32*)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
#else
    RENDER_VERIFY(currentVertexBufferPointer = (float32*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
#endif
#endif
}

void VboPool::UnmapBuffers()
{
#if USE_MAPPING
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glUnmapBufferOES(GL_ARRAY_BUFFER));
    RENDER_VERIFY(glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER));
#else
    RENDER_VERIFY(glUnmapBuffer(GL_ARRAY_BUFFER));
    RENDER_VERIFY(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
#endif
    currentVertexBufferPointer = 0;
    currentIndexBufferPointer = 0;
#endif
}

void VboPool::MapVertexBuffer()
{
#if USE_MAPPING
    uint32 vbid = currentDataObject->GetVertexBufferID();
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vbid));
    RENDER_VERIFY(currentVertexBufferPointer = (float32*)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
#else
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, vbid));
    RENDER_VERIFY(currentVertexBufferPointer = (float32*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
#endif
#endif
}

void VboPool::UnmapVertexBuffer()
{
#if USE_MAPPING
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glUnmapBufferOES(GL_ARRAY_BUFFER));
#else
    RENDER_VERIFY(glUnmapBuffer(GL_ARRAY_BUFFER));
#endif
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ARRAY_BUFFER, NULL));
    currentVertexBufferPointer = 0;
#endif
}

void VboPool::MapIndexBuffer()
{
#if USE_MAPPING
    uint32 ibid = currentDataObject->GetIndexBufferID();
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibid));
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
#else
    RENDER_VERIFY(currentIndexBufferPointer = (uint16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
#endif
#endif
}

void VboPool::UnmapIndexBuffer()
{
#if USE_MAPPING
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    RENDER_VERIFY(glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER));
#else
    RENDER_VERIFY(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
#endif
    RENDER_VERIFY(RenderManager::Instance()->HWglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL));
    currentIndexBufferPointer = 0;
#endif
}

void VboPool::RenewBuffers(uint32 size)
{
#if USE_MAPPING
    currentVertexBufferSize = size * vertexStride;
    currentIndexBufferSize = size * 2 * sizeof(uint16);
    uint32 count = (uint32)dataObjects.size();
    for (uint32 i = 0; i < count; ++i)
    {
        RenderDataObject* obj = dataObjects[i];
        obj->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, vertexStride, 0);
        obj->SetStream(EVF_COLOR, TYPE_FLOAT, 4, vertexStride, 0);
        obj->BuildVertexBuffer(size, false);
        obj->SetIndices(EIF_16, 0, size * 2);
        obj->BuildIndexBuffer(false);
    }
#endif
}

RenderDataObject* VboPool::GetDataObject() const
{
    return currentDataObject;
}

float32* VboPool::GetVertexBufferPointer() const
{
    return currentVertexBufferPointer;
}

uint16* VboPool::GetIndexBufferPointer() const
{
    return currentIndexBufferPointer;
}

uint32 VboPool::GetVertexBufferSize() const
{
    return currentVertexBufferSize;
}

uint32 VboPool::GetIndexBufferSize() const
{
    return currentIndexBufferSize;
}

RenderSystem2D::RenderSystem2D() 
    : pool(NULL)
    , spriteRenderObject(0)
    , spriteVertexStream(0)
    , spriteTexCoordStream(0)
    , spriteClipping(true)
    , useBatching(USE_BATCHING)
{
}

void RenderSystem2D::Init()
{
    if(useBatching && !pool)
    {
        // Create pool on first BeginFrame call
        pool = new VboPool(4096, 10);
        
#if !USE_MAPPING
        vertexBuffer2.resize(4096 * 32); //2048 points (XY UV RGBA)
        indexBuffer2.resize(8192);
#endif
    }
    else if (!useBatching && !spriteRenderObject) //used as flag 'isInited'
    {
        spriteRenderObject = new RenderDataObject();
        spriteVertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
        spriteTexCoordStream = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
        spriteColorStream = spriteRenderObject->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 0, 0);
    }
    
    if(FLAT_COLOR == NULL)
    {
        FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(FLAT_COLOR_SHADER, FastNameSet()));

        TEXTURE_MUL_FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, FastNameSet()));

        FastNameSet set;
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        TEXTURE_ADD_FLAT_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_ADD_FLAT_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        TEXTURE_MUL_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_MUL_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_MUL_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        set.Insert(FastName("ADD_COLOR"));
        TEXTURE_ADD_COLOR = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("ALPHA_TEST_ENABLED"));
        TEXTURE_ADD_COLOR_ALPHA_TEST = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));

        set.clear();
        set.Insert(FastName("VERTEX_COLOR"));
        set.Insert(FastName("ADD_COLOR"));
        set.Insert(FastName("IMAGE_A8"));
        TEXTURE_ADD_COLOR_IMAGE_A8 = SafeRetain(ShaderCache::Instance()->Get(TEXTURE_FLAT_COLOR_SHADER, set));
    }
}

RenderSystem2D::~RenderSystem2D()
{
    SafeDelete(pool);
    SafeRelease(spriteRenderObject);

    SafeRelease(FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_MUL_FLAT_COLOR_IMAGE_A8);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_ADD_FLAT_COLOR_IMAGE_A8);
    SafeRelease(TEXTURE_MUL_COLOR);
    SafeRelease(TEXTURE_MUL_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_MUL_COLOR_IMAGE_A8);
    SafeRelease(TEXTURE_ADD_COLOR);
    SafeRelease(TEXTURE_ADD_COLOR_ALPHA_TEST);
    SafeRelease(TEXTURE_ADD_COLOR_IMAGE_A8);
}

void RenderSystem2D::Reset()
{
    currentClip.x = 0;
    currentClip.y = 0;
    currentClip.dx = -1;
    currentClip.dy = -1;

    Setup2DMatrices();

    defaultSpriteDrawState.Reset();
    defaultSpriteDrawState.renderState = RenderState::RENDERSTATE_2D_BLEND;
    defaultSpriteDrawState.shader = TEXTURE_MUL_FLAT_COLOR;

    batches.clear();
    batches.reserve(1024);
    currentBatch.Reset();

    vertexIndex = 0;
    indexIndex = 0;
}

void RenderSystem2D::Setup2DMatrices()
{
    RenderManager::SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_VIEW, &viewMatrix, UPDATE_SEMANTIC_ALWAYS);
}

void RenderSystem2D::ScreenSizeChanged()
{
    Matrix4 glTranslate, glScale;

    Vector2 scale = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(Vector2(1.f, 1.f));
    Vector2 realDrawOffset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();

    glTranslate.glTranslate(realDrawOffset.x, realDrawOffset.y, 0.0f);
    glScale.glScale(scale.x, scale.y, 1.0f);
    viewMatrix = glScale * glTranslate;
}

void RenderSystem2D::SetClip(const Rect &rect)
{
    currentClip = rect;
    RenderManager::Instance()->SetHWClip(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(currentClip));
}

void RenderSystem2D::RemoveClip()
{
    currentClip = Rect(0,0,-1,-1);
    RenderManager::Instance()->SetHWClip(currentClip);
}

void RenderSystem2D::ClipRect(const Rect &rect)
{
    Rect r = currentClip;
    if(r.dx < 0)
    {
        r.dx = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx;
    }
    if(r.dy < 0)
    {
        r.dy = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
    }

    r = r.Intersection(rect);

    currentClip = r;
    RenderManager::Instance()->SetHWClip(VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(currentClip));
}

void RenderSystem2D::ClipPush()
{
    clipStack.push(currentClip);
}

void RenderSystem2D::ClipPop()
{
    if(clipStack.empty())
    {
        Rect r(0, 0, -1, -1);
        SetClip(r);
    }
    else
    {
        Rect r = clipStack.top();
        SetClip(r);
    }
    clipStack.pop();
}

void RenderSystem2D::SetSpriteClipping(bool clipping)
{
    spriteClipping = clipping;
}

bool RenderSystem2D::IsPreparedSpriteOnScreen(Sprite::DrawState * drawState)
{
    if (RenderManager::Instance()->IsRenderTarget())
        return true;

    Rect clipRect = currentClip;
    if (clipRect.dx == -1)
    {
        clipRect.dx = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dx;
    }
    if (clipRect.dy == -1)
    {
        clipRect.dy = (float32)VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize().dy;
    }

    float32 left = Min(Min(spriteTempVertices[0], spriteTempVertices[2]), Min(spriteTempVertices[4], spriteTempVertices[6]));
    float32 right = Max(Max(spriteTempVertices[0], spriteTempVertices[2]), Max(spriteTempVertices[4], spriteTempVertices[6]));
    float32 top = Min(Min(spriteTempVertices[1], spriteTempVertices[3]), Min(spriteTempVertices[5], spriteTempVertices[7]));
    float32 bottom = Max(Max(spriteTempVertices[1], spriteTempVertices[3]), Max(spriteTempVertices[5], spriteTempVertices[7]));

    const Rect spriteRect(left, top, right - left, bottom - top);
    return clipRect.RectIntersects(spriteRect);
}

void RenderSystem2D::Flush()
{
    /*
    Called on each EndFrame, particle draw, screen transitions preparing, screen borders draw
    */

    if (!useBatching)
    {
        return;
    }

    Setup2DMatrices();

    if (currentBatch.count > 0)
    {
        batches.push_back(currentBatch);
        currentBatch.Reset();
    }

    if (batches.empty())
    {
        vertexIndex = 0;
        indexIndex = 0;
        return;
    }

    spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLELIST;

#if !USE_MAPPING
    pool->SetVertexData(vertexIndex, &vertexBuffer2[0]);
    pool->SetIndexData(indexIndex, (uint8*)&indexBuffer2[0]);
#else
    // Need set streams because on pool constructor we can't set they (deadlock)
    pool->GetDataObject()->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 32, 0);
    pool->GetDataObject()->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 32, (void*)8);
    pool->GetDataObject()->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 32, (void*)16);
#endif
    
    RenderManager::Instance()->SetRenderData(pool->GetDataObject());

    Vector<RenderBatch2D>::iterator it = batches.begin();
    Vector<RenderBatch2D>::iterator eit = batches.end();
    for (; it != eit; ++it)
    {
        const RenderBatch2D& batch = *it;

        bool clip = batch.clipRect.dx != -1;
        if (clip)
        {
            ClipPush();
            SetClip(batch.clipRect);
        }

        RENDERER_UPDATE_STATS(spriteDrawCount++);

        RenderManager::Instance()->SetRenderState(batch.renderState);
        RenderManager::Instance()->SetTextureState(batch.textureHandle);
        RenderManager::Instance()->SetRenderEffect(batch.shader);
        RenderManager::Instance()->DrawElements(spritePrimitiveToDraw, batch.count, EIF_16, reinterpret_cast<void*>(batch.indexOffset * 2));

        if (clip)
        {
            ClipPop();
        }
    }

    RenderManager::Instance()->SetRenderData(NULL);

    batches.clear();

    vertexIndex = 0;
    indexIndex = 0;

    pool->Next();
}

void RenderSystem2D::PushBatch(UniqueHandle state, UniqueHandle texture, Shader* shader, Rect const& clip)
{
    Shader * convShader = shader;

    if (TEXTURE_MUL_FLAT_COLOR == shader)
    {
        convShader = TEXTURE_MUL_COLOR;
    }
    else if (TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST == shader)
    {
        convShader = TEXTURE_MUL_COLOR_ALPHA_TEST;
    }
    else if (TEXTURE_MUL_FLAT_COLOR_IMAGE_A8 == shader)
    {
        convShader = TEXTURE_MUL_COLOR_IMAGE_A8;
    }
    else if (TEXTURE_ADD_FLAT_COLOR == shader)
    {
        convShader = TEXTURE_ADD_COLOR;
    }
    else if (TEXTURE_ADD_FLAT_COLOR_ALPHA_TEST == shader)
    {
        convShader = TEXTURE_ADD_COLOR_ALPHA_TEST;
    }
    else if (TEXTURE_ADD_FLAT_COLOR_IMAGE_A8 == shader)
    {
        convShader = TEXTURE_ADD_COLOR_IMAGE_A8;
    }

    if (currentBatch.renderState != state
        || currentBatch.textureHandle != texture
        || currentBatch.shader != convShader
        || currentBatch.clipRect != currentClip)
    {
        if (currentBatch.count > 0)
        {
            batches.push_back(currentBatch);
        }
        currentBatch.Reset();

        currentBatch.renderState = state;
        currentBatch.textureHandle = texture;
        currentBatch.shader = convShader;
        currentBatch.clipRect = currentClip;
        currentBatch.indexOffset = indexIndex;
    }

    currentBatch.count += spriteIndexCount;
    indexIndex += spriteIndexCount;
    vertexIndex += spriteVertexCount;
}


void RenderSystem2D::Draw(Sprite * sprite, Sprite::DrawState * drawState /* = 0 */)
{
    if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

    if(!useBatching)
    {
        Setup2DMatrices();
    }

    Sprite::DrawState * state = drawState;
    if (!state)
    {
        state = &defaultSpriteDrawState;
    }

	float32 x, y;
    float32 scaleX = 1.0f;
    float32 scaleY = 1.0f;

    sprite->flags = 0;
    if (state->flags != 0)
    {
        sprite->flags |= Sprite::EST_MODIFICATION;
    }

    if(state->scale.x != 1.f || state->scale.y != 1.f)
    {
        sprite->flags |= Sprite::EST_SCALE;
        scaleX = state->scale.x;
        scaleY = state->scale.y;
    }

    if(state->angle != 0.f) sprite->flags |= Sprite::EST_ROTATE;

    int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);

    x = state->position.x - state->pivotPoint.x * state->scale.x;
    y = state->position.y - state->pivotPoint.y * state->scale.y;

    float32 **frameVertices = sprite->frameVertices;
    float32 **rectsAndOffsets = sprite->rectsAndOffsets;
    Vector2 spriteSize = sprite->size;

    if(sprite->flags & Sprite::EST_MODIFICATION)
    {
        if((state->flags & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
        {//HFLIP|VFLIP
            if(sprite->flags & Sprite::EST_SCALE)
            {//SCALE
                x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
                y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
                if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
                    spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
                    spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
                    spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
                }
                else
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y2
                    spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                    spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[5];//y1
                }
            }
            else
            {//NOT SCALE
                x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
                y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
                if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] + x;//x2 do not change this sequence. This is because of the cache reason
                    spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] + y;//y1
                    spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] + x;//x1
                    spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] + y;//y2
                }
                else
                {
                    spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x2
                    spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y2
                    spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                    spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[5];//y1
                }
            }
        }
        else
        {
            if(state->flags & ESM_HFLIP)
            {//HFLIP
                if(sprite->flags & Sprite::EST_SCALE)
                {//SCALE
                    x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scaleX;
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] * scaleX + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] * scaleX + y;//y1 //WEIRD: maybe scaleY should be used?
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] * scaleX + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y1
                        spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[1];//y2
                    }
                }
                else
                {//NOT SCALE
                    x += (spriteSize.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][2] + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][0] + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[2] = spriteTempVertices[6] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x2
                        spriteTempVertices[0] = spriteTempVertices[4] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[2];//x1
                        spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y1
                        spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[1];//y2
                    }
                }
            }
            else
            {//VFLIP
                if(sprite->flags & Sprite::EST_SCALE)
                {//SCALE
                    y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scaleY;
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] * scaleY + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] * scaleY + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y2
                        spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[0];//x2
                        spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[5];//y1
                    }
                }
                else
                {//NOT SCALE
                    y += (spriteSize.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
                    if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] + x;//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][1] + y;//y2
                        spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][5] + y;//y1
                        spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] + x;//x2
                    }
                    else
                    {
                        spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x1
                        spriteTempVertices[5] = spriteTempVertices[7] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y2
                        spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[0];//x2
                        spriteTempVertices[1] = spriteTempVertices[3] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[5];//y1
                    }
                }
            }
        }

    }
    else
    {//NO MODIFERS
        if(sprite->flags & Sprite::EST_SCALE)
        {//SCALE
            if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
            {
                spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] * scaleX + x;//x1
                spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] * scaleY + y;//y2
                spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] * scaleY + y;//y1
                spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] * scaleX + x;//x2 do not change this sequence. This is because of the cache reason
            }
            else
            {
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] * scaleX + x);//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] * scaleY + y);//y1
                spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * scaleX + spriteTempVertices[0];//x2
                spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * scaleY + spriteTempVertices[1];//y2
            }
        }
        else
        {//NOT SCALE
            if(!state || !state->usePerPixelAccuracy || (sprite->flags & Sprite::EST_ROTATE))
            {
                spriteTempVertices[0] = spriteTempVertices[4] = frameVertices[frame][0] + x;//x1
                spriteTempVertices[5] = spriteTempVertices[7] = frameVertices[frame][5] + y;//y2
                spriteTempVertices[1] = spriteTempVertices[3] = frameVertices[frame][1] + y;//y1
                spriteTempVertices[2] = spriteTempVertices[6] = frameVertices[frame][2] + x;//x2 do not change this sequence. This is because of the cache reason
            }
            else
            {
                spriteTempVertices[0] = spriteTempVertices[4] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalX(frameVertices[frame][0] + x);//x1
                spriteTempVertices[1] = spriteTempVertices[3] = VirtualCoordinatesSystem::Instance()->AlignVirtualToPhysicalY(frameVertices[frame][1] + y);//y1
                spriteTempVertices[2] = spriteTempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) + spriteTempVertices[0];//x2
                spriteTempVertices[5] = spriteTempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) + spriteTempVertices[1];//y2
            }

        }

    }

    if(!sprite->clipPolygon)
    {
        if(sprite->flags & Sprite::EST_ROTATE)
        {
            //SLOW CODE
            //			glPushMatrix();
            //			glTranslatef(drawCoord.x, drawCoord.y, 0);
            //			glRotatef(RadToDeg(rotateAngle), 0.0f, 0.0f, 1.0f);
            //			glTranslatef(-drawCoord.x, -drawCoord.y, 0);
            //			RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
            //			glPopMatrix();
            
            // Optimized code
            float32 sinA = sinf(state->angle);
            float32 cosA = cosf(state->angle);
            for(int32 k = 0; k < 4; ++k)
            {
                float32 x = spriteTempVertices[(k << 1)] - state->position.x;
                float32 y = spriteTempVertices[(k << 1) + 1] - state->position.y;

                float32 nx = (x) * cosA  - (y) * sinA + state->position.x;
                float32 ny = (x) * sinA  + (y) * cosA + state->position.y;

                spriteTempVertices[(k << 1)] = nx;
                spriteTempVertices[(k << 1) + 1] = ny;
            }
        }

        if (spriteClipping && !IsPreparedSpriteOnScreen(state))
        {
            // Skip draw for sprites out of screen
            return;
        }

        spriteVertexCount = 4;

        if (useBatching)
        {
            uint32 vi = vertexIndex * 8;
            uint32 ii = indexIndex;
#if USE_MAPPING
            pool->MapVertexBuffer();
            float32* vb = pool->GetVertexBufferPointer();
#else
            float32 * vb = &vertexBuffer2.front();
#endif

            spriteIndexCount = 6;

            const Color c = RenderManager::Instance()->GetColor();
            for (int32 i = 0; i < spriteVertexCount; ++i)
            {
                vb[vi++] = spriteTempVertices[i * 2];
                vb[vi++] = spriteTempVertices[i * 2 + 1];
                vb[vi++] = sprite->texCoords[frame][i * 2];
                vb[vi++] = sprite->texCoords[frame][i * 2 + 1];
                vb[vi++] = c.r;
                vb[vi++] = c.g;
                vb[vi++] = c.b;
                vb[vi++] = c.a;
            }

#if USE_MAPPING
            pool->UnmapVertexBuffer();
            pool->MapIndexBuffer();
            uint16* ib = pool->GetIndexBufferPointer();
#else
            uint16* ib = &indexBuffer2.front();
#endif

            static uint32 spriteIndeces[] = { 0, 1, 2, 1, 3, 2 };
            for (int32 i = 0; i < spriteIndexCount; ++i)
            {
                ib[ii++] = vertexIndex + spriteIndeces[i];
            }

#if USE_MAPPING
            pool->UnmapIndexBuffer();
#endif
        }
        else
        {
            spriteVertexStream->Set(TYPE_FLOAT, 2, 0, spriteTempVertices);
            spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, sprite->texCoords[frame]);
            spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
			DVASSERT(spriteVertexStream->pointer != 0);
			DVASSERT(spriteTexCoordStream->pointer != 0);
        }

    }
    else
    {
        spriteClippedVertices.clear();
        spriteClippedVertices.reserve(sprite->clipPolygon->GetPointCount());
        spriteClippedTexCoords.clear();
        spriteClippedTexCoords.reserve(sprite->clipPolygon->GetPointCount());

        Texture * t = sprite->GetTexture(frame);
        Vector2 virtualTexSize = VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtual(Vector2((float32)t->width, (float32)t->height), sprite->GetResourceSizeIndex());
        float32 adjWidth = 1.f / virtualTexSize.x;
        float32 adjHeight = 1.f / virtualTexSize.y;

        if (useBatching)
        {
            uint32 vi = vertexIndex * 8;
            uint32 ii = indexIndex;
#if USE_MAPPING
            pool->MapVertexBuffer();
            float32* vb = pool->GetVertexBufferPointer();
#else
            float32 * vb = &vertexBuffer2.front();
#endif

            spriteVertexCount = sprite->clipPolygon->GetPointCount();
            DVASSERT(spriteVertexCount >= 2);
            spriteIndexCount = (spriteVertexCount - 2) * 3;

            const Color c = RenderManager::Instance()->GetColor();
            for (int32 i = 0; i < spriteVertexCount; ++i)
            {

                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
                if (sprite->flags & Sprite::EST_SCALE)
                {
                    vb[vi++] = point.x * scaleX + x;
                    vb[vi++] = point.y * scaleY + y;

                }
                else
                {
                    vb[vi++] = point.x + x;
                    vb[vi++] = point.y + y;
                }
                vb[vi++] = sprite->texCoords[frame][0] + texCoord.x;
                vb[vi++] = sprite->texCoords[frame][1] + texCoord.y;
                vb[vi++] = c.r;
                vb[vi++] = c.g;
                vb[vi++] = c.b;
                vb[vi++] = c.a;
            }

#if USE_MAPPING
            pool->UnmapVertexBuffer();
            pool->MapIndexBuffer();
            uint16* ib = pool->GetIndexBufferPointer();
#else
            uint16* ib = &indexBuffer2.front();
#endif

            for (int32 i = 2; i < spriteVertexCount; ++i)
            {
                ib[ii++] = vertexIndex;
                ib[ii++] = vertexIndex + i - 1;
                ib[ii++] = vertexIndex + i;
            }

#if USE_MAPPING
            pool->UnmapIndexBuffer();
#endif
        }
        else
        {
            if (sprite->flags & Sprite::EST_SCALE)
            {
                for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
                {
                    const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                    spriteClippedVertices.push_back(Vector2(point.x*scaleX + x, point.y*scaleY + y));
                }
            }
            else
            {
                Vector2 pos(x, y);
                for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
                {
                    const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                    spriteClippedVertices.push_back( point + pos );
                }
            }

            for (int32 i = 0; i < sprite->clipPolygon->GetPointCount(); ++i)
            {
                const Vector2 &point = sprite->clipPolygon->GetPoints()[i];
                Vector2 texCoord((point.x - frameVertices[frame][0]) * adjWidth, (point.y - frameVertices[frame][1]) * adjHeight);
                spriteClippedTexCoords.push_back( Vector2( sprite->texCoords[frame][0] + texCoord.x, sprite->texCoords[frame][1] + texCoord.y ) );
            }

            spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedVertices.front());
            spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &spriteClippedTexCoords.front());
            spritePrimitiveToDraw = PRIMITIVETYPE_TRIANGLEFAN;
            spriteVertexCount = sprite->clipPolygon->pointCount;
			DVASSERT(spriteVertexStream->pointer != 0);
			DVASSERT(spriteTexCoordStream->pointer != 0);
        }
    }
    
    Rect clipRect(0,0,-1,-1);
    if(sprite->clipPolygon)
    {
        if( sprite->flags & Sprite::EST_SCALE )
        {
            float32 x = state->position.x - state->pivotPoint.x * state->scale.x;
            float32 y = state->position.y - state->pivotPoint.y * state->scale.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( state->frame, Sprite::X_OFFSET_TO_ACTIVE ) * state->scale.x + x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::Y_OFFSET_TO_ACTIVE ) * state->scale.y + y
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_WIDTH  ) * state->scale.x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_HEIGHT ) * state->scale.y );
        }
        else
        {
            float32 x = state->position.x - state->pivotPoint.x;
            float32 y = state->position.y - state->pivotPoint.y;
            clipRect = Rect(  sprite->GetRectOffsetValueForFrame( state->frame, Sprite::X_OFFSET_TO_ACTIVE ) + x
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::Y_OFFSET_TO_ACTIVE ) + y
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_WIDTH )
                            , sprite->GetRectOffsetValueForFrame( state->frame, Sprite::ACTIVE_HEIGHT ) );
        }
    }
    else
    {
        clipRect = currentClip;
    }

    if (useBatching)
    {
        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), state->shader, clipRect);
    }
    else
    {
        if (sprite->clipPolygon)
        {
            ClipPush();
            ClipRect(clipRect);
        }

        RENDERER_UPDATE_STATS(spriteDrawCount++);

        RenderManager::Instance()->SetRenderState(state->renderState);
        RenderManager::Instance()->SetTextureState(sprite->GetTextureHandle(state->frame));
        RenderManager::Instance()->SetRenderData(spriteRenderObject);
        RenderManager::Instance()->SetRenderEffect(state->shader);
        RenderManager::Instance()->DrawArrays(spritePrimitiveToDraw, 0, spriteVertexCount);

        if (sprite->clipPolygon)
        {
            ClipPop();
        }
    }

}

void RenderSystem2D::DrawStretched(Sprite * sprite, Sprite::DrawState * state, Vector2 stretchCapVector, UIControlBackground::eDrawType type, const UIGeometricData &gd, StretchDrawData ** pStreachData)
{
    if (!sprite)return;
	if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);
    const Vector2 &size = gd.size;

    if (stretchCapVector.x < 0.0f || stretchCapVector.y < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f)
        return;

    Vector2 stretchCap(Min(size.x * 0.5f, stretchCapVector.x),
        Min(size.y * 0.5f, stretchCapVector.y));

    UniqueHandle textureHandle = sprite->GetTextureHandle(frame);

    bool needGenerateData = false;
    StretchDrawData * stretchData = 0;
    if(pStreachData)
    {
        stretchData = *pStreachData;
    }
    if (!stretchData)
    {
        stretchData = new StretchDrawData();
        needGenerateData = true;
    }
    else
    {
        needGenerateData |= sprite != stretchData->sprite;
        needGenerateData |= frame != stretchData->frame;
        needGenerateData |= gd.size != stretchData->size;
        needGenerateData |= type != stretchData->type;
        needGenerateData |= stretchCap != stretchData->stretchCap;
    }

    StretchDrawData &sd = *stretchData;

    if (needGenerateData)
    {
        sd.sprite = sprite;
        sd.frame = frame;
        sd.size = gd.size;
        sd.type = type;
        sd.stretchCap = stretchCap;
        sd.GenerateStretchData();
    }

    Matrix3 transformMatr;
    gd.BuildTransformMatrix(transformMatr);

    if (needGenerateData || sd.transformMatr != transformMatr)
    {
        sd.transformMatr = transformMatr;
        sd.GenerateTransformData();
    }

	if (useBatching)
	{
        uint32 vi = vertexIndex * 8;
        uint32 ii = indexIndex;
#if USE_MAPPING
        pool->MapVertexBuffer();
        float32* vb = pool->GetVertexBufferPointer();
#else
        float32 * vb = &vertexBuffer2.front();
#endif

        spriteVertexCount = sd.transformedVertices.size();
        spriteIndexCount = sd.GetVertexInTrianglesCount();

        const Color c = RenderManager::Instance()->GetColor();
        for (int32 i = 0; i < spriteVertexCount; ++i)
        {
            vb[vi++] = (sd.transformedVertices[i].x);
            vb[vi++] = (sd.transformedVertices[i].y);
            vb[vi++] = (sd.texCoords[i].x);
            vb[vi++] = (sd.texCoords[i].y);
            vb[vi++] = (c.r);
            vb[vi++] = (c.g);
            vb[vi++] = (c.b);
            vb[vi++] = (c.a);
        }

#if USE_MAPPING
        pool->UnmapVertexBuffer();
        pool->MapIndexBuffer();
        uint16* ib = pool->GetIndexBufferPointer();
#else
        uint16* ib = &indexBuffer2.front();
#endif

        for (int32 i = 0; i < spriteIndexCount; ++i)
        {
            ib[ii++] = vertexIndex + sd.indeces[i];
        }

#if USE_MAPPING
        pool->UnmapIndexBuffer();
#endif

        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), TEXTURE_MUL_FLAT_COLOR, currentClip);
	}
	else
	{
        RENDERER_UPDATE_STATS(spriteDrawCount++);

		spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &sd.transformedVertices[0]);
		spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &sd.texCoords[0]);
		RenderManager::Instance()->SetTextureState(textureHandle);
		RenderManager::Instance()->SetRenderState(state->renderState);
		RenderManager::Instance()->SetRenderEffect(TEXTURE_MUL_FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(spriteRenderObject);
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, sd.GetVertexInTrianglesCount(), EIF_16, (void*)sd.indeces);
	}
}

void RenderSystem2D::DrawTiled(Sprite * sprite, Sprite::DrawState * state, const Vector2& stretchCapVector, const UIGeometricData &gd, TiledDrawData ** pTiledData)
{
    if (!sprite)return;
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

	int32 frame = Clamp(state->frame, 0, sprite->frameCount - 1);

    const Vector2 &size = gd.size;

    if( stretchCapVector.x < 0.0f || stretchCapVector.y < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f )
        return;

    Vector2 stretchCap( Min( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH) ),
                        Min( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT) ) );

    UniqueHandle textureHandle = sprite->GetTextureHandle(frame);

    stretchCap.x = Min( stretchCap.x * 0.5f, stretchCapVector.x );
    stretchCap.y = Min( stretchCap.y * 0.5f, stretchCapVector.y );

    bool needGenerateData = false;

	TiledDrawData * tiledData = 0;
	if( pTiledData )
	{
		tiledData = *pTiledData;
	}
    if( !tiledData )
    {
        tiledData = new TiledDrawData();
        needGenerateData = true;
    }
    else
    {
        needGenerateData |= stretchCap != tiledData->stretchCap;
        needGenerateData |= frame != tiledData->frame;
        needGenerateData |= sprite != tiledData->sprite;
        needGenerateData |= size != tiledData->size;
    }

    TiledDrawData &td = *tiledData;

    if( needGenerateData )
    {
        td.stretchCap = stretchCap;
        td.size = size;
        td.frame = frame;
        td.sprite = sprite;
        td.GenerateTileData();
    }

    Matrix3 transformMatr;
    gd.BuildTransformMatrix( transformMatr );

    if( needGenerateData || td.transformMatr != transformMatr )
    {
        td.transformMatr = transformMatr;
        td.GenerateTransformData();
    }


	if (useBatching)
	{
        uint32 vi = vertexIndex * 8;
        uint32 ii = indexIndex;
#if USE_MAPPING
        pool->MapVertexBuffer();
        float32* vb = pool->GetVertexBufferPointer();
#else
        float32 * vb = &vertexBuffer2.front();
#endif

		spriteVertexCount = td.transformedVertices.size();
        spriteIndexCount = td.indeces.size();

        const Color c = RenderManager::Instance()->GetColor();
        for (int32 i = 0; i < spriteVertexCount; ++i)
        {
            vb[vi++] = (td.transformedVertices[i].x);
            vb[vi++] = (td.transformedVertices[i].y);
            vb[vi++] = (td.texCoords[i].x);
            vb[vi++] = (td.texCoords[i].y);
            vb[vi++] = (c.r);
            vb[vi++] = (c.g);
            vb[vi++] = (c.b);
            vb[vi++] = (c.a);
        }

#if USE_MAPPING
        pool->UnmapVertexBuffer();
        pool->MapIndexBuffer();
        uint16* ib = pool->GetIndexBufferPointer();
#else
        uint16* ib = &indexBuffer2.front();
#endif

		for (int32 i = 0; i < spriteIndexCount; ++i)
        {
            ib[ii++] = vertexIndex + td.indeces[i];
        }

#if USE_MAPPING
        pool->UnmapIndexBuffer();
#endif

        PushBatch(state->renderState, sprite->GetTextureHandle(state->frame), TEXTURE_MUL_FLAT_COLOR, currentClip);

	}
	else
	{
        RENDERER_UPDATE_STATS(spriteDrawCount++);

		spriteVertexStream->Set(TYPE_FLOAT, 2, 0, &td.transformedVertices[0]);
		spriteTexCoordStream->Set(TYPE_FLOAT, 2, 0, &td.texCoords[0]);

		RenderManager::Instance()->SetTextureState(textureHandle);
		RenderManager::Instance()->SetRenderState(state->renderState);
		RenderManager::Instance()->SetRenderEffect(TEXTURE_MUL_FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(spriteRenderObject);
		RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, td.indeces.size(), EIF_16, &td.indeces[0]);
	}
}

void RenderSystem2D::DrawFilled(Sprite * sprite, Sprite::DrawState * state, const UIGeometricData& gd)
{
	Flush();
	RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
    if( gd.angle != 0.0f )
    {
        Polygon2 poly;
        gd.GetPolygon( poly );
        RenderHelper::Instance()->FillPolygon( poly, state->renderState );
    }
    else
    {
        RenderHelper::Instance()->FillRect( gd.GetUnrotatedRect(), state->renderState );
    }
}

void TiledDrawData::GenerateTileData()
{
    Texture *texture = sprite->GetTexture(frame);

    Vector< Vector3 > cellsWidth;
    GenerateAxisData( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX((float32)texture->GetWidth(), sprite->GetResourceSizeIndex()), stretchCap.x, cellsWidth );

    Vector< Vector3 > cellsHeight;
    GenerateAxisData( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT), VirtualCoordinatesSystem::Instance()->ConvertResourceToVirtualX((float32)texture->GetHeight(), sprite->GetResourceSizeIndex()), stretchCap.y, cellsHeight );

    int32 vertexCount = 4 * cellsHeight.size() * cellsWidth.size();
    vertices.resize( vertexCount );
    transformedVertices.resize( vertexCount );
    texCoords.resize( vertexCount );

    int32 indecesCount = 6 * cellsHeight.size() * cellsWidth.size();
    indeces.resize( indecesCount );

    int32 offsetIndex = 0;
    const float32 * textCoords = sprite->GetTextureCoordsForFrame(frame);
    Vector2 trasformOffset;
    const Vector2 tempTexCoordsPt( textCoords[0], textCoords[1] );
    for( uint32 row = 0; row < cellsHeight.size(); ++row )
    {
        Vector2 cellSize( 0.0f, cellsHeight[row].x );
        Vector2 texCellSize( 0.0f, cellsHeight[row].y );
        Vector2 texTrasformOffset( 0.0f, cellsHeight[row].z );
        trasformOffset.x = 0.0f;

        for( uint32 column = 0; column < cellsWidth.size(); ++column, ++offsetIndex )
        {
            cellSize.x = cellsWidth[column].x;
            texCellSize.x = cellsWidth[column].y;
            texTrasformOffset.x = cellsWidth[column].z;

            int32 vertIndex = offsetIndex*4;
            vertices[vertIndex + 0] = trasformOffset;
            vertices[vertIndex + 1] = trasformOffset + Vector2( cellSize.x, 0.0f );
            vertices[vertIndex + 2] = trasformOffset + Vector2( 0.0f, cellSize.y );
            vertices[vertIndex + 3] = trasformOffset + cellSize;

            const Vector2 texel = tempTexCoordsPt + texTrasformOffset;
            texCoords[vertIndex + 0] = texel;
            texCoords[vertIndex + 1] = texel + Vector2( texCellSize.x, 0.0f );
            texCoords[vertIndex + 2] = texel + Vector2( 0.0f, texCellSize.y );
            texCoords[vertIndex + 3] = texel + texCellSize;

            int32 indecesIndex = offsetIndex*6;
            indeces[indecesIndex + 0] = vertIndex;
            indeces[indecesIndex + 1] = vertIndex + 1;
            indeces[indecesIndex + 2] = vertIndex + 2;

            indeces[indecesIndex + 3] = vertIndex + 1;
            indeces[indecesIndex + 4] = vertIndex + 3;
            indeces[indecesIndex + 5] = vertIndex + 2;

            trasformOffset.x += cellSize.x;
        }
        trasformOffset.y += cellSize.y;
    }
}

void TiledDrawData::GenerateAxisData( float32 size, float32 spriteSize, float32 textureSize, float32 stretchCap, Vector< Vector3 > &axisData )
{
    int32 gridSize = 0;

    float32 sideSize = stretchCap;
    float32 sideTexSize = sideSize / textureSize;

    float32 centerSize = spriteSize - sideSize * 2.0f;
    float32 centerTexSize = centerSize / textureSize;

    float32 partSize = 0.0f;

    if( centerSize > 0.0f )
    {
        gridSize = (int32)ceilf( ( size - sideSize * 2.0f ) / centerSize );
        const float32 tileAreaSize = size - sideSize * 2.0f;
        partSize = tileAreaSize - floorf( tileAreaSize / centerSize ) * centerSize;
    }

    if( sideSize > 0.0f )
        gridSize += 2;

      axisData.resize( gridSize );

    int32 beginOffset = 0;
    int32 endOffset = 0;
    if( sideSize > 0.0f )
    {
        axisData.front() = Vector3( sideSize, sideTexSize, 0.0f );
        axisData.back() = Vector3( sideSize, sideTexSize, sideTexSize + centerTexSize );
        beginOffset = 1;
        endOffset = 1;
    }

    if( partSize > 0.0f )
    {
        ++endOffset;
        const int32 index = gridSize - endOffset;
        axisData[index].x = partSize;
        axisData[index].y = partSize / textureSize;
        axisData[index].z = sideTexSize;
    }

    if( centerSize > 0.0f )
    {
        std::fill( axisData.begin() + beginOffset, axisData.begin() + gridSize - endOffset, Vector3( centerSize, centerTexSize, sideTexSize ) );
    }
}

void TiledDrawData::GenerateTransformData()
{
    for( uint32 index = 0; index < vertices.size(); ++index )
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}

const uint16 StretchDrawData::indeces[18 * 3] = {
    0, 1, 4,
    1, 5, 4,
    1, 2, 5,
    2, 6, 5,
    2, 3, 6,
    3, 7, 6,

    4, 5, 8,
    5, 9, 8,
    5, 6, 9,
    6, 10, 9,
    6, 7, 10,
    7, 11, 10,

    8, 9, 12,
    9, 12, 13,
    9, 10, 13,
    10, 14, 13,
    10, 11, 14,
    11, 15, 14
};

uint32 StretchDrawData::GetVertexInTrianglesCount() const
{
    switch (type)
    {
    case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
    case UIControlBackground::DRAW_STRETCH_VERTICAL:
        return 18;
    case UIControlBackground::DRAW_STRETCH_BOTH:
        return 18 * 3;
    default:
        DVASSERT(0);
        return 0;
    }
}

void StretchDrawData::GenerateTransformData()
{
    for (uint32 index = 0; index < vertices.size(); ++index)
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}

void StretchDrawData::GenerateStretchData()
{
    const Vector2 sizeInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT));
    const Vector2 offsetInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE), sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE));
    const Vector2 &spriteSize = sprite->GetSize();

    const Vector2 xyLeftTopCap(offsetInTex - stretchCap);
    const Vector2 xyRightBottomCap(spriteSize - sizeInTex - offsetInTex - stretchCap);

    const Vector2 xyRealLeftTopCap(Max(0.0f, -xyLeftTopCap.x), Max(0.0f, -xyLeftTopCap.y));
    const Vector2 xyRealRightBottomCap(Max(0.0f, -xyRightBottomCap.x), Max(0.0f, -xyRightBottomCap.y));

    const Vector2 xyNegativeLeftTopCap(Max(0.0f, xyLeftTopCap.x), Max(0.0f, xyLeftTopCap.y));

    const Vector2 scaleFactor = (size - stretchCap*2.0f) / (spriteSize - stretchCap*2.0f);

    Vector2 xyPos;
    Vector2 xySize;

    if (UIControlBackground::DRAW_STRETCH_BOTH == type || UIControlBackground::DRAW_STRETCH_HORIZONTAL == type)
    {
        xySize.x = xyRealLeftTopCap.x + xyRealRightBottomCap.x + (sizeInTex.x - xyRealLeftTopCap.x - xyRealRightBottomCap.x) * scaleFactor.x;
        xyPos.x = stretchCap.x + xyNegativeLeftTopCap.x * scaleFactor.x - xyRealLeftTopCap.x;
    }
    else
    {
        xySize.x = sizeInTex.x;
        xyPos.x = offsetInTex.x + (size.x - spriteSize.x) * 0.5f;
    }

    if (UIControlBackground::DRAW_STRETCH_BOTH == type || UIControlBackground::DRAW_STRETCH_VERTICAL == type)
    {
        xySize.y = xyRealLeftTopCap.y + xyRealRightBottomCap.y + (sizeInTex.y - xyRealLeftTopCap.y - xyRealRightBottomCap.y) * scaleFactor.y;
        xyPos.y = stretchCap.y + xyNegativeLeftTopCap.y * scaleFactor.y - xyRealLeftTopCap.y;
    }
    else
    {
        xySize.y = sizeInTex.y;
        xyPos.y = offsetInTex.y + (size.y - spriteSize.y) * 0.5f;
    }

    const Texture* texture = sprite->GetTexture(frame);
    const Vector2 textureSize((float32)texture->GetWidth(), (float32)texture->GetHeight());

    const Vector2 uvPos(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE) / textureSize.x,
        sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE) / textureSize.y);

    VirtualCoordinatesSystem * vcs = VirtualCoordinatesSystem::Instance();

    const Vector2 uvSize = vcs->ConvertVirtualToResource(Vector2(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT)),
        sprite->GetResourceSizeIndex()
        ) / textureSize;
    const Vector2 uvLeftTopCap = vcs->ConvertVirtualToResource(xyRealLeftTopCap, sprite->GetResourceSizeIndex()) / textureSize;
    const Vector2 uvRightBottomCap = vcs->ConvertVirtualToResource(xyRealRightBottomCap, sprite->GetResourceSizeIndex()) / textureSize;

    switch (type)
    {
    case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
    {
        vertices.resize(8);
        transformedVertices.resize(8);
        texCoords.resize(8);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
        vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
        vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);

        vertices[4] = Vector2(xyPos.x, xyPos.y + xySize.y);
        vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
        vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
        texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
        texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);

        texCoords[4] = Vector2(uvPos.x, uvPos.y + uvSize.y);
        texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    case UIControlBackground::DRAW_STRETCH_VERTICAL:
    {
        vertices.resize(8);
        transformedVertices.resize(8);
        texCoords.resize(8);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[2] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[3] = Vector2(xyPos.x, xyPos.y + xySize.y);

        vertices[4] = Vector2(xyPos.x + xySize.x, xyPos.y);
        vertices[5] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[6] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
        texCoords[2] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[3] = Vector2(uvPos.x, uvPos.y + uvSize.y);

        texCoords[4] = Vector2(uvPos.x + uvSize.x, uvPos.y);
        texCoords[5] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    case UIControlBackground::DRAW_STRETCH_BOTH:
    {
        vertices.resize(16);
        transformedVertices.resize(16);
        texCoords.resize(16);

        vertices[0] = Vector2(xyPos.x, xyPos.y);
        vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
        vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
        vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);

        vertices[4] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xyRealLeftTopCap.y);
        vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);

        vertices[8] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[9] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[10] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
        vertices[11] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);

        vertices[12] = Vector2(xyPos.x, xyPos.y + xySize.y);
        vertices[13] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
        vertices[14] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
        vertices[15] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);

        texCoords[0] = Vector2(uvPos.x, uvPos.y);
        texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
        texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
        texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);

        texCoords[4] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
        texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvLeftTopCap.y);
        texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvLeftTopCap.y);
        texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);

        texCoords[8] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[9] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[10] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
        texCoords[11] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);

        texCoords[12] = Vector2(uvPos.x, uvPos.y + uvSize.y);
        texCoords[13] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
        texCoords[14] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
        texCoords[15] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
    }
    break;
    }
}

};
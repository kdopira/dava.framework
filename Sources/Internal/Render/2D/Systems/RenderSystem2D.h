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


#ifndef __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__
#define	__DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Render/RenderBase.h"
#include "Render/2D/Sprite.h"

#include "UI/UIControlBackground.h"

#define MAX_VERTEXES 4096

namespace DAVA
{

class Font;
class Sprite;
class TextBlock;
class RenderDataObject;
class RenderDataStream;
class UIGeometricData;

struct RenderBatch2D
{
    explicit RenderBatch2D() { Reset(); }
    inline void Reset()
    {
        primitiveType = PRIMITIVETYPE_TRIANGLELIST;
        renderState = 0;
        textureHandle = 0;
        shader = 0;
        clipRect = Rect(0,0,-1,-1);
        count = 0;
        indexOffset = 0;
    }

    UniqueHandle renderState;
    UniqueHandle textureHandle;
    Shader* shader;
    Rect clipRect;
    ePrimitiveType primitiveType;
    uint32 count;
    uint32 indexOffset;
};

struct TiledDrawData
{
    Vector< Vector2 > vertices;
    Vector< Vector2 > texCoords;
    Vector< uint16  > indeces;
    void GenerateTileData();
    void GenerateAxisData( float32 size, float32 spriteSize, float32 textureSize, float32 stretchCap, Vector< Vector3 > &axisData );

    Vector< Vector2 > transformedVertices;
    void GenerateTransformData();

    Sprite *sprite;
    int32 frame;
    Vector2 size;
    Vector2 stretchCap;
    Matrix3 transformMatr;
};

class VboPool
{
public:
    VboPool(uint32 size, uint8 count);
    void Next();

    void SetVertexData(uint32 count, float32 * data);
    void SetIndexData(uint32 count, uint8 * data);

    void MapBuffers();
    void UnmapBuffers();
    void RenewBuffers(uint32 size);
    void MapVertexBuffer();
    void UnmapVertexBuffer();
    void MapIndexBuffer();
    void UnmapIndexBuffer();
    
    RenderDataObject* GetDataObject() const;
    float32 * GetVertexBufferPointer() const;
    uint16 * GetIndexBufferPointer() const;
    uint32 GetVertexBufferSize() const;
    uint32 GetIndexBufferSize() const;

private:
    Vector<RenderDataObject*> dataObjects;
    uint8 currentDataObjectIndex;
    int32	vertexStride;
    int32	vertexFormat;

    RenderDataObject * currentDataObject;
    uint32 currentVertexBufferSize;
    uint32 currentIndexBufferSize;
    float32 * currentVertexBufferPointer;
    uint16 * currentIndexBufferPointer;
};

class RenderSystem2D : public Singleton<RenderSystem2D>
{
public:
    RenderSystem2D();
    virtual ~RenderSystem2D();
    
    void Draw(Sprite * sprite, Sprite::DrawState * state);
	void DrawStretched(Sprite * sprite, Sprite::DrawState * state, Vector2 streatchCap, Rect drawRect, UIControlBackground::eDrawType type);
    void DrawTiled(Sprite * sprite, Sprite::DrawState * state, const Vector2& streatchCap, const UIGeometricData &gd, TiledDrawData ** pTiledData);
    void DrawFilled(Sprite * sprite, Sprite::DrawState * state, const UIGeometricData& gd);

    void PushBatch(UniqueHandle state, UniqueHandle texture, Shader * shader, const Rect& clip);
    
    void Reset();
    void Flush();
    
    void SetClip(const Rect &rect);
	void ClipRect(const Rect &rect);
	void RemoveClip();
    
	void ClipPush();
	void ClipPop();
    
    void ScreenSizeChanged();
    
    void Setup2DMatrices();
    
private:
    Matrix4 viewMatrix;
	std::stack<Rect> clipStack;
	Rect currentClip;
    
    RenderDataObject * spriteRenderObject;
    RenderDataStream * spriteVertexStream;
	RenderDataStream * spriteTexCoordStream;
    RenderDataStream * spriteColorStream;

    float32 spriteTempVertices[8];
    Vector<Vector2> spriteClippedTexCoords;
	Vector<Vector2> spriteClippedVertices;
    ePrimitiveType spritePrimitiveToDraw;
    int32 spriteVertexCount;
    int32 spriteIndexCount;

    Sprite::DrawState defaultSpriteDrawState;

    Vector<float32> vertexBuffer2;
    Vector<uint16> indexBuffer2;

    Vector<RenderBatch2D> batches;
    RenderBatch2D currentBatch;
    uint32 vertexIndex;
    uint32 indexIndex;

    uint32 vboIDs[3];

    bool useBatching;
    bool useVBO;

    VboPool* pool;

};
    
} // ns

#endif	/* __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__ */

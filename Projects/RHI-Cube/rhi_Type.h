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


#ifndef __RHI_TYPE_H__
#define __RHI_TYPE_H__

    #include "Base/BaseTypes.h"
    #include "Base/FastName.h"
    using DAVA::uint32;
    using DAVA::uint8;



namespace rhi
{

typedef uint32 Handle;
static const uint32 InvalidHandle = 0;

enum
Api
{
    RHI_DX11,
    RHI_DX9,
    RHI_GLES2,
    RHI_METAL
};

enum
ProgType
{
    PROG_VERTEX,
    PROG_FRAGMENT
};


enum
PrimitiveType
{
    PRIMITIVE_TRIANGLELIST  = 1
};

struct
DepthStencilState
{
};

struct
SamplerState
{
};

enum
{
    MAX_CONST_BUFFER_COUNT  = 8,
    MAX_RENDER_TARGET_COUNT = 2
};

enum
VertexSemantics
{
    VS_POSITION     = 1,
    VS_NORMAL       = 2,
    VS_COLOR        = 3,
    VS_TEXCOORD     = 4,
    VS_TANGENT      = 5,
    VS_BINORMAL     = 6,
    VS_BLENDWEIGHT  = 7,
    VS_BLENDINDEX   = 8,

    VS_MAXCOUNT     = 16
};


//------------------------------------------------------------------------------

inline const char*
VertexSemanticsName( VertexSemantics vs )
{
    switch( vs )
    {
        case VS_POSITION    : return "position";
        case VS_NORMAL      : return "normal";
        case VS_COLOR       : return "color";
        case VS_TEXCOORD    : return "texcoord";
        case VS_TANGENT     : return "tangent";
        case VS_BINORMAL    : return "binormal";
        case VS_BLENDWEIGHT : return "blend_weight";
        case VS_BLENDINDEX  : return "blend_index";
    }

    return "<unknown>";
}


//------------------------------------------------------------------------------

enum
VertexDataType
{
    VDT_FLOAT   = 1,
    VDT_UINT8   = 2,
    VDT_INT16N  = 3,
    VDT_INT8N   = 4,
    VDT_UINT8N  = 5,
    VDT_HALF    = 6
};


//------------------------------------------------------------------------------

inline const char*
VertexDataTypeName( VertexDataType t )
{
    switch( t )
    {
        case VDT_FLOAT  : return "float";
        case VDT_UINT8  : return "uint8";
        case VDT_INT16N : return "int16n";
        case VDT_INT8N  : return "int8n";
        case VDT_UINT8N : return "uint8n";
        case VDT_HALF   : return "half";
        default         : return "<unknown>";
    }
}


////////////////////////////////////////////////////////////////////////////////
// pipeline-state

struct
BlendState
{
    struct 
    {
        uint32  colorFunc:2;
        uint32  colorSrc:3;
        uint32  colorDst:3;
        uint32  alphaFunc:2;
        uint32  alphaSrc:3;
        uint32  alphaDst:3;
        uint32  writeMask;
        uint32  blendEnabled:1;
        uint32  alphaToCoverage:1;        
    }   rtBlend[MAX_RENDER_TARGET_COUNT];
};


struct
ProgConstInfo
{
    DAVA::FastName  uid;            // name
    uint32          bufferIndex;
    uint32          offset;         // from start of buffer
    int             type;           // size deduced from type -- float4 = 4*sizeof(float) etc. 
};


class
VertexLayout
{
public:


                    VertexLayout();
                    ~VertexLayout();

    unsigned        Stride() const;
    unsigned        ElementCount() const;
    VertexSemantics ElementSemantics( unsigned elem_i ) const;
    VertexDataType  ElementDataType( unsigned elem_i ) const;
    unsigned        ElementDataCount( unsigned elem_i ) const;
    unsigned        ElementOffset( unsigned elem_i ) const;
    unsigned        ElementSize( unsigned elem_i ) const;

    bool            operator==( const VertexLayout& vl ) const;
    VertexLayout&   operator=( const VertexLayout& src );
    
    void            Clear();
    void            AddElement( VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension );
    void            InsertElement( unsigned pos, VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension );

    void            Dump() const;


private:

    enum
    {
        MaxElemCount    = 8
    };

    struct
    Element
    {
        uint32  usage:8;
        uint32  usage_index:8;
        uint32  data_type:8;
        uint32  data_count:8;
    };


    Element     _elem[MaxElemCount];
    uint32      _elem_count;
};


} // namespace rhi


//------------------------------------------------------------------------------
//
//  

template <typename src, typename dst> 
inline dst 
nonaliased_cast( src x )
{
    // (quite a funny way to) ensure types are the same size
// commented-out until acceptable way to stop the compiler-spamming is found
//    #pragma warning( disable: 177 ) // variable "WrongSizes" was declared but never referenced
//    static int  WrongSizes[sizeof(src) == sizeof(dst) ? 1 : -1];
//    #pragma warning( default: 177 )
    
    union 
    { 
        src s; 
        dst d; 
    }   tmp;

    tmp.s = x;

    return tmp.d;
}

const unsigned InvalidIndex = (unsigned)(-1);

#define     countof(array)              (sizeof(array)/sizeof(array[0]))
#define     L_ALIGNED_SIZE(size,align)  (((size) + ((align)-1)) & (~((align)-1)))

#define LCP Logger::Info("%s : %i",__FILE__,__LINE__);

#endif // __RHI_TYPE_H__

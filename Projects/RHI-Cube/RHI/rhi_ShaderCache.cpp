
    #include "../MCPP/mcpp_lib.h"

    #include "rhi_ShaderCache.h"

    

namespace rhi
{
static ShaderBuilder    _ShaderBuilder  = 0;

struct
ProgInfo
{
    DAVA::FastName      uid;
    std::vector<uint8>  bin;
};

static std::vector<ProgInfo> _ProgInfo;

static std::string*         _PreprocessedText = 0;

static int 
_mcpp__fputc( int ch, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                _PreprocessedText->push_back( (char)ch );
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }

    return ch;
}


static int
_mcpp__fputs( const char* str, OUTDEST dst )
{
    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += str;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return 0;
}


static int 
_mcpp__fprintf( OUTDEST dst, const char* format, ... )
{
    va_list     arglist;
    char        buf[2048];
    int         count     = 0;

    va_start( arglist, format );
    count = vsnprintf( buf, countof(buf), format, arglist );
    va_end( arglist );

    switch( dst )
    {
        case MCPP_OUT :
        {
            if( _PreprocessedText )
                *_PreprocessedText += buf;
        }   break;

        case MCPP_ERR :
        {
        }   break;

        case MCPP_DBG :
        {
        }   break;
    }
        
    return count;
}



namespace ShaderCache
{
//------------------------------------------------------------------------------

bool
Initialize( ShaderBuilder builder )
{
    _ShaderBuilder = builder;
    return true;
}


//------------------------------------------------------------------------------

void
Unitialize()
{
}


//------------------------------------------------------------------------------

void
Clear()
{
}


//------------------------------------------------------------------------------

void
Load( const char* binFileName )
{
}


//------------------------------------------------------------------------------

bool
GetProg( const DAVA::FastName& uid, std::vector<uint8>* bin )
{
    bool    success = false;

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin->clear();
            bin->insert( bin->begin(), _ProgInfo[i].bin.begin(), _ProgInfo[i].bin.end() );
            success = true;
            break;
        }
    }

    return success;
}


//------------------------------------------------------------------------------

static const char* _ShaderHeader_Metal =
"#include <metal_stdlib>\n"
"#include <metal_graphics>\n"
"#include <metal_matrix>\n"
"#include <metal_geometric>\n"
"#include <metal_math>\n"
"#include <metal_texture>\n"
"using namespace metal;\n\n"

"float4 mul( float4 v, float4x4 m );\n"
"float4 mul( float4 v, float4x4 m ) { return m*v; }\n"
"float3 mul( float3 v, float3x3 m );\n"
"float3 mul( float3 v, float3x3 m ) { return m*v; }\n"
;
    
static const char* _ShaderDefine_Metal =
"#define VPROG_IN_BEGIN          struct VP_Input {\n"
"#define VPROG_IN_POSITION       packed_float3 position; \n"
"#define VPROG_IN_NORMAL         packed_float3 normal; \n"
"#define VPROG_IN_TEXCOORD       packed_float2 texcoord; \n"
"#define VPROG_IN_END            };\n"

"#define VPROG_OUT_BEGIN         struct VP_Output {\n"
"#define VPROG_OUT_POSITION      float4 position [[ position ]]; \n"
"#define VPROG_OUT_TEXCOORD0(name,size)    float##size name [[ user(texcoord0) ]];\n"
"#define VPROG_OUT_TEXCOORD1(name,size)    float##size name [[ user(texcoord1) ]];\n"
"#define VPROG_OUT_END           };\n"

"#define DECL_VPROG_BUFFER(idx,sz) struct __VP_Buffer##idx { packed_float4 data[sz]; };\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( (float3)(float4(VP_Buffer##buf[reg+0])), (float3)(float4(VP_Buffer##buf[reg+1])), (float3)(float4(VP_Buffer##buf[reg+2])) );\n"

"#define VPROG_BEGIN             vertex VP_Output vp_main"
"("
"    constant VP_Input*  in    [[ buffer(0) ]]"
"    VPROG_IN_BUFFER_0 "
"    VPROG_IN_BUFFER_1 "
"    ,uint                vid   [[ vertex_id ]]"
")"
"{"
"    VPROG_BUFFER_0 "
"    VPROG_BUFFER_1 "
"    VP_Output   OUT;"
"    VP_Input    IN  = in[vid];\n"

"#define VPROG_END               return OUT;"
"}\n"

"#define VP_IN_POSITION          (float3(IN.position))\n"
"#define VP_IN_NORMAL            (float3(IN.normal))\n"
"#define VP_IN_TEXCOORD          (float2(IN.texcoord))\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.name\n"


"#define FPROG_IN_BEGIN              struct FP_Input { float4 position [[position]]; \n"
"#define FPROG_IN_TEXCOORD0(name,size)    float##size name [[ user(texcoord0) ]];\n"
"#define FPROG_IN_TEXCOORD1(name,size)    float##size name [[ user(texcoord1) ]];\n"
"#define FPROG_IN_END                };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color [[color(0)]];\n"
"#define FPROG_OUT_END           };\n"

//"#define DECL_SAMPLER2D(unit)    texture2d<float> Texture##unit [[ texture(unit) ]]; sampler TextureSampler##unit ;\n"
"#define DECL_SAMPLER2D(unit)    \n"

"#define FP_TEXTURE2D(unit,uv)   tex##unit.sample( tex##unit##_sampler, uv );\n"
"#define FP_IN(name)             IN.##name\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) struct __FP_Buffer##idx { packed_float4 data[sz]; };\n"

"#define FPROG_BEGIN             fragment FP_Output fp_main"
"("
"    FP_Input IN                 [[ stage_in ]]"
"    FPROG_IN_BUFFER_0 "
"    FPROG_IN_TEXTURE_0 "
")"
"{"
"    FPROG_BUFFER_0 "
"    FPROG_SAMPLER_0 "
//"    const packed_float4* FP_Buffer0 = buf0->data;"
"    FP_Output   OUT;\n"
"#define FPROG_END               return OUT; }\n"

;



static const char* _ShaderHeader_GLES2 =
"#define float2                 vec2\n"
"#define float3                 vec3\n"
"#define float4                 vec4\n"
"#define float4x4               mat4\n"
"#define float3x3               mat3\n"

"vec4 mul( vec4 v, mat4 m ) { return m*v; }\n"
"vec3 mul( vec3 v, mat3 m ) { return m*v; }\n"

"float4 tex2D( sampler2D s, vec2 t ) { return texture2D(s,t); }\n"
;

static const char* _ShaderDefine_GLES2 =
"#define VPROG_IN_BEGIN          \n"
"#define VPROG_IN_POSITION       attribute vec3 attr_position;\n"
"#define VPROG_IN_NORMAL         attribute vec3 attr_normal;\n"
"#define VPROG_IN_TEXCOORD       attribute vec2 attr_texcoord;\n"
"#define VPROG_IN_END            \n"

"#define VPROG_OUT_BEGIN         \n"
"#define VPROG_OUT_POSITION      \n"
"#define VPROG_OUT_TEXCOORD0(name,size)    varying vec##size var_##name;\n"
"#define VPROG_OUT_TEXCOORD1(name,size)    varying vec##size var_##name;\n"
"#define VPROG_OUT_END           \n"

"#define DECL_VPROG_BUFFER(idx,sz) uniform vec4 VP_Buffer##idx[sz];\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( float3(VP_Buffer##buf[reg+0]), float3(VP_Buffer##buf[reg+1]), float3(VP_Buffer##buf[reg+2]) );\n"

"#define VPROG_BEGIN             void main() {\n"
"#define VPROG_END               }\n"

"#define VP_IN_POSITION          attr_position\n"
"#define VP_IN_NORMAL            attr_normal\n"
"#define VP_IN_TEXCOORD          attr_texcoord\n"

"#define VP_OUT_POSITION         gl_Position\n"
"#define VP_OUT(name)            var_##name\n"


"#define FPROG_IN_BEGIN          \n"
"#define FPROG_IN_TEXCOORD0(name,size)    varying vec##size var_##name;\n"
"#define FPROG_IN_TEXCOORD1(name,size)    varying vec##size var_##name;\n"
"#define FPROG_IN_END            \n"

"#define FPROG_OUT_BEGIN         \n"
"#define FPROG_OUT_COLOR         \n"
"#define FPROG_OUT_END           \n"
            
"#define DECL_SAMPLER2D(unit)    uniform sampler2D Texture##unit;\n"
        
"#define FP_TEXTURE2D(unit,uv)   texture2D( Texture##unit, uv );\n"
"#define FP_IN(name)             var_##name\n"

"#define FP_OUT_COLOR            gl_FragColor\n"

"#define DECL_FPROG_BUFFER(idx,sz) uniform vec4 FP_Buffer##idx[sz];\n"

"#define FPROG_BEGIN             void main() {\n"
"#define FPROG_END               }\n"

;


static const char* _ShaderHeader_DX9 =
//"float4 tex2D( sampler2D s, float2 t ) { return texture2D(s,t); }\n"
""
;

static const char* _ShaderDefine_DX9 =
"#define VPROG_IN_BEGIN          struct VP_Input {\n"
"#define VPROG_IN_POSITION       float3 position : POSITION0;\n"
"#define VPROG_IN_NORMAL         float3 normal : NORMAL0;\n"
"#define VPROG_IN_TEXCOORD       float2 texcoord : TEXCOORD0;\n"
"#define VPROG_IN_END            };\n"

"#define VPROG_OUT_BEGIN         struct VP_Output {\n"
"#define VPROG_OUT_POSITION      float4 position : POSITION0;\n"
"#define VPROG_OUT_TEXCOORD0(name,size)    float##size name : TEXCOORD0;\n"
"#define VPROG_OUT_TEXCOORD1(name,size)    float##size name : TEXCOORD1;\n"
"#define VPROG_OUT_END           };\n"

"#define DECL_VPROG_BUFFER(idx,sz) uniform float4 VP_Buffer##idx[sz];\n"
"#define VP_BUF_FLOAT3X3(buf,reg)  float3x3( (float3)(float4(VP_Buffer##buf[reg+0])), (float3)(float4(VP_Buffer##buf[reg+1])), (float3)(float4(VP_Buffer##buf[reg+2])) );\n"

"#define VPROG_BEGIN             VP_Output vp_main( VP_Input IN ) { VP_Output OUT;\n"
"#define VPROG_END               return OUT; }\n"

"#define VP_IN_POSITION          IN.position\n"
"#define VP_IN_NORMAL            IN.normal\n"
"#define VP_IN_TEXCOORD          IN.texcoord\n"

"#define VP_OUT_POSITION         OUT.position\n"
"#define VP_OUT(name)            OUT.##name\n"


"#define FPROG_IN_BEGIN          struct FP_Input {\n"
"#define FPROG_IN_TEXCOORD0(name,size)    float##size name : TEXCOORD0;\n"
"#define FPROG_IN_TEXCOORD1(name,size)    float##size name : TEXCOORD1;\n"
"#define FPROG_IN_END            };\n"

"#define FPROG_OUT_BEGIN         struct FP_Output {\n"
"#define FPROG_OUT_COLOR         float4 color : COLOR0;\n"
"#define FPROG_OUT_END           };\n"

"#define DECL_SAMPLER2D(unit)    uniform sampler2D Texture##unit;\n"
    
"#define FP_TEXTURE2D(unit,uv)   tex2D( Texture##unit, uv );\n"
"#define FP_IN(name)             IN.##name\n"

"#define FP_OUT_COLOR            OUT.color\n"

"#define DECL_FPROG_BUFFER(idx,sz) uniform float4 FP_Buffer##idx[sz];\n"

"#define FPROG_BEGIN             FP_Output fp_main( FP_Input IN ) { FP_Output OUT;\n"
"#define FPROG_END               return OUT; }\n"

;



static void
PreProcessSource( Api targetApi, const char* srcText, std::string* preprocessedText )
{

    char    src[64*1024] = "";

    switch( targetApi )
    {
        case RHI_DX11   : break;

        case RHI_DX9 : 
        {
            strcat( src, _ShaderDefine_DX9 );
        }   break;

        case RHI_GLES2 : 
        {
            strcat( src, _ShaderDefine_GLES2 );
        }   break;
        
        case RHI_METAL : 
        {
            const char* s       = srcText;
            const char* decl;
            int         src_len = 0;
            bool        vp_buf_declared[16];
            bool        fp_buf_declared[16];
            bool        fp_tex_declared[16];

            for( unsigned i=0; i!=countof(vp_buf_declared); ++i )
                vp_buf_declared[i] = false;
            for( unsigned i=0; i!=countof(fp_buf_declared); ++i )
                fp_buf_declared[i] = false;
            for( unsigned i=0; i!=countof(fp_tex_declared); ++i )
                fp_tex_declared[i] = false;

            while( (decl = strstr( s, "DECL_FPROG_BUFFER" )) )
            {
                int i = 0;

                sscanf( decl, "DECL_FPROG_BUFFER(%i,", &i );

                src_len += sprintf( src+src_len, "#define FPROG_IN_BUFFER_%i  ,constant __FP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, i );
                src_len += sprintf( src+src_len, "#define FPROG_BUFFER_%i    constant packed_float4* FP_Buffer%i = buf%i->data; \n", i, i, i );
                fp_buf_declared[i] = true;

                s += strlen("DECL_FPROG_BUFFER");
            }
            for( unsigned i=0; i!=countof(fp_buf_declared); ++i )
            {
                if( !fp_buf_declared[i] )
                {
                    src_len += sprintf( src+src_len, "#define FPROG_IN_BUFFER_%i \n", i );
                    src_len += sprintf( src+src_len, "#define FPROG_BUFFER_%i \n", i );
                }
            }
            
            s = srcText;
            while( (decl = strstr( s, "DECL_SAMPLER2D" )) )
            {
                int i = 0;

                sscanf( decl, "DECL_SAMPLER2D(%i,", &i );

                src_len += sprintf( src+src_len, "#define FPROG_IN_TEXTURE_%i  , texture2d<float> tex%i [[ texture(%i) ]]\n", i, i, i );
                src_len += sprintf( src+src_len, "#define FPROG_SAMPLER_%i    sampler tex%i_sampler; \n", i, i );
                fp_tex_declared[i] = true;

                s += strlen("DECL_SAMPLER2D");
            }
            for( unsigned i=0; i!=countof(fp_tex_declared); ++i )
            {
                if( !fp_tex_declared[i] )
                {
                    src_len += sprintf( src+src_len, "#define FPROG_IN_TEXTURE_%i \n", i );
                    src_len += sprintf( src+src_len, "#define FPROG_SAMPLER_%i \n", i );
                }
            }
            

            s = srcText;
            while( (decl = strstr( s, "DECL_VPROG_BUFFER" )) )
            {
                int i   = 0;

                sscanf( decl, "DECL_VPROG_BUFFER(%i,", &i );

                src_len += sprintf( src+src_len, "#define VPROG_IN_BUFFER_%i  ,constant __VP_Buffer%i* buf%i [[ buffer(%i) ]]\n", i, i, i, 1+i );
                src_len += sprintf( src+src_len, "#define VPROG_BUFFER_%i    constant packed_float4* VP_Buffer%i = buf%i->data; \n", i, i, i );
                vp_buf_declared[i] = true;

                s += strlen("DECL_VPROG_BUFFER");
            }
            for( unsigned i=0; i!=countof(vp_buf_declared); ++i )
            {
                if( !vp_buf_declared[i] )
                {
                    src_len += sprintf( src+src_len, "#define VPROG_IN_BUFFER_%i \n", i );
                    src_len += sprintf( src+src_len, "#define VPROG_BUFFER_%i \n", i );
                }
            }

            strcat( src, _ShaderDefine_Metal );
        }   break;
    }

    strcat( src, srcText );


    char*   argv[] = 
    { 
        "<mcpp>",   // we just need first arg
        "-P",       // do not output #line directives 
        "-C",       // keep comments
        "<input>"
    };

DAVA::Logger::Info( "src=\n%s\n", src );
    _PreprocessedText = preprocessedText;
    mcpp__set_input( src, strlen(src) );

    mcpp_set_out_func( &_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf );
    mcpp_lib_main( countof(argv), argv );
    _PreprocessedText = 0;
    switch( targetApi )
    {
        case RHI_DX11   : break;
        case RHI_DX9    : preprocessedText->insert( 0, _ShaderHeader_DX9 ); break;
        case RHI_GLES2  : preprocessedText->insert( 0, _ShaderHeader_GLES2 ); break;
        case RHI_METAL  : preprocessedText->insert( 0, _ShaderHeader_Metal ); break;
    }
    ;
DAVA::Logger::Info( "pre-processed=\n%s\n", preprocessedText->c_str() );
}


//------------------------------------------------------------------------------

void
UpdateProg( Api targetApi, ProgType progType, const DAVA::FastName& uid, const char* srcText )
{
    std::string         txt;
    std::vector<uint8>* bin = 0;

    PreProcessSource( targetApi, srcText, &txt );

    for( unsigned i=0; i!=_ProgInfo.size(); ++i )
    {
        if( _ProgInfo[i].uid == uid )
        {
            bin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if( !bin )
    {
        _ProgInfo.push_back( ProgInfo() );
        
        _ProgInfo.back().uid = uid;        
        bin = &(_ProgInfo.back().bin);
    }

    bin->clear();
    bin->insert( bin->begin(), (const uint8*)(&(txt[0])), (const uint8*)(&(txt[txt.length()-1])+1) );
    bin->push_back( 0 );
}



} // namespace ShaderCache
} // namespace rhi
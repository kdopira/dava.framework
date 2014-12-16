
    #include "../RHI/rhi_Pool.h"
    #include "rhi_GLES2.h"

    #include "../rhi_Base.h"
    #include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Core/Core.h"

    #include "_gl.h"


namespace rhi
{

enum
CommandGLES2
{
    GLES2__BEGIN,
    GLES2__END,
    GLES2__CLEAR,

    GLES2__SET_STREAM_SOURCE,
    GLES2__SET_INDICES,

    GLES2__SET_PIPELINE_STATE,
    GLES2__SET_VERTEX_PROG_CONST_BUFFER,
    GLES2__SET_FRAGMENT_PROG_CONST_BUFFER,
    GLES2__SET_TEXTURE,

    GLES2__DRAW_PRIMITIVE,
    GLES2__DRAW_INDEXED_PRIMITIVE,


    GLES2__NOP
};

class
CommandBuffer_t
{
public:
                CommandBuffer_t();
                ~CommandBuffer_t();

    void        Begin();
    void        End();
    void        Replay();

    void        Command( uint32 cmd );
    void        Command( uint32 cmd, uint32 arg1 );
    void        Command( uint32 cmd, uint32 arg1, uint32 arg2 );
    void        Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3 );
    void        Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4 );
    void        Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5 );
    void        Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6 );


private:

    static const uint32   EndCmd = 0xFFFFFFFF;

    std::vector<uint32> _cmd;
};

typedef Pool<CommandBuffer_t>   CommandBufferPool;



namespace CommandBuffer
{

//------------------------------------------------------------------------------

Handle
Default()
{
    static Handle cb = 0;

    if( !cb )
        cb = CommandBufferPool::Alloc();

    return cb;
}


//------------------------------------------------------------------------------

void
Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__BEGIN );
}


//------------------------------------------------------------------------------

void
End( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__END );
}


//------------------------------------------------------------------------------

void
Clear( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__CLEAR, 0xFF808080, nonaliased_cast<float,uint32>(1.0f) );
}


//------------------------------------------------------------------------------

void
SetPipelineState( Handle cmdBuf, Handle ps )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_PIPELINE_STATE, ps );
}


//------------------------------------------------------------------------------

void
SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_STREAM_SOURCE, vb, streamIndex );
}


//------------------------------------------------------------------------------

void
SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, buffer, (uint32)(ConstBufferGLES2::Instance(buffer)) );
}


//------------------------------------------------------------------------------

void
SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
}


//------------------------------------------------------------------------------

void
SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_INDICES, ib );
}


//------------------------------------------------------------------------------

void
SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, buffer, (uint32)(ConstBufferGLES2::Instance(buffer)) );
}


//------------------------------------------------------------------------------

void
SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
//    L_ASSERT(tex);

    if( tex )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_TEXTURE, unitIndex, tex );
}


//------------------------------------------------------------------------------

void
SetDepthStencilState( Handle cmdBuf, const DepthStencilState& bs )
{
}


//------------------------------------------------------------------------------

void
SetSamplerState( Handle cmdBuf, const SamplerState& ss )
{
}


//------------------------------------------------------------------------------

void
DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    unsigned    v_cnt   = 0;
    int         mode    = GL_TRIANGLES;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            mode  = GL_TRIANGLES;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( GLES2__DRAW_PRIMITIVE, uint32(mode), v_cnt );
}


//------------------------------------------------------------------------------

void
DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    unsigned    v_cnt   = 0;
    int         mode    = GL_TRIANGLES;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            mode  = GL_TRIANGLES;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( GLES2__DRAW_INDEXED_PRIMITIVE, uint32(mode), v_cnt );
}


} // namespace CommandBuffer






CommandBuffer_t::CommandBuffer_t()
{
}


//------------------------------------------------------------------------------


CommandBuffer_t::~CommandBuffer_t()
{
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Begin()
{
    _cmd.clear();
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::End()
{
    _cmd.push_back( EndCmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1, uint32 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3 )
{
    _cmd.resize( _cmd.size()+1+3 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+3);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4 )
{
    _cmd.resize( _cmd.size()+1+4 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+4);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5 )
{
    _cmd.resize( _cmd.size()+1+5 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+5);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
}


//------------------------------------------------------------------------------

inline void
CommandBuffer_t::Command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6 )
{
    _cmd.resize( _cmd.size()+1+6 );

    std::vector<uint32>::iterator   b = _cmd.end() - (1+6);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
    b[6] = arg6; 
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::Replay()
{
    Handle  cur_ps = 0;
    Handle  vp_const[MAX_CONST_BUFFER_COUNT];
    void*   vp_const_data[MAX_CONST_BUFFER_COUNT];
    Handle  fp_const[MAX_CONST_BUFFER_COUNT];
    void*   fp_const_data[MAX_CONST_BUFFER_COUNT];

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        vp_const[i] = InvalidHandle;
        fp_const[i] = InvalidHandle;
    }

    for( std::vector<uint32>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint32                        cmd = *c;
        std::vector<uint32>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

        switch( cmd )
        {
            case GLES2__BEGIN :
            {
                GL_CALL(glFrontFace( GL_CW ));
                GL_CALL(glEnable( GL_CULL_FACE ));
                GL_CALL(glCullFace( GL_BACK ));
                
                GL_CALL(glEnable( GL_DEPTH_TEST ));
                GL_CALL(glDepthFunc( GL_LEQUAL ));
                GL_CALL(glDepthMask( GL_TRUE ));
            }   break;
            
            case GLES2__END :
            {
                glFlush();
            }   break;
            
            case GLES2__CLEAR :
            {
                uint32  clr (arg[0]);
                float   z   = nonaliased_cast<uint32,float>(arg[1]);
                
                glClearColor( float((clr>>0)&0xFF)/255.0f, float((clr>>8)&0xFF)/255.0f, float((clr>>16)&0xFF)/255.0f, float((clr>>24)&0xFF)/255.0f );
                glClearDepth( z );
                glClear( GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT );
                
                c += 2;    
            }   break;
            
            case GLES2__SET_STREAM_SOURCE :
            {
                VertexBufferGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 2;
            }   break;
            
            case GLES2__SET_INDICES :
            {
                IndexBufferGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case GLES2__SET_PIPELINE_STATE :
            {
//                PipelineState_SetToRHI( (PipelineState*)(arg[0]) );
                cur_ps = (Handle)(arg[0]);
                c += 1;
            }   break;
            
            case GLES2__SET_VERTEX_PROG_CONST_BUFFER :
            {
                vp_const[ arg[0] ]      = (Handle)(arg[1]);
                vp_const_data[ arg[0] ] = (void*)(arg[2]);
                c += 3;
            }   break;

            case GLES2__SET_FRAGMENT_PROG_CONST_BUFFER :
            {
                fp_const[ arg[0] ]      = (Handle)(arg[1]);
                fp_const_data[ arg[0] ] = (void*)(arg[2]);
                c += 3;
            }   break;
            
            case GLES2__DRAW_PRIMITIVE :
            {
                unsigned    v_cnt   = arg[1];
                int         mode    = arg[0];

                PipelineStateGLES2::SetToRHI( cur_ps );

                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( vp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( vp_const[i], vp_const_data[i] );
                }
                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( fp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( fp_const[i], fp_const_data[i] );
                }
                
                GL_CALL(glDrawArrays( mode, 0, v_cnt ));

                c += 2;    
            }   break;
            
            case GLES2__DRAW_INDEXED_PRIMITIVE :
            {
                unsigned    v_cnt   = arg[1];
                int         mode    = arg[0];

                PipelineStateGLES2::SetToRHI( cur_ps );

                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( vp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( vp_const[i], vp_const_data[i] );
                }
                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( fp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( fp_const[i], fp_const_data[i] );
                }
                
                GL_CALL(glDrawElements( mode, v_cnt, GL_UNSIGNED_SHORT, NULL ));

                c += 2;    
            }   break;

        }
    }
}



//------------------------------------------------------------------------------

void
Present()
{
    HWND    wnd = (HWND)DAVA::Core::Instance()->NativeWindowHandle();
    HDC     dc  = ::GetDC( wnd );

    Pool<CommandBuffer_t>::Get(CommandBuffer::Default())->Replay();

    SwapBuffers( dc );
}


} // namespace rhi
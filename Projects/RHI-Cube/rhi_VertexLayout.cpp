    #include "rhi_VertexLayout.h"

    #include "Debug/DVAssert.h"
    #include "Base/BaseTypes.h"
    using DAVA::uint32;
    using DAVA::uint16;
    using DAVA::uint8;
    using DAVA::int8;
    #include "FileSystem/Logger.h"
    using DAVA::Logger;


namespace rhi
{
//==============================================================================

VertexLayout::VertexLayout()
  : _elem_count(0)
{
}


//------------------------------------------------------------------------------

VertexLayout::~VertexLayout()
{
}


//------------------------------------------------------------------------------

void
VertexLayout::Clear()
{
    _elem_count = 0;
}


//------------------------------------------------------------------------------

void
VertexLayout::AddElement( VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension )
{
    DVASSERT(_elem_count < MaxElemCount);
    Element*    e = _elem + _elem_count;

    e->usage        = usage;
    e->usage_index  = usage_i;
    e->data_type    = type;
    e->data_count   = dimension;

    ++_elem_count;
}


//------------------------------------------------------------------------------
/*
void
VertexLayout::insert_elem( unsigned pos, VertexSemantics usage, unsigned usage_i, VertexDataType type, unsigned dimension )
{
    Element*    e = _elem.insert( pos );

    e->usage        = usage;
    e->usage_index  = usage_i;
    e->data_type    = type;
    e->data_count   = dimension;
}
*/

//------------------------------------------------------------------------------

unsigned
VertexLayout::Stride() const
{
    unsigned    sz = 0;

    for( unsigned e=0; e!=_elem_count; ++e )
        sz += ElementSize( e );

    return sz;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementCount() const
{
    return _elem_count;
}


//------------------------------------------------------------------------------

VertexSemantics
VertexLayout::ElementSemantics( unsigned elem_i ) const
{
    return (VertexSemantics)(_elem[elem_i].usage);
}


//------------------------------------------------------------------------------

VertexDataType
VertexLayout::ElementDataType( unsigned elem_i ) const
{
    return (VertexDataType)(_elem[elem_i].data_type);
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementDataCount( unsigned elem_i ) const
{
    return _elem[elem_i].data_count;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementOffset( unsigned elem_i ) const
{
    unsigned    off = 0;

    for( unsigned e=0; e<elem_i; ++e )
        off += ElementSize( e );

    return off;
}


//------------------------------------------------------------------------------

unsigned
VertexLayout::ElementSize( unsigned elem_i ) const
{
    const Element&  e = _elem[elem_i];

    switch( e.data_type )
    {
        case VDT_FLOAT   : return sizeof(float) * e.data_count;
        case VDT_HALF    : return sizeof(uint16) * e.data_count;
        case VDT_INT8N   : return sizeof(int8) * e.data_count;
        case VDT_UINT8N  : return sizeof(uint8) * e.data_count;
        case VDT_UINT8   : return sizeof(uint8) * e.data_count;
        case VDT_INT16N  : return sizeof(uint16) * e.data_count;
    }

    return 0;
}


//------------------------------------------------------------------------------

void
VertexLayout::Dump() const
{
    for( unsigned e=0; e!=_elem_count; ++e )
    {
        Logger::Info
        ( 
            "[%u] +%02u  %s%u  %s x%u", 
            e, ElementOffset(e),
            VertexSemanticsName( VertexSemantics(_elem[e].usage) ), _elem[e].usage_index,
            VertexDataTypeName( VertexDataType(_elem[e].data_type)), _elem[e].data_count 
        );

    }
    Logger::Info( "stride = %u\n", Stride() );
}


//------------------------------------------------------------------------------

bool
VertexLayout::operator==( const VertexLayout& vl ) const
{
    return (this->_elem_count == vl._elem_count)
           ? !memcmp( _elem, vl._elem, _elem_count*sizeof(Element) )
           : false;
}

//------------------------------------------------------------------------------

VertexLayout&
VertexLayout::operator=( const VertexLayout& src )
{
    this->_elem_count = src._elem_count;
    
    for( unsigned e=0; e!=_elem_count; ++e )
        this->_elem[e] = src._elem[e];

    return *this;
}


//==============================================================================
} // namespace rhi

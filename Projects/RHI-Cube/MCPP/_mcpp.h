#if !defined ___MCPP_H__
#define ___MCPP_H__

#define PREPROCESSED    0
#define HAVE_CONFIG_H   0
#define MCPP_LIB        1
#define DLL_EXPORT      0

//#if defined(__DAVAENGINE_WIN32__)
    #pragma warning( disable: 4018 4068 4101 4102 4146 )
//#endif


    #include <stdio.h>


FILE*   mcpp__fopen( const char* filename, const char* mode );
int     mcpp__fclose( FILE* file );
int     mcpp__ferror( FILE* file );
char*   mcpp__fgets( char* buf, int max_size, FILE* file );


void    mcpp__set_input( const void* data, unsigned data_sz );


#endif // ___MCPP_H__
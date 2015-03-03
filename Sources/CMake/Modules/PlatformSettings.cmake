
#compiller flags
if( NOT DISABLE_DUBUG )
    set( CMAKE_CXX_FLAGS_DEBUG     "${CMAKE_CXX_FLAGS_DEBUG} -D__DAVAENGINE_DEBUG__" )

endif  ()

if     ( ANDROID )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wno-invalid-offsetof" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -mfloat-abi=softfp -mfpu=neon -Wno-invalid-offsetof -frtti" )    
    
elseif ( IOS     ) 
    set( CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -mno-thumb"  )
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mno-thumb -fvisibility=hidden" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )

    set( CMAKE_IOS_SDK_ROOT Latest IOS )
    set( CMAKE_OSX_ARCHITECTURES armv7 armv7s i386 arm64 )

    if( NOT IOS_BUNDLE_IDENTIFIER )
        set( IOS_BUNDLE_IDENTIFIER com.davaconsulting.${PROJECT_NAME} )
        
    endif()
    
    # Fix try_compile
    set( MACOSX_BUNDLE_GUI_IDENTIFIER  ${IOS_BUNDLE_IDENTIFIER} )
    set( CMAKE_MACOSX_BUNDLE YES )
    set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer" )

elseif ( MACOS )
    set( CMAKE_OSX_DEPLOYMENT_TARGET "10.8" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
    set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14" )

elseif ( WIN32)
    set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP" ) 
    set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP" ) 
    set ( CMAKE_EXE_LINKER_FLAGS_RELEASE "/ENTRY:mainCRTStartup" )
    
    if( WARNINGS_AS_ERRORS_WIN32 )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
    endif  ()

endif  ()


if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
  
elseif ( MACOS )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

elseif ( WIN32)
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()

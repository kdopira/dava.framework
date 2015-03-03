include ( GlobalVariables      )

if( ANDROID AND NOT CMAKE_TOOLCHAIN_FILE )
    set( CMAKE_TOOLCHAIN_FILE ${DAVA_ROOT_DIR}/Sources/CMake/Toolchains/android.toolchain.cmake )
    find_package( AndroidTools REQUIRED )

    if( WIN32 )
        set( MAKE_PROGRAM ${ANDROID_NDK}/prebuilt/windows-x86_64/bin/make.exe ) 

    elseif( APPLE )
       set( MAKE_PROGRAM ${ANDROID_NDK}/prebuilt/darwin-x86_64/bin/make ) 

    endif()

    file( TO_CMAKE_PATH "${MAKE_PROGRAM}" MAKE_PROGRAM )
    set (CMAKE_MAKE_PROGRAM "${MAKE_PROGRAM}" CACHE STRING   "Program used to build from makefiles.")
    mark_as_advanced(CMAKE_MAKE_PROGRAM)

elseif( IOS AND NOT CMAKE_TOOLCHAIN_FILE )
    set( CMAKE_TOOLCHAIN_FILE ${DAVA_ROOT_DIR}/Sources/CMake/Toolchains/ios.toolchain.cmake )
 
endif()

include ( PlatformSettings     )
include ( MergeStaticLibrarees )
include ( FileTreeCheck        )
include ( DavaTemplate         )
include ( CMakeDependentOption )
include ( CMakeParseArguments  )


set( CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebinfo" CACHE STRING "limited configs" FORCE )

# Macro for precompiled headers
macro (enable_pch)
    if (MSVC)
        foreach (FILE ${SOURCE_FILES})
            if (FILE MATCHES \\.cpp$)
                if (FILE MATCHES Precompiled\\.cpp$)
                    set_source_files_properties (${FILE} PROPERTIES COMPILE_FLAGS "/YcPrecompiled.h")
                else ()
                    set_source_files_properties (${FILE} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.h")
                endif ()
            endif ()
        endforeach ()
    else ()
        # TODO: to enable usage of precompiled header in GCC, for now just make sure the correct Precompiled.h is found in the search
        foreach (FILE ${SOURCE_FILES})
            if (FILE MATCHES Precompiled\\.h$)
                get_filename_component (PATH ${FILE} PATH)
                include_directories (${PATH})
                break ()
            endif ()
        endforeach ()
    endif ()
endmacro ()

# Macro for defining source files with optional arguments as follows:
#  GLOB_CPP_PATTERNS <list> - Use the provided globbing patterns for CPP_FILES instead of the default *.cpp
#  GLOB_H_PATTERNS <list> - Use the provided globbing patterns for H_FILES instead of the default *.h
#  EXTRA_CPP_FILES <list> - Include the provided list of files into CPP_FILES result
#  EXTRA_H_FILES <list> - Include the provided list of files into H_FILES result
#  PCH - Enable precompiled header on the defined source files
#  PARENT_SCOPE - Glob source files in current directory but set the result in parent-scope's variable ${DIR}_CPP_FILES and ${DIR}_H_FILES instead
macro (define_source_files)
    # Parse extra arguments
    cmake_parse_arguments (ARG "PCH;PARENT_SCOPE" "GROUP" "EXTRA_CPP_FILES;EXTRA_H_FILES;GLOB_CPP_PATTERNS;GLOB_H_PATTERNS;GLOB_ERASE_FILES" ${ARGN})

    # Source files are defined by globbing source files in current source directory and also by including the extra source files if provided
    if (NOT ARG_GLOB_CPP_PATTERNS)
        set (ARG_GLOB_CPP_PATTERNS *.c *.cpp )    # Default glob pattern
        if( APPLE )  
            list ( APPEND ARG_GLOB_CPP_PATTERNS *.m *.mm )
        endif  ()
    endif ()
    
    if (NOT ARG_GLOB_H_PATTERNS)
        set (ARG_GLOB_H_PATTERNS *.h *.hpp)
    endif ()

    file (GLOB CPP_FILES ${ARG_GLOB_CPP_PATTERNS} )
    file (GLOB H_FILES ${ARG_GLOB_H_PATTERNS} )

    list (APPEND CPP_FILES ${ARG_EXTRA_CPP_FILES})
    list (APPEND H_FILES ${ARG_EXTRA_H_FILES})
    set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
    
    # Optionally enable PCH                                                           	                                                                                   	
    if (ARG_PCH)
        enable_pch ()
    endif ()

    if (ARG_PARENT_SCOPE)
        get_filename_component (DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    endif ()

    if ( ARG_GLOB_ERASE_FILES )
        foreach (ERASE_FILE ${ARG_GLOB_ERASE_FILES})
        foreach (FILE_PATH ${H_FILES})
            get_filename_component ( FILE_NAME ${FILE_PATH} NAME)
            if( ${FILE_NAME} STREQUAL  ${ERASE_FILE} )
                list (REMOVE_ITEM H_FILES ${FILE_PATH} )
            endif ()
        endforeach ()
        endforeach ()

        foreach (ERASE_FILE ${ARG_GLOB_ERASE_FILES})
        foreach (FILE_PATH ${CPP_FILES})
            get_filename_component ( FILE_NAME ${FILE_PATH} NAME)
            if( ${FILE_NAME} STREQUAL  ${ERASE_FILE} )
                list (REMOVE_ITEM CPP_FILES ${FILE_PATH} )
            endif ()
        endforeach ()
        endforeach ()
    endif ()
   
    # Optionally accumulate source files at parent scope
    if (ARG_PARENT_SCOPE)
        set (${DIR_NAME}_CPP_FILES ${CPP_FILES} PARENT_SCOPE)
        set (${DIR_NAME}_H_FILES ${H_FILES} PARENT_SCOPE)
    # Optionally put source files into further sub-group (only works for current scope due to CMake limitation)
    endif ()
        
endmacro ()

#
macro (define_source_folders )

    cmake_parse_arguments (ARG "RECURSIVE_CALL" "" "SRC_ROOT;GLOB_ERASE_FOLDERS" ${ARGN})
    
    IF( NOT ARG_RECURSIVE_CALL )
        set( PROJECT_SOURCE_FILES  ) 
        set( PROJECT_SOURCE_FILES_CPP  ) 
        set( PROJECT_SOURCE_FILES_HPP  ) 
         
        IF( ARG_SRC_ROOT )
            FOREACH( FOLDER_ITEM ${ARG_SRC_ROOT} )
                get_filename_component ( PATH ${FOLDER_ITEM} REALPATH ) 
                list ( APPEND  DAVA_FOLDERS ${PATH} ) 
            ENDFOREACH()
        ELSE()
            list ( APPEND DAVA_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR} ) 
        ENDIF()

    ENDIF()
    
    set( SOURCE_FOLDERS  )
    
    IF( ARG_SRC_ROOT )
    
        FOREACH( FOLDER_ITEM ${ARG_SRC_ROOT} )

            set ( CPP_PATTERNS ${FOLDER_ITEM}/*.c ${FOLDER_ITEM}/*.cpp )    
            if( APPLE )  
                list ( APPEND CPP_PATTERNS ${FOLDER_ITEM}/*.m  ${FOLDER_ITEM}/*.mm )
            endif  ()
        
            define_source_files ( GLOB_CPP_PATTERNS ${CPP_PATTERNS}
                                  GLOB_H_PATTERNS   ${FOLDER_ITEM}/*.h ${FOLDER_ITEM}/*.hpp )
                                  
            FILE( GLOB LIST_SOURCE_FOLDERS "${FOLDER_ITEM}/*" )

            list ( APPEND SOURCE_FOLDERS  ${LIST_SOURCE_FOLDERS} ) 
            list ( APPEND PROJECT_SOURCE_FILES_CPP  ${CPP_FILES} ) 
            list ( APPEND PROJECT_SOURCE_FILES_HPP  ${H_FILES}   ) 
            list ( APPEND PROJECT_SOURCE_FILES      ${CPP_FILES} ${H_FILES} )

        ENDFOREACH()

    ELSE()
        define_source_files ( )
        FILE( GLOB SOURCE_FOLDERS "*" )

        list ( APPEND PROJECT_SOURCE_FILES_CPP  ${CPP_FILES} ) 
        list ( APPEND PROJECT_SOURCE_FILES_HPP  ${H_FILES}   ) 
        list ( APPEND PROJECT_SOURCE_FILES      ${CPP_FILES} ${H_FILES} )

    ENDIF()
  
             
    FOREACH(FOLDER_ITEM ${SOURCE_FOLDERS})
        IF( IS_DIRECTORY "${FOLDER_ITEM}" )
            get_filename_component ( FOLDER_NAME ${FOLDER_ITEM} NAME ) 
            set( NOT_FIND_ERASE_ITEM 1 )
            FOREACH( ERASE_ITEM ${ARG_GLOB_ERASE_FOLDERS} )
                IF( ${FOLDER_NAME} STREQUAL ${ERASE_ITEM} )
                    set( NOT_FIND_ERASE_ITEM 0 )
                    break()     
                ENDIF()
            ENDFOREACH()
        
            IF( ${NOT_FIND_ERASE_ITEM} )
                FILE(GLOB FIND_CMAKELIST "${FOLDER_ITEM}/CMakeLists.txt")
                IF( FIND_CMAKELIST )
                    add_subdirectory ( ${FOLDER_ITEM} )
                    list ( APPEND PROJECT_SOURCE_FILES ${${FOLDER_NAME}_CPP_FILES} ${${FOLDER_NAME}_H_FILES} )    
    		        list ( APPEND PROJECT_SOURCE_FILES_CPP  ${${FOLDER_NAME}_CPP_FILES} ) 
                    list ( APPEND PROJECT_SOURCE_FILES_HPP  ${${FOLDER_NAME}_H_FILES}   ) 
                ELSE()
                    list (APPEND PROJECT_SOURCE_FILES ${CPP_FILES} ${H_FILES})
                    define_source_folders( SRC_ROOT ${FOLDER_ITEM} GLOB_ERASE_FOLDERS ${ARG_GLOB_ERASE_FOLDERS} RECURSIVE_CALL )
                ENDIF()
            ENDIF()
        ENDIF()
    ENDFOREACH()

endmacro ()

#
macro ( generate_source_groups_project )

    cmake_parse_arguments ( ARG "RECURSIVE_CALL"  "ROOT_DIR;GROUP_PREFIX" "SRC_ROOT;GROUP_FOLDERS" ${ARGN} )

    IF( ARG_ROOT_DIR )
        get_filename_component ( ROOT_DIR ${ARG_ROOT_DIR} REALPATH ) 

    else()
        set( ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} )

    ENDIF()

    IF( ARG_GROUP_PREFIX )
        set( GROUP_PREFIX  "${ARG_GROUP_PREFIX}\\" )
    else()
        set( GROUP_PREFIX "" )
    ENDIF()


    IF( ARG_SRC_ROOT ) 
        set( SRC_ROOT_LIST  )

        FOREACH( SRC_ITEM ${ARG_SRC_ROOT} )

            IF( "${SRC_ITEM}" STREQUAL "*" )
                list ( APPEND SRC_ROOT_LIST "*" )
            ELSE()
                get_filename_component ( SRC_ITEM ${SRC_ITEM} REALPATH ) 
                list ( APPEND SRC_ROOT_LIST ${SRC_ITEM}/* )
            ENDIF()
        ENDFOREACH()

    else()
        set( SRC_ROOT_LIST "*" )

    ENDIF()


    FOREACH( SRC_ROOT_ITEM ${SRC_ROOT_LIST} )
      
        file ( GLOB_RECURSE FILE_LIST ${SRC_ROOT_ITEM} )        

        FOREACH( ITEM ${FILE_LIST} )
            get_filename_component ( FILE_PATH ${ITEM} PATH ) 

            IF( "${FILE_PATH}" STREQUAL "${ROOT_DIR}" )
                STRING(REGEX REPLACE "${ROOT_DIR}" "" FILE_GROUP ${FILE_PATH} )
            ELSE()
                STRING(REGEX REPLACE "${ROOT_DIR}/" "" FILE_GROUP ${FILE_PATH} )
                STRING(REGEX REPLACE "/" "\\\\" FILE_GROUP ${FILE_GROUP})
            ENDIF()
            source_group( "${GROUP_PREFIX}${FILE_GROUP}" FILES ${ITEM} )

            #message( "<> "${GROUP_PREFIX}" ][ "${FILE_GROUP}" ][ "${ITEM} )
        ENDFOREACH()

    ENDFOREACH()

    IF( NOT ARG_RECURSIVE_CALL )
        FOREACH( GROUP_ITEM ${ARG_GROUP_FOLDERS} )
            if( IS_DIRECTORY "${${GROUP_ITEM}}" )
                generate_source_groups_project( RECURSIVE_CALL GROUP_PREFIX ${GROUP_ITEM}  ROOT_DIR ${${GROUP_ITEM}}  SRC_ROOT ${${GROUP_ITEM}}  )
            else()
                source_group( "${GROUP_ITEM}" FILES ${${GROUP_ITEM}} )
            endif()

        ENDFOREACH()
    ENDIF()

endmacro ()

#
macro ( install_libraries TARGET_NAME )

IF( DAVA_INSTALL )

install(
        TARGETS
        ${TARGET_NAME}
        DESTINATION
        ${DAVA_THIRD_PARTY_LIBRARIES_PATH} )

install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION
        "${DAVA_THIRD_PARTY_ROOT_PATH}/include/${TARGET_NAME}"
        FILES_MATCHING
        PATTERN
        "*.h" )

install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION
        "${DAVA_THIRD_PARTY_ROOT_PATH}/include/${TARGET_NAME}"
        FILES_MATCHING
        PATTERN
        "*.hpp" )

ENDIF()

endmacro ()

















  
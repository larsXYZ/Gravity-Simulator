####################################################################################################
# TGUI - Texus' Graphical User Interface
# Copyright (C) 2012-2024 Bruno Van de Velde (vdv_b@tgui.eu)
#
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it freely,
# subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented;
#    you must not claim that you wrote the original software.
#    If you use this software in a product, an acknowledgment
#    in the product documentation would be appreciated but is not required.
#
# 2. Altered source versions must be plainly marked as such,
#    and must not be misrepresented as being the original software.
#
# 3. This notice may not be removed or altered from any source distribution.
####################################################################################################

# Macro that helps defining an option.
# This code has a subtle difference compared to just calling "set(${var} ${default} CACHE ${type} ${docstring})":
#   If a normal (non-cache) variable already exists (e.g. for overwriting settings with TGUI in subdirectory),
#   then the cache variable is initialized with the value of the normal variable instead of the default value.
#   When re-running, the cache variable will always be reset to the explicitly set normal value. This is probably
#   better than keep showing the wrong value in the user interface and silently working with another value.
macro(tgui_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default} CACHE ${type} ${docstring} FORCE)
    else()
        set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
    endif()
endmacro()

# Macro to set a variable based on a boolean expression
# Usage: tgui_assign_bool(var cond1 AND cond2)
macro(tgui_assign_bool var)
    if(${ARGN})
        set(${var} ON)
    else()
        set(${var} OFF)
    endif()
endmacro()

# Set the compile options used by all targets
function(tgui_set_global_compile_flags target)
    if(TGUI_COMPILER_MSVC OR TGUI_COMPILER_CLANG_CL)
        target_compile_options(${target} PRIVATE
                               $<$<BOOL:${TGUI_WARNINGS_AS_ERRORS}>:/WX>
                               /W4
                               /permissive-
        )
    else()
        target_compile_options(${target} PRIVATE
                               $<$<BOOL:${TGUI_WARNINGS_AS_ERRORS}>:-Werror>
                               -Wall
                               -Wextra
                               -Wshadow
                               -Wsign-conversion
                               -Wfloat-conversion
                               -Wnon-virtual-dtor
                               -Wold-style-cast
                               -Wcast-align
                               -Wunused
                               -Woverloaded-virtual
                               -Wpedantic
                               -Wdouble-promotion
                               -Wformat=2
                               -Wimplicit-fallthrough
                               -Wnull-dereference
        )

        if(TGUI_COMPILER_GCC)
            target_compile_options(${target} PRIVATE
                                   -Wmisleading-indentation
                                   -Wduplicated-cond
                                   -Wlogical-op
            )
        endif()

        if(TGUI_COMPILER_GCC OR (TGUI_COMPILER_LLVM_CLANG AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11))
            target_compile_options(${target} PRIVATE -Wsuggest-override)
        endif()
    endif()

    if (NOT TGUI_BUILD_AS_CXX_MODULE)
        # We turn GNU extensions off to make sure that our code doesn't rely on them.
        # This isn't done when building TGUI as a module though. At least when using Clang 18, doing so required that the set_target_properties
        # function is also called on the user's target to avoid the "GNU extensions was enabled in PCH file but is currently disabled" error.
        set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
    endif()
    target_compile_features(${target} PUBLIC cxx_std_${TGUI_CXX_STANDARD})

    if(TGUI_USE_MULTI_PROCESSOR_COMPILATION)
        target_compile_options(${target} PRIVATE /MP)
    endif()
endfunction()

# Set the appropriate standard library on each platform for the given target
function(tgui_set_stdlib target)
    # Apply the TGUI_USE_STATIC_STD_LIBS option on windows when using GCC.
    if(TGUI_OS_WINDOWS AND TGUI_COMPILER_GCC)
        if(TGUI_USE_STATIC_STD_LIBS AND NOT TGUI_COMPILER_GCC_TDM)
            target_link_libraries(${target} PRIVATE "-static-libgcc" "-static-libstdc++")
        elseif(NOT TGUI_USE_STATIC_STD_LIBS AND TGUI_COMPILER_GCC_TDM)
            target_link_libraries(${target} PRIVATE "-shared-libgcc" "-shared-libstdc++")
        endif()
    endif()

    # Apply the TGUI_USE_STATIC_STD_LIBS option on windows when using Visual Studio.
    if(TGUI_COMPILER_MSVC OR TGUI_COMPILER_CLANG_CL)
        if(TGUI_USE_STATIC_STD_LIBS)
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
endfunction()

# Install the dlls next to the executables (both immediately after building and when installing them somewhere)
function(copy_dlls_to_exe post_build_destination install_destination target)
    if(TGUI_OS_WINDOWS)
        set(files_to_copy "")

        # Copy the TGUI dll if we built one
        if(TGUI_SHARED_LIBS)
            list(APPEND files_to_copy "$<TARGET_FILE:tgui>")
        endif()

        # Copy the FreeType dll if needed and if we know where it is
        if(TGUI_HAS_FONT_BACKEND_FREETYPE AND FREETYPE_WINDOWS_BINARIES_PATH AND NOT TGUI_USE_STATIC_STD_LIBS)
            # Turn backslashes into slashes on Windows, because the install() command can fail if the
            # FREETYPE_WINDOWS_BINARIES_PATH variable was initialized with an environment variable that contains backslashes.
            if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.20)
                cmake_path(SET FREETYPE_WINDOWS_BINARIES_PATH "${FREETYPE_WINDOWS_BINARIES_PATH}")
            else()
                file(TO_CMAKE_PATH "${FREETYPE_WINDOWS_BINARIES_PATH}" FREETYPE_WINDOWS_BINARIES_PATH)
            endif()

            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(freetype_dll "${FREETYPE_WINDOWS_BINARIES_PATH}/release dll/win64/freetype.dll")
            else()
                set(freetype_dll "${FREETYPE_WINDOWS_BINARIES_PATH}/release dll/win32/freetype.dll")
            endif()
            if(EXISTS "${freetype_dll}")
                list(APPEND files_to_copy "${freetype_dll}")
            else() # Look for file structure that was used prior to FreeType 2.11
                if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                    set(freetype_dll "${FREETYPE_WINDOWS_BINARIES_PATH}/win64/freetype.dll")
                else()
                    set(freetype_dll "${FREETYPE_WINDOWS_BINARIES_PATH}/win32/freetype.dll")
                endif()

                if(EXISTS "${freetype_dll}")
                    list(APPEND files_to_copy "${freetype_dll}")
                endif()
            endif()
        endif()

        # TODO: SFML, SDL, SDL_ttf, GLFW and raylib

        # Previously we were just listing the files to copy, now we will actually give the commands for the copying.
        # We are merely setting triggers here, the actual copying only happens after building or when installing.
        foreach(file_to_copy ${files_to_copy})
            add_custom_command(TARGET ${target} POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy "${file_to_copy}" "${post_build_destination}"
                               VERBATIM)

            if (TGUI_INSTALL)
                install(FILES "${file_to_copy}"
                        DESTINATION "${install_destination}"
                        COMPONENT ${target})
            endif()
        endforeach()
    endif()
endfunction()

# Bundles a set of source files into a c++20 module
function(tgui_create_module_from_sources source_file_list module_name)
    file(READ "TGUI-Module.cppm.in" file_contents)
    string(APPEND file_contents "\nexport module ${module_name};\n")
    string(APPEND file_contents "\nexport import tgui;\n")
    if(${ARGC} GREATER 2)
        foreach(extra_module_to_export ${ARGN})
            string(APPEND file_contents "export import ${extra_module_to_export};\n")
        endforeach()
    endif()
    string(APPEND file_contents "\n")
    foreach(source_file ${source_file_list})
        string(APPEND file_contents "#include \"${CMAKE_CURRENT_SOURCE_DIR}/${source_file}\"\n")
    endforeach()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/module_${module_name}.cppm" "${file_contents}")
    set(module_source "${CMAKE_CURRENT_BINARY_DIR}/module_${module_name}.cppm" PARENT_SCOPE)
endfunction()

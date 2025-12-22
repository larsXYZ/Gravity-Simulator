cmake_minimum_required(VERSION 3.3)

project(Gravity-Simulator)
set(Gravity_Simulator_VERSION_MAJOR 1)
set(Gravity_Simulator_VERSION_MINOR 1)
set(Gravity_Simulator_VERSION_PATCH 0)
set(Gravity_Simulator_HOMEPAGE_URL "https://www.youtube.com/@LarsGames_")
set(Gravity_Simulator_DESCRIPTION "A simple 2D gravity simulator")

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 
set(CMAKE_CXX_EXTENSIONS OFF)

set(TGUI_BUILD_GUI_BUILDER False)

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/)

set(TGUI_BACKEND "SFML_GRAPHICS")
file(GLOB_RECURSE SOURCE "src/*")

if(MSVC)
    add_compile_options($<$<CONFIG:Debug>:/ZI>)
    add_link_options($<$<CONFIG:Debug>:/INCREMENTAL>)

    # Optimization flags for Release builds
    add_compile_options($<$<CONFIG:Release>:/O2>)      # Maximize Speed
    add_compile_options($<$<CONFIG:Release>:/Oi>)      # Enable Intrinsic Functions
    add_compile_options($<$<CONFIG:Release>:/Ot>)      # Favor Fast Code
    add_compile_options($<$<CONFIG:Release>:/GL>)      # Whole Program Optimization
    add_compile_options($<$<CONFIG:Release>:/Gy>)      # Enable Function-Level Linking
    
    add_link_options($<$<CONFIG:Release>:/LTCG>)      # Link Time Code Generation
    add_link_options($<$<CONFIG:Release>:/OPT:REF>)   # Eliminate Unreferenced Data
    add_link_options($<$<CONFIG:Release>:/OPT:ICF>)   # Enable COMDAT Folding
endif()

# Detect the target architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
else()
    set(ARCHITECTURE "x86")
endif()

message(STATUS "Target architecture: ${ARCHITECTURE}")

add_subdirectory(SFML)
add_subdirectory(TGUI)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})

# Add executable with conditional subsystem
add_executable(Gravity-Simulator ${SOURCE} ${CMAKE_SOURCE_DIR}/resources/resources.rc)

set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

target_link_libraries(Gravity-Simulator sfml-graphics 
					sfml-window
					sfml-system
					sfml-audio
					tgui
					${SFML_DEPENDENCIES})

# Set the output directory for binaries
set_target_properties(Gravity-Simulator  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy DLLs to output directory
add_custom_command(TARGET Gravity-Simulator  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/SFML/lib ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/TGUI/lib ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/
)

install(FILES ${CMAKE_BINARY_DIR}/sansation.ttf DESTINATION "./bin")
install(DIRECTORY resources DESTINATION "./bin")
install(TARGETS Gravity-Simulator)

include(InstallRequiredSystemLibraries)

set(CPACK_WIX_VERSION 4)
set(CPACK_PACKAGE_NAME "Gravity-Simulator")
set(CPACK_PACKAGE_EXECUTABLES Gravity-Simulator "Gravity-Simulator")
set(CPACK_PACKAGE_VENDOR "Lars Games")
set(CPACK_PACKAGE_ICON  "${CMAKE_BINARY_DIR}/logo/logo.ico")
set(CPACK_PACKAGE_VERSION_MAJOR ${Gravity_Simulator_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Gravity_Simulator_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${Gravity_Simulator_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION ${Gravity_Simulator_DESCRIPTION})
set(CPACK_PACKAGE_HOMEPAGE_URL ${Gravity_Simulator_HOMEPAGE_URL})
set(CPACK_CREATE_DESKTOP_LINKS Gravity-Simulator)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_MONOLITHIC_INSTALL TRUE)

include(CPack)
cmake_minimum_required(VERSION 3.3)

project(Gravity-Simulator)
set(Gravity_Simulator_VERSION_MAJOR 1)
set(Gravity_Simulator_VERSION_MINOR 2)
set(Gravity_Simulator_VERSION_PATCH 2)
set(Gravity_Simulator_HOMEPAGE_URL "https://www.youtube.com/@LarsGames_")
set(Gravity_Simulator_DESCRIPTION "A simple 2D gravity simulator")

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 
set(CMAKE_CXX_EXTENSIONS OFF)

set(TGUI_BUILD_GUI_BUILDER False)

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/)

set(TGUI_BACKEND "SFML_GRAPHICS")
file(GLOB_RECURSE SOURCE "src/*")
list(FILTER SOURCE EXCLUDE REGEX "src/benchmark.cpp$")
list(FILTER SOURCE EXCLUDE REGEX "src/debug_heat.cpp$")

find_package(OpenMP)

# Define source for Benchmark
file(GLOB_RECURSE BENCHMARK_SOURCE "src/*")
list(FILTER BENCHMARK_SOURCE EXCLUDE REGEX "src/main.cpp$")
list(FILTER BENCHMARK_SOURCE EXCLUDE REGEX "src/debug_heat.cpp$")

# Define source for HeatDebug
file(GLOB_RECURSE HEATDEBUG_SOURCE "src/*")
list(FILTER HEATDEBUG_SOURCE EXCLUDE REGEX "src/main.cpp$")
list(FILTER HEATDEBUG_SOURCE EXCLUDE REGEX "src/benchmark.cpp$")

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

    if(OpenMP_CXX_FOUND)
        add_compile_options($<$<CONFIG:Release>:${OpenMP_CXX_FLAGS}>)
        add_compile_options($<$<CONFIG:Debug>:${OpenMP_CXX_FLAGS}>)
    endif()
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

set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_DEBUG "/DEBUG /SUBSYSTEM:CONSOLE")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/DEBUG /SUBSYSTEM:WINDOWS")
set_target_properties(Gravity-Simulator PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

target_link_libraries(Gravity-Simulator sfml-graphics 
					sfml-window
					sfml-system
					sfml-audio
					tgui
					${SFML_DEPENDENCIES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(Gravity-Simulator OpenMP::OpenMP_CXX)
endif()

# Benchmark executable
add_executable(Benchmark ${BENCHMARK_SOURCE})
target_link_libraries(Benchmark sfml-graphics 
					sfml-window
					sfml-system
					sfml-audio
					tgui
					${SFML_DEPENDENCIES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(Benchmark OpenMP::OpenMP_CXX)
endif()

set_target_properties(Benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# HeatDebug executable
add_executable(HeatDebug ${HEATDEBUG_SOURCE})
target_link_libraries(HeatDebug sfml-graphics 
					sfml-window
					sfml-system
					sfml-audio
					tgui
					${SFML_DEPENDENCIES})
if(OpenMP_CXX_FOUND)
    target_link_libraries(HeatDebug OpenMP::OpenMP_CXX)
endif()
set_target_properties(HeatDebug PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_custom_command(TARGET Benchmark POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/SFML/lib/$<CONFIG> $<TARGET_FILE_DIR:Benchmark>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/TGUI/lib/$<CONFIG> $<TARGET_FILE_DIR:Benchmark>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:Benchmark>/resources
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/sansation.ttf $<TARGET_FILE_DIR:Benchmark>/sansation.ttf
)

add_custom_command(TARGET HeatDebug POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/SFML/lib/$<CONFIG> $<TARGET_FILE_DIR:HeatDebug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/TGUI/lib/$<CONFIG> $<TARGET_FILE_DIR:HeatDebug>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:HeatDebug>/resources
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/resources/sansation.ttf $<TARGET_FILE_DIR:HeatDebug>/sansation.ttf
)

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
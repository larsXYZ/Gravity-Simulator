cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE Debug) 

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/)
set(TGUI_BACKEND "SFML_GRAPHICS")
file(GLOB_RECURSE SOURCE "src/*")


add_subdirectory(SFML)
add_subdirectory(TGUI)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
add_executable(Gravity-Simulator ${SOURCE})
target_link_libraries(Gravity-Simulator sfml-graphics sfml-window sfml-system tgui)

# Set the output directory for binaries
set_target_properties(Gravity-Simulator  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug
)

# Copy DLLs to output directory
add_custom_command(TARGET Gravity-Simulator  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/SFML/lib ${CMAKE_BINARY_DIR}/Debug
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/TGUI/lib ${CMAKE_BINARY_DIR}/Debug
)

cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

set(CMAKE_BUILD_TYPE Debug) 

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML)
set(TGUI_BACKEND "SFML_GRAPHICS")

file(GLOB SOURCE "*.h" "*.hpp" "*.cpp")

add_subdirectory(SFML)
add_subdirectory(TGUI)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
add_executable(Gravity-Simulator ${SOURCE})
target_link_libraries(Gravity-Simulator SFML TGUI)
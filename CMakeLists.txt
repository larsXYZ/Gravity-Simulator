cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

set(CMAKE_BUILD_TYPE Debug) 

add_subdirectory(SFML)
add_subdirectory(TGUI)

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/)
set(TGUI_BACKEND "SFML_GRAPHICS")

file(GLOB SOURCE "src/*")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
add_executable(Gravity-Simulator ${SOURCE})
target_link_libraries(Gravity-Simulator sfml-graphics sfml-window sfml-system tgui)
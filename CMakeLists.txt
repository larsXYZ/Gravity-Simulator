cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML)
set(TGUI_BACKEND "SFML_GRAPHICS")

file(GLOB SOURCE "*.h" "*.hpp" "*.cpp")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SFML)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/TGUI)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
add_executable(Gravity-Simulator ${SOURCE})
target_link_libraries(Gravity-Simulator ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${TGUI_LIBRARY})
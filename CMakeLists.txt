cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

find_package(SFML 2.3 COMPONENTS graphics audio window system network)
find_package(TGUI)

file(GLOB SOURCE "*.h" "*.hpp" "*.cpp")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} ${TGUI_INCLUDE_DIR})
add_executable(Gravity-Simulator ${SOURCE})
target_link_libraries(Gravity-Simulator ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} ${TGUI_LIBRARY})
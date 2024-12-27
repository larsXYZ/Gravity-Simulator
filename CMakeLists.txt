cmake_minimum_required(VERSION 3.0)

project(Gravity-Simulator)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 
set(CMAKE_CXX_EXTENSIONS OFF)

set(TGUI_BUILD_GUI_BUILDER False)

set(SFML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/Include)
set(TGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TGUI/Include)
set(SFML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SFML/)

set(TGUI_BACKEND "SFML_GRAPHICS")
file(GLOB_RECURSE SOURCE "src/*")

if(MSVC)
    add_compile_options(/ZI)
    add_link_options(/INCREMENTAL)
endif()

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
    ${CMAKE_BINARY_DIR}/SFML/lib ${CMAKE_BINARY_DIR}/Debug
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_BINARY_DIR}/TGUI/lib ${CMAKE_BINARY_DIR}/Debug
)

# Include CPack for installer creation
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "Gravity-Simulator")
set(CPACK_PACKAGE_VERSION "1.1.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gravity Simulator")
set(CPACK_PACKAGE_VENDOR "LarsGames")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")

# Windows specific options
if(WIN32)
    set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} \\ ${CPACK_PACKAGE_NAME}")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/logo/logo.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/logo/logo.ico")
endif()

include(CPack)

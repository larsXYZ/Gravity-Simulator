cmake_minimum_required(VERSION 3.27)

project(Gravity-Simulator)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
        tgui
        GIT_REPOSITORY https://github.com/texus/TGUI.git
        GIT_TAG        50ee3ed9163b90a7e3c162baae55fbf1fa612a2e # release-0.7.8
)
FetchContent_MakeAvailable(tgui)

find_package(SFML 2.6 COMPONENTS graphics window system)

include_directories(
        include
        ${tgui_SOURCE_DIR}/include
)

add_executable(Gravity-Simulator
    src/Effect.cpp
    src/main.cpp
    src/planet.cpp
    src/rom.cpp
    src/runsim.cpp
    src/Spaceship.cpp
)

target_link_libraries(Gravity-Simulator tgui)

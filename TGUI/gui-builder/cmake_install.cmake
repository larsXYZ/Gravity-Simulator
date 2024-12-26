# Install script for directory: C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Gravity-Simulator")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "gui-builder" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE FILE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/lib/Debug/tgui-d.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE FILE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/lib/Release/tgui.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE FILE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/lib/MinSizeRel/tgui.dll")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE FILE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/lib/RelWithDebInfo/tgui.dll")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "gui-builder" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE EXECUTABLE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder/Debug/gui-builder.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE EXECUTABLE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder/Release/gui-builder.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE EXECUTABLE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder/MinSizeRel/gui-builder.exe")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE EXECUTABLE FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder/RelWithDebInfo/gui-builder.exe")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "gui-builder" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE DIRECTORY FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/gui-builder/resources")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "gui-builder" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./gui-builder" TYPE DIRECTORY FILES "C:/Users/thehu/Documents/Temporary/gsim/Gravity-Simulator/TGUI/themes")
endif()


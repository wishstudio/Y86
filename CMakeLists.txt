cmake_minimum_required(VERSION 2.8.6)

project (Y86)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_AUTOMOC TRUE)

find_package(Qt4 COMPONENTS QtCore QtGui QtScript REQUIRED)

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

add_subdirectory(src)

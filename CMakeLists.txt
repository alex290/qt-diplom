cmake_minimum_required(VERSION 3.16)
project(AirportInspector VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Network)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/database.cpp
    src/flightsearch.cpp
    src/airportinfo.cpp
    src/ticketbooking.cpp
    src/userprofile.cpp
    src/airportloading.cpp
    include/mainwindow.h
    include/database.h
    include/flightsearch.h
    include/airportinfo.h
    include/ticketbooking.h
    include/userprofile.h
    include/airportloading.h
    ui/mainwindow.ui
    ui/flightsearch.ui
    ui/airportinfo.ui
    ui/ticketbooking.ui
    ui/userprofile.ui
    resources/resources.qrc
)

add_executable(AirportInspector ${PROJECT_SOURCES})

target_include_directories(AirportInspector PRIVATE include)

target_link_libraries(AirportInspector PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Install the executable
install(TARGETS AirportInspector
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
) 
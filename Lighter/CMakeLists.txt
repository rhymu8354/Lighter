# CMakeLists.txt for Lighter
#
# © 2019 by Richard Walters

cmake_minimum_required(VERSION 3.8)
set(This Lighter)

set(Sources
    src/main.cpp
    src/TimeKeeper.cpp
    src/TimeKeeper.hpp
)

add_executable(${This} ${Sources})
set_target_properties(${This} PROPERTIES
    FOLDER Applications
)

target_link_libraries(${This} PUBLIC
    Http
    HttpNetworkTransport
    O9KClock
    SystemAbstractions
    Timekeeping
)

if(UNIX AND NOT APPLE)
    target_link_libraries(${This} PRIVATE
        -static-libstdc++
    )
endif(UNIX AND NOT APPLE)

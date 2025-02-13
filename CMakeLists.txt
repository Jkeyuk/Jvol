cmake_minimum_required(VERSION 3.25.1)

project(jvol)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(src)
    
target_link_libraries(${PROJECT_NAME}  pulse
    ncurses
) 

install(TARGETS ${PROJECT_NAME})

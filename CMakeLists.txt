cmake_minimum_required(VERSION 3.26)
project(projectile_simulation C)

set(CMAKE_C_STANDARD 11)
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/libs/SDL2")

include_directories(${SDL2_DIR}/include)
link_directories(${SDL2_DIR}/lib)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()

add_executable(projectile_simulation main.c ball.c render.c utils.c)
target_link_libraries(projectile_simulation SDL2main SDL2)

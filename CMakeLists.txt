cmake_minimum_required(VERSION 3.12)
project(GravelBoy)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES 
    CONFIGURE_DEPENDS
    main.cpp 
    src/*.cpp 
    src/APU/*.cpp 
    src/Cartridge/*.cpp 
    src/CPU/*.cpp
    src/IO/*.cpp
    src/Memory/*.cpp
    src/Cartridge/MBC/*.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})


target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

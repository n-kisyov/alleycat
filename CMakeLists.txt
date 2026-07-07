cmake_minimum_required(VERSION 3.14)
project(alleycat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-devel-2.30.12-VC.zip
    SOURCE_DIR ${CMAKE_BINARY_DIR}/sdl2
)

FetchContent_GetProperties(SDL2)
if(NOT SDL2_POPULATED)
    FetchContent_Populate(SDL2)
endif()

set(SDL2_INCLUDE_DIR ${CMAKE_BINARY_DIR}/sdl2/include)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SDL2_LIBRARY ${CMAKE_BINARY_DIR}/sdl2/lib/x64/SDL2.lib)
    set(SDL2_DLL_DIR ${CMAKE_BINARY_DIR}/sdl2/lib/x64)
else()
    set(SDL2_LIBRARY ${CMAKE_BINARY_DIR}/sdl2/lib/x86/SDL2.lib)
    set(SDL2_DLL_DIR ${CMAKE_BINARY_DIR}/sdl2/lib/x86)
endif()

set(SDL2_LIBRARIES ${SDL2_LIBRARY})
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

include_directories(${SDL2_INCLUDE_DIRS})
add_definitions(-Dmain=SDL_main)

set(SOURCES
    src/main.c
    src/game.c
    src/sprites.c
    src/render.c
    src/player.c
    src/input.c
    src/sound.c
    src/scene.c
    src/scenes/title.c
    src/scenes/alley.c
    src/scenes/fishbowl.c
    src/scenes/kennel.c
    src/scenes/birdcage.c
    src/scenes/cheese.c
    src/scenes/love.c
    src/scenes/milk.c
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} winmm)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_DLL_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

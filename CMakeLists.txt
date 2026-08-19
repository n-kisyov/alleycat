cmake_minimum_required(VERSION 3.20)
project(alleycat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.c
    src/game.c
    src/sprites.c
    src/render.c
    src/player.c
    src/input.c
    src/sound.c
    src/scene.c
    src/scenes/room.c
    src/scenes/title.c
    src/scenes/alley.c
    src/scenes/gameover.c
    src/scenes/fishbowl.c
    src/scenes/kennel.c
    src/scenes/birdcage.c
    src/scenes/cheese.c
    src/scenes/love.c
    src/scenes/milk.c
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

#
# SDL2.  MSVC gets the prebuilt VC package fetched on demand; every other
# toolchain uses whatever SDL2 is installed, so the project also configures
# with MinGW, Clang and on Linux.
#
if(MSVC)
    include(FetchContent)
    FetchContent_Declare(SDL2
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.12/SDL2-devel-2.30.12-VC.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        SOURCE_DIR ${CMAKE_BINARY_DIR}/sdl2
    )
    # The archive carries no CMakeLists.txt, so this only populates it.
    FetchContent_MakeAvailable(SDL2)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_LIB_DIR ${CMAKE_BINARY_DIR}/sdl2/lib/x64)
    else()
        set(SDL2_LIB_DIR ${CMAKE_BINARY_DIR}/sdl2/lib/x86)
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/sdl2/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LIB_DIR}/SDL2.lib
        ${SDL2_LIB_DIR}/SDL2main.lib
        winmm
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_LIB_DIR}/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL2_LIBRARIES})
    if(NOT WIN32)
        target_link_libraries(${PROJECT_NAME} PRIVATE m)
    endif()
endif()

# SDL_main.h renames main() to SDL_main() on its own and SDL2main supplies the
# real entry point; the old -Dmain=SDL_main plus a hand-written WinMain only
# worked because the macro rewrote the call inside it too.
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

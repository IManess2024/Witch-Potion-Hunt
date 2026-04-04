cmake_minimum_required(VERSION 3.24)

project(WitchPotionHunt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_path(SFML_INCLUDE_DIR
    NAMES SFML/Graphics.hpp
    PATHS /usr/local/include
    REQUIRED
)

find_library(SFML_GRAPHICS_LIBRARY
    NAMES sfml-graphics
    PATHS /usr/local/lib
    REQUIRED
)

find_library(SFML_WINDOW_LIBRARY
    NAMES sfml-window
    PATHS /usr/local/lib
    REQUIRED
)

find_library(SFML_SYSTEM_LIBRARY
    NAMES sfml-system
    PATHS /usr/local/lib
    REQUIRED
)

add_executable(WitchPotionHunt
    main.cpp
    game_types.hpp
    gameplay.hpp
    levels.hpp
    rendering.hpp
)

target_include_directories(WitchPotionHunt PRIVATE ${SFML_INCLUDE_DIR})

target_link_libraries(WitchPotionHunt PRIVATE
    ${SFML_GRAPHICS_LIBRARY}
    ${SFML_WINDOW_LIBRARY}
    ${SFML_SYSTEM_LIBRARY}
)

if(MSVC)
    target_compile_options(WitchPotionHunt PRIVATE /W4 /permissive-)
else()
    target_compile_options(WitchPotionHunt PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT WitchPotionHunt)

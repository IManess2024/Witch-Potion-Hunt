cmake_minimum_required(VERSION 3.24)

project(WitchPotionHunt VERSION 0.1.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build SFML as static libraries so the first template run on Windows is simpler.
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/archive/refs/tags/3.0.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(SFML)

add_executable(WitchPotionHunt
    main.cpp
    game_types.hpp
    gameplay.hpp
    levels.hpp
    rendering.hpp
)

target_link_libraries(WitchPotionHunt PRIVATE SFML::Graphics)

if(WIN32)
    target_link_libraries(WitchPotionHunt PRIVATE SFML::Main)
    set_target_properties(WitchPotionHunt PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(MSVC)
    target_compile_options(WitchPotionHunt PRIVATE /W4 /permissive-)
else()
    target_compile_options(WitchPotionHunt PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT WitchPotionHunt)

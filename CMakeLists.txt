cmake_minimum_required(VERSION 3.23)
project(pill_game VERSION 1.0.0)

add_executable(pill_game)

file(GLOB_RECURSE PILL_GAME_SOURCE_FILES "src/pill_game/*.cpp")
file(GLOB_RECURSE PILL_GAME_HEADER_FILES "src/pill_game/*.h")
message(STATUS "${PILL_GAME_SOURCE_FILES} ${PILL_GAME_HEADER_FILES}")

target_sources(pill_game PRIVATE ${PILL_GAME_SOURCE_FILES} ${PILL_GAME_HEADER_FILES})

target_include_directories(pill_game PUBLIC ./src/)
target_compile_features(pill_game PRIVATE cxx_std_23)
target_compile_options(
    pill_game
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)
target_precompile_headers(pill_game PRIVATE src/pill_game/pch.h)
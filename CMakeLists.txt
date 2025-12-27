cmake_minimum_required(VERSION 3.23)
project(
    pill_game
    VERSION 1.0.0
    LANGUAGES C CXX
)

add_executable(pill_game)

file(GLOB_RECURSE PILL_GAME_SOURCE_FILES "src/pill_game/*.cpp")
file(GLOB_RECURSE PILL_GAME_HEADER_FILES "src/pill_game/*.h")
message(STATUS "${PILL_GAME_SOURCE_FILES} ${PILL_GAME_HEADER_FILES}")

target_sources(pill_game PRIVATE ${PILL_GAME_SOURCE_FILES} ${PILL_GAME_HEADER_FILES})

target_include_directories(pill_game PUBLIC ./src/)
target_compile_features(pill_game PRIVATE cxx_std_23)

if (MSVC)
  target_compile_options(pill_game PRIVATE /W4)
endif ()

target_precompile_headers(pill_game PRIVATE src/pill_game/pch.h)

find_package(OpenGL REQUIRED)

################################################################################
# | SDL |
################################################################################

include(fetchcontent)

fetchcontent_declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.28
)

FetchContent_MakeAvailable(SDL3)

target_link_libraries(
    pill_game
    PRIVATE
    OpenGL::GL
    SDL3::SDL3
)

add_custom_command(
    TARGET pill_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:SDL3::SDL3>
    $<TARGET_FILE_DIR:pill_game>
)
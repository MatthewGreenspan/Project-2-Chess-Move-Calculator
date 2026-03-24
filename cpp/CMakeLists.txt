# Cross-platform build (macOS, Linux, Windows)
#
# macOS / Linux (pkg-config — Homebrew, apt, etc.):
#   brew install sdl2 sdl2_ttf pkg-config cmake
#   cmake -B build -S .
#   cmake --build build
#
# Windows (vcpkg recommended — same deps everywhere):
#   vcpkg install sdl2 sdl2-ttf
#   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
#   cmake --build build --config Release
#
# Or set CMAKE_PREFIX_PATH to a folder that contains SDL2Config.cmake + SDL2_ttfConfig.cmake.

cmake_minimum_required(VERSION 3.16)
project(chess-move-calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig QUIET)

if(PkgConfig_FOUND AND NOT WIN32)
  pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
  pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
  set(CHESS_SDL2_TARGET PkgConfig::SDL2)
  set(CHESS_SDL2_TTF_TARGET PkgConfig::SDL2_TTF)
else()
  find_package(SDL2 CONFIG REQUIRED)
  find_package(SDL2_ttf CONFIG REQUIRED)
  set(CHESS_SDL2_TARGET SDL2::SDL2)
  set(CHESS_SDL2_TTF_TARGET SDL2_ttf::SDL2_ttf)
endif()

add_executable(chess-calc
  src/main.cpp
  src/stockfish.cpp
  src/opening_db.cpp
  src/chess_gui_helpers.cpp
  src/best_move.cpp
  src/gui_assets.cpp
  src/gui_render.cpp
  src/gui_input.cpp
  src/app_actions.cpp
)

target_include_directories(chess-calc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(chess-calc PRIVATE
  ${CHESS_SDL2_TARGET}
  ${CHESS_SDL2_TTF_TARGET}
)

target_compile_definitions(chess-calc PRIVATE NDEBUG)

if(MSVC)
  target_compile_definitions(chess-calc PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

add_custom_command(TARGET chess-calc POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:chess-calc>/assets
)

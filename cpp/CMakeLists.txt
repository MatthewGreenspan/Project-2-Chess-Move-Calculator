cmake_minimum_required(VERSION 3.14)
project(chess-move-calculator VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Find SDL2 (brew install sdl2 sdl2_image)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

add_executable(chess-calc src/main.cpp)
target_include_directories(chess-calc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SDL2_INCLUDE_DIRS}
  ${SDL2_IMAGE_INCLUDE_DIRS}
)
target_link_libraries(chess-calc PRIVATE
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
)
target_compile_definitions(chess-calc PRIVATE NDEBUG)

# Copy assets next to executable for development
add_custom_command(TARGET chess-calc POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:chess-calc>/assets
)

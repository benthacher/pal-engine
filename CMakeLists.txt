cmake_minimum_required(VERSION 3.29)
project(pal_engine)

if("${PAL_BACKEND}" STREQUAL "")
    message(FATAL_ERROR "PAL Backend not set! Before including PAL as a subdirectory, make sure to set the PAL_BACKEND variable to the source that implements PAL functions.")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_sprites.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_midi.cmake)

# Generate midi and sprite .c/.h files
generate_midi("${MIDI_FILES}" ${CMAKE_CURRENT_BINARY_DIR}/src/assets/midi ${CMAKE_CURRENT_BINARY_DIR}/include/assets/midi GENERATED_MIDI_C_FILES)
generate_sprites("${SPRITE_FILES}" ${CMAKE_CURRENT_BINARY_DIR}/src/assets/sprites ${CMAKE_CURRENT_BINARY_DIR}/include/assets/sprites GENERATED_SPRITE_C_FILES)

add_library(pal_engine
    src/mathutils.c
    src/game.c
    src/graphics.c
    src/font.c
    src/ui.c
    src/sprite.c
    src/physics.c
    src/entity.c
    src/audio.c
    src/midi_parse.c
    ${PAL_BACKEND}
    ${GENERATED_MIDI_C_FILES}
    ${GENERATED_SPRITE_C_FILES}
)

add_dependencies(pal_engine generated_midi_files generated_sprite_files)

target_include_directories(pal_engine PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include)

target_link_libraries(pal_engine m)
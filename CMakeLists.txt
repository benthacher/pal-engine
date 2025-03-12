cmake_minimum_required(VERSION 3.29)
project(pal_engine)

add_library(pal_platform_defs INTERFACE)

if("${PAL_USE_FLOAT32}" STREQUAL "1")
    message(STATUS "Using single precision floats")
    # Make sure floating point constants are floats
    target_compile_options(pal_platform_defs INTERFACE -fsingle-precision-constant)
    target_compile_definitions(pal_platform_defs INTERFACE PAL_USE_FLOAT32)
else()
    message(STATUS "Using double precision floats")
endif()

if("${PAL_BACKEND_SOURCES}" STREQUAL "")
    message(FATAL_ERROR "PAL Backend not set! Before including PAL as a subdirectory, make sure to set the PAL_BACKEND_SOURCES variable to the source that implements PAL functions.")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_sprites.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_midi.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_wav.cmake)

# Generate midi, wave sample, and sprite .c/.h files
generate_midi("${MIDI_FILES}" ${CMAKE_CURRENT_BINARY_DIR}/src/assets/midi ${CMAKE_CURRENT_BINARY_DIR}/include/assets/midi GENERATED_MIDI_C_FILES)
generate_wav("${WAV_FILES}" ${CMAKE_CURRENT_BINARY_DIR}/src/assets/samples ${CMAKE_CURRENT_BINARY_DIR}/include/assets/samples GENERATED_WAV_C_FILES)
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
    src/queue.c
    ${PAL_BACKEND_SOURCES}
    ${GENERATED_MIDI_C_FILES}
    ${GENERATED_WAV_C_FILES}
    ${GENERATED_SPRITE_C_FILES}
)

add_dependencies(pal_engine generated_midi_files generated_wav_files generated_sprite_files)

target_include_directories(pal_engine PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/include ${PAL_BACKEND_INCLUDES})

target_link_libraries(pal_engine pal_platform_defs m ${PAL_BACKEND_LIBRARIES})
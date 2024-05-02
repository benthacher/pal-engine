cmake_minimum_required(VERSION 3.29)
project(pal_example)

# MUST SET BACKEND for PAL engine to build
set(PAL_BACKEND ${CMAKE_SOURCE_DIR}/src/pal_backend.c)

set(ASSETS_MIDI_DIR ${CMAKE_SOURCE_DIR}/assets/midi)
set(ASSETS_SPRITES_DIR ${CMAKE_SOURCE_DIR}/assets/sprites)

set(MIDI_FILES
    # ${ASSETS_MIDI_DIR}/song.mid
)

set(SPRITE_FILES
    # ${ASSETS_SPRITES_DIR}/sprite.gif
)

# this is just for this example, this is probably terrible practice
add_subdirectory(./pal_engine)

add_executable(pal_example main.c)

target_link_libraries(pal_example pal_engine)

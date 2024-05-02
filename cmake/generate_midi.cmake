function(generate_midi MIDI_FILES OUTPUT_SRC_DIR OUTPUT_INC_DIR OUTPUT_SOURCE_FILES_LIST)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)

    # Make the desired output directories
    file(MAKE_DIRECTORY ${OUTPUT_SRC_DIR})
    file(MAKE_DIRECTORY ${OUTPUT_INC_DIR})

    # Setup python venv stuff
    set(MIDI_TO_C ${UTIL_DIR}/midi_to_c.py)

    # Iterate over each MIDI file and filter out files modified since the last build
    foreach(MIDI_FILE ${MIDI_FILES})
        # Get the stem (filename without extension) of the MIDI file
        get_filename_component(MIDI_STEM ${MIDI_FILE} NAME_WE)
        # Construct the output filenames for the generated C and H files
        set(MIDI_C_FILE ${OUTPUT_SRC_DIR}/${MIDI_STEM}.c)
        set(MIDI_H_FILE ${OUTPUT_INC_DIR}/${MIDI_STEM}.h)
        # Add a custom command to run the Python script for each MIDI file
        add_custom_command(
            OUTPUT ${MIDI_C_FILE} ${MIDI_H_FILE}
            COMMAND ${UTIL_PYTHON} ${MIDI_TO_C} ${MIDI_FILE} ${OUTPUT_SRC_DIR} ${OUTPUT_INC_DIR} "assets/midi"
            DEPENDS ${MIDI_FILE}
            COMMENT "Generating midi .c and .h files for ${MIDI_FILE}"
        )
        # Collect the output files
        list(APPEND GENERATED_MIDI_C_FILES ${MIDI_C_FILE})
        list(APPEND GENERATED_MIDI_H_FILES ${MIDI_H_FILE})
    endforeach()

    add_custom_target(generated_midi_files DEPENDS ${GENERATED_MIDI_C_FILES} ${GENERATED_MIDI_H_FILES})

    # send list out to function caller
    set(${OUTPUT_SOURCE_FILES_LIST} ${GENERATED_MIDI_C_FILES} PARENT_SCOPE)
endfunction()
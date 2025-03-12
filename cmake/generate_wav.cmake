function(generate_wav WAV_FILES OUTPUT_SRC_DIR OUTPUT_INC_DIR OUTPUT_SOURCE_FILES_LIST)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)

    # Make the desired output directories
    file(MAKE_DIRECTORY ${OUTPUT_SRC_DIR})
    file(MAKE_DIRECTORY ${OUTPUT_INC_DIR})

    # Setup python venv stuff
    set(WAV_TO_C ${UTIL_DIR}/wav_to_c.py)

    # Iterate over each wave file
    foreach(WAV_FILE ${WAV_FILES})
        # Get the stem (filename without extension) of the wave file
        get_filename_component(WAV_STEM ${WAV_FILE} NAME_WE)
        # Construct the output filenames for the generated C and H files
        set(WAV_C_FILE ${OUTPUT_SRC_DIR}/${WAV_STEM}.c)
        set(WAV_H_FILE ${OUTPUT_INC_DIR}/${WAV_STEM}.h)
        # Add a custom command to run the Python script for each wave file
        add_custom_command(
            OUTPUT ${WAV_C_FILE} ${WAV_H_FILE}
            COMMAND ${UTIL_PYTHON} ${WAV_TO_C} ${WAV_FILE} ${PAL_AUDIO_SAMPLE_RATE} ${OUTPUT_SRC_DIR} ${OUTPUT_INC_DIR} "assets/samples"
            DEPENDS ${WAV_FILE}
            COMMENT "Generating wave sample .c and .h files for ${WAV_FILE}"
        )
        # Collect the output files
        list(APPEND GENERATED_WAV_C_FILES ${WAV_C_FILE})
        list(APPEND GENERATED_WAV_H_FILES ${WAV_H_FILE})
    endforeach()

    add_custom_target(generated_wav_files DEPENDS ${GENERATED_WAV_C_FILES} ${GENERATED_WAV_H_FILES})

    # send list out to function caller
    set(${OUTPUT_SOURCE_FILES_LIST} ${GENERATED_WAV_C_FILES} PARENT_SCOPE)
endfunction()
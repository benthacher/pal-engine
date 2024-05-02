function(generate_sprites SPRITE_FILES OUTPUT_SRC_DIR OUTPUT_INC_DIR OUTPUT_SOURCE_FILES_LIST)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/util.cmake)

    # Make the desired output directories
    file(MAKE_DIRECTORY ${OUTPUT_SRC_DIR})
    file(MAKE_DIRECTORY ${OUTPUT_INC_DIR})

    # Setup python venv stuff
    set(IMG_TO_SPRITE ${UTIL_DIR}/img_to_sprite.py)

    # Iterate over each sprite file
    foreach(SPRITE_FILE ${SPRITE_FILES})
        # Get the stem (filename without extension) of the sprite file
        get_filename_component(SPRITE_STEM ${SPRITE_FILE} NAME_WE)
        # Construct the output filenames for the generated C and H files
        set(SPRITE_C_FILE ${OUTPUT_SRC_DIR}/${SPRITE_STEM}.c)
        set(SPRITE_H_FILE ${OUTPUT_INC_DIR}/${SPRITE_STEM}.h)
        # Add a custom command to run the Python script for each sprite file
        add_custom_command(
            OUTPUT ${SPRITE_C_FILE} ${SPRITE_H_FILE}
            COMMAND ${UTIL_PYTHON} ${IMG_TO_SPRITE} ${SPRITE_FILE} ${OUTPUT_SRC_DIR} ${OUTPUT_INC_DIR} "assets/sprites"
            DEPENDS ${SPRITE_FILE}
            COMMENT "Generating sprite .c and .h files for ${SPRITE_FILE}"
        )
        # Collect the output files
        list(APPEND GENERATED_SPRITE_C_FILES ${SPRITE_C_FILE})
        list(APPEND GENERATED_SPRITE_H_FILES ${SPRITE_H_FILE})
    endforeach()

    add_custom_target(generated_sprite_files DEPENDS ${GENERATED_SPRITE_C_FILES} ${GENERATED_SPRITE_H_FILES})

    # send list out to function caller
    set(${OUTPUT_SOURCE_FILES_LIST} ${GENERATED_SPRITE_C_FILES} PARENT_SCOPE)
endfunction()

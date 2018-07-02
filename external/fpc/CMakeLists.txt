set(SOURCES
    fpc.c
)

add_executable(fpc ${SOURCES})
target_link_libraries(fpc)
set_target_properties(fpc
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/compressors"
)
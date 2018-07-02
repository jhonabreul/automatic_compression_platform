set(SOURCES
    zlib_compressor.cpp
    snappy_compressor.cpp
    lzo_compressor.cpp
    bzip2_compressor.cpp
    lzma_compressor.cpp
    fpc_compressor.cpp
    round_robin_compressor.cpp
    single_compressor.cpp
    file_processing_strategy.cpp
    file_processor.cpp
    pre_compressing_file_processor.cpp
    training_compressor.cpp
    #autocomp_compressor.cpp
)

add_library(compression STATIC ${SOURCES})
target_link_libraries(compression
                      utils
                      zlib_library
                      snappy
                      lzo_library
                      bzip2_library
                      lzma_library
                      io
                      messaging
)
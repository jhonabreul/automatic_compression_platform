set(PROTO
    compressor.proto
    file_request_mode.proto
    file_initial_message.proto
    chunk_header.proto
    file_transmission_request.proto
    error_message.proto
)

set(protobuf_include_path "-I=${CMAKE_SOURCE_DIR}/src")
set(protobuf_output_path "--cpp_out=${CMAKE_SOURCE_DIR}/src")

foreach(file ${PROTO})
  get_filename_component(_file_name ${file} NAME_WE)
  get_filename_component(_abs_file_name ${file} ABSOLUTE)

  set(header_file "${CMAKE_SOURCE_DIR}/src/messaging/${_file_name}.pb.h")
  set(new_header_file "${CMAKE_SOURCE_DIR}/include/messaging/${_file_name}.pb.h")
  set(source_file "${CMAKE_SOURCE_DIR}/src/messaging/${_file_name}.pb.cc")

  set(_generated_files ${_generated_files} ${new_header_file} ${source_file})

  if(EXISTS ${new_header_file} AND EXISTS ${source_file})
    continue()    
  endif()

  add_custom_command(
    OUTPUT ${header_file} ${source_file}
    COMMAND ./protoc ${protobuf_include_path} ${protobuf_output_path} ${_abs_file_name}
    POST_BUILD COMMAND mv ${header_file} ${new_header_file}
    DEPENDS protobuf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
  )
endforeach()

set_source_files_properties(${_generated_files} PROPERTIES GENERATED TRUE)

add_library(messaging SHARED ${_generated_files})
target_link_libraries(messaging protobuf_library protobuf_lite_library)
# Set the library to be a shared library (DLL)
add_library(Payload SHARED Payload.cpp "Payload.h")

find_package(JNI REQUIRED)

# Include JNI headers
include_directories(${JNI_INCLUDE_DIRS})

# Link JNI library
target_link_libraries(Payload ${JNI_LIBRARIES})

set_target_properties(Payload PROPERTIES
    OUTPUT_NAME "Payload"  # The name of the DLL
)

if (WIN32)
    target_compile_definitions(Payload PRIVATE WIN32_LEAN_AND_MEAN)
endif()

target_compile_definitions(Payload PRIVATE PAYLOAD_EXPORTS)

include(FetchContent)
add_executable (Injector "Entry.c" "Entry.h" "Injector.h" "Injector.c"  "Filesystem.h" "Filesystem.c") 

set(MANIFEST_FILE "${CMAKE_SOURCE_DIR}/Injector/resources/Injector.manifest")

source_group("Resource Files" FILES ${MANIFEST_FILE})

IF (MSVC)
    IF (CMAKE_MAJOR_VERSION LESS 3)
        MESSAGE(WARNING "CMake version 3.0 or newer is required to use build variable TARGET_FILE")
    ELSE()
        ADD_CUSTOM_COMMAND(
            TARGET Injector
            POST_BUILD
            COMMAND mt.exe -manifest "${MANIFEST_FILE}" -outputresource:$<TARGET_FILE:Injector>\;\#1
            COMMENT "Adding manifest."
        )
    ENDIF()
ENDIF(MSVC)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/cofyc/argparse.git
    GIT_TAG v1.1.0
)

FetchContent_MakeAvailable(argparse)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Injector PROPERTY CXX_STANDARD 20)
endif()

if (WIN32)
    target_compile_definitions(Injector PRIVATE WIN32_LEAN_AND_MEAN)
endif()

target_link_libraries(Injector PRIVATE argparse)
target_link_libraries(Injector PRIVATE "Shared")

add_executable(Tests JarTraversalTest.cpp "RunTests.cpp")

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

find_package(JNI REQUIRED)

include_directories(${JNI_INCLUDE_DIRS})

target_link_libraries(Tests PRIVATE "Payload" gtest_main GTest::GTest ${JNI_LIBRARIES})

add_test(NAME JarTraversalTest COMMAND Tests)

add_custom_command(TARGET Tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:Payload> $<TARGET_FILE_DIR:Tests>)
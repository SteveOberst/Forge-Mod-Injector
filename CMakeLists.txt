cmake_minimum_required (VERSION 3.8)

# Hot Reload for MSVC-Compiler, if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#include(FetchContent)
#include(GoogleTest)

#FetchContent_Declare(
#    googletest
#    URL https://github.com/google/googletest/archive/refs/heads/main.zip
#)
#FetchContent_MakeAvailable(googletest)

project ("ForgeModInjector")

add_subdirectory ("Shared")
add_subdirectory ("Injector")
add_subdirectory ("Payload")  

enable_testing()
add_subdirectory ("Tests")

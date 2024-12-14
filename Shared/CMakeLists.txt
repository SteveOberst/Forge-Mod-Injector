add_library(Shared STATIC "Shared.h" "Shared.c" "PayloadParameters.h" "WinAPIDebugPrint.h")

target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
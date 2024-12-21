cmake_minimum_required (VERSION 3.20)

message("Current build mode: ${CMAKE_BUILD_TYPE}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HdrSwitcher")

# Release build settings
set(CMAKE_CXX_FLAGS_RELEASE "/O1")

# Fetch
include(FetchContent)

FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG v3.1
)

FetchContent_MakeAvailable(argparse)
# Fetch End

# Add source to this project's executable.
add_executable (HdrSwitcher 
	"src/HdrSwitcher.cpp"
	"src/filters.h" "src/util.h" "src/winapi.h" "src/display.h")

set_property(TARGET HdrSwitcher PROPERTY CXX_STANDARD 23)
target_link_libraries(HdrSwitcher PRIVATE argparse::argparse)

cmake_minimum_required (VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HdrSwitcher")

# Fetch CLI11

include(FetchContent)

FetchContent_Declare(
  cli11
  GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
  GIT_TAG v2.1.2 # Replace with the desired version
)

FetchContent_MakeAvailable(cli11)

# Fetch CLI11 End

# Add source to this project's executable.
add_executable (HdrSwitcher 
	"HdrSwitcher.cpp"
	)

set_property(TARGET HdrSwitcher PROPERTY CXX_STANDARD 20)
target_link_libraries(HdrSwitcher PRIVATE CLI11::CLI11)

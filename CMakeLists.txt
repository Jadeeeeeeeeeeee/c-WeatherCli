cmake_minimum_required(VERSION 3.8)
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Force Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

project("WeatherApp")

# Find vcpkg
if(NOT DEFINED VCPKG_ROOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../Utils/vcpkg")
        set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Utils/vcpkg")
    elseif(EXISTS "$ENV{VCPKG_ROOT}")
        set(VCPKG_ROOT "$ENV{VCPKG_ROOT}")
    else()
        message(FATAL_ERROR "Please set VCPKG_ROOT to your vcpkg installation directory")
    endif()
endif()

# Set vcpkg triplet and toolchain
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Debug output
message(STATUS "VCPKG_ROOT: ${VCPKG_ROOT}")
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

# Set CURL paths
set(CURL_ROOT "${VCPKG_ROOT}/installed/x64-windows")
set(CURL_DIR "${CURL_ROOT}/share/curl")
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CURL_ROOT};${CURL_DIR}")

message(STATUS "CURL_ROOT: ${CURL_ROOT}")
message(STATUS "CURL_DIR: ${CURL_DIR}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# Find packages
find_package(CURL CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(libfort CONFIG REQUIRED)

# Add executable
add_executable(WeatherApp 
    "src/main.cpp" 
    "include/main.h" 
    "src/weather.cpp" 
    "include/weather.h"
)

# Link libraries
target_link_libraries(WeatherApp PRIVATE CURL::libcurl)
target_link_libraries(WeatherApp PRIVATE libfort::fort)
target_link_libraries(WeatherApp PRIVATE nlohmann_json::nlohmann_json)

# Set C++20
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET WeatherApp PROPERTY CXX_STANDARD 20)
endif()

# Copy DLLs post-build
if(WIN32)
    set(CURL_DLL "${CURL_ROOT}/bin/libcurl.dll")
    if(EXISTS "${CURL_DLL}")
        add_custom_command(TARGET WeatherApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CURL_DLL}"
                $<TARGET_FILE_DIR:WeatherApp>
        )
    endif()
endif()
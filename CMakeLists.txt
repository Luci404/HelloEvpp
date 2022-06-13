cmake_minimum_required(VERSION 3.1.2)

project(HelloEvpp)

file(GLOB_RECURSE HELLOEVPP_DEMO_HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE HELLOEVPP_DEMO_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HELLOEVPP_DEMO_MODULES "${CMAKE_SOURCE_DIR}/src/*.ixx")
add_executable(HelloEvpp "${HELLOEVPP_DEMO_HEADERS}" "${HELLOEVPP_DEMO_SOURCES}" "${HELLOEVPP_DEMO_MODULES}")

target_include_directories(HelloEvpp PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
    "C:/Dev/vcpkg/packages/evpp_x64-windows/include")
    
target_link_libraries(HelloEvpp PRIVATE
    "C:/Dev/vcpkg/packages/evpp_x64-windows/lib/evmc_static.lib"
    "C:/Dev/vcpkg/packages/evpp_x64-windows/lib/evnsq_static.lib"
    "C:/Dev/vcpkg/packages/evpp_x64-windows/lib/evpp_lite_static.lib"
    "C:/Dev/vcpkg/packages/evpp_x64-windows/lib/evpp_static.lib")
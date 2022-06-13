cmake_minimum_required(VERSION 3.1.2)

project(HelloEvpp)

file(GLOB_RECURSE EVPP_DEMO_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/evpp/evpp/*.h")
file(GLOB_RECURSE EVPP_DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vendor/evpp/evpp/*.cc")
add_library(evpp_static STATIC "${EVPP_DEMO_HEADERS}" "${EVPP_DEMO_SOURCES}")

target_include_directories(evpp_static PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/evpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glog_x64-windows-static/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/gflags_x64-windows-static/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/rapidjson_x64-windows-static/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libevent_x64-windows-static/include")

target_link_libraries(evpp_static PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/glog_x64-windows-static/lib/glog.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/gflags_x64-windows-static/lib/gflags_static.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libevent_x64-windows-static/lib/event.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libevent_x64-windows-static/lib/event_core.lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/libevent_x64-windows-static/lib/event_extra.lib"
    gdi32
    wsock32
    ws2_32
    Iphlpapi
    shlwapi)

file(GLOB_RECURSE HELLOEVPP_DEMO_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB_RECURSE HELLOEVPP_DEMO_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HELLOEVPP_DEMO_MODULES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ixx")
add_executable(HelloEvpp "${HELLOEVPP_DEMO_HEADERS}" "${HELLOEVPP_DEMO_SOURCES}" "${HELLOEVPP_DEMO_MODULES}")

target_include_directories(HelloEvpp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(HelloEvpp PUBLIC evpp_static)
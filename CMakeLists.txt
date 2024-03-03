cmake_minimum_required(VERSION 3.18)

project(posnet)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TOOLS "Build the tools" ON)

if(Win32)
    message("")
endif()

function(target_builder TARGET_NAME SRCS HDRS LIBS LIBS_DIR OUTTPUT_DIR)
    add_executable(${TARGET_NAME} ${SRCS} ${HDRS})

    target_link_libraries(${TARGET_NAME} PUBLIC ${LIBS})

    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

    target_link_directories(${TARGET_NAME} PUBLIC ${LIBS_DIR})

    set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTTPUT_DIR}")
endfunction(target_builder)

set(CMAKE_CXX_STANDARD 20)

#******************************************************* Building posnet library *******************************************************#
set(LIB_NAME ${PROJECT_NAME}Core)
set(LIB_DIRS ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)
message(STATUS "POSNET_DIRS=${LIB_DIRS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set(LIB_HDRS 
include/net-iface/iface_manager.h
include/frame-viewers/ethernet_viewer.h
include/frame-viewers/ip_viewer.h
include/frame-viewers/udp_viewer.h
include/frame-viewers/tcp_viewer.h
include/frame-viewers/icmp_viewer.h
include/frame-viewers/arp_viewer.h
include/frame-builder/ethernet_builder.h
include/frame-builder/ip_builder.h
include/frame-builder/udp_builder.h
include/frame-builder/icmp_builder.h
include/utils/lazy.h
include/utils/result.h
include/utils/scoped_lock.h
include/utils/strict_mutex.h
include/utils/system_error.h
include/utils/sock_addr_convertor.h
include/utils/algorithms.h
include/definitions.h
include/base_frame.h
)

set(LIB_SRCS 
src/iface_manager.cpp
src/ethernet_viewer.cpp
src/ip_viewer.cpp
src/udp_viewer.cpp
src/tcp_viewer.cpp
src/icmp_viewer.cpp
src/arp_viewer.cpp
src/ethernet_builder.cpp
src/ip_builder.cpp
src/udp_builder.cpp
src/system_error.cpp
src/sock_addr_convertor.cpp
src/algorithms.cpp
src/base_frame.cpp
src/icmp_builder.cpp
)

message(STATUS "LIB_INSTALL_DIR=${CMAKE_BINARY_DIR}/lib")

if(BUILD_SHARED_LIBS)
    message(STATUS "BUILD_TYPE=DYNAMIC_LIBRARY")

    add_library(${LIB_NAME} SHARED ${LIB_HDRS} ${LIB_SRCS})
else()
    message(STATUS "BUILD_TYPE=STATIC_LIBRARY")

    add_library(${LIB_NAME} STATIC ${LIB_HDRS} ${LIB_SRCS})
endif()

target_include_directories(${LIB_NAME} PUBLIC ${LIB_DIRS})

set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})


#******************************************************* Build example dir *******************************************************#
if(BUILD_EXAMPLES)
    message(STATUS "BUILD_EXAMPLES=ON")

    target_builder("frame_shiffer" "examples/frame_sniffer.cpp" "" "posnet" "${CMAKE_BINARY_DIR}/lib" "examples")
    target_builder("udp_client" "examples/udp_client.cpp" "" "posnet" "${CMAKE_BINARY_DIR}/lib" "examples")
    target_builder("icmp_client" "examples/icmp_client.cpp" "" "posnet" "${CMAKE_BINARY_DIR}/lib" "examples")
    target_builder("native_icmp_client" "examples/native_icmp_client.cpp" "" "posnet" "${CMAKE_BINARY_DIR}/lib" "examples")
endif()

#******************************************************* Build tools dir *******************************************************#
if(BUILD_TOOLS)
    message(STATUS "BUILD_TOOLS=ON")

    target_builder("udp_server" "tools/udp_server.cpp" "" "" "" "tools")
endif()
cmake_minimum_required(VERSION 3.18)

project(posnet_core)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(HDRS 
net-iface/iface_manager.h
frame-viewers/ethernet_viewer.h
frame-viewers/ip_viewer.h
frame-viewers/udp_viewer.h
frame-viewers/tcp_viewer.h
frame-viewers/icmp_viewer.h
frame-viewers/arp_viewer.h
frame-builder/ethernet_builder.h
frame-builder/ip_builder.h
frame-builder/udp_builder.h
utils/lazy.h
utils/result.h
utils/scoped_lock.h
utils/strict_mutex.h
utils/system_error.h
utils/sock_addr_convertor.h
utils/algorithms.h
)

set(SRCS 
net-iface/iface_manager.cpp
frame-viewers/ethernet_viewer.cpp
frame-viewers/ip_viewer.cpp
frame-viewers/udp_viewer.cpp
frame-viewers/tcp_viewer.cpp
frame-viewers/icmp_viewer.cpp
frame-viewers/arp_viewer.cpp
frame-builder/ethernet_builder.cpp
frame-builder/ip_builder.cpp
frame-builder/udp_builder.cpp
utils/strict_mutex.cpp
utils/system_error.cpp
utils/sock_addr_convertor.cpp
utils/algorithms.cpp
)

set(DIRS ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/..)

if(BUILD_SHARED_LIBS)
    message(STATUS "BUILD_TYPE:=DYNAMIC_LIBRARY")
    add_library(${PROJECT_NAME} SHARED ${HDRS} ${SRCS})
else()
    add_library(${PROJECT_NAME} STATIC ${HDRS} ${SRCS})
    message(STATUS "BUILD_TYPE:=STATIC_LIBRARY")
endif()


target_include_directories(${PROJECT_NAME} PUBLIC ${DIRS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

cmake_minimum_required(VERSION 3.18)

project(core)

set(HDRS 
net-iface/iface_manager.h
frame-viewers/ethernet_viewer.h
frame-viewers/ip_viewer.h
frame-viewers/udp_viewer.h
frame-viewers/tcp_viewer.h
frame-viewers/icmp_viewer.h
frame-viewers/arp_viewer.h
utils/lazy.h
utils/result.h
utils/scoped_lock.h
utils/strict_mutex.h
utils/system_error.h
utils/sock_addr_convertor.h
)

set(SRCS 
net-iface/iface_manager.cpp
frame-viewers/ethernet_viewer.cpp
frame-viewers/ip_viewer.cpp
frame-viewers/udp_viewer.cpp
frame-viewers/tcp_viewer.cpp
frame-viewers/icmp_viewer.cpp
frame-viewers/arp_viewer.cpp
utils/strict_mutex.cpp
utils/system_error.cpp
utils/sock_addr_convertor.cpp
)

add_library(${PROJECT_NAME} STATIC ${HDRS} ${SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC . ../)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
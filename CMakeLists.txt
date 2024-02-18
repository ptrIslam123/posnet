cmake_minimum_required(VERSION 3.18)

project(posnet)

add_subdirectory(src)

function(target_builder TARGET_NAME SRCS HDRS LIBS)
    add_executable(${TARGET_NAME} ${SRCS} ${HDRS})

    target_link_libraries(${TARGET_NAME} PUBLIC ${LIBS})

    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)

    set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 20)
endfunction(target_builder)

# build example dir
target_builder("frame_shiffer" "examples/frame_sniffer.cpp" "" "posnet_core")
#target_builder("udp_client" "examples/udp_client.cpp" "" "posnet_core")

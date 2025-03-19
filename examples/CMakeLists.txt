# SPDX-FileCopyrightText: 2017 Niels Ole Salscheider <niels_ole@salscheider-online.de>
#
# SPDX-License-Identifier: CC0-1.0

macro(add_simple_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} example_${EXAMPLE_NAME}/example_${EXAMPLE_NAME}.cpp)
    target_link_libraries(${EXAMPLE_NAME} ${QXMPP_TARGET})
endmacro()

include_directories(${PROJECT_SOURCE_DIR}/src/base)
include_directories(${PROJECT_SOURCE_DIR}/src/client)
include_directories(${PROJECT_SOURCE_DIR}/src/server)
include_directories(${PROJECT_BINARY_DIR}/src)

add_simple_example(0_connected)
add_simple_example(1_echoClient)
add_simple_example(2_rosterHandling)
add_simple_example(3_transferHandling)
add_simple_example(7_archiveHandling)
add_simple_example(8_server)

add_subdirectory(example_5_rpcInterface)
add_subdirectory(example_6_rpcClient)
add_subdirectory(example_9_vCard)
if(WITH_GSTREAMER)
    add_subdirectory(audio_call)
endif()

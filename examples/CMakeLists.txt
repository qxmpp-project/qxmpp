macro(add_simple_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} example_${EXAMPLE_NAME}/example_${EXAMPLE_NAME}.cpp)
    target_link_libraries(${EXAMPLE_NAME} qxmpp)
endmacro()

include_directories(../src/base)
include_directories(../src/client)
include_directories(../src/server)

add_simple_example(0_connected)
add_simple_example(1_echoClient)
add_simple_example(2_rosterHandling)
add_simple_example(3_transferHandling)
add_simple_example(7_archiveHandling)
add_simple_example(8_server)

add_subdirectory(example_5_rpcInterface)
add_subdirectory(example_6_rpcClient)
add_subdirectory(example_9_vCard)


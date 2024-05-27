cmake_minimum_required(VERSION 3.10)
project(etsi-gs-qkd-004 VERSION 2.1.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

find_package(OpenSSL REQUIRED)
#find_package(PythonLibs REQUIRED)
include_directories(include
#        ${PYTHON_INCLUDE_DIRS}
        ${UUID_INCLUDE_DIRS}
        )

add_compile_definitions(
        QKD_LOG_INFO
#        QKD_LOG_DEBUG
)

add_library(etsi-gs-qkd-004
        src/qkd/copy.c
        src/qkd/interfaces.c
        src/qkd/serializers.c
        src/qkd/sizes.c
        src/qkd/ssl_socket.c
        src/qkd/ssl_socket_client.c
        src/qkd/ssl_socket_server.c
        src/qkd/utils.c
        src/write_key_to_file.c
        src/memory_free_utils.c
#        src/qkd/wrapper.c
        )
target_link_libraries(etsi-gs-qkd-004 PUBLIC uuid OpenSSL::SSL explain uuid ${PYTHON_LIBRARIES})


add_executable(client_example src/client_example.c)
target_link_libraries(client_example PUBLIC etsi-gs-qkd-004)

add_executable(server_example src/server_example.c)
target_link_libraries(server_example PUBLIC etsi-gs-qkd-004)

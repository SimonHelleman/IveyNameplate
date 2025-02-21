cmake_minimum_required(VERSION 3.27.8)
project(IveyNameplate LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-30-2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(asio)

set(SOURCE_FILES
    src/Main.cpp
    ../common/message/Message.cpp
    src/ClientConnection.cpp
    src/Server.cpp
)

set(HEADER_FILES
    ../common/message/Message.h
    src/ClientConnection.h
    src/Server.h
)

add_executable(server ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(server PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_include_directories(server PRIVATE ../common/message)
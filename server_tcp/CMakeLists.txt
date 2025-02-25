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

FetchContent_Declare(scrolls
        GIT_REPOSITORY https://github.com/SimonHelleman/scrolls.git
        GIT_TAG main
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(scrolls)

set(SOURCE_FILES
    src/Main.cpp
    ../common/message/Message.cpp
    src/ClientConnection.cpp
    src/DatabaseConnection.cpp
    src/Server.cpp
)

set(HEADER_FILES
    ../common/Student.h
    ../common/message/Message.h
    src/ClientConnection.h
    src/DatabaseConnection.h
    src/Server.h
    src/Secret.h
)


add_executable(server ${SOURCE_FILES} ${HEADER_FILES})

if (WIN32)
    # absolute path... I know... bad... I don't care
    set(POSTGRESQL_DIR "C:/Program Files/PostgreSQL/17")
    set(POSTGRESQL_INCLUDE_DIR "${POSTGRESQL_DIR}/include")
    set(POSTGRESQL_LIB_DIR "${POSTGRESQL_DIR}/lib")
    include_directories(${POSTGRESQL_INCLUDE_DIR})
    link_directories(${POSTGRESQL_LIB_DIR})

    target_link_libraries(server PRIVATE "${POSTGRESQL_LIB_DIR}/libpq.lib")
endif()

target_compile_definitions(server PRIVATE
    $<$<CONFIG:Release>:SCROLLS_DISABLE_DEBUG_LOG>
)

target_link_libraries(server PRIVATE scrolls)
target_include_directories(server PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_include_directories(server PRIVATE ${scrolls_SOURCE_DIR}/src)
target_include_directories(server PRIVATE ../common)
target_include_directories(server PRIVATE ../common/message)
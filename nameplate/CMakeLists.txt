cmake_minimum_required(VERSION 3.27.8)
project(IveyNameplate LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-30-2
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(asio)

#file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/**/*.cpp)
#file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/src/**/*.h)

set(SOURCE_FILES
    src/Nameplate.cpp
    src/Platform/SFML/DisplaySFML.cpp
    src/Platform/SFML/EntrySFML.cpp
)

set(HEADER_FILES
    src/Display.h
    src/Nameplate.h
    src/Util/Color.h
    src/Platform/SFML/DisplaySFML.h
)

add_executable(nameplate ${SOURCE_FILES} ${HEADER_FILES})
#target_include_directories(nameplate PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/asio-1.30.2/include)
target_include_directories(nameplate PRIVATE ${asio_SOURCE_DIR}/asio/include)

target_link_libraries(nameplate PRIVATE sfml-graphics)
target_compile_features(nameplate PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
        TARGET nameplate
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:nameplate>
        VERBATIM
    )
endif()
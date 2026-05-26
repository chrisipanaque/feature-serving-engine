cmake_minimum_required(VERSION 3.16)
project(feature-serving-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-32-0
)
FetchContent_MakeAvailable(asio)

find_package(Threads REQUIRED)

add_subdirectory(core/events)
add_subdirectory(core/features)
add_subdirectory(core/store)
add_subdirectory(core/queue)
add_subdirectory(core/concurrency)
add_subdirectory(core/retrieval)
add_subdirectory(core/persistence)
add_subdirectory(core/network)
add_subdirectory(core/inference)
add_subdirectory(core/profiling)
add_subdirectory(benchmarks)
add_subdirectory(cli)

add_executable(benchmarks benchmark_main.cpp)

target_include_directories(benchmarks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${asio_SOURCE_DIR}/asio/include
)

target_link_libraries(benchmarks PRIVATE
    core_store
    core_events
    core_features
    core_inference
    core_profiling
    Threads::Threads
)

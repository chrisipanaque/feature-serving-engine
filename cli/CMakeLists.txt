add_executable(feature-serving-engine main.cpp)

target_include_directories(feature-serving-engine PRIVATE ${asio_SOURCE_DIR}/asio/include)

target_link_libraries(feature-serving-engine PRIVATE
    core_concurrency
    core_retrieval
    core_persistence
    core_network
    core_inference
    Threads::Threads
)

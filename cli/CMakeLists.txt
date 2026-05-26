add_executable(feature-serving-engine main.cpp)

target_link_libraries(feature-serving-engine PRIVATE core_concurrency core_retrieval)

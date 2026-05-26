add_library(core_retrieval
    latency_stats.hpp
    feature_retriever.hpp
    feature_retriever.cpp
)

target_include_directories(core_retrieval PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_retrieval PUBLIC core_store)

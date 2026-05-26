add_library(core_concurrency
    producer.hpp
    producer.cpp
    consumer.hpp
    consumer.cpp
)

target_include_directories(core_concurrency PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_concurrency PUBLIC core_queue core_events core_store)

add_library(core_persistence
    event_log.hpp
    event_log.cpp
)

target_include_directories(core_persistence PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_persistence PUBLIC core_events)

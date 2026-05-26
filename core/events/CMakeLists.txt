add_library(core_events
    event.hpp
    generator.hpp
    generator.cpp
)

target_include_directories(core_events PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)

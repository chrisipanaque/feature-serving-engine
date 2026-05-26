add_library(core_features
    feature_vector.hpp
    feature_vector.cpp
)

target_include_directories(core_features PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_features PUBLIC core_events)

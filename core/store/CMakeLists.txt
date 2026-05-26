add_library(core_store
    feature_store.hpp
    feature_store.cpp
)

target_include_directories(core_store PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_store PUBLIC core_events core_features)

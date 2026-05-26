add_library(core_inference
    scorer.hpp
    scorer.cpp
    inference_service.hpp
    inference_service.cpp
)

target_include_directories(core_inference PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(core_inference PUBLIC core_store)

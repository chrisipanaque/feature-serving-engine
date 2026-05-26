add_library(core_network
    request.hpp
    response.hpp
    session.hpp
    session.cpp
    server.hpp
    server.cpp
)

target_include_directories(core_network PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${asio_SOURCE_DIR}/asio/include
)
target_link_libraries(core_network PUBLIC core_store)

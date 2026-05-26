#ifndef CORE_NETWORK_SESSION_HPP
#define CORE_NETWORK_SESSION_HPP

#define ASIO_STANDALONE
#include <asio.hpp>

#include "core/network/request.hpp"
#include "core/network/response.hpp"
#include "core/store/feature_store.hpp"

class Session {
public:
    Session(asio::ip::tcp::socket socket, const FeatureStore& store);
    void run();

private:
    asio::ip::tcp::socket  socket_;
    const FeatureStore&    store_;
};

#endif

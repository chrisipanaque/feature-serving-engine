#ifndef CORE_NETWORK_SERVER_HPP
#define CORE_NETWORK_SERVER_HPP

#define ASIO_STANDALONE
#include <asio.hpp>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "core/store/feature_store.hpp"

class Server {
public:
    Server(const FeatureStore& store, short port);
    ~Server();

    void start();
    void stop();

private:
    const FeatureStore&    store_;
    short                  port_;
    asio::io_context       io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::atomic<bool>      running_{false};
    std::thread            thread_;
};

#endif

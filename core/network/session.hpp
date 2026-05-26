#ifndef CORE_NETWORK_SESSION_HPP
#define CORE_NETWORK_SESSION_HPP

#define ASIO_STANDALONE
#include <asio.hpp>

#include "core/inference/inference_service.hpp"
#include "core/network/request.hpp"
#include "core/network/response.hpp"

class Session {
public:
    Session(asio::ip::tcp::socket socket, const InferenceService& service);
    void run();

private:
    asio::ip::tcp::socket       socket_;
    const InferenceService&     service_;
};

#endif

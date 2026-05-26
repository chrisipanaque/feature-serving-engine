#include "server.hpp"
#include "session.hpp"

#include <iostream>

Server::Server(const InferenceService& service, short port)
    : service_(service)
    , port_(port)
    , acceptor_(io_context_,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
{}

Server::~Server() {
    stop();
}

void Server::start() {
    running_ = true;
    thread_ = std::thread([this] {
        std::cout << "[Network] Listening on port " << port_ << "\n";
        while (running_) {
            asio::ip::tcp::socket socket(io_context_);
            asio::error_code ec;
            acceptor_.accept(socket, ec);
            if (!running_ || ec)
                break;

            std::thread([sock = std::move(socket), &svc = service_]() mutable {
                Session(std::move(sock), svc).run();
            }).detach();
        }
    });
}

void Server::stop() {
    running_ = false;
    acceptor_.close();
    if (thread_.joinable())
        thread_.join();
}

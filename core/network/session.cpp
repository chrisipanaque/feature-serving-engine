#include "session.hpp"

#include <iostream>

Session::Session(asio::ip::tcp::socket socket, const FeatureStore& store)
    : socket_(std::move(socket))
    , store_(store)
{}

void Session::run() {
    try {
        asio::streambuf buf;
        while (true) {
            asio::read_until(socket_, buf, '\n');

            std::string line(
                asio::buffers_begin(buf.data()),
                asio::buffers_end(buf.data())
            );
            buf.consume(line.size());

            if (!line.empty() && line.back() == '\n')
                line.pop_back();
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            Request req = Request::parse(line);
            std::string resp;

            if (req.type == Request::GetFeatures) {
                auto fv = store_.get_features(req.user_id);
                if (fv)
                    resp = Response::success(req.user_id, *fv);
                else
                    resp = Response::not_found(req.user_id);
            } else {
                resp = Response::parse_error();
            }

            resp += '\n';
            asio::write(socket_, asio::buffer(resp));
        }
    } catch (const asio::system_error&) {
        // client disconnected or error — session ends
    }
}

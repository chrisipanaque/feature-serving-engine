#ifndef CORE_NETWORK_REQUEST_HPP
#define CORE_NETWORK_REQUEST_HPP

#include <cstdint>
#include <string>
#include <string_view>

struct Request {
    enum Type { GetFeatures, Invalid };

    Type       type;
    uint64_t   user_id;

    static Request parse(std::string_view line) {
        constexpr std::string_view prefix = "GET_FEATURES ";
        if (line.size() <= prefix.size())
            return {Invalid, 0};

        auto body = line.substr(prefix.size());
        if (line.substr(0, prefix.size()) != prefix)
            return {Invalid, 0};

        uint64_t uid = 0;
        for (char c : body) {
            if (c < '0' || c > '9')
                return {Invalid, 0};
            uid = uid * 10 + static_cast<uint64_t>(c - '0');
        }

        return {GetFeatures, uid};
    }
};

#endif

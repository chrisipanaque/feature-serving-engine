#ifndef CORE_NETWORK_REQUEST_HPP
#define CORE_NETWORK_REQUEST_HPP

#include <cstdint>
#include <string>
#include <string_view>

struct Request {
    enum Type { GetFeatures, Predict, Invalid };

    Type       type;
    uint64_t   user_id;

    static Request parse(std::string_view line) {
        constexpr std::string_view get_prefix = "GET_FEATURES ";
        constexpr std::string_view pred_prefix = "PREDICT ";

        if (line.size() > get_prefix.size() &&
            line.substr(0, get_prefix.size()) == get_prefix) {
            return parse_uid(line.substr(get_prefix.size()), GetFeatures);
        }
        if (line.size() > pred_prefix.size() &&
            line.substr(0, pred_prefix.size()) == pred_prefix) {
            return parse_uid(line.substr(pred_prefix.size()), Predict);
        }
        return {Invalid, 0};
    }

private:
    static Request parse_uid(std::string_view body, Type type) {
        uint64_t uid = 0;
        for (char c : body) {
            if (c < '0' || c > '9')
                return {Invalid, 0};
            uid = uid * 10 + static_cast<uint64_t>(c - '0');
        }
        return {type, uid};
    }
};

#endif

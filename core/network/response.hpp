#ifndef CORE_NETWORK_RESPONSE_HPP
#define CORE_NETWORK_RESPONSE_HPP

#include <string>

#include "core/features/feature_vector.hpp"

struct Response {
    static std::string success(uint64_t user_id, const FeatureVector& fv) {
        return "user_id=" + std::to_string(user_id)
             + " views=" + std::to_string(fv.views)
             + " clicks=" + std::to_string(fv.clicks)
             + " purchases=" + std::to_string(fv.purchases);
    }

    static std::string not_found(uint64_t user_id) {
        return "ERROR user " + std::to_string(user_id) + " not found";
    }

    static std::string parse_error() {
        return "ERROR invalid request";
    }
};

#endif

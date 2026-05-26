#ifndef CORE_INFERENCE_SCORER_HPP
#define CORE_INFERENCE_SCORER_HPP

#include "core/features/feature_vector.hpp"

class Scorer {
public:
    explicit Scorer(double w_view = 1.0, double w_click = 3.0, double w_purchase = 10.0);

    double score(const FeatureVector& fv) const;

private:
    double w_view_;
    double w_click_;
    double w_purchase_;
};

#endif

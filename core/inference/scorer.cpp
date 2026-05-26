#include "scorer.hpp"

Scorer::Scorer(double w_view, double w_click, double w_purchase)
    : w_view_(w_view)
    , w_click_(w_click)
    , w_purchase_(w_purchase)
{}

double Scorer::score(const FeatureVector& fv) const {
    return w_view_ * static_cast<double>(fv.views)
         + w_click_ * static_cast<double>(fv.clicks)
         + w_purchase_ * static_cast<double>(fv.purchases);
}

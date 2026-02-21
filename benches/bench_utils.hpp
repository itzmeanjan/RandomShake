#pragma once
#include <algorithm>
#include <vector>

const auto compute_min = [](const std::vector<double>& vals) -> double { return *std::min_element(vals.begin(), vals.end()); };
const auto compute_max = [](const std::vector<double>& vals) -> double { return *std::max_element(vals.begin(), vals.end()); };

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

struct Measurement {
    std::string sensor_name;
    double value;
    bool valid;
};

namespace {
auto valid_only(const std::vector<Measurement>& readings) {
    return readings | std::views::filter([](const Measurement& m) { return m.valid; });
}
}  // namespace

double average_valid(const std::vector<Measurement>& readings) {
    double sum = 0.0;
    int count = 0;
    for (const Measurement& m : valid_only(readings)) {
        sum += m.value;
        ++count;
    }
    return count == 0 ? 0.0 : sum / count;
}

std::vector<std::string> names_above(const std::vector<Measurement>& readings, double threshold) {
    std::vector<std::string> out;
    auto view = valid_only(readings)
              | std::views::filter([threshold](const Measurement& m) { return m.value > threshold; })
              | std::views::transform([](const Measurement& m) { return m.sensor_name; });
    for (auto&& name : view) out.push_back(std::move(name));
    return out;
}

const Measurement* hottest(const std::vector<Measurement>& readings) {
    auto view = valid_only(readings);
    auto it = std::ranges::max_element(view, {}, &Measurement::value);
    return it == view.end() ? nullptr : &*it;
}

int main() {
    std::vector<Measurement> readings{
        {"temp1", 23.5, true},
        {"temp2", -999.0, false},
        {"temp3", 24.5, true},
        {"temp4", 22.0, true},
        {"temp5", 100.0, false},
    };

    assert(std::abs(average_valid(readings) - 70.0 / 3.0) < 1e-9);
    assert(average_valid({}) == 0.0);
    assert(average_valid({{"x", 1.0, false}}) == 0.0);

    assert((names_above(readings, 23.0) == std::vector<std::string>{"temp1", "temp3"}));
    assert(names_above(readings, 50.0).empty());

    const Measurement* h = hottest(readings);
    assert(h != nullptr && h->sensor_name == "temp3");
    assert(hottest({}) == nullptr);
    assert(hottest({{"x", 5.0, false}}) == nullptr);

    std::cout << "OK\n";
}

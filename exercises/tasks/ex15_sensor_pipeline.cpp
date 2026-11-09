// Rozdzial 2, zadanie 5: potok na danych z czujnikow
//
//   - average_valid:   srednia value z poprawnych odczytow (valid == true); 0.0 gdy brak
//   - names_above:     nazwy czujnikow poprawnych z wartoscia > threshold (filter + transform)
//   - hottest:         wskaznik na poprawny odczyt o najwiekszej wartosci albo nullptr
//                      (filter + ranges::max_element na widoku - zwroc adres elementu)
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

double average_valid(const std::vector<Measurement>& readings) {
    // TODO
    return -1.0;
}

std::vector<std::string> names_above(const std::vector<Measurement>& readings, double threshold) {
    // TODO
    return {};
}

const Measurement* hottest(const std::vector<Measurement>& readings) {
    // TODO: widok filter nie jest common_range - uzyj ranges::max_element(view, {}, &Measurement::value)
    return nullptr;
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

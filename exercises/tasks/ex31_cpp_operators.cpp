// Zadanie 31 (C++): przeciazanie operatorow
//
// Dla struktury Vec2 zaimplementuj (jako funkcje wolne):
//   a + b, a - b, -a, a * scalar, scalar * a, a += b (zwraca Vec2&), a == b,
//   operator<< wypisujacy "(x, y)" oraz dot(a, b).
// `a != b` dostajesz za darmo z operator== (C++20).
#include <cassert>
#include <iostream>
#include <sstream>

struct Vec2 {
    double x = 0;
    double y = 0;
};

Vec2 operator+(Vec2 a, Vec2 b) {
    // TODO
    return {};
}

Vec2 operator-(Vec2 a, Vec2 b) {
    // TODO
    return {};
}

Vec2 operator-(Vec2 a) {
    // TODO
    return {};
}

Vec2 operator*(Vec2 a, double k) {
    // TODO
    return {};
}

Vec2 operator*(double k, Vec2 a) {
    // TODO
    return {};
}

Vec2& operator+=(Vec2& a, Vec2 b) {
    // TODO
    return a;
}

bool operator==(const Vec2& a, const Vec2& b) {
    // TODO
    return false;
}

std::ostream& operator<<(std::ostream& os, const Vec2& v) {
    // TODO
    return os;
}

double dot(Vec2 a, Vec2 b) {
    // TODO
    return 0;
}

int main() {
    Vec2 a{1, 2}, b{3, 4};
    assert((a + b) == (Vec2{4, 6}));
    assert((b - a) == (Vec2{2, 2}));
    assert((-a) == (Vec2{-1, -2}));
    assert((a * 2) == (Vec2{2, 4}));
    assert((2 * a) == (Vec2{2, 4}));

    Vec2 c = a;
    c += b;
    assert(c == (Vec2{4, 6}));
    assert(&(c += b) == &c);

    assert(dot(a, b) == 11.0);
    assert(a != b);
    assert(!(a != a));

    std::ostringstream os;
    os << a << " " << Vec2{1.5, -2};
    assert(os.str() == "(1, 2) (1.5, -2)");

    std::cout << "OK\n";
}

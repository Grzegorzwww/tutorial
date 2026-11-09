// Zadanie 30 (C++): dziedziczenie i polimorfizm
//
// Shape jest klasa abstrakcyjna z wirtualnym destruktorem (gotowe).
// Uzupelnij:
//   Circle(r):        area = pi * r^2 (std::numbers::pi), name() == "circle"
//   Rectangle(w, h):  area = w * h,                       name() == "rectangle"
//   Square(s):        dziedziczy po Rectangle,            name() == "square"
//   total_area:       suma pol
//   largest:          wskaznik na ksztalt o najwiekszym polu albo nullptr gdy pusto
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <numbers>
#include <string>
#include <type_traits>
#include <vector>

class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual std::string name() const = 0;
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}
    double area() const override {
        // TODO
        return 0;
    }
    std::string name() const override {
        // TODO
        return "";
    }

private:
    double r_;
};

class Rectangle : public Shape {
public:
    Rectangle(double w, double h) : w_(w), h_(h) {}
    double area() const override {
        // TODO
        return 0;
    }
    std::string name() const override {
        // TODO
        return "";
    }

protected:
    double w_, h_;
};

class Square : public Rectangle {
public:
    explicit Square(double s) : Rectangle(s, s) {}
    // TODO: nadpisz name()
};

using ShapeList = std::vector<std::unique_ptr<Shape>>;

double total_area(const ShapeList& shapes) {
    // TODO
    return 0;
}

const Shape* largest(const ShapeList& shapes) {
    // TODO
    return nullptr;
}

static_assert(std::is_abstract_v<Shape>);
static_assert(std::has_virtual_destructor_v<Shape>);
static_assert(std::is_base_of_v<Rectangle, Square>);

int main() {
    Circle c(1.0);
    assert(std::abs(c.area() - std::numbers::pi) < 1e-9);
    assert(c.name() == "circle");

    Rectangle r(2, 3);
    assert(r.area() == 6.0 && r.name() == "rectangle");

    const Shape& s = Square(3);  // wywolanie przez referencje do klasy bazowej
    assert(s.area() == 9.0 && s.name() == "square");

    ShapeList shapes;
    assert(total_area(shapes) == 0.0);
    assert(largest(shapes) == nullptr);

    shapes.push_back(std::make_unique<Circle>(1.0));
    shapes.push_back(std::make_unique<Rectangle>(2, 3));
    shapes.push_back(std::make_unique<Square>(2));
    assert(std::abs(total_area(shapes) - (std::numbers::pi + 6.0 + 4.0)) < 1e-9);
    assert(largest(shapes) == shapes[1].get());
    assert(largest(shapes)->name() == "rectangle");

    std::cout << "OK\n";
}

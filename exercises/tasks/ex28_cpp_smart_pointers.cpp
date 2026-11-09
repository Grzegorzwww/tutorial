// Zadanie 28 (C++): inteligentne wskazniki
//
// Widget zlicza zywe instancje w Widget::alive.
//   make_widget:      zwraca unique_ptr<Widget> (uzyj std::make_unique)
//   consume:          przejmuje wlasnosc - Widget jest niszczony przy wyjsciu z funkcji
//   make_copies:      zwraca wektor n kopii shared_ptr (wspoldzielacych TEN SAM obiekt)
//   is_alive:         czy obiekt wskazywany przez weak_ptr jeszcze istnieje
//   id_or_minus1:     id obiektu przez weak_ptr (lock()), albo -1 gdy juz nie istnieje
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

struct Widget {
    static inline int alive = 0;
    int id;
    explicit Widget(int i) : id(i) { ++alive; }
    ~Widget() { --alive; }
    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;
};

std::unique_ptr<Widget> make_widget(int id) {
    // TODO
    return nullptr;
}

void consume(std::unique_ptr<Widget> w) {
    // TODO
}

std::vector<std::shared_ptr<Widget>> make_copies(const std::shared_ptr<Widget>& w, int n) {
    // TODO
    return {};
}

bool is_alive(const std::weak_ptr<Widget>& w) {
    // TODO
    return true;
}

int id_or_minus1(const std::weak_ptr<Widget>& w) {
    // TODO
    return 0;
}

int main() {
    {
        auto w = make_widget(7);
        assert(Widget::alive == 1);
        assert(w->id == 7);
    }
    assert(Widget::alive == 0);

    auto w = make_widget(1);
    consume(std::move(w));
    assert(!w);
    assert(Widget::alive == 0);

    auto sp = std::make_shared<Widget>(3);
    assert(sp.use_count() == 1);
    auto copies = make_copies(sp, 3);
    assert(copies.size() == 3);
    assert(sp.use_count() == 4);
    assert(Widget::alive == 1);
    copies.clear();
    assert(sp.use_count() == 1);

    std::weak_ptr<Widget> wp = sp;
    assert(is_alive(wp));
    assert(id_or_minus1(wp) == 3);
    assert(sp.use_count() == 1);  // weak_ptr nie zwieksza licznika
    sp.reset();
    assert(!is_alive(wp));
    assert(id_or_minus1(wp) == -1);
    assert(Widget::alive == 0);

    std::cout << "OK\n";
}

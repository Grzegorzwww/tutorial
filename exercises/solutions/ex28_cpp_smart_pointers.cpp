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
    return std::make_unique<Widget>(id);
}

void consume(std::unique_ptr<Widget> w) {
    // w jest niszczony przy wyjsciu z funkcji
}

std::vector<std::shared_ptr<Widget>> make_copies(const std::shared_ptr<Widget>& w, int n) {
    return std::vector<std::shared_ptr<Widget>>(static_cast<std::size_t>(n), w);
}

bool is_alive(const std::weak_ptr<Widget>& w) {
    return !w.expired();
}

int id_or_minus1(const std::weak_ptr<Widget>& w) {
    if (auto sp = w.lock()) return sp->id;
    return -1;
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

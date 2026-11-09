// Zadanie 40 (C++): perfect forwarding
//
//   make<T>(args...):         odpowiednik std::make_unique - przekaz argumenty dalej
//                             przez std::forward, tak aby nie bylo zbednych kopii
//   call_logged(count, f, ...): zwieksza count i wywoluje f z przekazanymi (forwardowanymi)
//                             argumentami, zwracajac jego wynik (decltype(auto))
//
// Tracker zlicza kopie i przeniesienia. Wskazowka: bez std::forward argument
// o nazwie jest ZAWSZE lvalue, wiec rvalue zostanie skopiowane zamiast przeniesione.
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

struct Tracker {
    static inline int copies = 0;
    static inline int moves = 0;
    static void reset() { copies = moves = 0; }

    Tracker() = default;
    Tracker(const Tracker&) { ++copies; }
    Tracker(Tracker&&) noexcept { ++moves; }
};

struct Holder {
    Tracker tracker;
    int n;
    Holder(Tracker t, int n_) : tracker(std::move(t)), n(n_) {}
};

template<typename T, typename... Args>
std::unique_ptr<T> make(Args&&... args) {
    // TODO: std::forward<Args>(args)...
    return std::unique_ptr<T>(new T(args...));
}

template<typename F, typename... Args>
decltype(auto) call_logged(int& call_count, F&& f, Args&&... args) {
    ++call_count;
    // TODO: przekaz f i args dalej przez std::forward
    return f(args...);
}

int main() {
    Tracker t;
    Tracker::reset();
    auto h1 = make<Holder>(t, 1);  // lvalue: 1 kopia do parametru, potem 1 przeniesienie do skladowej
    assert(h1->n == 1);
    assert(Tracker::copies == 1 && Tracker::moves == 1);

    Tracker::reset();
    auto h2 = make<Holder>(Tracker{}, 2);  // rvalue: zadnych kopii
    assert(h2->n == 2);
    assert(Tracker::copies == 0 && Tracker::moves == 2);

    auto kind = [](auto&& x) -> const char* {
        return std::is_lvalue_reference_v<decltype(x)> ? "lvalue" : "rvalue";
    };
    int calls = 0;
    int value = 5;
    assert(std::string(call_logged(calls, kind, value)) == "lvalue");
    assert(std::string(call_logged(calls, kind, 5)) == "rvalue");
    assert(calls == 2);

    auto add = [](int a, int b) { return a + b; };
    assert(call_logged(calls, add, 2, 3) == 5);
    assert(calls == 3);

    std::cout << "OK\n";
}

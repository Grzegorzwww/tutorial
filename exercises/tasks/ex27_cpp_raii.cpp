// Zadanie 27 (C++): RAII
//
// 1) ScopeGuard: w destruktorze wywoluje przekazana funkcje, chyba ze wczesniej
//    wywolano dismiss(). Ma sie wykonac takze podczas propagacji wyjatku.
//    Kopiowanie jest zabronione (juz jest `= delete`).
// 2) ScopedFlag: ustawia flage na podana wartosc, a w destruktorze PRZYWRACA
//    poprzednia wartosc (dziala poprawnie zagniezdzone).
#include <cassert>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

class ScopeGuard {
public:
    explicit ScopeGuard(std::function<void()> on_exit) : on_exit_(std::move(on_exit)) {}
    ~ScopeGuard() {
        // TODO
    }
    void dismiss() {
        // TODO
    }
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> on_exit_;
    bool active_ = true;
};

class ScopedFlag {
public:
    ScopedFlag(bool& flag, bool value) : flag_(flag) {
        // TODO
    }
    ~ScopedFlag() {
        // TODO
    }
    ScopedFlag(const ScopedFlag&) = delete;
    ScopedFlag& operator=(const ScopedFlag&) = delete;

private:
    bool& flag_;
    bool previous_ = false;
};

int main() {
    std::vector<int> log;
    {
        ScopeGuard g1([&] { log.push_back(1); });
        ScopeGuard g2([&] { log.push_back(2); });
        log.push_back(0);
    }
    assert((log == std::vector<int>{0, 2, 1}));  // niszczenie w odwrotnej kolejnosci

    log.clear();
    {
        ScopeGuard g([&] { log.push_back(9); });
        g.dismiss();
    }
    assert(log.empty());

    bool ran = false;
    try {
        ScopeGuard g([&] { ran = true; });
        throw std::runtime_error("boom");
    } catch (const std::exception&) {
    }
    assert(ran);

    bool busy = false;
    {
        ScopedFlag outer(busy, true);
        assert(busy);
        {
            ScopedFlag inner(busy, false);
            assert(!busy);
        }
        assert(busy);
    }
    assert(!busy);

    std::cout << "OK\n";
}

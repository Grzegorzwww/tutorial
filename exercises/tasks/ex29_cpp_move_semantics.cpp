// Zadanie 29 (C++): regula piatki i semantyka przenoszenia
//
// Buffer zarzadza dynamiczna tablica int. Zaimplementuj:
//   - konstruktor Buffer(n): n zer
//   - destruktor
//   - konstruktor kopiujacy i kopiujacy operator= (gleboka kopia, ++copies)
//   - konstruktor przenoszacy i przenoszacy operator= (kradnie zasoby, ++moves,
//     zrodlo zostaje puste: size()==0, data()==nullptr)
//   - move musi byc noexcept - dzieki temu vector przenosi zamiast kopiowac przy realokacji
#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

class Buffer {
public:
    static inline int copies = 0;
    static inline int moves = 0;

    explicit Buffer(std::size_t n) {
        // TODO
    }
    ~Buffer() {
        // TODO
    }
    Buffer(const Buffer& other) {
        // TODO
    }
    Buffer& operator=(const Buffer& other) {
        // TODO
        return *this;
    }
    Buffer(Buffer&& other) noexcept {
        // TODO
    }
    Buffer& operator=(Buffer&& other) noexcept {
        // TODO
        return *this;
    }

    std::size_t size() const { return size_; }
    const int* data() const { return data_; }
    int& operator[](std::size_t i) { return data_[i]; }
    const int& operator[](std::size_t i) const { return data_[i]; }

private:
    std::size_t size_ = 0;
    int* data_ = nullptr;
};

static_assert(std::is_nothrow_move_constructible_v<Buffer>);
static_assert(std::is_nothrow_move_assignable_v<Buffer>);

int main() {
    Buffer a(3);
    assert(a.size() == 3 && a[0] == 0 && a[2] == 0);
    a[0] = 1;
    a[1] = 2;
    a[2] = 3;

    Buffer::copies = Buffer::moves = 0;
    Buffer b = a;
    assert(Buffer::copies == 1 && Buffer::moves == 0);
    b[0] = 99;
    assert(a[0] == 1);  // gleboka kopia - niezalezne dane
    assert(b.data() != a.data());

    Buffer c = std::move(a);
    assert(Buffer::moves == 1);
    assert(a.size() == 0 && a.data() == nullptr);
    assert(c.size() == 3 && c[0] == 1 && c[2] == 3);

    Buffer d(1);
    d = c;
    assert(Buffer::copies == 2);
    assert(d.size() == 3 && d[1] == 2 && d.data() != c.data());

    Buffer e(5);
    e = std::move(c);
    assert(Buffer::moves == 2);
    assert(c.size() == 0 && c.data() == nullptr);
    assert(e.size() == 3 && e[2] == 3);

    Buffer& self = e;
    e = self;  // samoprzypisanie nie moze zepsuc danych
    assert(e.size() == 3 && e[2] == 3);

    Buffer::copies = Buffer::moves = 0;
    std::vector<Buffer> v;
    v.reserve(1);
    v.push_back(Buffer(2));
    v.push_back(Buffer(2));  // realokacja: elementy maja byc PRZENIESIONE
    assert(Buffer::copies == 0);
    assert(Buffer::moves >= 3);

    std::cout << "OK\n";
}

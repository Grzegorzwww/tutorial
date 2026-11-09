// Zadanie 32 (C++): szablony
//
//   clamp_value<T>:   zwraca referencje do v, lo albo hi (nie kopie!)
//   Stack<T>:         push, pop (zwraca wartosc), top, empty, size;
//                     pop() i top() na pustym stosie rzucaja std::out_of_range
//   FixedVector<T,N>: tablica o stalej pojemnosci; push_back zwraca false gdy pelna;
//                     size(), capacity() (constexpr static), operator[]
//   type_name<T>():   "int", "double", a dla pozostalych typow "unknown"
//                     (dopisz specjalizacje dla int i double)
#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

template<typename T>
const T& clamp_value(const T& v, const T& lo, const T& hi) {
    // TODO
    return v;
}

template<typename T>
class Stack {
public:
    void push(const T& value) {
        // TODO
    }
    T pop() {
        // TODO
        return T{};
    }
    const T& top() const {
        // TODO
        return items_.front();
    }
    bool empty() const {
        // TODO
        return false;
    }
    std::size_t size() const {
        // TODO
        return 99;
    }

private:
    std::vector<T> items_;
};

template<typename T, std::size_t N>
class FixedVector {
public:
    bool push_back(const T& value) {
        // TODO
        return false;
    }
    std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return N; }
    T& operator[](std::size_t i) { return data_[i]; }

private:
    T data_[N]{};
    std::size_t size_ = 0;
};

template<typename T>
std::string type_name() {
    return "unknown";
}
// TODO: template<> std::string type_name<int>() ...

static_assert(FixedVector<int, 3>::capacity() == 3);

int main() {
    int lo = 0, hi = 10, v = 15, mid = 5;
    assert(clamp_value(v, lo, hi) == 10);
    assert(&clamp_value(v, lo, hi) == &hi);
    assert(clamp_value(-1, 0, 10) == 0);
    assert(&clamp_value(mid, lo, hi) == &mid);
    assert(clamp_value(2.5, 1.0, 2.0) == 2.0);

    Stack<std::string> st;
    assert(st.empty() && st.size() == 0);
    st.push("a");
    st.push("b");
    st.push("c");
    assert(st.size() == 3 && st.top() == "c");
    assert(st.pop() == "c");
    assert(st.pop() == "b");
    assert(st.size() == 1);
    assert(st.pop() == "a");
    assert(st.empty());
    bool pop_threw = false, top_threw = false;
    try { st.pop(); } catch (const std::out_of_range&) { pop_threw = true; }
    try { st.top(); } catch (const std::out_of_range&) { top_threw = true; }
    assert(pop_threw && top_threw);

    FixedVector<int, 3> fv;
    assert(fv.push_back(1) && fv.push_back(2) && fv.push_back(3));
    assert(!fv.push_back(4));
    assert(fv.size() == 3 && fv[1] == 2);

    assert(type_name<int>() == "int");
    assert(type_name<double>() == "double");
    assert(type_name<char>() == "unknown");

    std::cout << "OK\n";
}

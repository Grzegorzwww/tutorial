#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

template<typename T>
const T& clamp_value(const T& v, const T& lo, const T& hi) {
    if (v < lo) return lo;
    if (hi < v) return hi;
    return v;
}

template<typename T>
class Stack {
public:
    void push(const T& value) { items_.push_back(value); }
    T pop() {
        if (items_.empty()) throw std::out_of_range("pop on empty Stack");
        T value = std::move(items_.back());
        items_.pop_back();
        return value;
    }
    const T& top() const {
        if (items_.empty()) throw std::out_of_range("top on empty Stack");
        return items_.back();
    }
    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
};

template<typename T, std::size_t N>
class FixedVector {
public:
    bool push_back(const T& value) {
        if (size_ == N) return false;
        data_[size_++] = value;
        return true;
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
template<>
std::string type_name<int>() {
    return "int";
}
template<>
std::string type_name<double>() {
    return "double";
}

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

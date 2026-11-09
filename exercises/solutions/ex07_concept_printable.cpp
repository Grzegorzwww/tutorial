#include <iostream>
#include <sstream>
#include <string>

template<typename T>
concept Printable = requires(std::ostream& os, const T& t) { os << t; };

std::string log_value(const Printable auto& v) {
    
    os << "[LOG] " << v;
    return os.str();
}

struct NoPrint {};

static_assert(Printable<int>);
static_assert(Printable<std::string>);
static_assert(Printable<double>);
static_assert(!Printable<NoPrint>);

template<typename T>
concept CanLog = requires(const T& t) { log_value(t); };
static_assert(CanLog<int>);
static_assert(!CanLog<NoPrint>);

int main() {
    if (log_value(42) != "[LOG] 42" || log_value(std::string("hi")) != "[LOG] hi" ||
        log_value(3.5) != "[LOG] 3.5") {
        std::cout << "FAIL\n";
        return 1;
    }
    std::cout << "OK\n";
}

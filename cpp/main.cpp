#include <iostream>
#include <concepts>
#include <string.h>
#include <vector>
template <typename T>

concept Numeric = std::integral<T> || std::floating_point<T>;

template <Numeric T>
T add(T a, T b)
{
    return a+b;
}



template <typename T>
requires std::integral<T> || std::floating_point<T>
T add_2(T a, T b){
    return a+b;
}


template <typename T>
concept HasSize = requires(T t){
    {t.size()} -> 
    std::convertible_to<std::size_t>;
};

template <HasSize T>
void print_size(const T &container){
    std::cout <<"Rozmiar " << container.size()<< std::endl;
}


int main() {
    std::cout << "Hello, World!" << std::endl;

    std::cout << add(2,3) <<std::endl;
    //std::cout << add(std::string("a"), std::string("b")) <<endl;
    std::vector<int> v = {1,2,3,4,5};
    print_size(v);


    return 0;
}
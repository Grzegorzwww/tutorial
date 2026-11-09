#include <iostream>
#include <string.h>
#include <vector>
#include <algorithm>
#include <ranges>


using namespace std;

int main() {
    std::cout << "Hello, World!" << std::endl;
    std::vector<int> even;
    std::vector<int> liczby{1,2,3,4,5,6,7,8,9,10};


    std::copy_if(liczby.begin(), liczby.end(), std::back_inserter(even), [](int n) {return (n % 2 == 0);});


    auto result = liczby | std::filter([](int n){})


    for (auto i : even){
        std::cout << i <<std::endl;
    }
    getchar();
    return 0;
}
#include <BPR/BPR.hpp>
#include <iostream>

int main()
{
    bpr::prng::Xoshiro256Star e;
    auto s = bpr::sequence(e, 0.1, 0.2, 1000);
    for (auto v : s) std::cout << v << std::endl;
}

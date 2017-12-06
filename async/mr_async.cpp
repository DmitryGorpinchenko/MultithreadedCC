#include <iostream>
#include <future>
#include <list>
#include <vector>
#include <algorithm>

template<typename It, typename Mapper, typename Reducer>
auto map_reduce(It p, It q, Mapper m, Reducer r, size_t threads) -> decltype(m(*p))
{   
    using Res = decltype(m(*p));
    std::vector<std::future<Res>> ft(threads);
    
    size_t incr = std::distance(p, q) / threads;
    
    for (size_t i = 0; i < threads - 1; ++i) {
        ft[i] = std::async(std::launch::async, [incr, p, m, r]() {
            return std::accumulate(std::next(p), std::next(p, incr), m(*p), [m, r](Res res, decltype(*p) val) { return r(res, m(val)); });
        });
        std::advance(p, incr);
    }
    ft[threads - 1] = std::async(std::launch::async, [p, q, m, r]() {
        return std::accumulate(std::next(p), q, m(*p), [m, r](Res res, decltype(*p) val) { return r(res, m(val)); });
    });
    return std::accumulate(std::next(ft.begin()), ft.end(), ft[0].get(), [r](Res res, decltype(*ft.begin()) f) { return r(res, f.get()); });
}

int main()
{
    std::list<int> l = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto sum = map_reduce(l.begin(), l.end(), [](int i) { return i; }, std::plus<int>(), 3);
    auto has_even = map_reduce(l.begin(), l.end(), [](int i) { return i % 2 == 0; }, std::logical_or<bool>(), 4);
    
    std::cout << sum << " " << has_even << std::endl;
    
    return 0;
}

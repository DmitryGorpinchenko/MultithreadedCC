#include <iostream>
#include <thread>
#include <list>
#include <vector>
#include <algorithm>

template<typename It, typename Mapper, typename Reducer>
auto map_reduce(It p, It q, Mapper m, Reducer r, size_t threads) -> decltype(m(*p))
{   
    using Res = decltype(m(*p));
    std::vector<std::thread> ths;
    std::vector<Res> results(threads);
    
    size_t incr = std::distance(p, q) / threads;
    
    for (size_t i = 0; i < threads - 1; ++i) {
        ths.emplace_back([incr, p, m, r, &results, i]() {
            results[i] = std::accumulate(std::next(p), std::next(p, incr), m(*p), [m, r](Res res, decltype(*p) val) {
                return r(res, m(val));
            });
        });
        std::advance(p, incr);
    }
    ths.emplace_back([p, q, m, r, &results, threads]() {
        results[threads - 1] = std::accumulate(std::next(p), q, m(*p), [m, r](Res res, decltype(*p) val) { return r(res, m(val)); });
    });
    for (auto& th : ths) { th.join(); }
    return std::accumulate(std::next(results.begin()), results.end(), results[0], [r](Res acc, Res cur) { return r(acc, cur); });
}

int main()
{
    std::list<int> l = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto sum = map_reduce(l.begin(), l.end(), [](int i) { return i; }, std::plus<int>(), 3);
    auto has_even = map_reduce(l.begin(), l.end(), [](int i) { return i % 2 == 0; }, std::logical_or<bool>(), 4);
    
    std::cout << sum << " " << has_even << std::endl;
    
    return 0;
}

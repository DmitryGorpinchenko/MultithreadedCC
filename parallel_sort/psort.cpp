#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

template <typename It>
void psort(It first, It last, int max_depth)
{
    if (last - first <= 1) {
        return;
    }
    if (max_depth <= 0) {
        std::sort(first, last);
        return;
    }
    
    const auto mid = first + (last - first) / 2;
   
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            psort(first, mid, max_depth - 1);
        }
        #pragma omp section
        {
            psort(mid, last, max_depth - 1);
        }
    }
    std::inplace_merge(first, mid, last);
}

template <typename It>
void psort(It first, It last) {
    psort(first, last, 2);
}

int main()
{
    std::vector<int> nums(std::istream_iterator<int>(std::cin),
                          std::istream_iterator<int>());
    psort(std::begin(nums), std::end(nums));
    std::copy(std::begin(nums),
              std::end(nums),
              std::ostream_iterator<int>(std::cout, " "));
    return 0;
}


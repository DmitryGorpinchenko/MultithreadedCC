#include <vector>
#include <list>
#include <algorithm>
#include <iterator>

template<class T>
class VectorList
{
private:
    using VectT = std::vector<T>;
    using ListT = std::list<VectT>;
    using ItrVT = typename VectT::const_iterator;
    using ItrLT = typename ListT::const_iterator;

public:
    using value_type = T;

    template<class It> void append(It p, It q) { if (p != q) { data_.push_back(VectT(p,q)); } }
    size_t size() const { return std::accumulate(data_.begin(), data_.end(), 0u, [](size_t res, const VectT& v) { return res + v.size(); }); }
    bool empty() const { return size() == 0; }

    class const_iterator : public std::iterator<std::bidirectional_iterator_tag, T, std::ptrdiff_t, T const *, T const &> {
    public:
        const_iterator() = default;
        const_iterator(ItrLT lit, ItrVT vit, ListT const *data) : lit(lit), vit(vit), data(data) {}
        
        const_iterator &operator++() {
            if (++vit == lit->cend() && ++lit != data->cend()) {
                vit = lit->cbegin();
            }
            return *this;
        }
        const_iterator &operator--() {
            if (vit == lit->cbegin()) {
                vit = (--lit)->cend();
            }
            --vit;
            return *this;
        }
        const_iterator operator++(int) { const auto tmp = *this; ++*this; return tmp; }
        const_iterator operator--(int) { const auto tmp = *this; --*this; return tmp; }
        bool operator==(const const_iterator& other) const { return vit == other.vit; }
        bool operator!=(const const_iterator& other) const { return !(*this == other); }
        typename std::iterator_traits<const_iterator>::reference operator*() const { return *vit; }
        typename std::iterator_traits<const_iterator>::pointer operator->() const { return &*vit; }
        
    private:
        ItrLT lit;
        ItrVT vit;
        ListT const *data;
    };

    const_iterator begin() const { return !empty() ? const_iterator(data_.cbegin(), data_.front().cbegin(), &data_) : const_iterator(); }
    const_iterator end()   const { return !empty() ? const_iterator(std::prev(data_.cend()), data_.back().cend(), &data_) : const_iterator(); }

    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend()   const { return const_reverse_iterator(begin()); }

private:
    ListT data_;
};


int main()
{
	return 0;
}

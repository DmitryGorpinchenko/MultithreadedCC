#include <cstring>

template <size_t N> class Allocator {
public:
    Allocator() { *((Header *) mem_) = { N - sizeof(Header), 0 }; }

    void *Alloc(size_t size);
    void *ReAlloc(void *ptr, size_t size);

    void Free(void *ptr) { (((Header *) ptr) - 1)->occupied = 0; }
private:
    struct Header {
        size_t offset   : 31;
        size_t occupied :  1;
    };
    char mem_[N];
};

template <size_t N> void *Allocator<N>::Alloc(size_t size)
{
    Header *ptr = (Header *) mem_;
    void *end = mem_ + (N - sizeof(Header));
    while (ptr < end) {
        Header *cur = ptr;
        size_t union_size = ptr->offset;
        while (!cur->occupied && union_size >= size) { // lazy fix of mem fragmentation
            cur = (Header *) (((char *) (cur + 1)) + cur->offset);
            if (cur >= end || cur->occupied) {
                break;
            }
            union_size += sizeof(Header) + cur->offset;
        }
        ptr->offset = union_size;
        if (!ptr->occupied && ptr->offset >= size) {
            break;
        }
        ptr = (Header *) (((char *) (ptr + 1)) + ptr->offset);
    }
    if (ptr >= end) {
        return nullptr;
    }
    int diff = ptr->offset - (size + sizeof(Header));
    if (diff > 0) {
        *((Header *) (((char *) (ptr + 1)) + size)) = { diff, 0 };
        ptr->offset = size;
    }
    ptr->occupied = 1;
    return ptr + 1;
}

template <size_t N> void *Allocator<N>::ReAlloc(void *ptr, size_t size)
{
    size_t old_size = (((Header *) ptr) - 1)->offset;
    if (old_size >= size) {
        int diff = old_size - (size + sizeof(Header));
        if (diff > 0) {
            *((Header *) (((char *) ptr) + size)) = { diff, 0 };
            (((Header *) ptr) - 1)->offset = size;
        }
        return ptr;
    }
    Free(ptr); // WARNING: ptr used below (Alloc should not be called while inside ReAlloc)
    void *new_ptr = Alloc(size);
    if (new_ptr && new_ptr != ptr) { // prevent undefinied behavior
        memcpy(new_ptr, ptr, std::min(size, old_size));
    }
    return new_ptr;
}

using SmallAllocator = Allocator<1024 * 1024>;

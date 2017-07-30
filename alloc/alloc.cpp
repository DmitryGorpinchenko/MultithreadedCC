#include <iostream>
#include <cstring>
#include <memory>

class SmallAllocator {
public:
	SmallAllocator();	
	
	void *Alloc(size_t size);
	void *ReAlloc(void *ptr, size_t size);
	void Free(void *ptr);
		
private:
	struct AllocImpl;
	std::unique_ptr<AllocImpl> pimpl_;
};

int main()
{	
	SmallAllocator A1;
	int *A1_P1 = (int *) A1.Alloc(sizeof(int));
	A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
	A1.Free(A1_P1);
	SmallAllocator A2;
	int *A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
	for(size_t i = 0; i < 10; i++) 
		A2_P1[i] = i;
	for(size_t i = 0; i < 10; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 1" << std::endl;
	int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
	for(size_t i = 0; i < 10; i++) 
		A2_P2[i] = -1;
	for(size_t i = 0; i < 10; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 2" << std::endl;
	for(size_t i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 3" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
	for(size_t i = 10; i < 20; i++) 
		A2_P1[i] = i;
	for(size_t i = 0; i < 20; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 4" << std::endl;
	for(size_t i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 5" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
	for(size_t i = 0; i < 5; i++) 
		if(A2_P1[i] != i) 
			std::cout << "ERROR 6" << std::endl;
	for(size_t i = 0; i < 10; i++) 
		if(A2_P2[i] != -1) 
			std::cout << "ERROR 7" << std::endl;
	A2.Free(A2_P1);
	A2.Free(A2_P2);
	return 0;
}

struct SmallAllocator::AllocImpl {
	static const int mem_size = 1024 * 1024;
	char *mem_;
	
	struct Header {
		size_t offset   : 31;
		size_t occupied :  1;
	
		Header(size_t offset, size_t occupied)
			: offset(offset)
			, occupied(occupied)
		{}
	};
	
	AllocImpl() {
		mem_ = new char[mem_size];
		*((Header *) mem_) = Header(mem_size - sizeof(Header), 0);		
	}
	
	~AllocImpl() {
		delete [] mem_;
	}	
	
	void *Alloc(size_t size);
	void *ReAlloc(void *ptr, size_t size);
	void Free(void *ptr);
};

SmallAllocator::SmallAllocator()
	: pimpl_(new AllocImpl())
{}

void *SmallAllocator::Alloc(size_t size) {
	return pimpl_->Alloc(size);
}

void *SmallAllocator::ReAlloc(void *ptr, size_t size) {
	return pimpl_->ReAlloc(ptr, size);
}

void SmallAllocator::Free(void *ptr) {
	pimpl_->Free(ptr);
}

void *SmallAllocator::AllocImpl :: Alloc(size_t size) 
{
	Header *ptr = (Header *) mem_;
	void *end = mem_ + (mem_size - sizeof(Header));
	while(ptr < end) {
		Header *cur = ptr;
		size_t union_size = ptr->offset;
		while(! cur->occupied) { // lazy fix of mem fragmentation
			if(union_size >= size) {
				break;
			}
			cur = (Header *) (((char *) (cur + 1)) + cur->offset);
			if(cur >= end || cur->occupied) {
				break;
			}
			union_size += sizeof(Header) + cur->offset;
		}
		ptr->offset = union_size;
		if((! ptr->occupied) && (ptr->offset >= size)) {
			break;
		}
		ptr = (Header *) (((char *) (ptr + 1)) + ptr->offset);
	}
	if(ptr >= end) {
		return nullptr;
	}
	int diff = ptr->offset - (size + sizeof(Header));
	if(diff > 0) {
		*((Header *) (((char *) (ptr + 1)) + size)) = Header(diff, 0);
		ptr->offset = size;
	}
	ptr->occupied = 1;
	return ptr + 1;
}

void *SmallAllocator::AllocImpl :: ReAlloc(void *ptr, size_t size)
{
	size_t old_size = (((Header *) ptr) - 1)->offset;
	if(old_size >= size) {
		int diff = old_size - (size + sizeof(Header));
		if(diff > 0) {
			*((Header *) (((char *) ptr) + size)) = Header(diff, 0);
			(((Header *) ptr) - 1)->offset = size;
		}
		return ptr;
	}
	Free(ptr);
	void *new_ptr = Alloc(size);
	if(new_ptr != ptr) { // prevent undefinied behavior
		memcpy(new_ptr, ptr, std::min(size, old_size));
	}
	return new_ptr;
}

void SmallAllocator::AllocImpl :: Free(void *ptr)
{
	(((Header *) ptr) - 1)->occupied = 0;
}


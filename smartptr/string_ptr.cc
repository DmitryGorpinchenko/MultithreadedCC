#include <iostream>

class StringPointer {
public:
    StringPointer(std::string* ptr) : ptr_(ptr ? ptr : new std::string()), owner(ptr == nullptr)  {}
    ~StringPointer() { if (owner) { delete ptr_; } }

    StringPointer(const StringPointer& other) = delete;
    StringPointer& operator=(const StringPointer& other) = delete;

    std::string& operator*() { return *ptr_; }
    std::string* operator->() { return ptr_; }

private:
    std::string* ptr_;
    bool owner;
};

int main() {
    std::string s1 = "Hello, world!";

    StringPointer sp1(&s1);
    StringPointer sp2(nullptr);

    std::cout << sp1->length() << std::endl;
    std::cout << *sp1 << std::endl; 
    std::cout << sp2->length() << std::endl;
    std::cout << *sp2 << std::endl;
    
    return 0;
}

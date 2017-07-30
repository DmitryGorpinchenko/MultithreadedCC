#include <iostream>

class StringPointer {
public:
    StringPointer(std::string* ptr);
    ~StringPointer();

    std::string* operator->();
    operator std::string*();

private:
    std::string* ptr_;
    bool is_null;
};

StringPointer::StringPointer(std::string* ptr) 
    : ptr_(ptr ? ptr : new std::string())
    , is_null(ptr == nullptr)
{}

StringPointer::~StringPointer() {
    if(is_null) {
        delete ptr_;
    }
}

std::string* StringPointer::operator->() {
    return ptr_;
}

StringPointer::operator std::string*() {
    return ptr_;
}

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

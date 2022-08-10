#include <iostream>
#include <string>

//Класс умный указатель для указателя std::string *
class StringPointer {

private:
    std::string *pointer;
    bool createdManually;

private:
    void createPointerManually()
    {
        createdManually = true;
        pointer = new std::string("");
    };

public:
    std::string *operator->() {
        if (!pointer)
            createPointerManually();

        return pointer;
    };
    operator std::string*() {
        if (!pointer)
            createPointerManually();

        return pointer;
    };
    StringPointer(std::string *Pointer)
        : pointer(Pointer),
          createdManually(false) {};
    ~StringPointer() {
        if (createdManually)
            delete pointer;

        pointer = NULL;
    };
};

int main(int argc, char *argv[])
{
    std::string s1 = "Hello, world!";

    StringPointer sp1(&s1);
    StringPointer sp2(NULL);

    std::cout << sp1->length() << std::endl;
    std::cout << *sp1 << std::endl;
    std::cout << *sp2 << std::endl;

    return 0;
}

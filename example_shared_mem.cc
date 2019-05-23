#include <shared_mutex.h>
#include <iostream>

void shared_mem_write(const char *data, size_t size)
{
    SharedMemory mem("/test_mem", size);
    void *ptr = NULL;

    if((ptr = mem.get())) {
        memcpy(ptr, data, size);
    } else {
        std::cerr << "Unable to get shared pointer\n";
    }
}

void shared_mem_read(size_t size)
{
    SharedMemory mem("/test_mem", size);
    void *ptr = NULL;

    if((ptr = mem.get())) {
        std::string str((const char *)ptr, size);
        std::cout << "String: \"" << str << "\"\n";
    } else {
        std::cerr << "Unable to get shared pointer\n";
    }
}

int main(int argc, char **argv)
{
    const std::string str = "Hello world!";
    const size_t len = str.size();

    if(argc < 2) {
        shared_mem_write(str.c_str(), len);
    } else {
        shared_mem_read(len);
    }

    return 0;
}

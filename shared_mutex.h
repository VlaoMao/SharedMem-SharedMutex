#pragma once

#include <shared_memory.h>
#include <string>

class shared_mutex_t;

class SharedMutex
{
public:
    SharedMutex(const std::string name);
    ~SharedMutex();
    void close();
    void destroy();
    int trylock();
    int unlock();
    bool isOk() {return (m_mutex_ptr != NULL);}
private:
    bool check_process_exist(pid_t pid);
private:
    shared_mutex_t *m_mutex_ptr;
    SharedMemory m_shared_mem;
    const std::string m_name;
};

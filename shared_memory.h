#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <string>

class SharedMemory
{
public:
    SharedMemory(const std::string name, const size_t size, const int flags = O_RDWR)
        : m_ptr(NULL), m_size(0), m_created(false), m_name(name)
    {
        m_fd = shm_open(name.c_str(), O_RDWR, 0660);

        if(errno == ENOENT) {
            // doesn't exist, create
            m_fd = shm_open(name.c_str(), flags | O_CREAT, 0660);
            m_created = true;
        }

        if(m_fd == -1)
            return ;

        if (ftruncate(m_fd, size) == -1)
            return ;

        m_ptr = mmap(NULL, size,
                     PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0);

        if(m_ptr == MAP_FAILED) {
            m_ptr = NULL;
            return ;
        }

        m_size = size;

        if(m_created)
            memset(m_ptr, 0, m_size);
    }
    ~SharedMemory()
    {
        close();
    }
    void *get()
    {
        return m_ptr;
    }

    bool isCreated() {return m_created;}

    bool destroy()
    {
        close();
        return !!shm_unlink(m_name.c_str());
    }
    void close()
    {
        if(m_ptr) {
            munmap(m_ptr, m_size);
            m_ptr = NULL;
            m_size = 0;
        }

        ::close(m_fd);
    }
private:
    void *m_ptr;
    size_t m_size;
    int m_fd;
    bool m_created;
    const std::string m_name;
};

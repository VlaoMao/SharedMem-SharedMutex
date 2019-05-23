#include "shared_mutex.h"

#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

typedef struct shared_mutex_t {
  pthread_mutex_t ptr; // Pthread mutex
  pid_t pid;           // Pid of locker
} __attribute__((__packed__)) shared_mutex_t;


SharedMutex::SharedMutex(const std::string name)
    : m_mutex_ptr(NULL), m_shared_mem(name, sizeof(shared_mutex_t)), m_name(name)
{
    m_mutex_ptr = (shared_mutex_t*)m_shared_mem.get();

    if(!m_mutex_ptr)
        return ;

    if(m_shared_mem.isCreated()) {
        pthread_mutexattr_t attr;
        if (pthread_mutexattr_init(&attr)) {
            perror("pthread_mutexattr_init");
            return ;
        }

        if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) {
            perror("pthread_mutexattr_setpshared");
            return ;
        }

        if (pthread_mutex_init(&m_mutex_ptr->ptr, &attr)) {
            perror("pthread_mutex_init");
            return ;
        }

        // not needed?
        // m_mutex_ptr->pid = getpid();
    }
}

SharedMutex::~SharedMutex()
{
    close();
}

void SharedMutex::close()
{
    m_shared_mem.close();
    m_mutex_ptr = NULL;
}

void SharedMutex::destroy()
{
    if(m_mutex_ptr) {
        if (pthread_mutex_destroy(&m_mutex_ptr->ptr)) {
            perror("pthread_mutex_destroy");
            return ;
        }

        close();

        m_shared_mem.destroy();
    }
}

int SharedMutex::unlock()
{
    if(m_mutex_ptr)
        return pthread_mutex_unlock(&m_mutex_ptr->ptr);

    return -1;
}

int SharedMutex::trylock()
{
    if(!m_mutex_ptr)
        return -1;

    if(!pthread_mutex_trylock(&m_mutex_ptr->ptr)) {
        // ok
        m_mutex_ptr->pid = getpid();
        return 0;
    }

    // not ok
    if(!check_process_exist(m_mutex_ptr->pid)) {
        // process died
        m_mutex_ptr->pid = getpid();
        return 0;
    }

    return -1;
}

/* man 2 kill
 *
 * If  sig  is  0,  then  no  signal is sent, but existence and permission
 * checks are still performed; this can be used to check for the existence
 * of  a  process  ID  or process group ID that the caller is permitted to
 * signal.
*/
bool SharedMutex::check_process_exist(pid_t pid)
{
    if(pid <= 0)
        return false;

    return !!(kill(pid, 0) == 0);
}
